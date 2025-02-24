/*
  Dcf77Receiver - Arduino libary receiving and decoding Dcf77 frames Copyright (c)
  2025 Wolfgang Schmieder.  All right reserved.

  Contributors:
  - Wolfgang Schmieder

  Project home: https://github.com/dac1e/Dcf77Receiver/

  This library is free software; you can redistribute it and/or modify it
  the terms of the GNU Lesser General Public License as under published
  by the Free Software Foundation; either version 3.0 of the License,
  or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
*/

#include <print.h>
#include "Dcf77tm.h"

#define DEBUG_TIMESTAMP_TO_TM false

namespace {

#if DEBUG_TIMESTAMP_TO_TM
#include "Arduino.h"
template<typename T> void print_variable(const T& v, const char* const varname) {
  Serial.print(varname);
  Serial.print('=');
  Serial.println(v);
}

#define STRINGIFY(s) #s
#define PRINT_VARIABLE(x) print_variable(x, STRINGIFY(x))
#else
#define PRINT_VARIABLE(x)
#endif

/* Move epoch from 01.01.1970 to 01.03.0000 (yes, Year 0) - this is the first
 * day of a 400-year long "era", right after additional day of leap year.
 * This adjustment is required only for date calculation, so instead of
 * modifying time_t value (which would require 64-bit operations to work
 * correctly) it's enough to adjust the calculated number of days since epoch.
 */
constexpr int32_t EPOCH_ADJUSTMENT_DAYS = 719468L;
/* 1st March of year 0 is Wednesday */
constexpr int ADJUSTED_EPOCH_WDAY = 3;
/* year to which the adjustment was made */
constexpr int ADJUSTED_EPOCH_YEAR = 0;
/* there are 97 leap years in 400-year periods. ((400 - 97) * 365 + 97 * 366) */
constexpr int32_t DAYS_PER_ERA = 146097L;
/* there are 24 leap years in 100-year periods. ((100 - 24) * 365 + 24 * 366) */
constexpr int32_t DAYS_PER_CENTURY = 36524L;
/* there is one leap year every 4 years */
constexpr int DAYS_PER_4_YEARS = 3 * 365 + 366;
/* number of days in a non-leap year */
constexpr int DAYS_PER_YEAR = 365;
/* number of years per era */
constexpr int YEARS_PER_ERA = 400;

constexpr int DAYSPERWEEK = 7;
constexpr int SECSPERMIN = 60;
constexpr int32_t SECSPERHOUR = SECSPERMIN * 60;
constexpr int32_t SECSPERDAY = SECSPERHOUR * 24;

const int month_yday[2][12] = {
  {-1, 30, 58, 89, 119, 150, 180, 211, 242, 272, 303, 333},
  {-1, 30, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
} ;

inline int isLeapYear(uint16_t rtc_year) {
  const int result = ( not (rtc_year % 4) && ( (rtc_year % 100) || not(rtc_year % 400) ) );;
  return result;
}

/**
 * Calculate the number of leap years since 1970 for a given year.
 * @param year The anno domini year.
 */
inline int leapYearsSince1970 (const int year) {
  const int yearsDiv4count   = (year-1968 /* first year that divides by   4 without rest. */) /   4;
  const int yearsDiv100count = (year-1900 /* first year that divides by 100 without rest. */) / 100;
  const int yearsDiv400count = (year-1600 /* first year that divides by 400 without rest. */) / 400;
  return yearsDiv4count - yearsDiv100count + yearsDiv400count;
}

/**
 * Calculate the expired days since 1st of January.
 */
inline int yday(const Dcf77tm& tm) {
  const int leapYear = isLeapYear(tm.year());
  const int month = tm.tm_mon;
  const int yday_ = month_yday[leapYear][month];
  const uint8_t day = tm.tm_mday;
  return yday_ + day;
}

} // anonymous namespace

#if HAS_STD_CTIME

size_t Dcf77tm::printTo(Print& p) const {
  char buffer[26];
  asctime_r(this, buffer);
  buffer[24] = '\0'; // remove /n
  return p.print(buffer);
}

#else

static const char* MO[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
static const char* WD[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

size_t Dcf77tm::printTo(Print& p) const {
  size_t n = 0;

  n+= p.print(WD[tm_wday]); // day of week
  n+= p.print(" ");
  n+= p.print(MO[tm_mon]);  // month
  n+= p.print(" ");
  n+= p.print(tm_mday);     // day of month

  n+= p.print(" ");
  if(tm_hour < 10) {n+= p.print('0');}
  n+= p.print(tm_hour); // hour
  n+= p.print(":");
  if(tm_min < 10) {n+= p.print('0');}
  n+= p.print(tm_min);  // minute
  n+= p.print(":");
  if(tm_sec < 10) {n+= p.print('0');}
  n+= p.print(tm_sec);  // second

  n+= p.print(" ");
  n+= p.print(tm_year + 1900); // year
  return n;
}

#endif

Dcf77time_t Dcf77tm::toTimeStamp() const {
  using time_t = Dcf77time_t;
  const bool leapYear = isLeapYear(year());
  const time_t leapYearsBeforeThisYear = leapYearsSince1970(year()) - leapYear;
  const time_t yearOffset = year() - 1970;
  const time_t result = tm_sec + (tm_min + (tm_hour + (yday(*this) + leapYearsBeforeThisYear + yearOffset * 365) * 24) * 60) * 60;
  return result;
}

void Dcf77tm::set(const Dcf77time_t timestamp, const int isdst)
{
  PRINT_VARIABLE(timestamp);
  long days = timestamp / SECSPERDAY + EPOCH_ADJUSTMENT_DAYS;
  PRINT_VARIABLE(days);
  long remain = timestamp % SECSPERDAY;
  PRINT_VARIABLE(remain);
  if (remain < 0) {
    remain += SECSPERDAY;
    --days;
  }
  PRINT_VARIABLE(remain);

  /* compute day of week */
  tm_wday = ((((ADJUSTED_EPOCH_WDAY + DAYSPERWEEK) + days) % DAYSPERWEEK));
  PRINT_VARIABLE(tm_wday);

  /* compute hour, min, and sec */
  tm_hour = (remain / SECSPERHOUR);
  remain %= SECSPERHOUR;
  PRINT_VARIABLE(remain);
  tm_min = (remain / SECSPERMIN);
  PRINT_VARIABLE(tm_min);
  tm_sec = (remain % SECSPERMIN);
  PRINT_VARIABLE(tm_sec);

  /* compute year, month, day & day of year. For description of this algorithm see
   * http://howardhinnant.github.io/date_algorithms.html#civil_from_days */
  const int era = (days >= 0 ? days : days - (DAYS_PER_ERA - 1)) / DAYS_PER_ERA;
  PRINT_VARIABLE(era);
  const unsigned long eraday = days - era * DAYS_PER_ERA; /* [0, 146096] */
  PRINT_VARIABLE(eraday);
  const unsigned erayear = (eraday - eraday / (DAYS_PER_4_YEARS - 1) + eraday / DAYS_PER_CENTURY -
      eraday / (DAYS_PER_ERA - 1)) / 365;         /* [0, 399] */
  PRINT_VARIABLE(erayear);
  const unsigned yearday = eraday - (DAYS_PER_YEAR * erayear + erayear / 4 - erayear / 100); /* [0, 365] */
  PRINT_VARIABLE(yearday);
  const unsigned m = (5 * yearday + 2) / 153;     /* [0, 11] */
  PRINT_VARIABLE(m);
  const unsigned month = m < 10 ? m + 2 : m - 10;
  PRINT_VARIABLE(month);

  tm_mday = yearday - (153 * m + 2) / 5 + 1;  /* [1, 31] */
  PRINT_VARIABLE(tm_mday);
  tm_mon = month;
  PRINT_VARIABLE(tm_mon);
  tm_year = ADJUSTED_EPOCH_YEAR - TM_YEAR_BASE + erayear + era * YEARS_PER_ERA + (month <= 1);
  PRINT_VARIABLE(tm_year);
  tm_isdst = isdst;
  PRINT_VARIABLE(tm_isdst);
}

