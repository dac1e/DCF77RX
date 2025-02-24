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

#ifndef DCF77_INTERNAL_UTIL_FIFO_HPP_
#define DCF77_INTERNAL_UTIL_FIFO_HPP_

#include <stddef.h>
#include "internal/ISR_ATTR.h"

namespace Dcf77util {

template<typename T, size_t SIZE> class Fifo {

  static constexpr size_t ENTRIES = SIZE + 1;
	typedef T element_type;
	element_type mArray[ENTRIES];
	volatile size_t writeIndex = 0;
	volatile size_t readIndex = 0;

	TEXT_ISR_ATTR_3_INLINE
	size_t indexDifference() {
		return (writeIndex - readIndex) % ENTRIES;
	}

public:
	/**
	 * Push a value to the Fifo. Value is dropped,  when
	 * the fifo is full.
	 *
	 * @param[in] value The value to be pushed to the Fifo.
	 *
	 * @return the number of free entries in the Fifo BEFORE
	 *  the element was pushed.
	 */
	TEXT_ISR_ATTR_3_INLINE
	size_t push(const element_type &value) {
	  const size_t freeEntries = SIZE - indexDifference() ;
	  if (freeEntries > 0) {
			writeIndex = (writeIndex + 1) % ENTRIES;
			mArray[writeIndex] = value;
		}
		return freeEntries;
	}

	/**
	 * Pop an element from the Fifo.
	 * @param[out] value The value of the element that was
	 *  popped from the Fifo.
	 *
	 * @return number of elements in the Fifo BEFORE the
	 *  element was popped.
	 *  .
	 */
	inline size_t pop(element_type &value) {
		const size_t returnValue = indexDifference();
		if (returnValue) {
			const size_t i = readIndex;
			readIndex = (readIndex + 1) % ENTRIES;
			value = mArray[i];
		}
		return returnValue;
	}
};

} // namespace Dcf77util

#endif /* DCF77_INTERNAL_UTIL_FIFO_HPP_ */
