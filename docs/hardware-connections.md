# Hardware Connections

This document lists the final hardware connections used in the Smart Environment Station project.

## Main Board

- Board: Texas Instruments EK-TM4C123GXL
- Microcontroller: TM4C123GH6PM
- Logic voltage: 3.3 V

The 5 V pin is not used in this project.

## Final Pin Assignment

| Module | Module Pin | Tiva C Pin | Function |
|---|---|---|---|
| DHT11 | S | PD0 | Temperature and humidity data |
| DHT11 | + | 3.3 V | Power |
| DHT11 | - | GND | Ground |
| LDR module | AO | PE3 / AIN0 | Analog light measurement |
| LDR module | VCC | 3.3 V | Power |
| LDR module | GND | GND | Ground |
| OLED | SCL | PB2 / I2C0SCL | I2C clock |
| OLED | SDA | PB3 / I2C0SDA | I2C data |
| OLED | VCC | 3.3 V | Power |
| OLED | GND | GND | Ground |
| UART0 | RX | PA0 | UART receive |
| UART0 | TX | PA1 | UART transmit |

## Power Distribution

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

The modules are connected in parallel to the same supply lines.

## LDR Module

The LDR module has four pins:

```text
VCC
GND
AO
DO
```

Only the analog output is used:

```text
LDR AO -> PE3 / AIN0
LDR DO -> not connected
```

The `DO` pin provides only a digital threshold result. The `AO` pin provides the changing analog voltage required for ADC measurement.

## DHT11 Module

The DHT11 module has three pins:

```text
S
+
-
```

Final connection:

```text
DHT11 S -> PD0
DHT11 + -> 3.3 V
DHT11 - -> GND
```

The DHT11 data line was initially connected to PB2. It was later moved to PD0 because PB2 and PB3 were required for OLED I2C communication.

## OLED Display

The OLED module has four pins:

```text
GND
VCC
SCL
SDA
```

Final connection:

```text
OLED GND -> GND
OLED VCC -> 3.3 V
OLED SCL -> PB2
OLED SDA -> PB3
```

The display was detected at I2C address:

```text
0x3C
```

## UART Connection

UART0 is routed through the Stellaris Virtual Serial Port.

```text
PA0 -> UART0 RX
PA1 -> UART0 TX
```

Serial configuration:

```text
Baud rate: 115200
Data bits: 8
Parity: None
Stop bits: 1
Flow control: None
```

## Important Notes

- Do not connect the LDR `DO` pin to PE3 when analog measurement is required.
- Do not use the 5 V pin for the connected modules.
- Disconnect USB power before changing jumper connections.
- Make sure all modules share a common GND.
- Check the printed pin labels on each module instead of relying only on physical pin order.
