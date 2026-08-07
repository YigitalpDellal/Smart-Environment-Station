# Development Log

This document summarizes the main development stages of the Smart Environment Station.

## Stage 1 - Toolchain and Board Setup

The EK-TM4C123GXL board was first configured in Code Composer Studio.

Main setup tasks:

* Installed Code Composer Studio
* Selected the EK-TM4C123GXL target
* Used TI ARM Compiler 20.2.7.LTS
* Added the TivaWare include path
* Linked `driverlib.lib`
* Verified the startup file configuration
* Connected the board through the DEBUG USB port

The first successful test was based on the TivaWare blinky example.

## Stage 2 - GPIO Test

The on-board RGB LED was used to confirm that the compiler, debugger, GPIO configuration, and delay code were working.

Two variations were tested:

* Green LED with a faster blinking rate
* Red LED with a slower blinking rate

This stage confirmed that code could be built, loaded, and executed on the board.

## Stage 3 - UART Communication

UART0 was configured using:

```text
PA0 -> UART0 RX
PA1 -> UART0 TX
```

Serial settings:

```text
115200 baud
8 data bits
No parity
1 stop bit
No flow control
```

The first test transmitted a fixed text message:

```text
Hello from EK-TM4C123GXL
```

A counter was then added:

```text
Counter: 0
Counter: 1
Counter: 2
```

This stage confirmed that both text and numerical values could be sent to PuTTY.

## Stage 4 - LDR and ADC

The LDR module's analog output was connected to PE3/AIN0.

The ADC was configured as:

* ADC0
* Sample Sequencer 3
* Processor trigger
* AIN0 input
* 12-bit result

The raw ADC value was transmitted through UART.

The LDR was tested in two conditions:

* Covered by hand
* Illuminated with a phone flashlight

The raw value changed as expected.

## Stage 5 - ADC Averaging

The LDR reading showed small fluctuations.

To make the output more stable, 16 ADC samples were collected and averaged.

```text
Average = sum of 16 samples / 16
```

This reduced small changes in the displayed value.

## Stage 6 - Light Percentage Conversion

The first percentage calculation used the full ADC range:

```text
0 to 4095
```

The result initially moved in the wrong direction because the module produced:

```text
Strong light -> lower ADC value
Darkness     -> higher ADC value
```

The conversion was inverted.

## Stage 7 - LDR Calibration

Measured limits were recorded:

```text
Bright condition: 175
Dark condition:   3504
```

These values were added to the program:

```c
#define LDR_RAW_BRIGHT 175U
#define LDR_RAW_DARK   3504U
```

After calibration:

```text
Strong light -> 100%
Darkness     -> 0%
```

## Stage 8 - DHT11 Integration

The DHT11 module was added to measure temperature and humidity.

The sensor was first connected to PB2.

The DHT11 protocol required:

* Start signal from the microcontroller
* Sensor response
* 40-bit data reception
* Checksum validation

The first implementation returned:

```text
Temperature: 0 C
Humidity: 0 %
```

The HIGH pulse timing was being interpreted incorrectly.

The reading method was changed so that the data line was sampled approximately 40 microseconds after each HIGH pulse began.

After this change, valid measurements were received.

## Stage 9 - Shared Power Distribution

The LDR, DHT11, and OLED all required 3.3 V.

The available 3.3 V and GND connections were shared through the breadboard.

```text
Tiva 3.3 V
├── LDR VCC
├── DHT11 +
└── OLED VCC

Tiva GND
├── LDR GND
├── DHT11 -
└── OLED GND
```

The 5 V pin was not used.

## Stage 10 - OLED Wiring and Pin Reassignment

The OLED required I2C0:

```text
PB2 -> SCL
PB3 -> SDA
```

PB2 was already used by DHT11, so the DHT11 data line was moved to PD0.

Final assignment:

```text
DHT11 S  -> PD0
OLED SCL -> PB2
OLED SDA -> PB3
```

## Stage 11 - OLED I2C Address Test

The OLED was tested at address:

```text
0x3C
```

UART output confirmed:

```text
OLED detected at address 0x3C.
```

This verified the power, SCL, SDA, and I2C0 configuration.

## Stage 12 - OLED Initialization

The SSD1306 initialization sequence was added.

The screen was tested by:

1. Writing `0xFF` to all display memory
2. Waiting approximately one second
3. Writing `0x00` to clear the screen

The OLED turned fully on and then became blank.

## Stage 13 - OLED Linker Error

During integration, linker errors appeared for functions such as:

```text
OLED\_Init
OLED\_Fill
OLED\_SendCommand
OLED\_SendData
```

The cause was missing helper function definitions.

The functions were added in dependency order:

```text
I2C0\_Init
I2C\_DeviceResponds
OLED\_I2CWriteByte
OLED\_SendCommand
OLED\_SendData
OLED\_Init
OLED\_SelectFullScreen
OLED\_Fill
OLED text functions
main
```

The project built successfully after the missing functions were added.

## Stage 14 - Live OLED Output

A small 5x7 font was added.

The OLED displays:

```text
SMART ENV
TEMP: 30 C
HUM: 37 %
LIGHT: 42 %
```

The values are updated approximately every two seconds.

If DHT11 communication fails, the screen displays:

```text
DHT ERROR
```

## Stage 15 - Final Integration

The complete system now performs the following tasks:

* Reads temperature from DHT11
* Reads humidity from DHT11
* Reads ambient light through ADC
* Averages 16 LDR samples
* Converts the LDR value to a calibrated percentage
* Displays all values on the OLED
* Sends the same values through UART
* Reports DHT11 communication errors

## Final Status

The complete breadboard setup is working.

The final system uses:

```text
DHT11 S  -> PD0
LDR AO   -> PE3 / AIN0
OLED SCL -> PB2
OLED SDA -> PB3
UART0 RX -> PA0
UART0 TX -> PA1
```



