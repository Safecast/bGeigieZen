
# bGeigieZen

A modern radiation monitoring device based on the M5Stack hardware platform, developed by the Safecast community.

## Overview

bGeigieZen is a portable radiation monitoring device that combines precision sensing with a user-friendly interface. Built on the M5Stack platform, it provides real-time radiation measurements with GPS logging capabilities.

## Features

- Real-time radiation monitoring
- GPS location tracking
- Data logging to SD card
- User-friendly display interface
- Long battery life
- Mobile app connectivity
- Compatible with the Safecast API

## Hardware Requirements

- M5Stack Core device
- LND-7317 radiation sensor
- GPS module
- SD card for logging

## Software Setup

1. **Install Required Development Environment **:
   - VS Code with PlatformIO or PlatformIO standalone

2. **Install Required Libraries**:
   - m5stack/M5Unified
   - claypuppet/SensorReporter
   - beakes/TeenyUbloxConnect
   - alextaujenis/RBD_Timer

3. **Clone the Repository**:
   ```bash
   git clone https://github.com/Safecast/bGeigieZen.git
   ```

## Building and Flashing

1. Open the project in VS Code with PlatformIO, or invoke PlatformIO from the shell/CMD.
2. Select your M5Stack board.
3. Compile and upload.

## Usage

1. Power on the device.
2. Wait for GPS signal acquisition.
3. Radiation measurements will display on screen.
4. Data logs automatically save to the SD card.

## Contributing

We welcome contributions! Please follow these steps:

1. Fork the repository.
2. Create a feature branch.
3. Submit a Pull Request.

## License

This project is licensed under [appropriate license].

## Support

- Visit [Safecast.org](https://safecast.org)
- Join our community forums
- Report issues on GitHub

## Credits

Developed and maintained by the Safecast community.

This repo is for development of the bGeigieZen.
Specs can be found at https://github.com/Safecast/bGeigieZen/wiki/Specification
Current version is V3.2.4

