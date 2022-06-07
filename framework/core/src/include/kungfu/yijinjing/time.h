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

#ifndef YIJINJING_TIME_H
#define YIJINJING_TIME_H

#include <memory>
#include <string>
#include <time.h>

#define KUNGFU_DATETIME_FORMAT "%F %T"
#define KUNGFU_TIMESTAMP_FORMAT "%F %T.%N"
#define KUNGFU_TRADING_DAY_FORMAT "%Y%m%d"
#define KUNGFU_HISTORY_DAY_FORMAT "%Y-%m-%d"

namespace kungfu::yijinjing {
struct time_unit {
  static constexpr int64_t MILLISECONDS_PER_SECOND = 1000;
  static constexpr int64_t MICROSECONDS_PER_MILLISECOND = 1000;
  static constexpr int64_t NANOSECONDS_PER_MICROSECOND = 1000;

  static constexpr int64_t SECONDS_PER_MINUTE = 60;
  static constexpr int64_t MINUTES_PER_HOUR = 60;
  static constexpr int64_t HOURS_PER_DAY = 24;
  static constexpr int64_t SECONDS_PER_HOUR = SECONDS_PER_MINUTE * MINUTES_PER_HOUR;

  static constexpr int64_t NANOSECONDS_PER_MILLISECOND = NANOSECONDS_PER_MICROSECOND * MICROSECONDS_PER_MILLISECOND;
  static constexpr int64_t NANOSECONDS_PER_SECOND = NANOSECONDS_PER_MILLISECOND * MILLISECONDS_PER_SECOND;
  static constexpr int64_t NANOSECONDS_PER_MINUTE = NANOSECONDS_PER_SECOND * SECONDS_PER_MINUTE;
  static constexpr int64_t NANOSECONDS_PER_HOUR = NANOSECONDS_PER_SECOND * SECONDS_PER_HOUR;
  static constexpr int64_t NANOSECONDS_PER_DAY = NANOSECONDS_PER_HOUR * HOURS_PER_DAY;

  static constexpr int64_t UTC_OFFSET = NANOSECONDS_PER_HOUR * 8;
};

struct time_point_info {
  int64_t system_clock_count;
  int64_t steady_clock_count;
};

class time {
public:
  /**
   * Get timestamp in nano seconds.
   * @return current nano time in int64_t (unix-timestamp * 1e9 + nano-part)
   */
  static int64_t now_in_nano();

  static uint32_t nano_hashed(int64_t nano_time);

  /**
   * Given a timestamp, returns the start point of next minute.
   * @param nanotime timestamp in nano seconds
   * @return start time point of next minute in nano seconds
   */
  static int64_t next_minute(int64_t nanotime);

  /**
   * Given a timestamp, returns the next end of trading time, i.e. 15:30 of today if the argument is before that,
   * otherwise 15:30 of tomorrow.
   * @param nanotime timestamp in nano seconds
   * @return the next trading session end time point in nano seconds
   */
  static int64_t next_trading_day_end(int64_t nanotime);

  /**
   * Given a timestamp, returns the start time of the corresponding calendar day.
   * @param nanotime timesamp in nano seconds
   * @return the start time point of the corresponding calendar day in nano seconds
   */
  static int64_t calendar_day_start(int64_t nanotime);

  /**
   * Start time of today (00:00:00) in nano seconds.
   * @return start time of today in nano seconds
   */
  static int64_t today_start();

  /**
   * Parse string time to nano time.
   * @param time_string string-formatted time
   * @param format ref: https://en.cppreference.com/w/cpp/io/manip/put_time + %N for nanoseconds {:09d}
   * @return nano time in int64_t
   */
  static int64_t strptime(const std::string &time_string, const std::string &format = KUNGFU_TIMESTAMP_FORMAT);

  /**
   * Parse string time to nano time by matching one of the formats in list.
   * @param time_string string-formatted time
   * @param formats ref: https://en.cppreference.com/w/cpp/io/manip/put_time + %N for nanoseconds {:09d}
   * @return nano time in int64_t, -1 if no format matched
   */
  static int64_t strptime(const std::string &time_string, std::initializer_list<std::string> formats);

  /**
   * Format nano seconds to string.
   * @param nanotime nano time in int64_t
   * @param format ref: https://en.cppreference.com/w/cpp/io/manip/put_time + %N for nanoseconds {:09d}
   * @return string-formatted time
   */
  static std::string strftime(int64_t nanotime, const std::string &format = KUNGFU_TIMESTAMP_FORMAT);

  /**
   * Format now to string.
   * @param format ref: https://en.cppreference.com/w/cpp/io/manip/put_time + %N for nanoseconds {:09d}
   * @return string-formatted time
   */
  static std::string strfnow(const std::string &format = KUNGFU_TIMESTAMP_FORMAT);

  /**
   * Gets time base point of current process.
   * @return time base point which is set by the singleton method.
   */
  static time_point_info get_base();

  /**
   * Reset time base point so that multiprocess application can have synced time.
   * @param system_clock_count std::chrono::system_clock nano seconds count
   * @param steady_clock_count std::chrono::steady_clock nano seconds count
   */
  static void reset(int64_t system_clock_count, int64_t steady_clock_count);

private:
  time_point_info base_;
  time();

  static const time &get_instance();
};


/*****************************************************************************
*  @Copyright (c) 2022, Marsjliu
*  @All rights reserved

*  @date     : 2022/06/06 11:15
*  @brief    :ʱ��ת��������
*****************************************************************************/
/*
 * ��ȷ�������ĵ���ʱ��
 * Ϊ�������壬ԭ���Ͻ�ֹ�ⲿֱ�Ӵ���TimeZone���֣����紫��8������ʱ������
 * ��ȷ�������ǣ�������ȷ��ʱ�����ͣ�LocationTime::Beijing
 */
enum class LocationTimeType : char {
  London,           // �׶�ʱ��
  Beijing,          // ����ʱ��
  Singapore,        // �¼���ʱ�䣬Ŀǰ�뱱��ʱ����ͬ
  Tokyo,            // ����ʱ��
  AmericaEastern,   // ��������ʱ�䣨�����Ƿ�����ʱ�����ܾ���Ϊ EST/EDT��
  AmericaCentral,   // �����в�ʱ�䣨�����Ƿ�����ʱ�����ܾ���Ϊ CST/CDT��
  AustraliaEastern, // ���ݶ���ʱ�䣨�����Ƿ�����ʱ�����ܾ���Ϊ AEST/ADST��
  Berlin            // ����ʱ�䣨�����Ƿ�����ʱ�����ܾ���Ϊ CEST/CET��
};

/*
 * ʱ�������ο���http://zh.thetimenow.com/time-zones-abbreviations.php
 */
enum class ZoneTimeType : char {
  BST,         // �׶ر�׼ʱ��
  BDT,         // �׶�����ʱ
  BeijingTime, // ����ʱ�䣨ԭ��дCST �����б�׼ʱ�䶨���ظ�������ֱ�Ӷ���BeijingTime��
  JST,         // ����ʱ��
  SGT,         // �¼���ʱ��
  EST,         // ����������׼ʱ��
  EDT,         // ������������ʱ
  CST,         // �����в���׼ʱ��
  CDT,         // �����в�����ʱ
  AEST,        // �Ĵ����Ƕ�����׼ʱ��
  AEDT,        // �Ĵ����Ƕ�������ʱ
  CET,         // ��ŷ��׼ʱ��
  CEST         // ��ŷ����ʱ
};

struct LocalTimeInfo {
  time_t seconds = 0;

  bool has_summer_day = false;
  bool is_summer_day = false;

  ZoneTimeType zone_time_type = ZoneTimeType::BST;
};
namespace TimeUtil {
	// ֧�� yyyy-mm-dd hh:mm:ss�������ַ�����ʽ��������2017-08-10 10:10:10
	time_t TimeToSeconds(const std::string &time, bool is_gmt);
	time_t TimeToSeconds(int year, int month, int day, int hour, int minute, int second, bool is_gmt);

	//���ڵ�ʱ�� 2017-08-10 ��0ʱ0��0������ת��
    time_t DateToSeconds(const std::string &time, bool is_gmt);

	time_t TranslateLocalTimeToGMTime(time_t local_seconds, LocationTimeType loc_type, LocalTimeInfo *info = nullptr);
    std::shared_ptr<LocalTimeInfo> TranslateGMTimeToLocalTime(time_t gmt, LocationTimeType loc_type);
 } // namespace kungfu::yijinjing::TimeUtil

} // namespace kungfu::yijinjing

#endif // YIJINJING_TIME_H