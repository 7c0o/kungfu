/*****************************************************************************
 * Copyright [www.kungfu-trader.com]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

#include <kungfu/common.h>
#include <kungfu/yijinjing/journal/page.h>
#include <kungfu/yijinjing/util/os.h>

namespace kungfu::yijinjing::journal {
page::page(data::location_ptr location, uint32_t dest_id, const uint32_t page_id, const size_t size, const bool lazy,
           uintptr_t address)
    : location_(std::move(location)), dest_id_(dest_id), page_id_(page_id), lazy_(lazy), size_(size),
      header_(reinterpret_cast<page_header *>(address)) {
  assert(address > 0);
}

page::~page() {
  if (not os::release_mmap_buffer(address(), size_, lazy_)) {
    SPDLOG_ERROR("can not release page {}/{:08x}.{}.journal", location_->uname, dest_id_, page_id_);
  }
}

void page::set_last_frame_position(uint64_t position) {
  const_cast<page_header *>(header_)->last_frame_position = position;
}

page_ptr page::load(const data::location_ptr &location, uint32_t dest_id, uint32_t page_id, bool is_writing,
                    bool lazy) {
  uint32_t page_size = find_page_size(location, dest_id);
  std::string path = get_page_path(location, dest_id, page_id);

  uintptr_t address = os::load_mmap_buffer(path, page_size, is_writing, lazy);
  if (address < 0) {
    throw journal_error("unable to load page for " + path);
  }

  auto header = reinterpret_cast<page_header *>(address);

  if (header->last_frame_position == 0) {
    header->version = __JOURNAL_VERSION__;
    header->page_header_length = sizeof(page_header);
    header->page_size = page_size;
    header->frame_header_length = sizeof(frame_header);
    header->last_frame_position = header->page_header_length;
  }

  if (header->version != __JOURNAL_VERSION__) {
    uint32_t v = header->version;
    throw journal_error(fmt::format("{} version mismatch, required {}, found {}", path, __JOURNAL_VERSION__, v));
  }
  if (header->page_header_length != sizeof(page_header)) {
    uint32_t l = header->page_header_length;
    throw journal_error(fmt::format("{} header length mismatch, required {}, found {}", path, sizeof(page_header), l));
  }
  if (header->page_size != page_size) {
    uint32_t s = header->page_size;
    throw journal_error(
        fmt::format("page size mismatch, required {}, found {}, location {}, path {}, dest_id {}, page_id {}",
                    page_size, s, location->uname, path, dest_id, page_id));
  }

  return std::shared_ptr<page>(new page(location, dest_id, page_id, page_size, lazy, address));
}

std::string page::get_page_path(const data::location_ptr &location, uint32_t dest_id, uint32_t page_id) {
  auto page_name = fmt::format("{:08x}.{}", dest_id, page_id);
  return location->locator->layout_file(location, longfist::enums::layout::JOURNAL, page_name);
}

uint32_t page::find_page_id(const data::location_ptr &location, uint32_t dest_id, int64_t time) {
  std::vector<uint32_t> page_ids = location->locator->list_page_id(location, dest_id);
  if (page_ids.empty()) {
    return 1;
  }
  if (time == 0) {
    return page_ids.front();
  }
  for (int i = static_cast<int>(page_ids.size()) - 1; i >= 0; i--) {
    if (page::load(location, dest_id, page_ids[i], false, true)->begin_time() < time) {
      return page_ids[i];
    }
  }
  return page_ids.front();
}
} // namespace kungfu::yijinjing::journal
