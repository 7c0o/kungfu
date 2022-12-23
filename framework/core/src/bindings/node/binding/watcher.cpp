// SPDX-License-Identifier: Apache-2.0

//
// Created by Keren Dong on 2020/1/14.
//

#include "watcher.h"
#include "commission_store.h"
#include "config_store.h"
#include "history.h"
#include "kungfu/yijinjing/cache/ringqueue.h"
#include <sstream>

using namespace kungfu::rx;
using namespace kungfu::longfist;
using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::wingchun;
using namespace kungfu::yijinjing;
using namespace kungfu::yijinjing::cache;
using namespace kungfu::yijinjing::data;

namespace kungfu::node {
inline std::string format(uint32_t uid) { return fmt::format("{:08x}", uid); }

Napi::FunctionReference Watcher::constructor = {};

inline location_ptr GetWatcherLocation(const Napi::CallbackInfo &info) {
  if (not IsValid(info, 0, &Napi::Value::IsString)) {
    throw Napi::Error::New(info.Env(), "Invalid runtime dirname");
  }

  if (not IsValid(info, 1, &Napi::Value::IsString)) {
    throw Napi::Error::New(info.Env(), "Invalid node app name");
  }

  auto runtime_dir = info[0].As<Napi::String>().Utf8Value();
  auto name = info[1].As<Napi::String>().Utf8Value();
  auto result = std::make_shared<location>(mode::LIVE, category::SYSTEM, "node", name, GetRuntimeLocator(runtime_dir));
  log::copy_log_settings(result, result->name);
  return result;
}

inline bool GetBypassRestore(const Napi::CallbackInfo &info) {
  if (not IsValid(info, 2, &Napi::Value::IsBoolean)) {
    throw Napi::Error::New(info.Env(), "Invalid bypassRestore argument");
  }
  return info[2].As<Napi::Boolean>().Value();
}

inline bool GetBypassAccounting(const Napi::CallbackInfo &info) {
  if (not IsValid(info, 3, &Napi::Value::IsBoolean)) {
    throw Napi::Error::New(info.Env(), "Invalid bypassAccounting argument");
  }
  return info[3].As<Napi::Boolean>().Value();
}

inline bool GetBypassTradingData(const Napi::CallbackInfo &info) {
  if (not IsValid(info, 4, &Napi::Value::IsBoolean)) {
    throw Napi::Error::New(info.Env(), "Invalid bypassTradingData argument");
  }
  return info[4].As<Napi::Boolean>().Value();
}

inline bool GetRefreshLedgerBeforeSync(const Napi::CallbackInfo &info) {
  if (not IsValid(info, 5, &Napi::Value::IsBoolean)) {
    throw Napi::Error::New(info.Env(), "Invalid refreshBeforeSync argument");
  }
  return info[5].As<Napi::Boolean>().Value();
}

inline int GetMillisecondsSleepAfterStep(const Napi::CallbackInfo &info) {
  if (not IsValid(info, 6, &Napi::Value::IsNumber)) {
    throw Napi::Error::New(info.Env(), "Invalid millisecondsSleepAfterStep argument");
  }
  return info[6].As<Napi::Number>().Int32Value();
}

WatcherAutoClient::WatcherAutoClient(yijinjing::practice::apprentice &app, bool bypass_trading_data)
    : SilentAutoClient(app), bypass_trading_data_(bypass_trading_data) {}

void WatcherAutoClient::connect(const event_ptr &event, const longfist::types::Register &register_data) {
  auto resume_time_point = get_resume_policy().get_connect_time(app_, register_data);
  auto app_uid = register_data.location_uid;

  // for write msg and get msg from ledger public
  auto ledger_uid = app_.get_ledger_home_location()->uid;
  if ((uint32_t)app_uid == (uint32_t)ledger_uid) {
    // resume time has to be 0, otherwise the broker state be lost in cli mode
    app_.request_read_from_public(app_.now(), ledger_uid, 0);
    return;
  }

  if (bypass_trading_data_) {
    auto app_location = app_.get_location(app_uid);

    if (app_location->category == category::MD and should_connect_md(app_location)) {
      app_.request_write_to(app_.now(), app_uid);
      SPDLOG_INFO("resume {} connection from {}", app_.get_location_uname(app_uid), time::strftime(resume_time_point));
    }

    if (app_location->category == category::TD and should_connect_td(app_location)) {
      app_.request_write_to(app_.now(), app_uid);
      SPDLOG_INFO("resume {} connection from {}", app_.get_location_uname(app_uid), time::strftime(resume_time_point));
    }

    if (app_location->category == category::STRATEGY and should_connect_strategy(app_location)) {
      app_.request_write_to(app_.now(), app_location->uid);
      SPDLOG_INFO("resume {} connection from {}", app_.get_location_uname(app_uid), time::strftime(resume_time_point));
    }
  } else {
    wingchun::broker::SilentAutoClient::connect(event, register_data);
  }
}

void WatcherAutoClient::connect(const event_ptr &event, const longfist::types::Band &band) { return; }

Watcher::Watcher(const Napi::CallbackInfo &info)
    : ObjectWrap(info),                                                                                   //
      apprentice(GetWatcherLocation(info), true),                                                         //
      bypass_accounting_(GetBypassAccounting(info)),                                                      //
      bypass_trading_data_(GetBypassTradingData(info)),                                                   //
      refresh_trading_data_before_sync_(GetRefreshLedgerBeforeSync(info)),                                //
      milliseconds_sleep_after_step_(GetMillisecondsSleepAfterStep(info)),                                //
      broker_client_(*this, bypass_trading_data_),                                                        //
      bookkeeper_(*this, broker_client_),                                                                 //
      state_ref_(Napi::ObjectReference::New(Napi::Object::New(info.Env()), 1)),                           //
      ledger_ref_(Napi::ObjectReference::New(Napi::Object::New(info.Env()), 1)),                          //
      app_states_ref_(Napi::ObjectReference::New(Napi::Object::New(info.Env()), 1)),                      //
      history_ref_(Napi::ObjectReference::New(History::NewInstance({info[0]}).ToObject(), 1)),            //
      config_ref_(Napi::ObjectReference::New(ConfigStore::NewInstance({info[0]}).ToObject(), 1)),         //
      commission_ref_(Napi::ObjectReference::New(CommissionStore::NewInstance({info[0]}).ToObject(), 1)), //
      strategy_states_ref_(Napi::ObjectReference::New(Napi::Object::New(info.Env()), 1)),                 //
      update_state(state_ref_),                                                                           //
      update_ledger(ledger_ref_),                                                                         //
      publish(*this, state_ref_),                                                                         //
      reset_cache(*this, ledger_ref_) {
  serialize::InitStateMap(info, state_ref_, "state");
  serialize::InitStateMap(info, ledger_ref_, "ledger");

  auto today = time::today_start();
  auto config_store = ConfigStore::Unwrap(config_ref_.Value());

  bool sync_schema = not get_io_device()->is_usable();
  if (sync_schema) {
    config_store->profile_.setup();
  }

  SPDLOG_INFO("Watcher created for {}", get_home_uname());

  // byPassRestore will be true after ui browserWindow reopen by crashed
  if (GetBypassRestore(info) or bypass_trading_data_) {
    return;
  }

  for (const auto &item : config_store->profile_.get_all(Location{})) {
    auto saved_location = location::make_shared(item, get_locator());
    if (saved_location->category == longfist::enums::category::SYSTEM) {
      continue;
    }
    // add_location(now(), saved_location);
    RestoreState(saved_location, today, INT64_MAX, sync_schema);
  }
  RestoreState(ledger_home_location_, today, INT64_MAX, sync_schema);

  shift(ledger_home_location_) >> state_bank_; // Load positions to restore bookkeeper
}

Watcher::~Watcher() {
  uv_work_.data = nullptr;
  strategy_states_ref_.Unref();
  commission_ref_.Unref();
  config_ref_.Unref();
  history_ref_.Unref();
  app_states_ref_.Unref();
  ledger_ref_.Unref();
  state_ref_.Unref();
}

void Watcher::NoSet(const Napi::CallbackInfo &info, const Napi::Value &value) {
  SPDLOG_WARN("do not manipulate watcher internals");
}

Napi::Value Watcher::HasLocation(const Napi::CallbackInfo &info) {
  uint32_t uid = 0;
  if (info[0].IsNumber()) {
    uid = info[0].ToNumber().Uint32Value();
  }
  if (info[0].IsString()) {
    std::stringstream ss;
    ss << std::hex << info[0].ToString().Utf8Value();
    ss >> uid;
  }

  return Napi::Boolean::New(info.Env(), has_location(uid));
}

Napi::Value Watcher::GetLocation(const Napi::CallbackInfo &info) {
  auto location = FindLocation(info);
  if (not location) {
    return {};
  }
  auto locationObj = Napi::Object::New(info.Env());
  locationObj.Set("category", Napi::String::New(info.Env(), get_category_name(location->category)));
  locationObj.Set("group", Napi::String::New(info.Env(), location->group));
  locationObj.Set("name", Napi::String::New(info.Env(), location->name));
  locationObj.Set("mode", Napi::String::New(info.Env(), get_mode_name(location->mode)));
  locationObj.Set("uname", Napi::String::New(info.Env(), location->uname));
  locationObj.Set("uid", Napi::Number::New(info.Env(), location->uid));
  return locationObj;
}

Napi::Value Watcher::GetLocationUID(const Napi::CallbackInfo &info) {
  auto target_location = ExtractLocation(info, 0, get_locator());
  return Napi::Number::New(info.Env(), target_location->uid);
}

// TODO: 返回十进制，但需要16进制
Napi::Value Watcher::GetInstrumentUID(const Napi::CallbackInfo &info) {
  auto exchange_id = info[0].ToString().Utf8Value();
  auto instrument_id = info[1].ToString().Utf8Value();
  auto key = hash_instrument(exchange_id.c_str(), instrument_id.c_str());
  return Napi::Number::New(info.Env(), key);
}

Napi::Value Watcher::GetInstrumentType(const Napi::CallbackInfo &info) {
  auto exchange_id = info[0].ToString().Utf8Value();
  auto instrument_id = info[1].ToString().Utf8Value();
  auto instrument_type = get_instrument_type(exchange_id, instrument_id);
  return Napi::Number::New(info.Env(), int(instrument_type));
}

Napi::Value Watcher::GetConfig(const Napi::CallbackInfo &info) { return config_ref_.Value(); }

Napi::Value Watcher::GetHistory(const Napi::CallbackInfo &info) { return history_ref_.Value(); }

Napi::Value Watcher::GetCommission(const Napi::CallbackInfo &info) { return commission_ref_.Value(); }

Napi::Value Watcher::GetState(const Napi::CallbackInfo &info) { return state_ref_.Value(); }

Napi::Value Watcher::GetLedger(const Napi::CallbackInfo &info) { return ledger_ref_.Value(); }

Napi::Value Watcher::GetAppStates(const Napi::CallbackInfo &info) { return app_states_ref_.Value(); }

Napi::Value Watcher::GetStrategyStates(const Napi::CallbackInfo &info) { return strategy_states_ref_.Value(); }

Napi::Value Watcher::GetTradingDay(const Napi::CallbackInfo &info) {
  return Napi::String::New(ledger_ref_.Env(), time::strftime(get_trading_day(), KUNGFU_TRADING_DAY_FORMAT));
}

Napi::Value Watcher::Now(const Napi::CallbackInfo &info) {
  return Napi::BigInt::New(ledger_ref_.Env(), time::now_in_nano());
}

Napi::Value Watcher::IsUsable(const Napi::CallbackInfo &info) { return Napi::Boolean::New(info.Env(), is_usable()); }

Napi::Value Watcher::IsLive(const Napi::CallbackInfo &info) { return Napi::Boolean::New(info.Env(), is_live()); }

Napi::Value Watcher::IsStarted(const Napi::CallbackInfo &info) { return Napi::Boolean::New(info.Env(), is_started()); }

Napi::Value Watcher::RequestStop(const Napi::CallbackInfo &info) {
  auto app_location = ExtractLocation(info, 0, get_locator());

  // stop master
  if (app_location->category == category::SYSTEM && app_location->group == "master") {
    if (not has_writer(master_cmd_location_->uid)) {
      return Napi::Boolean::New(info.Env(), false);
    }
    get_writer(master_cmd_location_->uid)->mark(now(), RequestStop::tag);
    return Napi::Boolean::New(info.Env(), true);
  }

  if (not has_writer(app_location->uid)) {
    return Napi::Boolean::New(info.Env(), false);
  }
  get_writer(app_location->uid)->mark(now(), RequestStop::tag);
  return Napi::Boolean::New(info.Env(), true);
}

Napi::Value Watcher::PublishState(const Napi::CallbackInfo &info) {
  if (IsValid(info, 0, &Napi::Value::IsObject)) {
    publish(info[0].ToObject());
  }
  return {};
}

Napi::Value Watcher::IsReadyToInteract(const Napi::CallbackInfo &info) {
  auto account_location = ExtractLocation(info, 0, get_locator());
  return Napi::Boolean::New(info.Env(), account_location and has_writer(account_location->uid));
}

Napi::Value Watcher::IssueBlockMessage(const Napi::CallbackInfo &info) {
  SPDLOG_INFO("issue block message manually");
  return InteractWithTD<BlockMessage>(info, &BlockMessage::block_id);
}

Napi::Value Watcher::IssueOrder(const Napi::CallbackInfo &info) {
  SPDLOG_INFO("issue order manually");
  return InteractWithTD<OrderInput>(info, &OrderInput::order_id);
}

Napi::Value Watcher::CancelOrder(const Napi::CallbackInfo &info) {
  SPDLOG_INFO("cancel order manually");
  return InteractWithTD<OrderAction>(info, &OrderAction::order_action_id);
}

Napi::Value Watcher::RequestMarketData(const Napi::CallbackInfo &info) {
  if (not IsValid(info, 0, &Napi::Value::IsObject)) {
    return Napi::Boolean::New(info.Env(), false);
  }

  if (not IsValid(info, 1, &Napi::Value::IsString)) {
    return Napi::Boolean::New(info.Env(), false);
  }

  if (not IsValid(info, 2, &Napi::Value::IsString)) {
    return Napi::Boolean::New(info.Env(), false);
  }

  auto md_location = ExtractLocation(info, 0, get_locator());
  auto exchange_id = info[1].ToString().Utf8Value();
  auto instrument_id = info[2].ToString().Utf8Value();

  SPDLOG_INFO("request market data {}@{} from {}", exchange_id, instrument_id, md_location->uname);

  if (not has_writer(md_location->uid)) {
    return Napi::Boolean::New(info.Env(), false);
  }

  auto writer = get_writer(md_location->uid);
  uint32_t key = hash_instrument(exchange_id.c_str(), instrument_id.c_str());
  InstrumentKey instrument_key = {};
  instrument_key.key = key;
  strcpy(instrument_key.instrument_id, instrument_id.c_str());
  strcpy(instrument_key.exchange_id, exchange_id.c_str());
  instrument_key.instrument_type = get_instrument_type(exchange_id, instrument_id);
  writer->write(now(), instrument_key);
  subscribed_instruments_.emplace(key, instrument_key);

  return Napi::Boolean::New(info.Env(), true);
}

void Watcher::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func =
      DefineClass(env, "Watcher",
                  {
                      InstanceMethod("now", &Watcher::Now),                                             //
                      InstanceMethod("isUsable", &Watcher::IsUsable),                                   //
                      InstanceMethod("isLive", &Watcher::IsLive),                                       //
                      InstanceMethod("isStarted", &Watcher::IsStarted),                                 //
                      InstanceMethod("requestStop", &Watcher::RequestStop),                             //
                      InstanceMethod("hasLocation", &Watcher::HasLocation),                             //
                      InstanceMethod("getLocation", &Watcher::GetLocation),                             //
                      InstanceMethod("getLocationUID", &Watcher::GetLocationUID),                       //
                      InstanceMethod("getInstrumentType", &Watcher::GetInstrumentType),                 //
                      InstanceMethod("publishState", &Watcher::PublishState),                           //
                      InstanceMethod("isReadyToInteract", &Watcher::IsReadyToInteract),                 //
                      InstanceMethod("issueBlockMessage", &Watcher::IssueBlockMessage),                 //
                      InstanceMethod("issueOrder", &Watcher::IssueOrder),                               //
                      InstanceMethod("cancelOrder", &Watcher::CancelOrder),                             //
                      InstanceMethod("requestMarketData", &Watcher::RequestMarketData),                 //
                      InstanceMethod("start", &Watcher::Start),                                         //
                      InstanceMethod("sync", &Watcher::Sync),                                           //
                      InstanceMethod("quit", &Watcher::Quit),                                           //
                      InstanceAccessor("config", &Watcher::GetConfig, &Watcher::NoSet),                 //
                      InstanceAccessor("history", &Watcher::GetHistory, &Watcher::NoSet),               //
                      InstanceAccessor("commission", &Watcher::GetCommission, &Watcher::NoSet),         //
                      InstanceAccessor("state", &Watcher::GetState, &Watcher::NoSet),                   //
                      InstanceAccessor("ledger", &Watcher::GetLedger, &Watcher::NoSet),                 //
                      InstanceAccessor("appStates", &Watcher::GetAppStates, &Watcher::NoSet),           //
                      InstanceAccessor("strategyStates", &Watcher::GetStrategyStates, &Watcher::NoSet), //
                      InstanceAccessor("tradingDay", &Watcher::GetTradingDay, &Watcher::NoSet),         //
                  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Watcher", func);
}

void Watcher::on_react() {
  SPDLOG_INFO("watcher on react");
  // for receive history data
  auto before_start_events = events_ | take_until(events_ | is(RequestStart::tag));
  before_start_events | is_subscribed(subscribed_instruments_) | $$(feed_state_data(event, data_bank_));
  before_start_events | is(Instrument::tag) | $$(Feed(event, event->data<Instrument>()));
  before_start_events | skip_while(while_is(Quote::tag)) | is_trading_data() |
      $$(feed_trading_data(event, trading_bank_));
  before_start_events | skip_while(while_is(Quote::tag, Instrument::tag)) | skip_while(while_is_trading_data) |
      $$(feed_state_data(event, data_bank_));
}

void Watcher::on_start() {
  broker_client_.on_start(events_);

  if (not bypass_accounting_ and not bypass_trading_data_) {
    bookkeeper_.on_start(events_);
    bookkeeper_.guard_positions();
    bookkeeper_.add_book_listener(std::make_shared<BookListener>(*this));

    // for receive runtime data
    events_ | is(Quote::tag) | is_subscribed(subscribed_instruments_) | $$(feed_state_data(event, data_bank_));
    events_ | is(Instrument::tag) | $$(Feed(event, event->data<Instrument>()));
    events_ | skip_while(while_is(Quote::tag)) | is_trading_data() | $$(feed_trading_data(event, trading_bank_));
    events_ | skip_while(while_is(Quote::tag, Instrument::tag)) | skip_while(while_is_trading_data) |
        $$(feed_state_data(event, data_bank_));

    events_ | is(Quote::tag) | is_subscribed(subscribed_instruments_) | $$(UpdateBook(event, event->data<Quote>()));
    events_ | is(OrderInput::tag) | $$(UpdateBook(event, event->data<OrderInput>()));
    events_ | is(Order::tag) | $$(UpdateBook(event, event->data<Order>()));
    events_ | is(Trade::tag) | $$(UpdateBook(event, event->data<Trade>()));
    events_ | is(Position::tag) | $$(UpdateBook(event, event->data<Position>()));
    events_ | is(PositionEnd::tag) | $$(UpdateAsset(event, event->data<PositionEnd>().holder_uid));
    refresh_books();
  }

  events_ | is(Channel::tag) | $$(InspectChannel(event->gen_time(), event->data<Channel>()));
  events_ | is(Register::tag) | $$(OnRegister(event->gen_time(), event->data<Register>()));
  events_ | is(Deregister::tag) | $$(OnDeregister(event->gen_time(), event->data<Deregister>()));
  events_ | is(BrokerStateUpdate::tag) |
      $$(UpdateBrokerState(event->source(), event->dest(), event->data<BrokerStateUpdate>()));
  events_ | is(StrategyStateUpdate::tag) | $$(UpdateStrategyState(event->source(), event->data<StrategyStateUpdate>()));
  events_ | is(CacheReset::tag) | $$(UpdateEventCache(event));
}

void Watcher::refresh_books() {
  for (const auto &pair : bookkeeper_.get_books()) {
    if (pair.second->asset.ledger_category == LedgerCategory::Account) {
      refresh_account_book(now(), pair.first);
    }
  }
}

void Watcher::refresh_account_book(int64_t trigger_time, uint32_t account_uid) {
  auto account_location = get_location(account_uid);
  auto group = account_location->group;
  auto md_location = location::make_shared(account_location->mode, category::MD, group, group, get_locator());
  auto book = bookkeeper_.get_book(account_uid);
  auto subscribe_positions = [&](auto positions) {
    for (const auto &pair : positions) {
      auto &position = pair.second;
      broker_client_.subscribe(md_location, position.exchange_id, position.instrument_id);
    }
  };

  subscribe_positions(book->long_positions);
  subscribe_positions(book->short_positions);
}

void Watcher::Feed(const event_ptr &event, const Instrument &instrument) {
  uint32_t uid = instrument.uid();
  if (feeded_instruments_.find(uid) == feeded_instruments_.end()) {
    data_bank_ << typed_event_ptr<Instrument>(event);
    feeded_instruments_.insert(uid);
  }
}

void Watcher::RestoreState(const location_ptr &state_location, int64_t from, int64_t to, bool sync_schema) {
  add_location(0, state_location);
  serialize::JsRestoreState(ledger_ref_, state_location)(from, to, sync_schema);
}

Napi::Value Watcher::Start(const Napi::CallbackInfo &info) {
  StartWorker();
  return {};
}

void Watcher::Sync(const Napi::CallbackInfo &info) {
  std::lock_guard<std::mutex> guard(feed_mutex_);
  SyncEventCache();
  SyncAppStates();
  SyncStrategyStates();
  SyncLedger();
  TryRefreshTradingData(info);
  SyncTradingData();
}

void Watcher::SyncLedger() {
  boost::hana::for_each(StateDataTypes, [&](auto it) { UpdateLedger(+boost::hana::second(it)); });
}

void Watcher::TryRefreshTradingData(const Napi::CallbackInfo &info) {
  if (refresh_trading_data_before_sync_) {
    serialize::InitTradingDataMap(info, ledger_ref_, "ledger");
  }
}

void Watcher::SyncTradingData() {
  boost::hana::for_each(TradingDataTypes, [&](auto it) { UpdateTradingData(+boost::hana::second(it)); });
}

void Watcher::SyncAppStates() {
  for (auto &s : location_uid_states_map_) {
    auto app_state = Napi::Number::New(app_states_ref_.Env(), s.second);
    app_states_ref_.Set(format(s.first), app_state);
  }
}

void Watcher::SyncStrategyStates() {
  for (auto &s : location_uid_strategy_states_map_) {
    auto strategy_state_obj = Napi::Object::New(strategy_states_ref_.Env());
    strategy_state_obj.Set("state", Napi::Number::New(strategy_states_ref_.Env(), int(s.second.state)));
    strategy_state_obj.Set("update_time", Napi::Number::New(strategy_states_ref_.Env(), s.second.update_time));
    strategy_state_obj.Set("info_a", Napi::String::New(strategy_states_ref_.Env(), s.second.info_a));
    strategy_state_obj.Set("info_b", Napi::String::New(strategy_states_ref_.Env(), s.second.info_b));
    strategy_state_obj.Set("info_c", Napi::String::New(strategy_states_ref_.Env(), s.second.info_c));
    strategy_state_obj.Set("value", Napi::String::New(strategy_states_ref_.Env(), s.second.value));
    strategy_states_ref_.Set(format(s.first), strategy_state_obj);
  }
}

void Watcher::SyncEventCache() {
  if (reset_cache_states_.size()) {
    for (auto &reset_state : reset_cache_states_) {
      reset_cache(reset_state);
    }
    reset_cache_states_.clear();
  }
}

void Watcher::UpdateEventCache(const event_ptr &event) {
  const auto &request = event->data<CacheReset>();
  boost::hana::for_each(StateDataTypes, [&](auto it) {
    using DataType = typename decltype(+boost::hana::second(it))::type;
    if (DataType::tag == request.msg_type) {
      auto hana_type = boost::hana::type_c<DataType>;
      using DelMap = std::unordered_map<uint64_t, state<DataType>>;
      auto &del_map = const_cast<DelMap &>(data_bank_[hana_type]);
      auto iter = del_map.begin();
      while (iter != del_map.end()) {
        auto s = iter->second;
        auto source_id = s.source;
        auto dest_id = s.dest;
        if ((source_id == event->source() and dest_id == event->dest()) || source_id == event->dest()) {
          iter = del_map.erase(iter);
        } else {
          iter++;
        }
      }
    }
  });
  reset_cache_states_.push_back(state<CacheReset>(event));
}

location_ptr Watcher::FindLocation(const Napi::CallbackInfo &info) {
  if (info.Length() == 0) {
    return get_io_device()->get_home();
  }
  uint32_t uid = 0;
  if (info[0].IsNumber()) {
    uid = info[0].ToNumber().Uint32Value();
  }
  if (info[0].IsString()) {
    std::stringstream ss;
    ss << std::hex << info[0].ToString().Utf8Value();
    ss >> uid;
  }
  if (has_location(uid)) {
    return get_location(uid);
  }
  return location_ptr();
}

void Watcher::InspectChannel(int64_t trigger_time, const Channel &channel) {
  if (bypass_trading_data_) {
    return;
  }

  if (channel.source_id == cached_home_location_->uid or channel.dest_id == cached_home_location_->uid) {
    return;
  }

  if (channel.source_id != get_live_home_uid() and channel.dest_id != get_live_home_uid()) {
    reader_->join(get_location(channel.source_id), channel.dest_id, trigger_time);
  }
}

void Watcher::MonitorMarketData(int64_t trigger_time, const location_ptr &md_location) {
  events_ | is(Quote::tag) | from(md_location->uid) | first() |
      $(
          [&, trigger_time, md_location](const event_ptr &event) {
            location_uid_states_map_.insert_or_assign(md_location->uid, int(BrokerState::Ready));
            events_ | from(md_location->uid) | is(Quote::tag) | timeout(std::chrono::seconds(15)) |
                $(noop_event_handler(), [&, trigger_time, md_location](std::exception_ptr e) {
                  if (is_location_live(md_location->uid)) {
                    location_uid_states_map_.insert_or_assign(md_location->uid, int(BrokerState::Idle));
                    MonitorMarketData(trigger_time, md_location);
                  }
                });
          },
          error_handler_log(fmt::format("monitor md {}", md_location->uname)));
}

void Watcher::OnRegister(int64_t trigger_time, const Register &register_data) {
  auto app_uid = register_data.location_uid;
  if (app_uid == get_home_uid()) {
    return;
  }

  auto app_location = get_location(app_uid);
  if (app_location->category == category::MD or app_location->category == category::TD) {
    location_uid_states_map_.insert_or_assign(app_location->uid, int(BrokerState::Pending));
  }

  if (app_location->category == category::MD and app_location->mode == mode::LIVE) {
    MonitorMarketData(trigger_time, app_location);
  }
}

void Watcher::OnDeregister(int64_t trigger_time, const Deregister &deregister_data) {
  auto app_location = location::make_shared(deregister_data, get_locator());
  if (app_location->category == category::MD or app_location->category == category::TD) {
    location_uid_states_map_.insert_or_assign(app_location->uid, int(BrokerState::Pending));
  }

  if (app_location->category == category::SYSTEM and app_location->group == "master" and
      app_location->name == "master") {
    CancelWorker();
  }
}

void Watcher::StartWorker() {
  uv_work_.data = (void *)this;
  uv_work_live_ = true;
  auto worker = [](uv_work_t *req) {
    auto watcher = (Watcher *)(req->data);
    while (req->data && watcher->uv_work_live_) {

      if (not watcher->is_live() and not watcher->is_started() and watcher->is_usable()) {
        watcher->setup();
      }
      if (watcher->is_live() && watcher->feed_mutex_.try_lock()) {
        watcher->step();
        watcher->feed_mutex_.unlock();
      }
      std::this_thread::sleep_for(std::chrono::microseconds(watcher->milliseconds_sleep_after_step_));
    }
    watcher->signal_stop();
    watcher->pause();
    SPDLOG_INFO("Watcher uv loop stopped");
  };
  auto after = [](uv_work_t *req, int status) {
    SPDLOG_INFO("Watcher uv loop completed");
    // have to wait for master down totally
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    auto watcher = (Watcher *)(req->data);
    // have to be at this position, for deleting old journal securitily
    watcher->AfterMasterDown();
    watcher->set_begin_time(time::now_in_nano());
    SPDLOG_INFO("Restart watcher uv loop");
    // master may quit within watcher running time,
    // so, once master deregistered, the uv logic in watcher need to be restarte.
    watcher->StartWorker();
  };
  uv_queue_work(uv_default_loop(), &uv_work_, worker, after);
}

void Watcher::CancelWorker() { uv_work_live_ = false; }

void Watcher::Quit(const Napi::CallbackInfo &info) { uv_work_live_ = false; }

void Watcher::AfterMasterDown() {
  reader_->disjoin(master_cmd_location_->uid);
  writers_.clear();
}

void Watcher::UpdateBrokerState(uint32_t source_id, uint32_t dest_id, const BrokerStateUpdate &state) {
  auto source_location = get_location(state.location_uid);
  if (source_location->category == category::TD or source_location->category == category::MD) {
    location_uid_states_map_.insert_or_assign(source_location->uid, int(state.state));
  }
}

void Watcher::UpdateStrategyState(uint32_t strategy_uid, const StrategyStateUpdate &state) {
  auto app_location = get_location(strategy_uid);
  location_uid_strategy_states_map_.insert_or_assign(app_location->uid, state);
}

void Watcher::UpdateAsset(const event_ptr &event, uint32_t book_uid) {
  auto book = bookkeeper_.get_book(book_uid);
  book->update(event->gen_time());
  state<Asset> cache_state_asset(ledger_home_location_->uid, book_uid, event->gen_time(), book->asset);
  feed_state_data_bank(cache_state_asset, data_bank_);
  state<AssetMargin> cache_state_asset_margin(ledger_home_location_->uid, book_uid, event->gen_time(),
                                              book->asset_margin);
  feed_state_data_bank(cache_state_asset_margin, data_bank_);
}

void Watcher::UpdateBook(const event_ptr &event, const Quote &quote) {
  auto ledger_uid = ledger_home_location_->uid;
  for (const auto &item : bookkeeper_.get_books()) {
    auto &book = item.second;
    auto holder_uid = book->asset.holder_uid;

    if (holder_uid == ledger_uid) {
      continue;
    }

    bool has_long_position_for_quote = book->has_long_position_for(quote);
    bool has_short_position_for_quote = book->has_short_position_for(quote);

    if (has_long_position_for_quote) {
      UpdateBook(event, book->get_position_for(Direction::Long, quote));
    }
    if (has_short_position_for_quote) {
      UpdateBook(event, book->get_position_for(Direction::Short, quote));
    }

    if (has_short_position_for_quote or has_long_position_for_quote) {
      state<Asset> cache_state_asset(ledger_uid, holder_uid, event->gen_time(), book->asset);
      feed_state_data_bank(cache_state_asset, data_bank_);
      state<AssetMargin> cache_state_asset_margin(ledger_uid, holder_uid, event->gen_time(), book->asset_margin);
      feed_state_data_bank(cache_state_asset_margin, data_bank_);
    }
  }
}

void Watcher::UpdateBook(const event_ptr &event, const Position &position) {
  auto book = bookkeeper_.get_book(position.holder_uid);
  auto &book_position = book->get_position_for(position.direction, position);
  auto &book_oppsite_position = book->get_oppsite_position_for(position.direction, position);
  state<Position> cache_state_position(position.holder_uid, event->dest(), event->gen_time(), book_position);
  feed_state_data_bank(cache_state_position, data_bank_);
  state<Position> cache_state_oppsite_position(book_oppsite_position.holder_uid, event->dest(), event->gen_time(),
                                               book_oppsite_position);
  feed_state_data_bank(cache_state_oppsite_position, data_bank_);
}

Watcher::BookListener::BookListener(Watcher &watcher) : watcher_(watcher) {}

void Watcher::BookListener::on_asset_sync_reset(const Asset &old_asset, const Asset &new_asset) {
  auto book = watcher_.bookkeeper_.get_book(new_asset.holder_uid);
  book->update(watcher_.now());
  state<Asset> cache_state(watcher_.ledger_home_location_->uid, book->asset.holder_uid, book->asset.update_time,
                           book->asset);
  watcher_.feed_state_data_bank(cache_state, watcher_.data_bank_);
}

void Watcher::BookListener::on_asset_margin_sync_reset(const AssetMargin &old_asset_margin,
                                                       const AssetMargin &new_asset_margin) {
  auto book = watcher_.bookkeeper_.get_book(new_asset_margin.holder_uid);
  book->update(watcher_.now());
  state<AssetMargin> cache_state(watcher_.ledger_home_location_->uid, book->asset_margin.holder_uid,
                                 book->asset_margin.update_time, book->asset_margin);
  watcher_.feed_state_data_bank(cache_state, watcher_.data_bank_);
}

void Watcher::BookListener::on_position_sync_reset(const book::Book &old_book, const book::Book &new_book) {
  auto fun_update_st_position = [&](book::PositionMap &position_map) {
    for (auto &st_pair : position_map) {
      auto &position = st_pair.second;
      state<Position> cache_state(watcher_.ledger_home_location_->uid, position.holder_uid, position.update_time,
                                  position);
      watcher_.feed_state_data_bank(cache_state, watcher_.data_bank_);
    }
  };

  for (auto &bk_pair : watcher_.bookkeeper_.get_books()) {
    auto &st_book = bk_pair.second;
    fun_update_st_position(st_book->long_positions);
    fun_update_st_position(st_book->short_positions);
  }
}

} // namespace kungfu::node