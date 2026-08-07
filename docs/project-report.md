# Project Report

## 1. Introduction

The Smart Environment Station is an embedded system developed with the Texas Instruments EK-TM4C123GXL Tiva C LaunchPad.

The system measures temperature, humidity, and ambient light level. The values are displayed on an SSD1306 OLED screen and transmitted to a computer through UART.

The project was developed step by step so that each peripheral could be tested separately before the final integration.

## 2. Project Objectives

The main objectives were:

- Configure the TM4C123GH6PM system clock
- Use GPIO for basic hardware testing
- Configure UART0 for serial communication
- Read an analog signal through ADC0
- Calibrate an LDR-based light measurement
- Read temperature and humidity from DHT11
- Configure I2C0 communication
- Drive an SSD1306 OLED display
- Handle sensor and communication errors
- Integrate all modules into one working system

## 3. Hardware Components

- EK-TM4C123GXL Tiva C LaunchPad
- TM4C123GH6PM microcontroller
- DHT11 temperature and humidity sensor module
- LDR sensor
- LM393-based LDR module
- SSD1306 I2C OLED display
- Breadboard
- Jumper wires
- Micro-USB cable

## 4. Software Tools

- Code Composer Studio
- TivaWare for C Series 2.2.0.295
- TI ARM Compiler 20.2.7.LTS
- PuTTY
- Stellaris ICDI drivers

## 5. Final Pin Assignment

| Module | Signal | Tiva C Pin |
|---|---|---|
| DHT11 | Data | PD0 |
| LDR module | AO | PE3 / AIN0 |
| OLED | SCL | PB2 / I2C0SCL |
| OLED | SDA | PB3 / I2C0SDA |
| UART0 | RX | PA0 |
| UART0 | TX | PA1 |

All modules use the same 3.3 V and GND lines through the breadboard.

## 6. Development Process

### 6.1 GPIO Test

The TivaWare blinky example was used to verify the board, compiler, debugger, and GPIO configuration.

The on-board RGB LED was tested with different colors and blinking speeds.

### 6.2 UART Communication

UART0 was configured on PA0 and PA1.

The first test transmitted a fixed text message. A counter was then added to confirm that numerical values could be converted and transmitted correctly.

UART settings:

```text
115200 baud
8 data bits
No parity
1 stop bit
No flow control
```

### 6.3 LDR and ADC

The LDR module's analog output was connected to PE3/AIN0.

ADC0 Sample Sequencer 3 was configured for processor-triggered conversion.

The raw ADC value was transmitted through UART.

### 6.4 ADC Averaging

The LDR output showed small fluctuations.

To reduce this effect, 16 samples were read and averaged.

### 6.5 LDR Direction and Calibration

The module produced lower ADC values under stronger light and higher ADC values in darkness.

Measured limits:

```text
Bright condition: 175
Dark condition:   3504
```

These values were used to convert the raw ADC reading to a light percentage.

### 6.6 DHT11 Integration

The DHT11 was connected to PD0.

The sensor communication includes:

- Start signal
- Sensor response
- 40-bit data reception
- Checksum verification

The first timing method returned zero values. The reading method was changed so that the signal was sampled approximately 40 microseconds after each HIGH pulse began.

### 6.7 OLED and I2C

The OLED was connected to I2C0:

```text
PB2 -> SCL
PB3 -> SDA
```

The display responded at address:

```text
0x3C
```

After address detection, the SSD1306 initialization sequence was sent.

The screen was tested by filling all pixels and then clearing the display memory.

### 6.8 Live Display

A small 5x7 font was added.

Final OLED layout:

```text
SMART ENV
TEMP: 30 C
HUM: 37 %
LIGHT: 42 %
```

The values are updated approximately every two seconds.

## 7. Main Problems and Solutions

### 7.1 LDR AO and DO Confusion

The `DO` pin was mistakenly connected to PE3/AIN0.

This caused the ADC value to remain near 4095.

The connection was corrected:

```text
LDR AO -> PE3 / AIN0
LDR DO -> not connected
```

### 7.2 Light Percentage Direction

The first percentage formula moved in the wrong direction.

The formula was inverted and then replaced with a calibrated conversion using the measured bright and dark limits.

### 7.3 DHT11 Zero Data

The first DHT11 implementation returned 0 C and 0% humidity.

The data timing method was changed and an all-zero packet check was added.

### 7.4 Pin Conflict

DHT11 was initially connected to PB2.

PB2 and PB3 were later needed for OLED I2C communication, so DHT11 was moved to PD0.

### 7.5 OLED Linker Errors

Some OLED helper functions were called before their definitions were added.

The missing functions were implemented and ordered according to their dependencies.

## 8. Test Results

The following tests were completed successfully:

- GPIO LED blinking
- UART fixed text
- UART counter
- LDR raw ADC reading
- LDR averaging
- LDR calibration
- DHT11 temperature reading
- DHT11 humidity reading
- DHT11 checksum control
- OLED I2C detection
- OLED initialization
- OLED fill and clear
- Live OLED output
- Simultaneous UART and OLED output

## 9. Final System Behavior

The final system:

- Measures temperature
- Measures humidity
- Measures ambient light
- Converts light level to a calibrated percentage
- Displays all values on the OLED
- Sends the same values through UART
- Reports DHT11 errors
- Updates approximately every two seconds

## 10. Limitations

- DHT11 has limited accuracy and update speed.
- The light percentage depends on the measured calibration values.
- The program uses blocking delays.
- The OLED font includes only the required characters.
- Measurements are not stored.

## 11. Possible Improvements

- Use a more accurate temperature and humidity sensor
- Replace blocking delays with hardware timers
- Split the firmware into separate driver files
- Add threshold warnings
- Add data logging
- Add wireless communication
- Add a real-time clock
- Design a PCB
- Add low-power operation

## 12. Conclusion

The Smart Environment Station was completed successfully.

The project demonstrated the use of GPIO, UART, ADC, digital sensor timing, I2C communication, OLED control, calibration, and error handling on the TM4C123GH6PM microcontroller.

The final system worked with all modules connected at the same time and displayed the measured values through both OLED and UART.
