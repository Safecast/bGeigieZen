/*
   Simple library for Arduino implementing a counter using the interrupt pin
}}}u   for a Geigier counter for example

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

#include "InterruptCounter.h"
#include <limits.h>

hw_timer_t * timer = NULL;
volatile SemaphoreHandle_t timerSemaphore;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

unsigned long _running_count;
unsigned long _finished_count;
int _new_count_available = false;

// Private Methods
void timeIsUp(){
  // timer interrupt routine
  portENTER_CRITICAL_ISR(&timerMux);
  _finished_count = _running_count;
  _running_count = 0;
  portEXIT_CRITICAL_ISR(&timerMux);

  // Give a semaphore that we can check in the loop
  xSemaphoreGiveFromISR(timerSemaphore, NULL);
}

void interruptRoutine()
{
  // Geiger event interrupt routine
  portENTER_CRITICAL_ISR(&timerMux);
  _running_count++;
  portEXIT_CRITICAL_ISR(&timerMux);
}

// Public methods

// Constructor
void interruptCounterSetup(int interrupt_pin, unsigned int period_us)
{
  // Setup the counter pin
  pinMode(interrupt_pin, INPUT);
  attachInterrupt(interrupt_pin, interruptRoutine, RISING);

  // Create semaphore to inform us when the timer has fired
  timerSemaphore = xSemaphoreCreateBinary();

  _running_count = 0;
  _finished_count = 0;

  // Timer configuration stuff

  // Use 1st timer of 4 (counted from zero).
  // Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more
  // info).
  timer = timerBegin(0, 80, true);

  // Attach onTimer function to our timer.
  timerAttachInterrupt(timer, &timeIsUp, true);

  // Set alarm to call onTimer function every second (value in microseconds).
  // Repeat the alarm (third parameter)
  timerAlarmWrite(timer, period, true);

  // Start an alarm
  timerAlarmEnable(timer);
}

// call this to start the counter
void interruptCounterStop()
{
  timerEnd(timer);
  // set count to zero (optional)
  _running_count = 0;
  _finished_count = 0;
}

// This indicates when the count over the determined period is over
int interruptCounterAvailable()
{
  return xSemaphoreTake(timerSemaphore, 0) == pdTRUE;
}

// return current number of counts
unsigned long interruptCounterCount()
{
  // reset the count available flag
  return _finished_count;
}
