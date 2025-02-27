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

#include "Dcf77base.h"
#include <Arduino.h>

namespace Dcf77util {

/**
 * Number of milliseconds to elapse before we assume a "1",
 * if we receive a falling edge before - its a 0.
 */
constexpr int DCF_SPLIT_MILLIS = 170;
/**
 * There is no signal in second 59 - detect the beginning of
 * a new minute.
 */
constexpr int DCF_SYNC_MILLIS = 1200;


constexpr int DCF_SIGNAL_STATE_LOW  = 0;
constexpr int DCF_SIGNAL_STATE_HIGH = !DCF_SIGNAL_STATE_LOW;

/**
 * DCF time format struct
 */
struct DCF77bits {
  uint64_t prefix:15;
  uint64_t R			:1;
  uint64_t A1			:1;
  uint64_t Z1			:1; // Set to 1 when CEST is in effect
  uint64_t Z2			:1; // Set to 1 when CET  is in effect
  uint64_t A2			:1;
  uint64_t S			:1;
  uint64_t Min		:7;	// minutes
  uint64_t P1			:1;	// parity minutes
  uint64_t Hour		:6;	// hours
  uint64_t P2			:1;	// parity hours
  uint64_t Day		:6;	// day
  uint64_t Weekday:3;	// day of week
  uint64_t Month	:5;	// month
  uint64_t Year		:8;	// year (last 2 digits)
  uint64_t P3			:1;	// parity
};

struct {
	unsigned char parity_flag	:1;
	unsigned char parity_min	:1;
	unsigned char parity_hour	:1;
	unsigned char parity_date	:1;
} flags;

/**
 * Interrupthandler for signal pin
 */
void Dcf77Base::onPinInterrupt(int pin) {
	// check the value again - since it takes some time to activate
	// the interrupt routine, we get a clear signal.
	Dcf77pulse dcf77signal;
	dcf77signal.mPulseLevel = digitalRead(pin);
	dcf77signal.mPulseLength = millis();
	pushPulse(dcf77signal);
}

void Dcf77Base::dcf77frame2time(Dcf77tm &time, const uint64_t& dcf77frame) {
	const DCF77bits& bits = reinterpret_cast<const DCF77bits&>(dcf77frame);
	time.tm_sec = 0;
	time.tm_min = bits.Min - ((bits.Min / 16) * 6);
	time.tm_hour = bits.Hour - ((bits.Hour / 16) * 6);
	time.tm_wday = (bits.Weekday - ((bits.Weekday / 16) * 6)) % 7;
	time.tm_mday = bits.Day - ((bits.Day / 16) * 6);
	time.tm_mon = bits.Month - ((bits.Month / 16) * 6) - 1;
	time.tm_yday = -1; // unknown
	time.tm_year = 100 + bits.Year - ((bits.Year / 16) * 6);
	time.tm_isdst = bits.Z1;
}

bool Dcf77Base::concludeReceivedBits(uint64_t& dcf77frame) {
  bool successfullUpdate = mRxCurrentBitBufferPosition == 59;
  dcf77frame = mRxBitBuffer;

  // reset buffer
  mRxCurrentBitBufferPosition = 0;
  mRxBitBuffer = 0;

	if (successfullUpdate) {
		successfullUpdate = flags.parity_min == reinterpret_cast<struct DCF77bits&>(dcf77frame).P1
				&& flags.parity_hour == reinterpret_cast<struct DCF77bits&>(dcf77frame).P2
				&& flags.parity_date == reinterpret_cast<struct DCF77bits&>(dcf77frame).P3;
	}

	return successfullUpdate;
}

inline void Dcf77Base::appendReceivedBit(const unsigned signalBit) {
	if (mRxCurrentBitBufferPosition < 59) {
		mRxBitBuffer = mRxBitBuffer | static_cast<uint64_t>(signalBit) << mRxCurrentBitBufferPosition;

		// Update the parity bits. First: Reset when minute, hour or date starts.
		if (mRxCurrentBitBufferPosition == 21 || mRxCurrentBitBufferPosition == 29 || mRxCurrentBitBufferPosition == 36) {
			flags.parity_flag = 0;
		}

		// Save the parity when the corresponding segment ends
		if (mRxCurrentBitBufferPosition == 28) {
			flags.parity_min = flags.parity_flag;
		};

		if (mRxCurrentBitBufferPosition == 35) {
			flags.parity_hour = flags.parity_flag;
		};

		if (mRxCurrentBitBufferPosition == 58) {
			flags.parity_date = flags.parity_flag;
		};

		// When we received a 1, toggle the parity flag
		if (signalBit == 1) {
			flags.parity_flag = flags.parity_flag ^ 1;
		}

		mRxCurrentBitBufferPosition++;
	}
}

void Dcf77Base::processReceivedBits() {
	Dcf77pulse dcf77signal;
	while (popPulse(dcf77signal)) {
		if (dcf77signal.mPulseLevel == DCF_SIGNAL_STATE_LOW) {
			if (mPreviousPulse.mPulseLevel != DCF_SIGNAL_STATE_LOW) {
				/* falling edge */
				if ((dcf77signal.mPulseLength - mPreviousPulse.mPulseLength) > DCF_SYNC_MILLIS) {
					uint64_t dcf77frame;
					if(concludeReceivedBits(dcf77frame)) {
						onDcf77FrameReceived(dcf77frame);
					}
				}
				mPreviousPulse = dcf77signal;
			}
		} else {
			if (mPreviousPulse.mPulseLevel != DCF_SIGNAL_STATE_HIGH) {
				/* rising edge */
				const uint32_t difference = dcf77signal.mPulseLength - mPreviousPulse.mPulseLength;
				const unsigned bit = difference < DCF_SPLIT_MILLIS ? 0 : 1;
				appendReceivedBit(bit);
				mPreviousPulse.mPulseLevel = dcf77signal.mPulseLevel;
			}
		}
	}
}

void Dcf77Base::begin(int pin, void (*intHandler)()) {
	pinMode(pin, INPUT_PULLUP);
	mPreviousPulse.mPulseLevel = digitalRead(pin);
	attachInterrupt(digitalPinToInterrupt(pin), intHandler, CHANGE);
}

} // namespace Dcf77util

