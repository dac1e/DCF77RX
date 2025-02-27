/*
  DCF77Receiver- Arduino libary receiving and decoding Dcf77 frames Copyright (c)
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

#ifndef DCF77_INTERNAL_DCF77_BASE_HPP_
#define DCF77_INTERNAL_DCF77_BASE_HPP_

#include <stdint.h>
#include "DCF77tm.h"
#include "ISR_ATTR.h"

/**
 * This base class does the main work to receive and
 * decode Dcf77 frames. The derived template class
 * Dcf77Receiver provides only the PIN to be used.
 */
class DCF77Base {
public:
  /**
   * To be called by the interrupt handler.
   *
   * @param[in] the pin for which the interrupt was triggered.
   */
  TEXT_ISR_ATTR_1
  void onPinInterrupt(int pin);

  /**
   * Convert a dcf77 frame to a time structure. Type DCF77tm
   * is of type to std::tm in case the platform supports it.
   *
   * @param[out] time The dcf77 bits as time structure.
   * @param[in] dcf77frame The dcf77 frame.
   */
	static void dcf77frame2time(DCF77tm &time, const uint64_t& dcf77frame);

protected:
	struct DCF77pulse {uint32_t mPulseTime = 0; int mPulseLevel = 1;};

	/**
	 * Establish interrupt handler for pin.
	 */
	void begin(int pin, void (*intHandler)());

	TEXT_ISR_ATTR_2_INLINE
	void processPulse(const DCF77pulse &dcf77signal);

	/**
	 * Append a received bit to the rx buffer.
	 */
	TEXT_ISR_ATTR_3_INLINE
	void appendReceivedBit(const unsigned signalBit);

	/**
	 * Obtain a valid dcf77 frame.
	 * Check whether the receive buffer contains is a completed
	 * valid frame, and reset the receive buffer.
	 *
	 * @param[out] dcf77frame. The received dcf77 frame, if
	 *  the receive buffer contained a valid one.
	 *
	 * @ return true, if the receive buffer contained a valid
	 *  frame. Otherwise false.
	 */
	TEXT_ISR_ATTR_3_INLINE
	bool concludeReceivedBits(uint64_t& dcf77frame);

	/**
	 * Callback function to be overridden by the base class to
	 * obtain a received dcf77 frame. Note that this function
	 * runs within the interrupt context and must be executed
	 * quickly in order not to prevent other lower priority
	 * interrupts to be serviced.
	 */
	TEXT_ISR_ATTR_4
	virtual void onDCF77FrameReceived(const uint64_t dcf77frame,
	    const uint32_t systick) = 0;

private:
  uint64_t mRxBitBuffer = 0;
  size_t mRxBitBufPos = 0;
  DCF77pulse mPreviousPulse;
};

#endif /* DCF77_INTERNAL_DCF77_BASE_HPP_ */
