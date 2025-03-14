/*
  DCF77RX - Arduino libary receiving and decoding DCF77 frames Copyright (c)
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

#ifndef DCF77QINT_INTERNAL_DCF77TM_HPP_
#define DCF77QINT_INTERNAL_DCF77TM_HPP_


#include <stdint.h>
#include <Printable.h>
#include <Print.h>

#ifndef ARDUINO_ARCH_AVR
#define HAS_STD_CTIME true
#endif

#ifdef ARDUINO_ARCH_MBED
typedef arduino::Printable printable_t;
typedef arduino::Print print_t;
#else
typedef Printable printable_t;
typedef Print print_t;
#endif

#if HAS_STD_CTIME
  /** Use std::tm and std::time_t */
  #include <ctime>

  // See  https://en.cppreference.com/w/cpp/chrono/c/time_t
  using DCF77time_t = std::time_t;


  // See https://en.cppreference.com/w/cpp/chrono/c/tm
  struct DCF77tm : public std::tm, public printable_t {

  static size_t print(print_t& p, const std::tm& time);
#else
  /** Define own tm and time_t */

  // See https://en.cppreference.com/w/cpp/chrono/c/time_t
  using DCF77time_t = uint32_t;

  // See https://en.cppreference.com/w/cpp/chrono/c/tm
  struct DCF77tm : public printable_t {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;   // [0..11]
    int tm_year;  // years since 1900
    int tm_wday;
    int tm_yday;
    int tm_isdst; // daylight savings active = 1

    static size_t print(print_t& p, const DCF77tm& time) {
  		return time.printTo(p);
  	}

#endif /* HAS_STD_CTIME */

    /**
     * TM_YEAR_BASE is the offset between the Anno Domini
     * year and tm_year field in the tm structure.
     */
   static constexpr int TM_YEAR_BASE = 1900;

    /**
     * @return the Anno Domini year from the tm_year field
     *  of this structure.
     */
    int year() const {return tm_year + TM_YEAR_BASE;}

    /**
     * Convert the tm structure to a time_t timestamp
     *
     * @return Expired seconds since 1 Jan 0:00:00  1970
     */
    DCF77time_t toTimeStamp() const;

    /**
     * Set this tm structure from a time_t timestamp and
     * daylight savings flag.
     */
    void set(const DCF77time_t timestamp, const int isdst);

    /**
     * Implementation of the Printable interface, which
     * allows to print this tm structure.
     *
     * @return The number of printed characters.
     *
     * Example:
     *   DCF77tm tm;
     *
     *   tm.tm_hour = 15;
     *   tm.tm_min = 10;
     *   tm.tm_sec = 30;
     *   tm.tm_mday = 23;
     *   tm.tm_mon = 1;
     *   tm.tm_year = 2025 - DCF77tm::TM_YEAR_BASE;
     *   tm.tm_isdst = 0;
     *
     *   Serial.println(tm);
     */
    size_t printTo(print_t& p) const override;
  };


#endif /* DCF77QINT_INTERNAL_DCF77TM_HPP_ */
