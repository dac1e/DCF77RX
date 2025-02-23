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

class Dcf77clock : public Dcf77Receiver<DCF77_PIN, FIFO_SIZE> {
  uint64_t mLastDcf77Frame;
  uint32_t mSystickAtLastFrame;

public:
  Dcf77clock() : mLastDcf77Frame(0), mSystickAtLastFrame(0) {}

  bool getTime(Dcf77tm& tm, unsigned* millisec) {
    if(mLastDcf77Frame) {
      // Disable interrupts to avoid race condition.
      noInterrupts();
      const uint32_t millisSinceLastFrame = millis() - mSystickAtLastFrame;
      const uint64_t dcf77frame = mLastDcf77Frame;
      interrupts();

      dcf77frame2time(tm, dcf77frame);
      const uint32_t secSinceLastFrame = millisSinceLastFrame / 1000;
      tm += secSinceLastFrame;

      if(millisec != nullptr) {
        *millisec = millisSinceLastFrame % 1000;
      }

      return true;
    }
    return false;
  }
private:
  void onDcf77FrameReceived(const uint64_t dcf77frame) override {
    mSystickAtLastFrame = millis();
    mLastDcf77Frame = dcf77frame;
  }

#if DETECT_FIFO_OVERFLOW
  using baseClass = Dcf77Receiver<DCF77_PIN, FIFO_SIZE>;
  bool pushPulse(const Dcf77pulse &pulse) override {
   Serial.println()
   const bool ok = baseClass::pushPulse(pulse);
   if(not ok) {
     Serial.print("overflow");
   }
   return ok;
  }
#endif
};

Dcf77clock dcf77Clock;

//The setup function is called once at startup of the sketch
void setup()
{
  Serial.begin(9600);
  delay(300);
  Serial.println("---------- Dcf77clock -----------");
  Serial.println("First frame may take some minutes");
  dcf77Clock.begin();
}


// The loop function is called in an endless loop
void loop()
{
  static uint32_t lastSystick = 0;
  static uint32_t counter = 0;

  // Frequently process received bits.
  dcf77Clock.processReceivedBits();

  const uint32_t systick = millis();
  if(systick - lastSystick >= 1000) {
    Dcf77tm tm;
    if(dcf77Clock.getTime(tm, nullptr) ) {
      Serial.print(tm);
      Serial.print(", isdst=");
      Serial.println(tm.tm_isdst);
    } else {
      Serial.print('[');
      Serial.print(counter++);
      Serial.print(']');
      Serial.println(" Waiting for dcf77 frame.");
    }
    lastSystick = systick;
  }
}
