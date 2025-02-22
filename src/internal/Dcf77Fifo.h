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

namespace Dcf77util {

template<typename T, size_t SIZE> class Fifo {
	typedef T element_type;
	element_type mArray[SIZE];
	volatile size_t writeIndex = 0;
	volatile size_t readIndex = 0;

	inline size_t indexDifference() {
		return (writeIndex - readIndex) % SIZE;
	}

public:
	/**
	 * Push a value to the fifo. Value is dropped,  when
	 * the fifo is full.
	 *
	 * @param[in] value The value to be pushed to the fifo.
	 *
	 * @return true, if successful. false if the value
	 *   couldn't be pushed due to an overflow.
	 */
	bool push(const element_type &value) {
		if (indexDifference() < (SIZE - 1)) {
			writeIndex = (writeIndex + 1) % SIZE;
			mArray[writeIndex] = value;
			return true;
		}
		return false;
	}

	/**
	 * Pop a value from the fifo.
	 * @param[out] value The value that was popped from
	 * 	the fifo.
	 *
	 * @return false when fifo is empty. Otherwise true.
	 */
	bool pop(element_type &value) {
		const bool returnValue = indexDifference() > 0;
		if (returnValue) {
			const size_t i = readIndex;
			readIndex = (readIndex + 1) % SIZE;
			value = mArray[i];
		}
		return returnValue;
	}
};

} // namespace Dcf77util

#endif /* DCF77_INTERNAL_UTIL_FIFO_HPP_ */
