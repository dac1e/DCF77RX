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

// Set the following macro to true if you want to watch when the clock
// is updated from a received Dcf77 frame.
#define PRINT_DCF77FRAME_EVENT true

static constexpr int DCF77_PIN = 2;

// Create alarm, if there are no frames received for longer than this time.
// Unit is minutes.
static constexpr unsigned DCF77_FRAME_MISSING_ALARM_TIMEOUT = 3;
static constexpr int LED_OUT_OF_SYNCH = LED_BUILTIN;
static constexpr unsigned MSEC_PER_MINUTE = 60000;

/**
 * The clock needs an initial Dcf77 frame to start. Seconds
 * since the last received Dcf77 are calculated via systick
 * from function millis() and used to keep the clock running.
 * The clock needs a Dcf77 frame update at least every
 * 2**32 milliseconds, which is approximately every 49 days.
 * Otherwise there will be a systick overrun and the clock
 * will provide wrong results.
 */
class Dcf77clock : public Dcf77Receiver<DCF77_PIN> {
  using baseClass = Dcf77Receiver<DCF77_PIN>;

public:
  Dcf77clock()
    : mLastDcf77Frame(0), mState(INVALID), mSystickAtLastFrame(0), mAlarm(OUT_OF_SYNCH) {
  }

  void begin() {
    pinMode(LED_OUT_OF_SYNCH, OUTPUT);
    digitalWrite(LED_OUT_OF_SYNCH, HIGH);
    baseClass::begin();
  }

  /**
   * Read the current time.
   *
   * @param[out] tm The actual time.
   * @param[out] millisec The number of expired milliseconds
   *  within the current second.
   *
   * @return false, as long as no Dcf77 frame was received.
   */
  bool getTime(Dcf77tm& tm, unsigned* millisec) {
    if(mState != INVALID) {
      // Disable interrupts to avoid race condition with onDcf77FrameReceived()
      // which is updating mSystickAtLastFrame and mLastDcf77Frame.
      noInterrupts();
      const uint32_t millisSinceLastFrame = millis() - mSystickAtLastFrame;
      const uint64_t dcf77frame = mLastDcf77Frame;
      interrupts();

      const uint32_t secSinceLastFrame = millisSinceLastFrame / 1000;
      dcf77frame2time(tm, dcf77frame);
      const Dcf77time_t timestamp = tm.toTimeStamp();

      tm.set(timestamp + secSinceLastFrame, tm.tm_isdst);
      if(millisec != nullptr) {
        *millisec = millisSinceLastFrame % 1000;
      }
      return true;
    }
    return false;
  }

  bool checkAlarm() {
#if PRINT_DCF77FRAME_EVENT
    if(mState == VALID) {
      noInterrupts();
      const uint64_t dcf77frame = mLastDcf77Frame;
      interrupts();
      Dcf77tm tm;
      dcf77frame2time(tm, dcf77frame);
      Serial.print("Dcf77 frame received: ");
      Serial.println(tm);
      mState = VALID_AND_REPORTED;
    }
#endif

    if(mAlarm != OUT_OF_SYNCH) {
      const uint32_t millisSinceLastFrame =  millis() - mSystickAtLastFrame;
      if(static_cast<uint32_t>(DCF77_FRAME_MISSING_ALARM_TIMEOUT) * MSEC_PER_MINUTE
          <= millisSinceLastFrame) {
        Serial.println("Alarm: Dcf77 connection lost.");
        digitalWrite(LED_OUT_OF_SYNCH, HIGH);
        mAlarm = OUT_OF_SYNCH;
      }
    } else {
      if(mAlarm == SYNCH_RECOVERED) {
        Serial.println("Alarm: Dcf77 connection recovered.");
        digitalWrite(LED_OUT_OF_SYNCH, LOW);
        mAlarm = IN_SYNC;
      }
    }
    return mAlarm;
  }

private:
  /**
   * This function runs within the interrupt context and must
   * be executed quickly in order not to prevent other lower
   * priority interrupts to be serviced.
   */
  void onDcf77FrameReceived(const uint64_t dcf77frame, const uint32_t systick) override {
    mSystickAtLastFrame = millis();
    mLastDcf77Frame = dcf77frame;
    mState = VALID;
    mAlarm = SYNCH_RECOVERED;
  }

  uint32_t mSystickAtLastFrame;
  uint64_t mLastDcf77Frame;

#if PRINT_DCF77FRAME_EVENT
  enum STATE : int8_t {INVALID, VALID, VALID_AND_REPORTED};
#else
  enum STATE : int8_t {INVALID, VALID};
#endif
  STATE mState;

  enum ALARM : int8_t {OUT_OF_SYNCH, SYNCH_RECOVERED, IN_SYNC};
  ALARM mAlarm;
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
  // Frequently check for alarms.
  dcf77Clock.checkAlarm();

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
