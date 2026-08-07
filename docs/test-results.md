# Test Results

This document summarizes the tests completed during the development of the Smart Environment Station.

## Test Environment

- Board: EK-TM4C123GXL
- Microcontroller: TM4C123GH6PM
- IDE: Code Composer Studio
- Compiler: TI ARM Compiler 20.2.7.LTS
- TivaWare: 2.2.0.295
- UART terminal: PuTTY
- UART configuration: 115200 baud, 8 data bits, no parity, 1 stop bit
- OLED I2C address: 0x3C

## Test Summary

| ID | Test | Expected Result | Actual Result | Status |
|---|---|---|---|---|
| T01 | GPIO LED blink | On-board RGB LED blinks | LED blinked correctly | Passed |
| T02 | GPIO color change | Selected RGB color is displayed | Green and red tests worked | Passed |
| T03 | UART fixed text | Text is visible in PuTTY | Message was received correctly | Passed |
| T04 | UART counter | Counter increases over time | Counter values increased correctly | Passed |
| T05 | LDR raw ADC reading | Raw value changes with light | ADC value changed with light | Passed |
| T06 | LDR averaging | Output becomes more stable | 16-sample average reduced fluctuations | Passed |
| T07 | LDR direction correction | Light percentage increases with light | Inverted conversion fixed direction | Passed |
| T08 | LDR calibration | Bright condition is near 100% and dark condition is near 0% | Bright and dark tests reached 100% and 0% | Passed |
| T09 | DHT11 response | Temperature and humidity are received | Valid values were received | Passed |
| T10 | DHT11 checksum | Invalid packets are rejected | Checksum control worked | Passed |
| T11 | DHT11 timeout handling | Missing or invalid response reports an error | Timeout messages were displayed | Passed |
| T12 | OLED I2C detection | OLED responds at 0x3C | Device was detected at 0x3C | Passed |
| T13 | OLED initialization | OLED can be initialized | Initialization completed successfully | Passed |
| T14 | OLED fill test | All pixels turn on | Screen turned fully on | Passed |
| T15 | OLED clear test | Screen becomes blank | Screen was cleared successfully | Passed |
| T16 | OLED live data | Temperature, humidity, and light are displayed | All values appeared on the OLED | Passed |
| T17 | UART and OLED together | Both outputs update at the same time | Both outputs worked simultaneously | Passed |
| T18 | Full system test | All sensors and outputs work together | Complete breadboard setup worked | Passed |

## GPIO Test

The on-board RGB LED was used as the first hardware test.

Two variations were tested:

- Green LED with a faster blinking rate
- Red LED with a slower blinking rate

The test confirmed that the board, compiler, debugger, GPIO configuration, and delay logic were working.

## UART Test

### Fixed Text

The first UART test sent:

```text
Hello from EK-TM4C123GXL
```

The message was received through the Stellaris Virtual Serial Port in PuTTY.

### Counter

A counter was then transmitted:

```text
Counter: 0
Counter: 1
Counter: 2
Counter: 3
```

This test confirmed that numerical values could be converted to text and sent through UART.

## LDR Test

### Raw ADC Reading

The LDR module's `AO` pin was connected to PE3/AIN0.

The raw ADC value changed when:

- The LDR was covered by hand
- A phone flashlight was pointed at the LDR

### Averaging

Sixteen ADC samples were added and divided by 16.

This reduced small fluctuations in the displayed light value.

### Calibration

Measured limits:

```text
Bright condition: 175
Dark condition:   3504
```

The calibrated output reached:

```text
Strong light -> 100%
Darkness     -> 0%
```

The final calculation used the measured sensor range instead of the theoretical 0-4095 ADC range.

## DHT11 Test

The DHT11 was tested through PD0.

The first timing method incorrectly returned:

```text
Temperature: 0 C
Humidity: 0 %
```

After changing the bit sampling method, valid values were received.

Example successful output:

```text
Temperature: 30 C | Humidity: 37 %
```

The following error conditions were also handled:

```text
Response timeout
Data timeout
Checksum error
```

## OLED Test

### I2C Address Detection

The OLED responded at:

```text
0x3C
```

This confirmed that power, SCL, SDA, and the I2C configuration were correct.

### Fill and Clear

The display memory was filled with `0xFF`, causing all pixels to turn on.

It was then filled with `0x00`, causing the display to become blank.

### Live Display

The final OLED layout was:

```text
SMART ENV
TEMP: 30 C
HUM: 37 %
LIGHT: 42 %
```

The values were updated approximately every two seconds.

## Full System Test

The final setup was tested with all components connected:

- DHT11 on PD0
- LDR AO on PE3/AIN0
- OLED SCL on PB2
- OLED SDA on PB3
- UART0 through PA0 and PA1
- Shared 3.3 V and GND lines

The OLED and PuTTY displayed the same sensor data at the same time.

## Final Result

The Smart Environment Station completed all planned core tests successfully.

The system can:

- Measure temperature
- Measure humidity
- Measure ambient light
- Convert the light measurement to a calibrated percentage
- Display live values on an OLED
- Send the same values through UART
- Report DHT11 communication errors
