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
#include "Arduino.h"
#include "Dcf77Fifo.h"
#include "Dcf77tm.h"


namespace Dcf77util {

class Dcf77Base {
	struct Dcf77pulse {uint32_t mLength = 0; int mLevel = 1;};

	uint32_t 				mPreviousFallingEdgeTime = 0;
	int							mPreviousDcfSignalState	= 1;
	size_t 					mRxCurrentBitBufferPosition = 0;
	uint64_t 				mRxBitBuffer = 0;
protected:
	static void dcf77bits2tm(Dcf77tm &time, const uint64_t& dcf77bits);
	void begin(int pin, void (*intHandler)());

	void processReceivedBits();
	virtual void onDcf77BitsReceived(const uint64_t dcf77bits) = 0;

	void appendReceivedBit(const unsigned int signalBit);
	bool concludeReceivedBits(uint64_t& dcf77bits);

	virtual bool popPulse(Dcf77pulse &pulse) = 0;
	virtual void pushPulse(const Dcf77pulse &pulse) = 0;
public:
	void onPinInterrupt(int pin);
};

} // namespace Dcf77util

#endif /* DCF77_INTERNAL_DCF77_BASE_HPP_ */
