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
#include <Arduino.h>
#include "internal/Dcf77base.h"

/**
 * Receive dcf77 pulses on a digital pin. The pin to be used is RECEIVER_PIN.
 *
 * Usage:
 *
 * constexpr int PIN = 3;
 *
 * class MyDcf77Receiver : public Dcf77Receiver<PIN> {
 *   virtual void onDcf77BitsReceived(const uint64_t dcf77bits) override {
 *     // convert bit to time structure.
 *     Dcf77tm time;
 *     dcf77bits2tm(time, dcf77bits);
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
 * and overwrite function 'void onDcf77BitsReceived(const uint64_t dcf77bits)'.
 *
 * The overridden function will be called whenever a valid dcf77 frame has
 * been received. The frame will be passed as uin64_t integer.
 * Do whatever you want with it. E.g. store it somewhere or convert it to a
 * time structure and print it out. For the conversion to a time structure
 * a static function dcf77bits2time() is available.
 *
 * The time structure is of type std::tm in case the platform supports it.
 * Otherwise it is a proprietary structure with the same fields as std::tm.
 */
template<int RECEIVER_PIN, size_t PULSE_FIFO_SIZE = 6>
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

	/**
	 * Convert a dcf77 frame to a time structure. Type Dcf77tm
	 * is of type to std::tm in case the platform supports it.
	 *
	 * @param[out] time The dcf77 bits as time structure.
	 * @param[in] dcf77bits The dcf77 frame.
	 */
  inline static void dcf77bits2tm(Dcf77tm &time, const uint64_t& dcf77bits) {
    Dcf77util::Dcf77Base::dcf77bits2tm(time, dcf77bits);
  }

	/**
	 * This function needs to be called frequently in the loop(). It
	 * will pick up the received pulses that the interrupt
	 * handler has stored in a fifo. If a complete dcf77 frame has
	 * been received, the function onDcf77BitsReceived() will be called.
	 */
  inline void processReceivedBits() {
    Dcf77util::Dcf77Base::processReceivedBits();
  }

private:
	typedef Dcf77util::Fifo<Dcf77pulse, PULSE_FIFO_SIZE> PulseFifo;
	PulseFifo mPulseFifo;

	/* The instance that is responsible for pin RECEIVE_PIN. */
	static Dcf77Base* mInstance;

	/**
	 * Push a received pulse to the fifo. That function
	 * will be called by the interrupt handler when a
	 * pulse has been received.
	 */
	void pushPulse(const Dcf77pulse &pulse) override {
		mPulseFifo.push(pulse);
	}

	/**
	 * Pop a received pulse from the fifo. That function will be called
	 * by the processReceivedBits() in order to evaluate pulses that
	 * have been received.
	 */
	bool popPulse(Dcf77pulse &pulse) override {
		noInterrupts();
		const bool result = mPulseFifo.pop(pulse);
		interrupts();
		return result;
	}

	/**
	 * The interrupt handler that is called upon a level change on
	 * the RECEIVER_PIN.
	 */
	static void intHandler() {
		mInstance->onPinInterrupt(RECEIVER_PIN);
	}
};

template<int RECEIVER_PIN, size_t PULSE_FIFO_SIZE>

Dcf77util::Dcf77Base *Dcf77Receiver<RECEIVER_PIN, PULSE_FIFO_SIZE>::mInstance = nullptr;

#endif /* DCF77_RECEIVER_HPP_ */
