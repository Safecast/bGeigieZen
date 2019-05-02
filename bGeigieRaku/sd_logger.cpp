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

// Link to arduino library
#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <SD.h>
#include <avr/io.h>
#include "sd_logger.h"
//#include "sd_reader_int.h"

#define LINE_MAX_SIZE 100

// Various messages that we store in flash
#define BG_ERR_MSG_SIZE 40

/* sd card power */
int sd_log_pwr;

/* sd card detect */
int sd_log_detect;
int sd_log_initialized;
int sd_log_inserted;
int sd_log_file_open;
int sd_log_last_write;

/* sd chip select pin */
static int sd_log_cs;

/* the FILE */
File dataFile;

// Initialize SD card
int sd_log_init(int pin_pwr, int pin_detect, int pin_cs)
{
  // initialize pins
  sd_log_pwr = pin_pwr;
  pinMode(sd_log_pwr, OUTPUT);
  //sd_log_pwr_off();

  sd_log_detect = pin_detect;
  pinMode(sd_log_detect, INPUT);
  digitalWrite(sd_log_detect, HIGH);  // activate pull-up

  sd_log_cs = pin_cs;
  pinMode(sd_log_cs, OUTPUT);
  digitalWrite(sd_log_cs, HIGH);

  // initialize status variables
  sd_log_initialized = 0;
  sd_log_inserted = 0;
  sd_log_last_write = 1; // because we assume things are working when we start

  // check if Card is inserted
  if (sd_log_card_missing())
  {
    return 0;
  }

  // set card to inserted
  sd_log_inserted = 1;

  // turn on SD card
  //sd_log_pwr_on();

  // see if the card can be initialized:
  if (!SD.begin(sd_log_cs)) {
    return 0;
  }

  // set card as initialized
  sd_log_initialized = 1;

  // return success
  return 1;
}

// SD diagnostic routine
void sd_log_card_diagnostic()
{
  char tmp_file[13];
  char tmp[BG_ERR_MSG_SIZE];

  // check if Card is inserted
  strcpy_P(tmp, PSTR("SD inserted,"));
  Serial.print(tmp);
  if (sd_log_card_missing())
  {
    strcpy_P(tmp, PSTR("no"));
    Serial.println(tmp);
  }
  else
  {
    strcpy_P(tmp, PSTR("yes"));
    Serial.println(tmp);
  }

  // see if the card can be initialized:
  strcpy_P(tmp, PSTR("SD initialized,"));
  Serial.print(tmp);
  if (sd_log_initialized) {
    strcpy_P(tmp, PSTR("yes"));
    Serial.println(tmp);
  }
  else
  {
    strcpy_P(tmp, PSTR("no"));
    Serial.println(tmp);
  }

  // Test file read/write
  strcpy_P(tmp_file, PSTR("BG_TEST.TXT"));
  strcpy_P(tmp, PSTR("This is a bGeigie test"));
  sd_log_writeln(tmp_file, tmp);

  // file open
  strcpy_P(tmp, PSTR("SD open file,"));
  Serial.print(tmp);
  if (sd_log_file_open)
  {
    strcpy_P(tmp, PSTR("yes"));
    Serial.println(tmp);
    // delete test file
    if (!(SD.remove(tmp_file)))
    {
      strcpy_P(tmp, PSTR("SD test : can't remove test file"));
      Serial.println(tmp);
    }
  }
  else
  {
    strcpy_P(tmp, PSTR("no"));
    Serial.println(tmp);
  }

  // SD R/W
  strcpy_P(tmp, PSTR("SD read write,"));
  Serial.print(tmp);
  if (sd_log_last_write)
  {
    strcpy_P(tmp, PSTR("yes"));
    Serial.println(tmp);
  }
  else
  {
    strcpy_P(tmp, PSTR("no"));
    Serial.println(tmp);
  }

}

// write a line to a file in SD card
int sd_log_writeln(char *filename, char *log_line)
{
  // assume everything fails. Change flag as things succeed.
  sd_log_inserted = 0;
  sd_log_file_open = 0;
  sd_log_last_write = 0;

  // test for card presence
  if (sd_log_card_missing())
    return 0;

  // SD card was inserted if we get here
  sd_log_inserted = 1;

  // open file
  dataFile = SD.open(filename, FILE_WRITE);
  if (dataFile)
  {
    // diagnostic variable : could open file (twice actually)
    sd_log_file_open = 1;

    // Clear interrupts when writing
    uint8_t sreg_old = SREG;  // save sreg
    cli();                    // disable global interrupts

    int len = strnlen(log_line, LINE_MAX_SIZE);
    int nbytes = dataFile.print(log_line);
    nbytes += dataFile.print('\n');

    dataFile.close();

    // re-enable interrupts
    SREG = sreg_old;

    // verify correct number of bytes was written
    if (nbytes == len+1)
    {
      // Reopen the file in read mode and verify
      // everything was correctly written
      dataFile = SD.open(filename, FILE_READ);
      if (dataFile)
      {
        // seek to begining of line
        dataFile.seek(dataFile.size() - nbytes);
        int k = 0;
        // check every character
        char c;
        while ((c = dataFile.read()) != '\n' && k < len)
        {
          if (c != log_line[k])
            break;
          k++;
        }
        // k should match line length
        if (k == len)
          sd_log_last_write = 1;

        dataFile.close();
        delay(100);
      }
      else
      {
        // diagnostic variable : couldn't open file second time
        sd_log_file_open = 0;
      }
    }
  }

  return sd_log_last_write;
}

