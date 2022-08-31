//
// Created by Keren Dong on 2020/3/27.
//

#include <fstream>
#include <kungfu/yijinjing/index/session.h>

using namespace sqlite_orm;
using namespace kungfu::longfist;
using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::cache;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::journal;

namespace kungfu::yijinjing::index {
std::string get_index_db_file(const io_device_ptr &io_device) {
  auto locator = io_device->get_locator();
  auto index_location = location::make_shared(mode::LIVE, category::SYSTEM, "journal", "index", locator);
  return locator->layout_file(index_location, layout::SQLITE, "index");
}

session_finder::session_finder(const io_device_ptr &io_device)
    : io_device_(io_device),
      session_storage_(cache::make_storage_ptr(get_index_db_file(io_device), longfist::SessionDataTypes)) {
  if (not session_storage_->sync_schema_simulate().empty()) {
    session_storage_->sync_schema();
  }
}

session_finder::~session_finder() { io_device_.reset(); }

int64_t session_finder::find_last_active_time(const data::location_ptr &source_location) {
  auto range = where(eq(&Session::location_uid, source_location->uid));
  auto order = order_by(&Session::begin_time).desc();
  auto sessions = session_storage_->get_all<Session>(range, order, limit(1));
  return sessions.empty() ? INT64_MIN : sessions.front().end_time;
}

SessionVector session_finder::find_sessions(int64_t from, int64_t to) {
  auto bt = &Session::begin_time;
  auto range = where(greater_or_equal(bt, from) and lesser_or_equal(bt, to));
  return session_storage_->get_all<Session>(range, order_by(bt));
}

SessionVector session_finder::find_sessions_for(const location_ptr &source_location, int64_t from, int64_t to) {
  auto bt = &Session::begin_time;
  auto match_uid = eq(&Session::location_uid, source_location->uid);
  auto range = where(match_uid and greater_or_equal(bt, from) and lesser_or_equal(bt, to));
  return session_storage_->get_all<Session>(range, order_by(bt));
}

session_builder::session_builder(const io_device_ptr &io_device) : session_finder(io_device) {
  if (not session_storage_->sync_schema_simulate().empty()) {
    session_storage_->sync_schema();
  }
  session_storage_->pragma.journal_mode(sqlite_orm::journal_mode::WAL);
}

int64_t session_builder::find_last_active_time(const data::location_ptr &source_location) {
  return session_finder::find_last_active_time(source_location);
}

Session &session_builder::open_session(const location_ptr &source_location, int64_t time) {
  auto pair = live_sessions_.try_emplace(source_location->uid);
  auto &session = pair.first->second;
  if (pair.second) {
    session.location_uid = source_location->uid;
    session.category = source_location->category;
    session.group = source_location->group;
    session.name = source_location->name;
    session.mode = source_location->mode;
  }
  session.begin_time = time;
  session.end_time = 0;
  session.update_time = time;
  session_storage_->replace(session);
  return session;
}

void session_builder::close_session(const location_ptr &source_location, int64_t time) {
  if (live_sessions_.find(source_location->uid) == live_sessions_.end()) {
    return;
  }

  auto &session = live_sessions_.at(source_location->uid);
  session.end_time = time;
  session.update_time = time;
  session_storage_->replace(session);
}

SessionMap &session_builder::close_all_sessions(int64_t time) {
  for (auto &pair : live_sessions_) {
    auto &session = pair.second;
    session.end_time = time;
    session.update_time = time;
    session_storage_->replace(session);
  }
  return live_sessions_;
}

void session_builder::update_session(const frame_ptr &frame) {
  if (live_sessions_.find(frame->source()) == live_sessions_.end()) {
    return;
  }
  Session &session = live_sessions_.at(frame->source());
  session.update_time = frame->gen_time();
  session.frame_count++;
  session.data_size += frame->frame_length();
}

void session_builder::rebuild_index_db() {
  std::unordered_map<std::string, location_ptr> formatstr_to_locations = {};
  auto locator = io_device_->get_locator();
  auto reader = io_device_->open_reader_to_subscribe();
  for (const auto &location : locator->list_locations("*", "*", "*", "*")) {
    SPDLOG_TRACE("investigating journal for [{:08x}] {}", location->uid, location->uname);

    if (location->category != category::SYSTEM or location->group != "master") {
      formatstr_to_locations.emplace(fmt::format("{:08x}", location->uid), location);
    }

    if (location->category == category::SYSTEM and location->group == "master" and location->name == "master") {
      formatstr_to_locations.emplace(location->name, location);
    }

    for (const auto dest_uid : locator->list_location_dest(location)) {
      reader->join(location, dest_uid, 0);
    }
  }

  session_storage_->remove_all<Session>();
  while (reader->data_available()) {
    auto location = reader->current_page()->get_location();
    std::string uid_str = location->name;
    auto frame = reader->current_frame();
    if (formatstr_to_locations.find(uid_str) == formatstr_to_locations.end()) {
      reader->next();
      continue;
    }

    try {
      if (frame->msg_type() == SessionStart::tag) {
        open_session(formatstr_to_locations.at(uid_str), frame->gen_time());
      } else if (frame->msg_type() == SessionEnd::tag) {
        close_session(formatstr_to_locations.at(uid_str), frame->gen_time());
      } else if (location->category != category::SYSTEM or location->group != "master") {
        update_session(frame);
      } else if (location->category == category::SYSTEM and location->group == "master" and
                 location->name == "master") {
        update_session(frame);
      }
    } catch (const std::exception &ex) {
      SPDLOG_ERROR("problematic frame at {}, {}, {}", location->uname, formatstr_to_locations.at(uid_str)->uname,
                   ex.what());
    }
    reader->next();
  }
  for (const auto &pair : live_sessions_) {
    session_storage_->replace(pair.second);
  }
}
} // namespace kungfu::yijinjing::index