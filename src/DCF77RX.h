/*
  DCF77RX - Arduino libary receiving and decoding DCFf77 frames Copyright (c)
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

#ifndef DCF77RX_HPP_
#define DCF77RX_HPP_

#include <stdint.h>
#include "internal/ISR_ATTR.h"
#include "internal/DCF77Base.h"
#include <Arduino.h>

/**
 * Dcf77Receiver is the main API class. It receives dcf77 pulses on a digital pin.
 * The pin where the receiver is connected to, is given by parameter RECEIVER_PIN.
 *
 * Usage:
 *
 * static constexpr int DCF77_PIN = 3;
 *
 * class MyDcf77Receiver : public Dcf77Receiver<DCF77_PIN> {
 *   // This function will be called whenever a valid dcf77 frame
 *   // has been received. The actual system tick time stamp in
 *   // unit of milliseconds is passed in addition.
 *   void onDCF77FrameReceived(const uint64_t dcf77frame, const uint32_t systick) override {
 *     // convert dcf77frame to time structure.
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
 * }
 *
 */
template<int RECEIVER_PIN> class DCF77RX : public DCF77Base {
public:
  DCF77RX() {
	  // Make this object responsible for receiving
	  // Dcf77 signals from the pin RECEIVER_PIN.
		mInstance = this;
	}

	/**
	 * Start receiving dcf77 frames. To be called once during
	 * setup().
	 */
	void begin() {
		DCF77Base::begin(RECEIVER_PIN, intHandler);
	}

private:
	/* The instance that is responsible for pin RECEIVE_PIN. */
	static DCF77Base* mInstance;

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

DCF77Base *DCF77RX<RECEIVER_PIN>::mInstance = nullptr;

#endif /* DCF77RX_HPP_ */
