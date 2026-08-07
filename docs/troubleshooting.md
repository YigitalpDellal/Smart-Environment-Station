# Troubleshooting

This document records the main problems encountered during the development of the Smart Environment Station and how they were solved.

## 1. Project Build Completed but Post-Build Error Appeared

### Symptom

Code Composer Studio displayed:

```text
post-build Error 2 (ignored)
```

The project still produced the `.out` file and showed:

```text
Build finished
```

### Cause

The main compile and link stages were successful. The error came from a secondary post-build command that attempted to generate another output file.

### Solution

The generated `.out` file was checked first. Since it existed and the main build had completed, development continued.

The post-build warning did not prevent the program from being loaded onto the board.

---

## 2. Debug Session Already Running

### Symptom

Code Composer Studio reported:

```text
There is already a Code Composer debug session running.
```

In some cases, it also showed:

```text
Unable to communicate with the device.
```

### Cause

A previous debug session was still active or Code Composer Studio had not released the debug connection correctly.

### Solution

1. The active debug session was stopped.
2. Code Composer Studio was closed.
3. The USB cable was disconnected.
4. The board was reconnected through the DEBUG USB port.
5. Code Composer Studio was reopened.
6. The correct project was selected and debugged again.

---

## 3. UART Terminal Did Not Appear in CCS

### Symptom

The terminal opened from Code Composer Studio was a normal Windows command prompt instead of a serial terminal.

### Cause

The installed CCS version did not provide the expected serial terminal interface in the visible menus.

### Solution

PuTTY was installed and used as the serial terminal.

The connection settings were:

```text
Serial line: COM5
Speed: 115200
Data bits: 8
Parity: None
Stop bits: 1
Flow control: None
```

The COM port number was found in Windows Device Manager under:

```text
Ports (COM & LPT)
```

---

## 4. LDR Value Stayed Near 4095

### Symptom

The UART terminal displayed values close to:

```text
LDR Raw: 4094
Light: 0 %
```

The value did not change when light was applied to the sensor.

### Cause

The digital output pin `DO` of the LDR module was connected to PE3/AIN0 instead of the analog output pin `AO`.

The `DO` pin only provides a digital threshold result and cannot be used for continuous ADC measurement.

### Solution

The wiring was corrected:

```text
LDR AO -> PE3 / AIN0
LDR DO -> not connected
```

After this change, the ADC value changed according to the light level.

---

## 5. Light Percentage Decreased When Light Increased

### Symptom

When stronger light was applied to the LDR, the calculated light percentage decreased.

### Cause

The LDR module produced a lower ADC value under strong light and a higher ADC value in darkness.

The first percentage formula assumed the opposite behavior.

### Solution

The conversion formula was inverted:

```c
return ((4095U - rawValue) * 100U) / 4095U;
```

Later, the sensor was calibrated using measured limits:

```c
#define LDR_RAW_BRIGHT 175U
#define LDR_RAW_DARK   3504U
```

The calibrated calculation became:

```c
return ((LDR_RAW_DARK - rawValue) * 100U) /
       (LDR_RAW_DARK - LDR_RAW_BRIGHT);
```

After calibration:

```text
Strong light -> approximately 100%
Darkness     -> approximately 0%
```

---

## 6. DHT11 Returned 0 C and 0% Humidity

### Symptom

The UART output showed:

```text
Temperature: 0 C
Humidity: 0 %
```

The checksum also appeared valid.

### Cause

All 40 data bits were being interpreted as zero because the HIGH pulse timing was measured incorrectly.

Since all data bytes were zero, the checksum calculation also resulted in zero and incorrectly appeared valid.

### Solution

The bit-reading method was changed.

Instead of measuring the complete HIGH pulse with a software counter, the data line was sampled approximately 40 microseconds after the HIGH pulse began.

The logic became:

```text
Line LOW after 40 us  -> bit 0
Line HIGH after 40 us -> bit 1
```

An additional check was added to reject a completely zero data packet.

After this change, valid temperature and humidity values were received.

---

## 7. DHT11 Pin Conflict with OLED I2C

### Symptom

The DHT11 was initially connected to PB2, but PB2 was later required for the OLED I2C clock line.

### Cause

PB2 and PB3 are used by I2C0:

```text
PB2 -> I2C0SCL
PB3 -> I2C0SDA
```

A single GPIO pin cannot be used for DHT11 data and I2C clock at the same time.

### Solution

The DHT11 data pin was moved from PB2 to PD0.

Final assignment:

```text
DHT11 S  -> PD0
OLED SCL -> PB2
OLED SDA -> PB3
```

The DHT11 GPIO definitions and initialization code were updated accordingly.

---

## 8. OLED Was Detected but the Screen Stayed Black

### Symptom

UART reported:

```text
OLED detected at address 0x3C.
```

The screen still remained black.

### Cause

The I2C address test only confirmed that the device responded. The SSD1306 initialization commands had not yet been sent.

### Solution

The SSD1306 initialization sequence was added.

The initialization included:

- Display OFF
- Clock configuration
- Multiplex ratio
- Display offset
- Charge pump enable
- Memory addressing mode
- Segment remap
- COM scan direction
- Contrast configuration
- Display ON

After initialization, the screen fill and clear test was performed successfully.

---

## 9. OLED Linker Errors

### Symptom

The build failed with unresolved symbol errors for functions such as:

```text
OLED_Init
OLED_Fill
OLED_SendCommand
OLED_SendData
```

### Cause

The functions were called from `main()`, but some helper functions had not yet been defined.

The missing dependency chain included:

```text
OLED_I2CWriteByte
OLED_SendCommand
OLED_SendData
```

### Solution

The OLED functions were added and ordered according to their dependencies:

```text
I2C0_Init
I2C_DeviceResponds
OLED_I2CWriteByte
OLED_SendCommand
OLED_SendData
OLED_Init
OLED_SelectFullScreen
OLED_Fill
OLED text functions
main
```

After the missing functions were added, the project linked successfully.

---

## 10. OLED Fill and Clear Test

### Test

After successful initialization, all OLED pixels were turned on by writing:

```text
0xFF
```

Then the display memory was cleared by writing:

```text
0x00
```

### Result

The screen turned fully on for approximately one second and then became black.

This confirmed that:

- The OLED was powered correctly.
- The I2C connection was working.
- SSD1306 initialization was successful.
- Display memory could be written.
- The screen could be cleared.

---

## 11. Shared 3.3 V Connection

### Problem

The accessible 3.3 V pin on the Tiva C board was already used by the LDR module, while the DHT11 and OLED also required 3.3 V.

### Solution

The existing 3.3 V and GND connections were shared through the breadboard.

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

The modules were connected in parallel to the same supply lines.

The 5 V pin was not used.

---

## 12. General Checks

When a module does not work, check the following:

- Confirm the module pin labels.
- Confirm that VCC is connected to 3.3 V.
- Confirm that all modules share GND.
- Confirm that signal pins match the software definitions.
- Disconnect USB power before changing jumper wires.
- Check whether an old debug session is still running.
- Confirm the correct COM port in Device Manager.
- Confirm UART settings are `115200 8N1`.
- Confirm the OLED address is `0x3C`.
- Confirm that only one startup file is included in the build.
- Confirm that TivaWare include paths and `driverlib.lib` are configured.
