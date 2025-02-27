/*
  DCF77Receiver - Arduino libary receiving and decoding DCFf77 frames Copyright (c)
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
 * Receive frames from dcf77, convert them to DCF77tm time structure and
 * print them on Serial.
 */

#include "DCF77Receiver.h"

static constexpr int DCF77_PIN = 2;

static constexpr size_t PRINTOUT_PERIOD = 1;
static int32_t counter = 0;
static uint32_t lastSystick = 0;

class MyDCF77Receiver : public DCF77Receiver<DCF77_PIN> {
  uint64_t mDcf77frame = 0;

  /**
   * This function runs within the interrupt context and must be executed
   * quickly in order not to prevent other lower priority interrupts to
   * be serviced.
   */
  void onDCF77FrameReceived(const uint64_t dcf77frame, const uint32_t systick) override {
    counter = -1;
    mDcf77frame = dcf77frame;
  }

public:
  uint64_t getDCF77frame() const {
    noInterrupts();
    const uint64_t result = mDcf77frame;
    interrupts();
    return result;
  }
};

MyDCF77Receiver myReceiver;

//The setup function is called once at startup of the sketch
void setup()
{
  Serial.begin(9600);
  Serial.println("-------- PrintDCF77Time ---------");
  Serial.println("First frame may take some minutes");
  myReceiver.begin();
  lastSystick = millis() - PRINTOUT_PERIOD * 1000;
}

// The loop function is called in an endless loop
void loop()
{
  const uint32_t systick = millis();
  if(systick - lastSystick >= PRINTOUT_PERIOD * 1000) {
    if(counter < 0) {
      // Frame received.
      const uint64_t dcf77frame = myReceiver.getDCF77frame();
      if( dcf77frame ) {
        // convert frame to time structure.
        DCF77tm time;
        myReceiver.dcf77frame2time(time, dcf77frame);
        Serial.print("DCF77 frame received: ");
        Serial.println(time);
      }
    } else {
      // Waiting for next frame;
      Serial.print('[');
      Serial.print(counter);
      Serial.print("s]");
      Serial.print(" Waiting for completion of DCF77 frame on Arduino pin ");
      Serial.println(DCF77_PIN);
    }

    counter += PRINTOUT_PERIOD;
    lastSystick = systick;
  }
}
