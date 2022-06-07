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

#include <chrono>
#include <ctime>
#include <fmt/format.h>
#include <iomanip>
#include <regex>
#include <sstream>
#include <assert.h>

#include <kungfu/common.h>
#include <kungfu/yijinjing/time.h>

using namespace std::chrono;

namespace kungfu::yijinjing {
int64_t time::now_in_nano() {
  auto duration = steady_clock::now().time_since_epoch().count() - get_instance().base_.steady_clock_count;
  return get_instance().base_.system_clock_count + duration;
}

uint32_t time::nano_hashed(int64_t nano_time) {
  return kungfu::hash_32((const unsigned char *)&nano_time, sizeof(nano_time));
}

int64_t time::next_minute(int64_t nanotime) {
  return nanotime - nanotime % time_unit::NANOSECONDS_PER_MINUTE + time_unit::NANOSECONDS_PER_MINUTE;
}

int64_t time::next_trading_day_end(int64_t nanotime) {
  int64_t end_offset = 15 * time_unit::NANOSECONDS_PER_HOUR + 30 * time_unit::NANOSECONDS_PER_MINUTE;
  int64_t trading_day = nanotime - nanotime % time_unit::NANOSECONDS_PER_DAY - time_unit::UTC_OFFSET + end_offset;
  if (trading_day < now_in_nano()) {
    trading_day += time_unit::NANOSECONDS_PER_DAY;
  }
  return trading_day;
}

int64_t time::calendar_day_start(int64_t nanotime) {
  return nanotime - (nanotime % time_unit::NANOSECONDS_PER_DAY) - time_unit::UTC_OFFSET;
}

int64_t time::today_start() { return calendar_day_start(time::now_in_nano()); }

int64_t time::strptime(const std::string &time_string, const std::string &format) {
  int64_t nano = 0;
  std::string normal_timestr = time_string;
  std::string normal_format = format;

  std::regex nano_format_regex("%N");
  if (std::regex_search(format, nano_format_regex)) {
    normal_format = std::regex_replace(format, nano_format_regex, "");

    std::regex nano_regex("(\\d{9})");

    auto nano_begin = std::sregex_iterator(normal_timestr.begin(), normal_timestr.end(), nano_regex);
    auto nano_end = std::sregex_iterator();
    for (std::sregex_iterator i = nano_begin; i != nano_end; ++i) {
      std::smatch match = *i;
      nano = stol(match.str());
      normal_timestr = std::regex_replace(time_string, nano_regex, "");
    }
  }

  std::tm result = {};
  std::istringstream iss(time_string);
  iss >> std::get_time(&result, normal_format.c_str());
  std::time_t parsed_time = std::mktime(&result);
  auto tp_system = system_clock::from_time_t(parsed_time);
  return duration_cast<nanoseconds>(tp_system.time_since_epoch()).count() + nano;
}

int64_t time::strptime(const std::string &time_string, std::initializer_list<std::string> formats) {
  for (const auto &format : formats) {
    auto t = strptime(time_string, format);
    if (strftime(t, format) == time_string) {
      return t;
    }
  }
  return -1;
}

std::string time::strftime(int64_t nanotime, const std::string &format) {
  if (nanotime == INT64_MAX) {
    return "end of world";
  }
  time_point<steady_clock> tp_steady((nanoseconds(nanotime)));
  auto tp_epoch_steady = time_point<steady_clock>{};
  auto tp_diff = tp_steady - tp_epoch_steady;

  auto tp_epoch_system = time_point<system_clock>{};
  std::time_t time_since_epoch =
      system_clock::to_time_t(tp_epoch_system + duration_cast<system_clock::duration>(tp_diff));

  std::string normal_format = format;
  std::regex nano_format_regex("%N");
  if (std::regex_search(format, nano_format_regex)) {
    int64_t nano = tp_diff.count() % time_unit::NANOSECONDS_PER_SECOND;
    normal_format = std::regex_replace(format, nano_format_regex, fmt::format("{:09d}", nano));
  }

  std::ostringstream oss;
  oss << std::put_time(std::localtime(&time_since_epoch), normal_format.c_str());
  if (nanotime > 0) {
    return oss.str();
  } else if (nanotime == 0) {
    return std::regex_replace(oss.str(), std::regex("\\d"), "0");
  } else {
    return std::regex_replace(oss.str(), std::regex("\\d"), "#");
  }
}

std::string time::strfnow(const std::string &format) { return strftime(now_in_nano(), format); }

time_point_info time::get_base() { return get_instance().base_; }

void time::reset(int64_t system_clock_count, int64_t steady_clock_count) {
  time_point_info &base = const_cast<time &>(get_instance()).base_;
  base.system_clock_count = system_clock_count;
  base.steady_clock_count = steady_clock_count;
}

/**
 * start_time_system_ sample: 1560144011373015000
 * start_time_steady_ sample: 867884767983511
 */
time::time() : base_() {
  auto system_clock_now = system_clock::now();
  auto steady_clock_now = steady_clock::now();
  base_.system_clock_count = duration_cast<nanoseconds>(system_clock_now.time_since_epoch()).count();
  base_.steady_clock_count = steady_clock_now.time_since_epoch().count();
}

const time &time::get_instance() {
  static time instance = {};
  return instance;
}


#define ONE_HOUR_SECOND 3600
#define ONE_DAY_SECOND (ONE_HOUR_SECOND * 24)

/*
 * ����ʱ����Ϣ
 */
struct LocationTimeData {
public:
  LocationTimeData(int time_zone, ZoneTimeType standard_type) : zone(time_zone) { zone_times.push_back(standard_type); }

  LocationTimeData(int time_zone, const std::function<__int64(__int64)> &summer_begin,
                   const std::function<__int64(__int64)> &summer_end, ZoneTimeType standard_type,
                   ZoneTimeType daylight_type)
      : zone(time_zone), has_summer_day(true), f_summer_begin(summer_begin), f_summer_end(summer_end) {
    zone_times.push_back(standard_type);
    zone_times.push_back(daylight_type);
  }

public:
  int zone = 0;                // ʱ����TimeZone�������磺����ʱ��Ϊ8
  bool has_summer_day = false; // �õ����Ƿ�������ʱ

  std::function<__int64(__int64)> f_summer_begin;
  std::function<__int64(__int64)> f_summer_end;

  std::vector<ZoneTimeType> zone_times;
};

/*
 * ����ʱʱ����Ϣ
 * ����ʱһ��Ķ��������month+1�·ݵ�sunday_index�������գ�hourСʱ��ʼ���������
 * ps: �� sunday_index <0 ��ʾ���򣬱������һ�������վ���-1
 */
time_t CalculateSummerDayTimeByMonth_Sunday_Hour(time_t time, int month, int sunday_index, int hour) {
  // ������Ҫ�жϵ�ʱ������ó���������ʱ��
  tm t_gmt = {0};
  gmtime_s(&t_gmt, &time);

  tm summer_day;

  // �����Ҫ�봫��ʱ��������ͬ
  summer_day.tm_year = t_gmt.tm_year;

  // ��sunday_index<0 ʱ����Ҫ�����¸��µ�һ�쵹��
  summer_day.tm_mon = sunday_index > 0 ? month : (month + 1);
  summer_day.tm_mday = 1;
  summer_day.tm_hour = hour;
  summer_day.tm_min = 0;
  summer_day.tm_sec = 0;

  time_t summer_day_seconds = _mkgmtime(&summer_day);
  gmtime_s(&summer_day, &summer_day_seconds);

  int week = summer_day.tm_wday;
  int offset_weeks = sunday_index > 0 ? (sunday_index - 1) : sunday_index;

  // ����ǰ���뵽����
  summer_day_seconds += (7 - week) % 7 * ONE_DAY_SECOND;

  // �ٰ��ܽ���ƫ��
  summer_day_seconds += offset_weeks * 7 * ONE_DAY_SECOND;
  return summer_day_seconds;
}

#define STByM_S_H(t, m, s, h)                                                                                          \
  ([=](__int64 t) -> __int64 { return CalculateSummerDayTimeByMonth_Sunday_Hour(t, m, s, h); })

static const std::unordered_map<LocationTimeType, LocationTimeData> g_locationTimeMap = {
    {LocationTimeType::Beijing, LocationTimeData(8, ZoneTimeType::BeijingTime)},
    {LocationTimeType::Singapore, LocationTimeData(8, ZoneTimeType::SGT)},
    {LocationTimeType::Tokyo, LocationTimeData(9, ZoneTimeType::JST)},
    {LocationTimeType::AmericaEastern,
     // ����ʱ������ʱ��[3�µڶ����������賿2�㣬11�µ�һ�������賿2��]
     LocationTimeData(-5, STByM_S_H(t, 2, 2, 2), STByM_S_H(t, 10, 1, 2), ZoneTimeType::EST, ZoneTimeType::EDT)},
    {LocationTimeType::AmericaCentral,
     // ����ʱ������ʱ����������
     LocationTimeData(-6, STByM_S_H(t, 2, 2, 2), STByM_S_H(t, 10, 1, 2), ZoneTimeType::CST, ZoneTimeType::CDT)},
    {LocationTimeType::London,
     // �׶�ʱ������ʱ��[3�����һ���������賿1�㣬10�����һ���������賿2��]
     LocationTimeData(0, STByM_S_H(t, 2, -1, 1), STByM_S_H(t, 9, -1, 2), ZoneTimeType::BST, ZoneTimeType::BDT)},
    {LocationTimeType::AustraliaEastern,
     // ���޶���ʱ������ʱ��[10�µ�һ���������賿2�㣬4�µ�һ���������賿3��]
     LocationTimeData(10, STByM_S_H(t, 9, 1, 2), STByM_S_H(t, 3, 1, 3), ZoneTimeType::AEST, ZoneTimeType::AEDT)},
    {LocationTimeType::Berlin,
     // �¹�ʱ������ʱ��[3�����һ���������賿2�㣬10�����һ���������賿3��]
     LocationTimeData(1, STByM_S_H(t, 2, -1, 2), STByM_S_H(t, 9, -1, 3), ZoneTimeType::CEST, ZoneTimeType::CET)}
    // Add more LocationTime here...
};

namespace TimeUtil {
time_t TimeToSeconds(const std::string &time, bool is_gmt) {
  int year, month, day, hour, minute, second;
  sscanf(time.c_str(), "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);

  return TimeToSeconds(year, month, day, hour, minute, second, is_gmt);
}

time_t TimeToSeconds(int year, int month, int day, int hour, int minute, int second, bool is_gmt) {
  tm t_temp = {0};
  t_temp.tm_year = year - 1900;
  t_temp.tm_mon = month - 1;
  t_temp.tm_mday = day;
  t_temp.tm_hour = hour;
  t_temp.tm_min = minute;
  t_temp.tm_sec = second;
  t_temp.tm_isdst = 0;

  return is_gmt ? _mkgmtime(&t_temp) : mktime(&t_temp);
}

time_t DateToSeconds(const std::string &time, bool is_gmt) {
  int year, month, day;
  sscanf(time.c_str(), "%d-%d-%d", &year, &month, &day);

  return TimeToSeconds(year, month, day, 0, 0, 0, is_gmt);
}

const LocationTimeData &GetLocationTimeDataByType(LocationTimeType loc_type) {
  std::unordered_map<LocationTimeType, LocationTimeData>::const_iterator it = g_locationTimeMap.find(loc_type);
  if (it == g_locationTimeMap.end()) {
    assert("find loc_type fail��");

    // �Ҳ�����Ӧ��Location���Ȱ�����ʱ�䷵��
    it = g_locationTimeMap.find(LocationTimeType::Beijing);
  }
  return it->second;
}

std::shared_ptr<LocalTimeInfo> TranslateGMTimeToLocalTime(time_t gmt, const LocationTimeData &info) {
  std::shared_ptr<LocalTimeInfo> t_local = std::make_shared<LocalTimeInfo>();
  do {
    t_local->seconds = gmt + info.zone * ONE_HOUR_SECOND;
    t_local->has_summer_day = info.has_summer_day;

    if (!info.has_summer_day) {
      break;
    }

    __int64 begin = info.f_summer_begin(gmt);
    __int64 end = info.f_summer_end(gmt);

    /*
     * ���������ڣ�һ���ʾ��������ʱ���������ڱ����ڵ�begin>end����case��˵��������ǡǡ��ʾ��������ʱ��
     * ����ʹ�� begin > end ���ȡ������һ��
     */
    t_local->is_summer_day = (gmt >= std::min(begin, end) && gmt <= std::max(begin, end)) ^ (begin > end);
    if (t_local->is_summer_day) {
      t_local->seconds += ONE_HOUR_SECOND;
    }

  } while (false);

  t_local->zone_time_type = info.zone_times[t_local->is_summer_day];
  return t_local;
}

time_t TranslateLocalTimeToGMTime(time_t local_seconds, LocationTimeType loc_type, LocalTimeInfo *info /*= nullptr*/) {
  const LocationTimeData &data = GetLocationTimeDataByType(loc_type);

  /*
   * ͬһ�� local_seconds�п��ܶ�Ӧ����ʱ������ʱ�Ĳ�ͬʱ��
   * �������Ȱ����Ƿ���Location�б���������ڵĽ����жϣ����ַ�ʽ�п��ܾ�ȷ�����Ԥ�ڲ�������������Ӱ��ҵ���߼�
   */
  time_t assume_gmt = local_seconds - data.zone * ONE_HOUR_SECOND;
  std::shared_ptr<LocalTimeInfo> ret = TranslateGMTimeToLocalTime(assume_gmt, data);

  // ���������ʱ����ô�����ټ�һ��Сʱ��Ϊ����
  if (ret->is_summer_day) {
    ret->seconds -= ONE_HOUR_SECOND;
    assume_gmt -= ONE_HOUR_SECOND;
  }

  if (info) {
    *info = *ret;
  }
  return assume_gmt;
}

std::shared_ptr<LocalTimeInfo> TranslateGMTimeToLocalTime(time_t gmt, LocationTimeType loc_type) {
  const LocationTimeData &info = GetLocationTimeDataByType(loc_type);
  return TranslateGMTimeToLocalTime(gmt, info);
}
} // namespace TimeUtil

} // namespace kungfu::yijinjing
