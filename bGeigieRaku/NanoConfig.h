#ifndef _NANO_CONFIG_H_
#define _NANO_CONFIG_H_

//
// bGeigie Nano definitions
//

#define NANO_DEVICE_ID        0000
#define NANO_VERSION       "2.0.1"
#define NANO_HEADER        "BNRDD"
#define NANO_CPM_FACTOR        334
#define NANO_BQM2_FACTOR        37

//
// Enable or Disable features
//

#define ENABLE_DEBUG             0
#define ENABLE_DIAGNOSTIC        0
#define ENABLE_SLEEPMODE         0
#define ENABLE_SSD1306           1
#define ENABLE_HARDWARE_COUNTER  0
#define ENABLE_OPENLOG           0
#define ENABLE_WAIT_GPS_FOR_LOG  1
#define ENABLE_GPS_NMEA_LOG      0
#define ENABLE_100M_TRUNCATION   0
#define ENABLE_MEDIATEK          1
#define ENABLE_SKYTRAQ           0
#define ENABLE_EEPROM_DOSE       1
#define ENABLE_CUSTOM_FN         1 // enable custom function button
#define ENABLE_LND_DEADTIME      1 // enable dead-time compensation for LND7317
#define ENABLE_GEIGIE_SWITCH     1 // switch between bGeigie and xGeigie type
#define ENABLE_NANOKIT_PIN       1 // use the nano kit configuration
#define ENABLE_NANOPCBKIT_PIN    0 // use the nano pcb kit configuration
#define ENABLE_INTEGRATEDNANOKIT_PIN    1 // use the integrated nano pcb kit configuration

#if ENABLE_SSD1306 // high memory usage (avoid logs)
#undef ENABLE_DEBUG // disable debug log output
#endif

//
// Pins definition
//
#warning INTEGRATED NANO PCB used!
#define OLED_RESET D7
#define I2C1_SDA D0
#define I2C1_SCL D1
#define SPI3_MOSI D2
#define SPI3_MISO D3
#define SPI3_SCK D4
#define CS_LOG D5
#define PWR_ON D6
#define LCD_RST D7
#define RX_GPS D8
#define TX_GPS D9
#define IROVER D10
#define ENC_SW D11
#define BAT_MEASURE A4
#define PWR_OFF D13
#define ENC_A D14
#define ENC_B D15
#define RX_XBEE D16
#define TX_XBEE D17

// The pin to keep power on and trigger the log/alarm led is the same
#define LOGALARM_LED_PIN PWR_ON

// HardwareCounter pin
// the timer1 pin on the 328p is D5
#define HARDWARE_COUNTER_TIMER1 5

// InterruptCounter pin
// 0 = D2, 1 = D3
#define INTERRUPT_COUNTER_PIN 0

// bGeigie <-> xGeigie switch pin
#define GEIGIE_TYPE_PIN A5
#define GEIGIE_TYPE_THRESHOLD 500

// Voltage divider
// GND -- R2 --A0 -- R1 -- VBAT
// https://en.wikipedia.org/wiki/Voltage_divider
#define VOLTAGE_PIN BAT_MEASURE
#define VOLTAGE_R1 9100
#define VOLTAGE_R2 1000

#endif
