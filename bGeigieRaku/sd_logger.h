/*
   Simple library to do logging to an SD card. Lots of checks built-in.
   Robust and simple to use.

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

#ifndef __SD_LOGGER_H__
#define __SD_LOGGER_H__

/* macros to turn on and off SD card */
extern int sd_log_pwr;  // pwr pin
#define sd_log_pwr_on()   digitalWrite(sd_log_pwr, LOW);  \
                          delay(20)
// To really turn the SD card off the
// it seems necessary to pull low all the
// SPI lines, wait a little, then CS high.
// This might be because the card can be
// turned on from any pin
#define sd_log_pwr_off()  digitalWrite(sd_log_pwr, HIGH); \
                          digitalWrite(MISO, LOW);        \
                          digitalWrite(MOSI, LOW);        \
                          digitalWrite(SCK, LOW);         \
                          {                               \
                            uint8_t r = SREG;             \
                            cli();                        \
                            digitalWrite(cs_sd, LOW);     \
                            delay(10);                    \
                            digitalWrite(cs_sd, HIGH);    \
                            SREG = r;                     \
                          }                       

// check if SD card is inserted
extern int sd_log_detect;
#define sd_log_card_missing() (digitalRead(sd_log_detect))

// keep track of status
extern int sd_log_initialized;
extern int sd_log_inserted;
extern int sd_log_last_write;
extern int sd_log_file_open;

/* initialize SD card */
int sd_log_init(int pin_pwr, int pin_detect, int pin_cs);

/* Run SD card diagnostic */
void sd_log_card_diagnostic();

/* write a log line to specified file */
int sd_log_writeln(char *filename, char *log_line);

#endif /* __SD_LOGGER_H__ */

