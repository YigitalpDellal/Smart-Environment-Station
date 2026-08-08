# Changelog

All notable changes to the Smart Environment Station project are documented in this file.

## [1.0.1] - 2026-08-08

### Added
- Final project report in PDF and Word formats
- Full-system and LDR demo videos embedded in the README

### Changed
- Updated README documentation links
- Updated the repository structure section
- Refined the OLED linker troubleshooting explanation
- Cleaned the development log formatting

## [1.0.0] - 2026-08-06

### Added

- GPIO test using the on-board RGB LED
- UART0 communication through PA0 and PA1
- Fixed text and counter output through PuTTY
- ADC0 reading through PE3/AIN0
- 16-sample averaging for LDR measurements
- Calibrated light percentage conversion
- DHT11 temperature and humidity measurement through PD0
- DHT11 timeout and checksum handling
- I2C0 communication through PB2 and PB3
- SSD1306 OLED detection at address `0x3C`
- OLED initialization, fill, clear, and text output
- Live OLED display for temperature, humidity, and light
- Simultaneous OLED and UART output
- README and technical documentation
- Hardware connection guide
- Troubleshooting guide
- Development log
- Test results
- User manual
- Project report

### Changed

- Moved the DHT11 data pin from PB2 to PD0 to free PB2 and PB3 for I2C0
- Replaced the initial LDR percentage formula with a calibrated conversion
- Replaced DHT11 pulse-length counting with fixed-time sampling
- Renamed the main source file to `main.c`

### Fixed

- Incorrect LDR `DO` connection to PE3/AIN0
- Reversed light percentage behavior
- DHT11 readings stuck at `0 C` and `0 %`
- OLED helper function linker errors
- OLED remaining black before SSD1306 initialization

## [0.1.0] - 2026-08-01

### Added

- Initial CCS project setup
- TivaWare configuration
- Basic board and GPIO test
