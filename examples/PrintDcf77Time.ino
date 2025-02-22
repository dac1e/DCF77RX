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

#include "Dcf77Receiver.h"

constexpr unsigned int DCF77_PIN = 23;

class MyDcf77Receiver : public Dcf77Receiver<DCF77_PIN> {
  virtual void onDcf77BitsReceived(const uint64_t dcf77bits) override {
    // convert bit to time structure.
    Dcf77tm time;
    dcf77bits2tm(time, dcf77bits);
    Serial.println(time);
    return;
  }
};

MyDcf77Receiver myReceiver;

//The setup function is called once at startup of the sketch
void setup()
{
  // Add your initialization code here
  Serial.begin(9600);
  myReceiver.begin();
}

// The loop function is called in an endless loop
void loop()
{
  //Add your repeated code here
  myReceiver.processReceivedBits();
}
