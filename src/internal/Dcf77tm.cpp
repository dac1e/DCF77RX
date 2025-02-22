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
