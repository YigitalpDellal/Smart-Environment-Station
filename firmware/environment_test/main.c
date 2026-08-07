/*
 * Smart Environment Station
 * EK-TM4C123GXL / TM4C123GH6PM
 *
 * DHT11: PD0
 * LDR AO: PE3 / AIN0
 * OLED I2C: PB2 (SCL), PB3 (SDA)
 * UART0: PA0 (RX), PA1 (TX)
 */

#include <stdint.h>
#include <stdbool.h>
#include "driverlib/i2c.h"
#include "inc/hw_memmap.h"

#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"

#define LDR_RAW_BRIGHT    175U
#define LDR_RAW_DARK      3504U
#define OLED_I2C_BASE      I2C0_BASE
#define OLED_I2C_ADDRESS   0x3CU

#define DHT_GPIO_BASE     GPIO_PORTD_BASE
#define DHT_GPIO_PIN      GPIO_PIN_0

typedef enum
{
    DHT_STATUS_OK = 0,
    DHT_STATUS_RESPONSE_TIMEOUT,
    DHT_STATUS_DATA_TIMEOUT,
    DHT_STATUS_CHECKSUM_ERROR
} DHT_Status;

typedef struct
{
    uint8_t humidity;
    uint8_t temperature;
} DHT11_Data;

static void DelayMicroseconds(uint32_t microseconds)
{
    while (microseconds > 0U)
    {
        SysCtlDelay(40U / 3U);
        microseconds--;
    }
}

static void DelayMilliseconds(uint32_t milliseconds)
{
    while (milliseconds > 0U)
    {
        DelayMicroseconds(1000U);
        milliseconds--;
    }
}

static void UART_SendChar(char character)
{
    UARTCharPut(UART0_BASE, character);
}

static void UART_SendString(const char *text)
{
    while (*text != '\0')
    {
        UART_SendChar(*text);
        text++;
    }
}

static void UART_SendUnsignedInteger(uint32_t value)
{
    char digits[10];
    uint32_t digitCount = 0U;

    if (value == 0U)
    {
        UART_SendChar('0');
        return;
    }

    while (value > 0U)
    {
        digits[digitCount] = (char)('0' + (value % 10U));
        digitCount++;
        value /= 10U;
    }

    while (digitCount > 0U)
    {
        digitCount--;
        UART_SendChar(digits[digitCount]);
    }
}

static void UART0_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0))
    {
    }

    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA))
    {
    }

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);

    GPIOPinTypeUART(
        GPIO_PORTA_BASE,
        GPIO_PIN_0 | GPIO_PIN_1
    );

    UARTConfigSetExpClk(
        UART0_BASE,
        SysCtlClockGet(),
        115200U,
        UART_CONFIG_WLEN_8 |
        UART_CONFIG_STOP_ONE |
        UART_CONFIG_PAR_NONE
    );
}
static void I2C0_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_I2C0))
    {
    }

    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB))
    {
    }

    GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    GPIOPinConfigure(GPIO_PB3_I2C0SDA);

    GPIOPinTypeI2CSCL(
        GPIO_PORTB_BASE,
        GPIO_PIN_2
    );

    GPIOPinTypeI2C(
        GPIO_PORTB_BASE,
        GPIO_PIN_3
    );

    I2CMasterInitExpClk(
        OLED_I2C_BASE,
        SysCtlClockGet(),
        false
    );
}
static bool I2C_DeviceResponds(uint8_t address)
{
    uint32_t errorStatus;

    I2CMasterSlaveAddrSet(
        OLED_I2C_BASE,
        address,
        false
    );

    I2CMasterDataPut(
        OLED_I2C_BASE,
        0x00U
    );

    I2CMasterControl(
        OLED_I2C_BASE,
        I2C_MASTER_CMD_SINGLE_SEND
    );

    while (I2CMasterBusy(OLED_I2C_BASE))
    {
    }

    errorStatus = I2CMasterErr(OLED_I2C_BASE);

    return errorStatus == I2C_MASTER_ERR_NONE;
}

static void OLED_GetGlyph(char character, uint8_t glyph[5])
{
    uint32_t index;

    for (index = 0U; index < 5U; index++)
    {
        glyph[index] = 0x00U;
    }

    switch (character)
    {
        case '0': glyph[0]=0x3EU; glyph[1]=0x51U; glyph[2]=0x49U; glyph[3]=0x45U; glyph[4]=0x3EU; break;
        case '1': glyph[0]=0x00U; glyph[1]=0x42U; glyph[2]=0x7FU; glyph[3]=0x40U; glyph[4]=0x00U; break;
        case '2': glyph[0]=0x42U; glyph[1]=0x61U; glyph[2]=0x51U; glyph[3]=0x49U; glyph[4]=0x46U; break;
        case '3': glyph[0]=0x21U; glyph[1]=0x41U; glyph[2]=0x45U; glyph[3]=0x4BU; glyph[4]=0x31U; break;
        case '4': glyph[0]=0x18U; glyph[1]=0x14U; glyph[2]=0x12U; glyph[3]=0x7FU; glyph[4]=0x10U; break;
        case '5': glyph[0]=0x27U; glyph[1]=0x45U; glyph[2]=0x45U; glyph[3]=0x45U; glyph[4]=0x39U; break;
        case '6': glyph[0]=0x3CU; glyph[1]=0x4AU; glyph[2]=0x49U; glyph[3]=0x49U; glyph[4]=0x30U; break;
        case '7': glyph[0]=0x01U; glyph[1]=0x71U; glyph[2]=0x09U; glyph[3]=0x05U; glyph[4]=0x03U; break;
        case '8': glyph[0]=0x36U; glyph[1]=0x49U; glyph[2]=0x49U; glyph[3]=0x49U; glyph[4]=0x36U; break;
        case '9': glyph[0]=0x06U; glyph[1]=0x49U; glyph[2]=0x49U; glyph[3]=0x29U; glyph[4]=0x1EU; break;

        case 'A': glyph[0]=0x7EU; glyph[1]=0x11U; glyph[2]=0x11U; glyph[3]=0x11U; glyph[4]=0x7EU; break;
        case 'C': glyph[0]=0x3EU; glyph[1]=0x41U; glyph[2]=0x41U; glyph[3]=0x41U; glyph[4]=0x22U; break;
        case 'D': glyph[0]=0x7FU; glyph[1]=0x41U; glyph[2]=0x41U; glyph[3]=0x22U; glyph[4]=0x1CU; break;
        case 'E': glyph[0]=0x7FU; glyph[1]=0x49U; glyph[2]=0x49U; glyph[3]=0x49U; glyph[4]=0x41U; break;
        case 'G': glyph[0]=0x3EU; glyph[1]=0x41U; glyph[2]=0x49U; glyph[3]=0x49U; glyph[4]=0x7AU; break;
        case 'H': glyph[0]=0x7FU; glyph[1]=0x08U; glyph[2]=0x08U; glyph[3]=0x08U; glyph[4]=0x7FU; break;
        case 'I': glyph[0]=0x00U; glyph[1]=0x41U; glyph[2]=0x7FU; glyph[3]=0x41U; glyph[4]=0x00U; break;
        case 'L': glyph[0]=0x7FU; glyph[1]=0x40U; glyph[2]=0x40U; glyph[3]=0x40U; glyph[4]=0x40U; break;
        case 'M': glyph[0]=0x7FU; glyph[1]=0x02U; glyph[2]=0x0CU; glyph[3]=0x02U; glyph[4]=0x7FU; break;
        case 'N': glyph[0]=0x7FU; glyph[1]=0x04U; glyph[2]=0x08U; glyph[3]=0x10U; glyph[4]=0x7FU; break;
        case 'O': glyph[0]=0x3EU; glyph[1]=0x41U; glyph[2]=0x41U; glyph[3]=0x41U; glyph[4]=0x3EU; break;
        case 'P': glyph[0]=0x7FU; glyph[1]=0x09U; glyph[2]=0x09U; glyph[3]=0x09U; glyph[4]=0x06U; break;
        case 'R': glyph[0]=0x7FU; glyph[1]=0x09U; glyph[2]=0x19U; glyph[3]=0x29U; glyph[4]=0x46U; break;
        case 'S': glyph[0]=0x46U; glyph[1]=0x49U; glyph[2]=0x49U; glyph[3]=0x49U; glyph[4]=0x31U; break;
        case 'T': glyph[0]=0x01U; glyph[1]=0x01U; glyph[2]=0x7FU; glyph[3]=0x01U; glyph[4]=0x01U; break;
        case 'U': glyph[0]=0x3FU; glyph[1]=0x40U; glyph[2]=0x40U; glyph[3]=0x40U; glyph[4]=0x3FU; break;
        case 'V': glyph[0]=0x1FU; glyph[1]=0x20U; glyph[2]=0x40U; glyph[3]=0x20U; glyph[4]=0x1FU; break;

        case ':': glyph[0]=0x00U; glyph[1]=0x36U; glyph[2]=0x36U; glyph[3]=0x00U; glyph[4]=0x00U; break;
        case '%': glyph[0]=0x63U; glyph[1]=0x13U; glyph[2]=0x08U; glyph[3]=0x64U; glyph[4]=0x63U; break;
        case '-': glyph[0]=0x08U; glyph[1]=0x08U; glyph[2]=0x08U; glyph[3]=0x08U; glyph[4]=0x08U; break;
        case ' ': default: break;
    }
}

static bool OLED_SetWindow(uint8_t startColumn, uint8_t endColumn, uint8_t page)
{
    if (!OLED_SendCommand(0x21U)) return false;
    if (!OLED_SendCommand(startColumn)) return false;
    if (!OLED_SendCommand(endColumn)) return false;

    if (!OLED_SendCommand(0x22U)) return false;
    if (!OLED_SendCommand(page)) return false;
    if (!OLED_SendCommand(page)) return false;

    return true;
}

static bool OLED_ClearLine(uint8_t page)
{
    uint32_t column;

    if (!OLED_SetWindow(0U, 127U, page))
    {
        return false;
    }

    for (column = 0U; column < 128U; column++)
    {
        if (!OLED_SendData(0x00U))
        {
            return false;
        }
    }

    return true;
}

static bool OLED_WriteChar(uint8_t page, uint8_t column, char character)
{
    uint8_t glyph[5];
    uint32_t index;

    if (column > 122U)
    {
        return false;
    }

    OLED_GetGlyph(character, glyph);

    if (!OLED_SetWindow(column, (uint8_t)(column + 5U), page))
    {
        return false;
    }

    for (index = 0U; index < 5U; index++)
    {
        if (!OLED_SendData(glyph[index]))
        {
            return false;
        }
    }

    return OLED_SendData(0x00U);
}

static bool OLED_WriteString(uint8_t page, uint8_t column, const char *text)
{
    while ((*text != '\0') && (column <= 122U))
    {
        if (!OLED_WriteChar(page, column, *text))
        {
            return false;
        }

        column = (uint8_t)(column + 6U);
        text++;
    }

    return true;
}

static bool OLED_WriteUnsignedInteger(
    uint8_t page,
    uint8_t column,
    uint32_t value
)
{
    char digits[10];
    uint32_t digitCount = 0U;

    if (value == 0U)
    {
        return OLED_WriteChar(page, column, '0');
    }

    while (value > 0U)
    {
        digits[digitCount] = (char)('0' + (value % 10U));
        digitCount++;
        value /= 10U;
    }

    while (digitCount > 0U)
    {
        digitCount--;

        if (!OLED_WriteChar(page, column, digits[digitCount]))
        {
            return false;
        }

        column = (uint8_t)(column + 6U);
    }

    return true;
}

static bool OLED_UpdateEnvironment(
    const DHT11_Data *dhtData,
    DHT_Status dhtStatus,
    uint32_t lightPercent
)
{
    if (!OLED_ClearLine(0U)) return false;
    if (!OLED_ClearLine(2U)) return false;
    if (!OLED_ClearLine(4U)) return false;
    if (!OLED_ClearLine(6U)) return false;

    if (!OLED_WriteString(0U, 0U, "SMART ENV")) return false;

    if (dhtStatus == DHT_STATUS_OK)
    {
        if (!OLED_WriteString(2U, 0U, "TEMP:")) return false;
        if (!OLED_WriteUnsignedInteger(2U, 36U, dhtData->temperature)) return false;
        if (!OLED_WriteString(2U, 54U, " C")) return false;

        if (!OLED_WriteString(4U, 0U, "HUM:")) return false;
        if (!OLED_WriteUnsignedInteger(4U, 30U, dhtData->humidity)) return false;
        if (!OLED_WriteString(4U, 48U, " %")) return false;
    }
    else
    {
        if (!OLED_WriteString(2U, 0U, "DHT ERROR")) return false;
    }

    if (!OLED_WriteString(6U, 0U, "LIGHT:")) return false;
    if (!OLED_WriteUnsignedInteger(6U, 42U, lightPercent)) return false;
    if (!OLED_WriteString(6U, 60U, " %")) return false;

    return true;
}

static void ADC0_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0))
    {
    }

    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE))
    {
    }

    GPIOPinTypeADC(
        GPIO_PORTE_BASE,
        GPIO_PIN_3
    );

    ADCSequenceDisable(
        ADC0_BASE,
        3
    );

    ADCSequenceConfigure(
        ADC0_BASE,
        3,
        ADC_TRIGGER_PROCESSOR,
        0
    );

    ADCSequenceStepConfigure(
        ADC0_BASE,
        3,
        0,
        ADC_CTL_CH0 |
        ADC_CTL_IE |
        ADC_CTL_END
    );

    ADCSequenceEnable(
        ADC0_BASE,
        3
    );

    ADCIntClear(
        ADC0_BASE,
        3
    );
}

static uint32_t ADC0_ReadAIN0(void)
{
    uint32_t adcValue;

    ADCProcessorTrigger(
        ADC0_BASE,
        3
    );

    while (!ADCIntStatus(
        ADC0_BASE,
        3,
        false
    ))
    {
    }

    ADCIntClear(
        ADC0_BASE,
        3
    );

    ADCSequenceDataGet(
        ADC0_BASE,
        3,
        &adcValue
    );

    return adcValue;
}

static uint32_t LDR_ReadAverage(void)
{
    uint32_t sampleIndex;
    uint32_t sampleSum = 0U;

    for (sampleIndex = 0U; sampleIndex < 16U; sampleIndex++)
    {
        sampleSum += ADC0_ReadAIN0();
    }

    return sampleSum / 16U;
}

static uint32_t LDR_ConvertRawToPercent(uint32_t rawValue)
{
    if (rawValue <= LDR_RAW_BRIGHT)
    {
        return 100U;
    }

    if (rawValue >= LDR_RAW_DARK)
    {
        return 0U;
    }

    return ((LDR_RAW_DARK - rawValue) * 100U) /
           (LDR_RAW_DARK - LDR_RAW_BRIGHT);
}

static void DHT11_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD))
    {
    }

    GPIOPinTypeGPIOInput(
        DHT_GPIO_BASE,
        DHT_GPIO_PIN
    );
}

static bool DHT11_WaitWhileLevel(
    uint32_t level,
    uint32_t timeoutMicroseconds
)
{
    while ((GPIOPinRead(DHT_GPIO_BASE, DHT_GPIO_PIN) != 0U) == level)
    {
        if (timeoutMicroseconds == 0U)
        {
            return false;
        }

        DelayMicroseconds(1U);
        timeoutMicroseconds--;
    }

    return true;
}

static DHT_Status DHT11_Read(DHT11_Data *data)
{
    uint8_t bytes[5] = {0U, 0U, 0U, 0U, 0U};
    uint32_t bitIndex;
    uint8_t checksum;

    GPIOPinTypeGPIOOutput(
        DHT_GPIO_BASE,
        DHT_GPIO_PIN
    );

    GPIOPinWrite(
        DHT_GPIO_BASE,
        DHT_GPIO_PIN,
        0U
    );

    DelayMilliseconds(20U);

    GPIOPinWrite(
        DHT_GPIO_BASE,
        DHT_GPIO_PIN,
        DHT_GPIO_PIN
    );

    DelayMicroseconds(30U);

    GPIOPinTypeGPIOInput(
        DHT_GPIO_BASE,
        DHT_GPIO_PIN
    );

    if (!DHT11_WaitWhileLevel(true, 100U))
    {
        return DHT_STATUS_RESPONSE_TIMEOUT;
    }

    if (!DHT11_WaitWhileLevel(false, 100U))
    {
        return DHT_STATUS_RESPONSE_TIMEOUT;
    }

    if (!DHT11_WaitWhileLevel(true, 100U))
    {
        return DHT_STATUS_RESPONSE_TIMEOUT;
    }

    for (bitIndex = 0U; bitIndex < 40U; bitIndex++)
    {

    if (!DHT11_WaitWhileLevel(false, 100U))
    {
        return DHT_STATUS_DATA_TIMEOUT;
    }

    DelayMicroseconds(40U);

    bytes[bitIndex / 8U] <<= 1U;

    if (GPIOPinRead(DHT_GPIO_BASE, DHT_GPIO_PIN) != 0U)
    {
        bytes[bitIndex / 8U] |= 1U;
    }

    if (GPIOPinRead(DHT_GPIO_BASE, DHT_GPIO_PIN) != 0U)
    {
        if (!DHT11_WaitWhileLevel(true, 100U))
        {
            return DHT_STATUS_DATA_TIMEOUT;
        }
    }
}

    checksum = (uint8_t)(
        bytes[0] +
        bytes[1] +
        bytes[2] +
        bytes[3]
    );

    if (checksum != bytes[4])
    {
        return DHT_STATUS_CHECKSUM_ERROR;
    }

    if ((bytes[0] == 0U) &&
    (bytes[1] == 0U) &&
    (bytes[2] == 0U) &&
    (bytes[3] == 0U))
{
    return DHT_STATUS_DATA_TIMEOUT;
}

    data->humidity = bytes[0];
    data->temperature = bytes[2];

    return DHT_STATUS_OK;
}

static bool OLED_I2CWriteByte(uint8_t controlByte, uint8_t value)
{
    uint32_t errorStatus;

    I2CMasterSlaveAddrSet(
        OLED_I2C_BASE,
        OLED_I2C_ADDRESS,
        false
    );

    I2CMasterDataPut(
        OLED_I2C_BASE,
        controlByte
    );

    I2CMasterControl(
        OLED_I2C_BASE,
        I2C_MASTER_CMD_BURST_SEND_START
    );

    while (I2CMasterBusy(OLED_I2C_BASE))
    {
    }

    errorStatus = I2CMasterErr(OLED_I2C_BASE);

    if (errorStatus != I2C_MASTER_ERR_NONE)
    {
        return false;
    }

    I2CMasterDataPut(
        OLED_I2C_BASE,
        value
    );

    I2CMasterControl(
        OLED_I2C_BASE,
        I2C_MASTER_CMD_BURST_SEND_FINISH
    );

    while (I2CMasterBusy(OLED_I2C_BASE))
    {
    }

    errorStatus = I2CMasterErr(OLED_I2C_BASE);

    return errorStatus == I2C_MASTER_ERR_NONE;
}

static bool OLED_SendCommand(uint8_t command)
{
    return OLED_I2CWriteByte(0x00U, command);
}

static bool OLED_SendData(uint8_t data)
{
    return OLED_I2CWriteByte(0x40U, data);
}

static bool OLED_Init(void)
{
    DelayMilliseconds(100U);

    if (!OLED_SendCommand(0xAEU)) return false;

    if (!OLED_SendCommand(0xD5U)) return false;
    if (!OLED_SendCommand(0x80U)) return false;

    if (!OLED_SendCommand(0xA8U)) return false;
    if (!OLED_SendCommand(0x3FU)) return false;

    if (!OLED_SendCommand(0xD3U)) return false;
    if (!OLED_SendCommand(0x00U)) return false;

    if (!OLED_SendCommand(0x40U)) return false;

    if (!OLED_SendCommand(0x8DU)) return false;
    if (!OLED_SendCommand(0x14U)) return false;

    if (!OLED_SendCommand(0x20U)) return false;
    if (!OLED_SendCommand(0x00U)) return false;

    if (!OLED_SendCommand(0xA1U)) return false;
    if (!OLED_SendCommand(0xC8U)) return false;

    if (!OLED_SendCommand(0xDAU)) return false;
    if (!OLED_SendCommand(0x12U)) return false;

    if (!OLED_SendCommand(0x81U)) return false;
    if (!OLED_SendCommand(0x7FU)) return false;

    if (!OLED_SendCommand(0xD9U)) return false;
    if (!OLED_SendCommand(0xF1U)) return false;

    if (!OLED_SendCommand(0xDBU)) return false;
    if (!OLED_SendCommand(0x40U)) return false;

    if (!OLED_SendCommand(0xA4U)) return false;
    if (!OLED_SendCommand(0xA6U)) return false;
    if (!OLED_SendCommand(0xAFU)) return false;

    return true;
}

static bool OLED_SelectFullScreen(void)
{
    if (!OLED_SendCommand(0x21U)) return false;
    if (!OLED_SendCommand(0x00U)) return false;
    if (!OLED_SendCommand(0x7FU)) return false;

    if (!OLED_SendCommand(0x22U)) return false;
    if (!OLED_SendCommand(0x00U)) return false;
    if (!OLED_SendCommand(0x07U)) return false;

    return true;
}

static bool OLED_Fill(uint8_t pattern)
{
    uint32_t byteIndex;

    if (!OLED_SelectFullScreen())
    {
        return false;
    }

    for (byteIndex = 0U; byteIndex < 1024U; byteIndex++)
    {
        if (!OLED_SendData(pattern))
        {
            return false;
        }
    }

    return true;
}

int main(void)
{
    uint32_t ldrRawValue;
    uint32_t lightPercent;

    DHT11_Data dhtData;
    DHT_Status dhtStatus;

    SysCtlClockSet(
        SYSCTL_SYSDIV_5 |
        SYSCTL_USE_PLL |
        SYSCTL_XTAL_16MHZ |
        SYSCTL_OSC_MAIN
    );

    UART0_Init();
    ADC0_Init();
    DHT11_Init();
    I2C0_Init();

    UART_SendString("\r\nSystem started\r\n");
    UART_SendString("Environment sensor test started.\r\n");
    UART_SendString("Checking OLED I2C address...\r\n");

    if (I2C_DeviceResponds(OLED_I2C_ADDRESS))
{
    UART_SendString("OLED detected at address 0x3C.\r\n");

    if (OLED_Init())
    {
        UART_SendString("OLED initialization successful.\r\n");

        if (OLED_Fill(0x00U))
        {
            UART_SendString(
                "OLED cleared and ready for live data.\r\n"
            );
        }
        else
        {
            UART_SendString(
                "OLED clear failed.\r\n"
            );
        }
    }
    else
    {
        UART_SendString(
            "OLED initialization failed.\r\n"
        );
    }
}
else
{
    UART_SendString(
        "OLED not detected at address 0x3C.\r\n"
    );
}

    while (1)
    {
        ldrRawValue = LDR_ReadAverage();
        lightPercent = LDR_ConvertRawToPercent(ldrRawValue);

        dhtStatus = DHT11_Read(&dhtData);

        if (!OLED_UpdateEnvironment(
            &dhtData,
            dhtStatus,
            lightPercent
        ))
        {
            UART_SendString("OLED update error.\r\n");
        }

        if (dhtStatus == DHT_STATUS_OK)
        {
            UART_SendString("Temperature: ");
            UART_SendUnsignedInteger(dhtData.temperature);
            UART_SendString(" C");

            UART_SendString(" | Humidity: ");
            UART_SendUnsignedInteger(dhtData.humidity);
            UART_SendString(" %");
        }
        else
        {
            UART_SendString("DHT11 Error: ");

            if (dhtStatus == DHT_STATUS_RESPONSE_TIMEOUT)
            {
                UART_SendString("Response timeout");
            }
            else if (dhtStatus == DHT_STATUS_DATA_TIMEOUT)
            {
                UART_SendString("Data timeout");
            }
            else if (dhtStatus == DHT_STATUS_CHECKSUM_ERROR)
            {
                UART_SendString("Checksum error");
            }
            else
            {
                UART_SendString("Unknown error");
            }
        }

        UART_SendString(" | Light: ");
        UART_SendUnsignedInteger(lightPercent);
        UART_SendString(" %");

        UART_SendString(" | LDR Raw: ");
        UART_SendUnsignedInteger(ldrRawValue);

        UART_SendString("\r\n");

        DelayMilliseconds(2000U);
    }
}