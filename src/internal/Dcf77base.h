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

#ifndef DCF77_INTERNAL_DCF77_BASE_HPP_
#define DCF77_INTERNAL_DCF77_BASE_HPP_

#include <stdint.h>
#include "Dcf77Fifo.h"
#include "Dcf77tm.h"


namespace Dcf77util {

/**
 * This base class does the main work to receive and
 * decode Dcf77 frames. The derived template class
 * Dcf77Receiver provides only the PIN to be used and
 * the Fifo size as compile time template parameters.
 */
class Dcf77Base {

  uint32_t mPreviousFallingEdgeTime = 0;
  int mPreviousDcfSignalState = 1;
  size_t mRxCurrentBitBufferPosition = 0;
  uint64_t mRxBitBuffer = 0;

protected:
	struct Dcf77pulse {uint32_t mLength = 0; int mLevel = 1;};

	/**
	 * Convert a received dcf77 frame to a tm structure.
	 *
	 * @param[out] time The tm structure that will receive the
	 *  result.
	 * @param[in]  dcf77frame The frame to be converted.
	 */
	static void dcf77frame2time(Dcf77tm &time, const uint64_t& dcf77frame);

	/**
	 * Establish interrupt handler for pin.
	 */
	void begin(int pin, void (*intHandler)());

  /**
   * Refer to description of Dcf77Receiver::processReceivedBits().
   */
	void processReceivedBits();

	/**
	 * Callback function to be overridden by the base class to
	 * obtain a received dcf77 frame.
	 */
	virtual void onDcf77FrameReceived(const uint64_t dcf77frame) = 0;

	/**
	 * Append a received bit to the rx buffer.
	 */
	void appendReceivedBit(const unsigned int signalBit);

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
	bool concludeReceivedBits(uint64_t& dcf77frame);

	/**
	 * To be overridden by the derived class, to store a pulse
	 * in the Fifo. Called by onPinInterrupt().
	 */
	virtual size_t pushPulse(const Dcf77pulse &pulse) = 0;

	/**
	 * To be overridden by the derived class, to pop a pulse
	 * from the Fifo. Called by processReceivedBits().
	 */
	virtual size_t popPulse(Dcf77pulse &pulse) = 0;

public:

  /**
   * To be called by the interrupt handler.
   *
   * @param[in] the pin for which the interrupt was triggered.
   */
	void onPinInterrupt(int pin);
};

} // namespace Dcf77util

#endif /* DCF77_INTERNAL_DCF77_BASE_HPP_ */
