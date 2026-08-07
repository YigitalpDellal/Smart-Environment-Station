# User Manual

This manual explains how to assemble, power, run, and check the Smart Environment Station.

## 1. System Overview

The Smart Environment Station measures:

- Temperature
- Humidity
- Ambient light level

The values are shown on an SSD1306 OLED display and are also sent to a computer through UART.

## 2. Required Hardware

- EK-TM4C123GXL Tiva C LaunchPad
- DHT11 temperature and humidity sensor module
- LDR sensor
- LM393-based LDR module
- 0.96-inch I2C OLED display
- Breadboard
- Jumper wires
- Micro-USB cable
- Computer with Code Composer Studio
- PuTTY or another serial terminal application

## 3. Final Wiring

### DHT11

```text
DHT11 S -> PD0
DHT11 + -> 3.3 V
DHT11 - -> GND
```

### LDR Module

```text
LDR AO  -> PE3 / AIN0
LDR VCC -> 3.3 V
LDR GND -> GND
LDR DO  -> not connected
```

### OLED

```text
OLED SCL -> PB2
OLED SDA -> PB3
OLED VCC -> 3.3 V
OLED GND -> GND
```

### UART0

```text
PA0 -> UART0 RX
PA1 -> UART0 TX
```

## 4. Power Distribution

The LDR module, DHT11, and OLED share the same 3.3 V and GND lines through the breadboard.

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

The 5 V pin is not used.

## 5. Safety Notes

- Disconnect the USB cable before changing jumper wires.
- Do not connect the modules to 5 V.
- Make sure all modules share the same GND.
- Check the printed pin labels before making connections.
- Do not connect the LDR `DO` pin to PE3/AIN0.
- Do not reverse the DHT11 or OLED power connections.

## 6. Software Setup

Required software:

- Code Composer Studio
- TivaWare for C Series 2.2.0.295
- TI ARM Compiler 20.2.7.LTS
- Stellaris ICDI drivers
- PuTTY

## 7. Importing the CCS Project

1. Open Code Composer Studio.
2. Select the workspace.
3. Open the project import menu.
4. Select the project folder:

```text
firmware/environment_test
```

5. Complete the import.
6. Confirm that the project appears in Project Explorer.

## 8. Project Configuration Checks

Before building, check the following:

- TI ARM Compiler 20.2.7.LTS is selected.
- The TivaWare include path is configured.
- `driverlib.lib` is included in the linker settings.
- Only one startup file is included in the build.
- The correct target configuration is selected.

## 9. Building the Project

1. Select the `environment_test` project.
2. Save all files.
3. Run:

```text
Project -> Clean Project
Project -> Build Project
```

A successful build should produce the project output file without unresolved symbol errors.

A post-build warning may appear even when the main `.out` file is created successfully.

## 10. Connecting the Board

1. Check all breadboard connections.
2. Connect the USB cable to the board's DEBUG port.
3. Confirm that the board power LED is on.
4. Select the `environment_test` project in CCS.
5. Start:

```text
Run -> Debug Project
```

6. Resume execution if the processor stops at the beginning of `main()`.

## 11. Opening PuTTY

Use the following serial settings:

```text
Connection type: Serial
Serial line: COM5
Speed: 115200
Data bits: 8
Parity: None
Stop bits: 1
Flow control: None
```

The COM port number may be different on another computer.

To find it:

1. Open Windows Device Manager.
2. Open `Ports (COM & LPT)`.
3. Find `Stellaris Virtual Serial Port`.
4. Use the COM number shown there.

## 12. Expected UART Output

A normal output line looks similar to:

```text
Temperature: 30 C | Humidity: 37 % | Light: 42 % | LDR Raw: 2100
```

The values are updated approximately every two seconds.

## 13. OLED Display

The OLED should show:

```text
SMART ENV
TEMP: 30 C
HUM: 37 %
LIGHT: 42 %
```

The values are updated approximately every two seconds.

If DHT11 communication fails, the display shows:

```text
DHT ERROR
```

## 14. Testing the LDR

To test the light sensor:

1. Watch the `LIGHT` value on the OLED or PuTTY.
2. Cover the LDR with your hand.
3. The light percentage should decrease.
4. Point a phone flashlight at the LDR.
5. The light percentage should increase.

The calibrated range used in the current setup is:

```text
Bright: 175
Dark:   3504
```

Different LDR modules may require different calibration values.

## 15. Testing the DHT11

To test the DHT11:

1. Watch the temperature and humidity values.
2. Wait several seconds after powering the system.
3. Breathe near the sensor without touching it.
4. The humidity value may increase slightly.
5. Hold a warm hand near the sensor.
6. The temperature may increase slowly.

The DHT11 updates more slowly than the LDR.

## 16. Common Error Messages

### Response Timeout

```text
DHT11 Error: Response timeout
```

Possible causes:

- DHT11 signal wire is disconnected.
- The signal wire is connected to the wrong GPIO pin.
- VCC or GND is missing.
- The sensor has not stabilized after power-up.

### Data Timeout

```text
DHT11 Error: Data timeout
```

Possible causes:

- Timing problem
- Loose signal connection
- Invalid sensor response

### Checksum Error

```text
DHT11 Error: Checksum error
```

The received data did not match the checksum byte.

### OLED Not Detected

```text
OLED not detected at address 0x3C.
```

Check:

- OLED VCC
- OLED GND
- PB2 to SCL
- PB3 to SDA
- I2C address
- Loose jumper wires

### OLED Update Error

```text
OLED update error.
```

The OLED was detected, but a later I2C transfer failed.

Check the OLED wiring and power.

## 17. If the LDR Value Stays Near 4095

Check that:

```text
LDR AO -> PE3 / AIN0
LDR DO -> not connected
```

The `DO` pin should not be used for analog light measurement.

## 18. If PuTTY Shows No Output

Check:

- Correct COM port
- Speed set to 115200
- Data bits set to 8
- Parity set to None
- Stop bits set to 1
- Flow control set to None
- Program is running in CCS
- Board is connected through the DEBUG port

Press the board RESET button once if necessary.

## 19. Normal Shutdown

1. Close PuTTY.
2. Stop the debug session in CCS.
3. Disconnect the USB cable.
4. Do not change jumper wires while the board is powered.

## 20. Final Operating State

When the system is working correctly:

- The OLED displays temperature, humidity, and light.
- PuTTY displays the same values.
- The LDR value changes with light.
- The DHT11 provides valid measurements.
- The system updates approximately every two seconds.
