/*
   Simple library for Arduino implementing a counter using the interrupt pin
   for a Geigier counter for example

   Copyright (c) 2011, Robin Scheibler aka FakuFaku
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Application.h"
#include "InterruptCounter.h"
#include <limits.h>

unsigned long _running_count;
unsigned long _finished_count;
int _new_count_available = false;

// Private Methods

void time_is_up()
{
  ATOMIC_BLOCK()
  {
    // timer interrupt routine
    _finished_count = _running_count;
    _running_count = 0;
  }
  _new_count_available = true;
}

void interrupt_routine()
{
  // Geiger event interrupt routine
  _running_count++;
}

// Declare variables here
Timer timer = Timer(1000, time_is_up);

// Public methods

// Constructor
void interruptCounterSetup(int interrupt_pin, unsigned int period)
{
  Serial.println("Setting things up.");
  pinMode(interrupt_pin, INPUT);
  attachInterrupt(interrupt_pin, interrupt_routine, RISING);

  _running_count = 0;
  _finished_count = 0;
  _new_count_available = false;

  timer.changePeriod(period);
  timer.start();

}

// call this to start the counter
void interruptCounterReset()
{
  timer.reset();
  // set count to zero (optional)
  _running_count = 0;
  _finished_count = 0;
  _new_count_available = false;
}

// This indicates when the count over the determined period is over
int interruptCounterAvailable()
{
  return _new_count_available;
}

// return current number of counts
unsigned long interruptCounterCount()
{
  // reset the count available flag
  _new_count_available = false;
  return _finished_count;
}
