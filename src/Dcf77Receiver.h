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

#ifndef DCF77_RECEIVER_HPP_
#define DCF77_RECEIVER_HPP_

#include <stdint.h>
#include "internal/ISR_ATTR.h"
#include "internal/Dcf77base.h"
#include <Arduino.h>

/**
 * Dcf77Receiver is the main API class. It receives dcf77 pulses on a digital pin.
 * The pin where the receiver is connected is given by parameter RECEIVER_PIN.
 *
 * Usage:
 *
 * static constexpr size_t FIFO_SIZE = 6; // FIFO_SIZE is an optional parameter.
 * static constexpr int DCF77_PIN = 3;
 *
 * class MyDcf77Receiver : public Dcf77Receiver<DCF77_PIN, FIFO_SIZE> {
 *   void onDcf77FrameReceived(const uint64_t dcf77frame) override {
 *     // convert bit to time structure.
 *     Dcf77tm time;
 *     dcf77frame2time(time, dcf77frame);
 *     ...
 *     return;
 *   }
 * };
 *
 * MyDcf77Receiver myReceiver;
 *
 * void setup() {
 *   ...
 *   myReceiver.begin();
 *   ...
 * }
 *
 * void loop() {
 *   ...
 *   myReceiver.processReceivedBits();
 *   ...
 * }
 *
 * As shown in the above example, derive your own class from Dcf77Receiver
 * and overwrite function 'void onDcf77FrameReceived(const uint64_t dcf77frame)'.
 *
 * The overridden function will be called whenever a valid dcf77 frame has
 * been received. The frame will be passed as uin64_t integer.
 * Do whatever you want with it. E.g. store it somewhere or convert it to a
 * time structure and print it out. For the conversion to a time structure
 * a static function dcf77frame2time() is available.
 *
 * The time structure is of type std::tm in case the platform supports it.
 * Otherwise it is a proprietary structure with the same fields as std::tm.
 *
 * class MyDcf77Receiver : public Dcf77Receiver<DCF77_PIN, FIFO_SIZE> {
 *   ...
 *   using baseClass = Dcf77Receiver<DCF77_PIN, FIFO_SIZE>;
 *   bool pushPulse(const Dcf77pulse &pulse) override {
 *     const bool ok = baseClass::pushPulse(pulse);
 *     if(not ok) {
 *       Serial.print("overflow");
 *     }
 *     return ok;
 *   }
 *   ...
 * };
 */
template<int RECEIVER_PIN>
class Dcf77Receiver : public Dcf77util::Dcf77Base {
public:
	Dcf77Receiver() {
	  // Make this object responsible for receiving
	  // Dcf77 signals from the pin RECEIVER_PIN.
		mInstance = this;
	}

	/**
	 * Start receiving dcf77 frames. To be called once during
	 * setup().
	 */
	void begin() {
		Dcf77Base::begin(RECEIVER_PIN, intHandler);
	}

private:
	/* The instance that is responsible for pin RECEIVE_PIN. */
	static Dcf77Base* mInstance;

	/**
	 * The interrupt handler that is called upon a level change on
	 * the RECEIVER_PIN.
	 */
	TEXT_ISR_ATTR_0
	static void intHandler() {
		mInstance->onPinInterrupt(RECEIVER_PIN);
	}
};

template<int RECEIVER_PIN>

Dcf77util::Dcf77Base *Dcf77Receiver<RECEIVER_PIN>::mInstance = nullptr;

#endif /* DCF77_RECEIVER_HPP_ */
