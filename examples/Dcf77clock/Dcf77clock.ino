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

#define PRINT_DCF77FRAME_EVENT true

// Increase FIFO_SIZE if overflows happen. Overflow may happen, when
// processReceivedBits() isn't called frequently enough in loop().
#define DETECT_FIFO_OVERFLOW false
static constexpr size_t FIFO_SIZE = 6;
static constexpr int DCF77_PIN = 2;

/**
 * The clock needs an initial Dcf77 frame to start. Seconds
 * since the last received Dcf77 are calculated via systick
 * from function millis() and used to keep the clock running.
 * The clock need a Dcf77 frame update at least every
 * 2**32 milliseconds, which is approximately every 49 days.
 * Otherwise there will be a systick overrun and the clock
 * will provide wrong results.
 */
class Dcf77clock : public Dcf77Receiver<DCF77_PIN, FIFO_SIZE> {
  Dcf77time_t mLastDcf77timestamp;
  uint32_t mSystickAtLastFrame;
  int mIsdst;

public:
  Dcf77clock() : mLastDcf77timestamp(0), mIsdst(-1), mSystickAtLastFrame(0) {}

  /**
   * Read the current time.
   *
   * @param[out] tm. The actual time.
   * @param[out] millisec The number of expired milliseconds
   *  within the current second.
   * @return false, as long as no Dcf77 frame was received.
   */
  bool getTime(Dcf77tm& tm, unsigned* millisec) {
    if(mIsdst >= 0) {
      // Disable interrupts to avoid race condition.
      const uint32_t millisSinceLastFrame = millis() - mSystickAtLastFrame;
      const uint32_t secSinceLastFrame = millisSinceLastFrame / 1000;
      tm.set(mLastDcf77timestamp + secSinceLastFrame, mIsdst);
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
    Dcf77tm tm;
    dcf77frame2time(tm, dcf77frame);
    mLastDcf77timestamp = tm.toTimeStamp();
    mIsdst = tm.tm_isdst;
#if PRINT_DCF77FRAME_EVENT
    Serial.print("Dcf77 frame received: ");
    Serial.println(tm);
#endif
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

static constexpr size_t PRINTOUT_PERIOD = 1;
static uint32_t counter = 0;
static uint32_t lastSystick = 0;

//The setup function is called once at startup of the sketch
void setup()
{
  Serial.begin(9600);
  Serial.println("---------- Dcf77clock -----------");
  Serial.println("First frame may take some minutes");
  dcf77Clock.begin();

  lastSystick = millis() - PRINTOUT_PERIOD * 1000;
}


// The loop function is called in an endless loop
void loop()
{
  // Frequently process received bits.
  dcf77Clock.processReceivedBits();

  const uint32_t systick = millis();
  if(systick - lastSystick >= PRINTOUT_PERIOD * 1000) {
    Dcf77tm tm;
    if(dcf77Clock.getTime(tm, nullptr) ) {
      Serial.print(tm);
      Serial.print(", isdst=");
      Serial.println(tm.tm_isdst);
    } else {
      Serial.print('[');
      Serial.print(counter);
      Serial.print("s]");
      Serial.print(" Waiting for completion of Dcf77 frame on Arduino pin ");
      Serial.println(DCF77_PIN);
      counter += PRINTOUT_PERIOD;
    }
    lastSystick = systick;
  }
}
