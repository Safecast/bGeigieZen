# ULP Geiger Pulse Counter Test (CoreS3)

This PlatformIO project demonstrates using the ESP32-S3 ULP (Ultra Low Power) co-processor to count Geiger tube pulses on GPIO32 (RTC-capable) for the bGeigieZen hardware. The ULP runs independently of the main CPU, allowing for ultra-low-power pulse counting.

## Features
- Counts rising edges (pulses) on GPIO32 using the ULP.
- Stores the pulse count in RTC slow memory.
- Main CPU periodically reads the count, calculates CPM (counts per minute), and prints to Serial.
- Uses [UlpDebug](https://github.com/tanakamasayuki/UlpDebug) for easy ULP integration and debugging in Arduino.

## Usage
1. **Hardware:**
   - Connect your Geiger tube pulse output to GPIO32 on the M5Stack CoreS3.
2. **Build & Upload:**
   - Open this folder in PlatformIO or VSCode.
   - Build and upload the project to your CoreS3 device.
3. **Monitor Output:**
   - Open the Serial Monitor at 115200 baud.
   - You will see the pulse count and calculated CPM every 10 seconds.

## File Overview
- `platformio.ini` — PlatformIO configuration for CoreS3 and UlpDebug.
- `ulp_geiger_counter.cpp` — Main Arduino sketch with ULP logic and CPM calculation.

## Customization
- To change the ULP polling interval, adjust `I_DELAY(10000)` and `ulp_set_wakeup_period(0, 10000)` in the sketch.
- For a different board or pin, update the PlatformIO `board` and the GPIO number in the code.

## References
- [UlpDebug Library](https://github.com/tanakamasayuki/UlpDebug)
- [Espressif ULP Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/ulp.html) 