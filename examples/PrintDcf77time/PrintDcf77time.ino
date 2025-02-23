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

/**
 * Receive frames from dcf77, convert them to Dcf77tm time structure and
 * print them on Serial.
 */

#include "Dcf77Receiver.h"

// Increase FIFO_SIZE if overflows happen. Overflow may happen, when
// processReceivedBits() isn't called frequently enough in loop().
#define DETECT_FIFO_OVERFLOW false
static constexpr size_t FIFO_SIZE = 6;
static constexpr int DCF77_PIN = 2;

static constexpr size_t PRINTOUT_PERIOD = 3;
static uint32_t counter = 0;
static uint32_t lastSystick = 0;

class MyDcf77Receiver : public Dcf77Receiver<DCF77_PIN, FIFO_SIZE> {
  void onDcf77FrameReceived(const uint64_t dcf77frame) override {
    counter = 0;

    // convert frame to time structure.
    Dcf77tm time;
    dcf77frame2time(time, dcf77frame);
    Serial.print("Dcf77 frame received: ");
    Serial.println(time);
  }

#if DETECT_FIFO_OVERFLOW
  using baseClass = Dcf77Receiver<DCF77_PIN, FIFO_SIZE>;
  bool pushPulse(const Dcf77pulse &pulse) override {
   const bool ok = baseClass::pushPulse(pulse);
   if(not ok) {
     Serial.print("overflow");
   }
   return ok;
  }
#endif
};

MyDcf77Receiver myReceiver;

//The setup function is called once at startup of the sketch
void setup()
{
  Serial.begin(9600);
  Serial.println("-------- PrintDcf77time ---------");
  Serial.println("First frame may take some minutes");
  myReceiver.begin();
  lastSystick = millis() - PRINTOUT_PERIOD * 1000;
}

// The loop function is called in an endless loop
void loop()
{
  // Frequently process received bits.
  myReceiver.processReceivedBits();

  const uint32_t systick = millis();
  if(systick - lastSystick >= PRINTOUT_PERIOD * 1000) {
    Serial.print('[');
    Serial.print(counter);
    Serial.print("s]");
    Serial.print(" Waiting for completion of Dcf77 frame on Arduino pin ");
    Serial.println(DCF77_PIN);
    counter += PRINTOUT_PERIOD;
    lastSystick = systick;
  }
}
