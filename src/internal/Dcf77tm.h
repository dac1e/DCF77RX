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

#pragma once

#ifndef DCF77QINT_INTERNAL_DCF77TM_H_
#define DCF77QINT_INTERNAL_DCF77TM_H_


#include <stdint.h>
#include <printable.h>

#ifndef ARDUINO_ARCH_AVR
#define HAS_STD_CTIME true
#endif

#if HAS_STD_CTIME
  // Use std::tm
  #include <ctime>

  using Dcf77time_t = std::time_t;

  // See https://en.cppreference.com/w/cpp/chrono/c/tm
  struct Dcf77tm : public std::tm, public Printable {
    static constexpr int TM_YEAR_BASE = 1900;

    int year() const {return tm_year + TM_YEAR_BASE;}

    Dcf77time_t toTimeStamp() const;
    void set(const std::time_t timestamp, const uint8_t isdst);

    const Dcf77tm& operator+=(const Dcf77time_t& sec);
  private:
    size_t printTo(Print& p) const override;
  };

#else
  // Define own std::tm
  using Dcf77time_t = uint64_t;

  // See https://en.cppreference.com/w/cpp/chrono/c/tm
  struct Dcf77tm : public Printable {
    static constexpr int TM_YEAR_BASE = 1900;

    int year() const {return tm_year + TM_YEAR_BASE;}

    Dcf77time_t toTimeStamp() const;
    void set(const Dcf77time_t timestamp, const uint8_t isdst);

    const Dcf77tm& operator+=(const Dcf77time_t& sec);

    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;   // [0..11]
    int tm_year;  // years since 1900
    int tm_wday;
    int tm_yday;
    int tm_isdst;

  private:
    size_t printTo(Print& p) const override;
  };

#endif
  inline const Dcf77tm& Dcf77tm::operator+=(const Dcf77time_t& sec) {
    if((tm_sec + sec) < 60) {
      tm_sec += sec;
    } else {
      const Dcf77time_t timestamp = toTimeStamp() + sec;
      set(timestamp, tm_isdst);
    }
    return *this;
  }


#endif /* DCF77QINT_INTERNAL_DCF77TM_H_ */
