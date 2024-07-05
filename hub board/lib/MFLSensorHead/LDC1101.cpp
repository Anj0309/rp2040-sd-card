#include "LDC1101.h"

SPISettings SensorSPI(10000000, MSBFIRST, SPI_MODE0);
Debugger::Debugger(bool enabled) : enabled(enabled) {}

void Debugger::setEnabled(bool en)
{
    enabled = en;
}

void Debugger::print(int value)
{
    if (enabled)
    {
        Serial.print(value);
    }
}

void Debugger::print(unsigned int value)
{
    if (enabled)
    {
        Serial.print(value);
    }
}

void Debugger::print(byte value)
{
    if (enabled)
    {
        Serial.print(value);
    }
}

void Debugger::print(int value, int format)
{
    if (enabled)
    {
        Serial.print(value, format);
    }
}

void Debugger::print(unsigned int value, int format)
{
    if (enabled)
    {
        Serial.print(value, format);
    }
}

void Debugger::print(float value, int Digits)
{
    if (enabled)
    {
        Serial.print(value, Digits);
    }
}

void Debugger::print(byte value, int format)
{
    if (enabled)
    {
        Serial.print(value, format);
    }
}

void Debugger::print(unsigned long value)
{
    if (enabled)
    {
        Serial.print(value);
    }
}

void Debugger::println(unsigned long value)
{
    if (enabled)
    {
        Serial.println(value);
    }
}

void Debugger::println(int value)
{
    if (enabled)
    {
        Serial.println(value);
    }
}

void Debugger::println(unsigned int value)
{
    if (enabled)
    {
        Serial.println(value);
    }
}

void Debugger::println(byte value)
{
    if (enabled)
    {
        Serial.println(value);
    }
}

void Debugger::println(int value, int format)
{
    if (enabled)
    {
        Serial.println(value, format);
    }
}

void Debugger::println(unsigned int value, int format)
{
    if (enabled)
    {
        Serial.println(value, format);
    }
}

void Debugger::println(byte value, int format)
{
    if (enabled)
    {
        Serial.println(value, format);
    }
}

void Debugger::print(const String &message)
{
    if (enabled)
    {
        Serial.print(message);
    }
}

void Debugger::println(const String &message)
{
    if (enabled)
    {
        Serial.println(message);
    }
}

MFLSensors::MFLSensors() : debug(false)
{

    int numLDC1101 = 0;
    int numHallEffect = 0;
}

void MFLSensors::enableDebugging(bool en)
{
    debug.setEnabled(en);
}

void MFLSensors::initSPI(const int TotalLDC, const int TotalTMAG,const int LDCpins[], const int TMAGpin[])
{
    totalLDC1101 = TotalLDC;
    totalTMAG5170 = TotalTMAG;
    for (int i = 0; i < TotalLDC; i++)
    {
        LDC_CSpin[i] = LDCpins[i];
    }
    for (int i = 0; i < TotalTMAG; i++)
    {
        TMAG_CSpin[i] = TMAGpin[i];
    }
    SPI1.setRX(MISO_PIN);
    SPI1.setSCK(SCK_PIN);
    SPI1.setTX(MOSI_PIN);
    SPI1.begin(false);
    SR.LVC595_init();
}

void MFLSensors::LDC1101(int CS)
{
    if (numLDC1101 < totalLDC1101)
    {
        LDC_CSpin[numLDC1101++] = CS;
        pinMode(CS, OUTPUT);
        digitalWrite(CS, HIGH);
        // Additional initialization for LDC1101 as needed
    }
}

void MFLSensors::TMAG5170(int CS)
{
    if (numTMAG5170 < totalTMAG5170)
    {
        TMAG_CSpin[numTMAG5170++] = CS;
        pinMode(CS, OUTPUT);
        digitalWrite(CS, HIGH);
        // Additional initialization for Hall Effect sensor as needed
    }
}

// Check Communication with all sensors
bool MFLSensors::CheckSensors()
{
    SPI1.beginTransaction(SensorSPI);
    bool SensorOK = 0;
    SR.SelectCS(LDC_CSpin[0] - 1);

    // LDC Sensors
    debug.println("LDC1101 COM check:- ");
    for (int i = 0; i < totalLDC1101; i++)
    {
        // //digitalWrite(LDC_CSpin[i], LOW);
        SR.ShiftCS();
        SPI1.transfer(readReg | LDC1101_REG_CHIP_ID);
        // SPI1.transfer(0x00) == 0xD4 ? SensorOK = true : SensorOK = false;
        //  Checks LDC Chip ID, should be D4 at 3F address
        //  //digitalWrite(LDC_CSpin[i], HIGH);
        debug.println(SPI1.transfer(0x00), HEX);
        // debug.println(SensorOK);
        /* if (!SensorOK)
         {
             SPI1.endTransaction();
             return SensorOK;
         }*/
    }
    // TMAG Sensors
    debug.println("TMAG COM check:- ");
    for (int i = 0; i < totalTMAG5170; i++)
    {
        // Disable CRC
        // //digitalWrite(TMAG_CSpin[i], LOW);

        SR.SelectCS(TMAG_CSpin[i]);
        SPI1.transfer(writeReg | TEST_CONFIG);
        SPI1.transfer(0x00);
        SPI1.transfer(CRC_DIS_CRCdisabled);
        SPI1.transfer(0x07);
        // //digitalWrite(TMAG_CSpin[i], HIGH);
        SR.High_all_CS();
        delayMicroseconds(1);
        // Check if CRC Disabled and also get device ID
        // //digitalWrite(TMAG_CSpin[i], LOW);
        SR.SelectCS(TMAG_CSpin[i]);
        SPI1.transfer(readReg | TEST_CONFIG);
        SPI1.transfer(0x00);
        // SPI1.transfer(0x00) == 0x54 ? SensorOK = true : SensorOK = false;
        debug.println(SPI1.transfer(0x00), HEX);
        SPI1.transfer(0x00);
        // //digitalWrite(TMAG_CSpin[i], HIGH);
        SR.High_all_CS();
        // debug.println(SensorOK);
        /*if (!SensorOK)
        {
            SPI1.endTransaction();
            return SensorOK;
        }*/
    }
    SPI1.endTransaction();
    return SensorOK;
}

// Configure and Diagnose all Sensors
void MFLSensors::ConfigureSensors()
{
    // debug.setEnabled(1);
    debug.println("Configuring Sensors - LDC1101s");
    SPI1.beginTransaction(SensorSPI);
    bool SensorOK = 0;
    // LDC Sensors
    for (int i = 0; i < totalLDC1101; i++)
    {
        debug.print("LDC1101 ");
        debug.println(i);
        // Configure LDC Sensors and Diagnose them
        for (byte RegAddress = 0x01; RegAddress <= 0x3F; RegAddress++)
        {

            switch (RegAddress)
            {
            case /*0x01*/ LDC1101_REG_CFG_RP_MEASUREMENT_DYNAMIC_RANGE:
                /* code */
                debug.print("0x");
                debug.print(RegAddress, HEX);
                // //digitalWrite(LDC_CSpin[i], LOW);
                SR.SelectCS(LDC_CSpin[i]);
                SPI1.transfer(writeReg | RegAddress);
                SPI1.transfer(0x34); // 27
                // //digitalWrite(LDC_CSpin[i], HIGH);
                SR.High_all_CS();
                delayMicroseconds(1);
                // //digitalWrite(LDC_CSpin[i], LOW);
                SR.SelectCS(LDC_CSpin[i]);
                SPI1.transfer(readReg | RegAddress);
                debug.println((SPI1.transfer(0x00) == 0x34) ? " - OK" : " - Error");
                // //digitalWrite(LDC_CSpin[i], HIGH);
                SR.High_all_CS();

                break;

            case /*0x02*/ LDC1101_REG_CFG_INTERNAL_TIME_CONSTANT_1:
                /* code */
                debug.print("0x");
                debug.print(RegAddress, HEX);
                // //digitalWrite(LDC_CSpin[i], LOW);
                SR.SelectCS(LDC_CSpin[i]);
                SPI1.transfer(writeReg | RegAddress);
                SPI1.transfer(0xDE); // D0
                // //digitalWrite(LDC_CSpin[i], HIGH);
                SR.High_all_CS();
                delayMicroseconds(1);
                // //digitalWrite(LDC_CSpin[i], LOW);
                SR.SelectCS(LDC_CSpin[i]);
                SPI1.transfer(readReg | RegAddress);
                debug.println((SPI1.transfer(0x00) == 0xDE) ? " - OK" : " - Error");
                // //digitalWrite(LDC_CSpin[i], HIGH);
                SR.High_all_CS();
                break;

            case /*0x03*/ LDC1101_REG_CFG_INTERNAL_TIME_CONSTANT_2:
                /* code */
                debug.print("0x");
                debug.print(RegAddress, HEX);
                // //digitalWrite(LDC_CSpin[i], LOW);
                SR.SelectCS(LDC_CSpin[i]);
                SPI1.transfer(writeReg | RegAddress);
                SPI1.transfer(0xF9); // E0
                // //digitalWrite(LDC_CSpin[i], HIGH);
                SR.High_all_CS();
                delayMicroseconds(1);
                // //digitalWrite(LDC_CSpin[i], LOW);
                SR.SelectCS(LDC_CSpin[i]);
                SPI1.transfer(readReg | RegAddress);
                debug.println((SPI1.transfer(0x00) == 0xF9) ? " - OK" : " - Error");
                // //digitalWrite(LDC_CSpin[i], HIGH);
                SR.High_all_CS();
                break;

            case /*0x04*/ LDC1101_REG_CFG_RPL_CONVERSION_INTERVAL:
                /* code */
                debug.print("0x");
                debug.print(RegAddress, HEX);
                // //digitalWrite(LDC_CSpin[i], LOW);
                SR.SelectCS(LDC_CSpin[i]);
                SPI1.transfer(writeReg | RegAddress);
                SPI1.transfer(0xE2); // 04
                // //digitalWrite(LDC_CSpin[i], HIGH);
                SR.High_all_CS();
                delayMicroseconds(1);
                // //digitalWrite(LDC_CSpin[i], LOW);
                SR.SelectCS(LDC_CSpin[i]);
                SPI1.transfer(readReg | RegAddress);
                debug.println((SPI1.transfer(0x00) == 0xE2) ? " - OK" : " - Error");
                // //digitalWrite(LDC_CSpin[i], HIGH);
                SR.High_all_CS();
                break;

            case /*0x05*/ LDC1101_REG_CFG_ADDITIONAL_DEVICE:
                /* code */
                debug.print("0x");
                debug.print(RegAddress, HEX);
                // //digitalWrite(LDC_CSpin[i], LOW);
                SR.SelectCS(LDC_CSpin[i]);
                SPI1.transfer(writeReg | RegAddress);
                SPI1.transfer(0x01);
                // //digitalWrite(LDC_CSpin[i], HIGH);
                SR.High_all_CS();
                delayMicroseconds(1);
                // //digitalWrite(LDC_CSpin[i], LOW);
                SR.SelectCS(LDC_CSpin[i]);
                SPI1.transfer(readReg | RegAddress);
                debug.println((SPI1.transfer(0x00) == 0x01) ? " - OK" : " - Error");
                // //digitalWrite(LDC_CSpin[i], HIGH);
                SR.High_all_CS();
                break;

            case /*0x0C*/ LDC1101_REG_AMPLITUDE_CONTROL_REQUIREMENT:
                /* code */
                debug.print("0x");
                debug.print(RegAddress, HEX);
                // //digitalWrite(LDC_CSpin[i], LOW);
                SR.SelectCS(LDC_CSpin[i]);
                SPI1.transfer(writeReg | RegAddress);
                SPI1.transfer(0x01);
                // //digitalWrite(LDC_CSpin[i], HIGH);
                SR.High_all_CS();
                delayMicroseconds(1);
                // //digitalWrite(LDC_CSpin[i], LOW);
                SR.SelectCS(LDC_CSpin[i]);
                SPI1.transfer(readReg | RegAddress);
                debug.println((SPI1.transfer(0x00) == 0x01) ? " - OK" : " - Error");
                // //digitalWrite(LDC_CSpin[i], HIGH);
                SR.High_all_CS();
                break;

            case /*0x30*/ LDC1101_REGLHR_RCOUNTLSB:
                /* code */
                debug.print("0x");
                debug.print(RegAddress, HEX);
                // //digitalWrite(LDC_CSpin[i], LOW);
                SR.SelectCS(LDC_CSpin[i]);
                SPI1.transfer(writeReg | RegAddress);
                SPI1.transfer(0x32); // 40
                // //digitalWrite(LDC_CSpin[i], HIGH);
                SR.High_all_CS();
                delayMicroseconds(1);
                // //digitalWrite(LDC_CSpin[i], LOW);
                SR.SelectCS(LDC_CSpin[i]);
                SPI1.transfer(readReg | RegAddress);
                debug.println((SPI1.transfer(0x00) == 0x32) ? " - OK" : " - Error");
                // //digitalWrite(LDC_CSpin[i], HIGH);
                SR.High_all_CS();
                break;

            case /*0x31*/ LDC1101_REGLHR_RCOUNT_MSB:
                /* code */
                debug.print("0x");
                debug.print(RegAddress, HEX);
                // //digitalWrite(LDC_CSpin[i], LOW);
                SR.SelectCS(LDC_CSpin[i]);
                SPI1.transfer(writeReg | RegAddress);
                SPI1.transfer(0x00);
                // //digitalWrite(LDC_CSpin[i], HIGH);
                SR.High_all_CS();
                delayMicroseconds(1);
                // //digitalWrite(LDC_CSpin[i], LOW);
                SR.SelectCS(LDC_CSpin[i]);
                SPI1.transfer(readReg | RegAddress);
                debug.println((SPI1.transfer(0x00) == 0x00) ? " - OK" : " - Error");
                // //digitalWrite(LDC_CSpin[i], HIGH);
                SR.High_all_CS();
                break;

            case /*0x34*/ LDC1101_REG_CFGLHR:
                /* code */
                debug.print("0x");
                debug.print(RegAddress, HEX);
                // //digitalWrite(LDC_CSpin[i], LOW);
                SR.SelectCS(LDC_CSpin[i]);
                SPI1.transfer(writeReg | RegAddress);
                SPI1.transfer(0x01);
                // //digitalWrite(LDC_CSpin[i], HIGH);
                SR.High_all_CS();
                delayMicroseconds(1);
                // //digitalWrite(LDC_CSpin[i], LOW);
                SR.SelectCS(LDC_CSpin[i]);
                SPI1.transfer(readReg | RegAddress);
                debug.println((SPI1.transfer(0x00) == 0x01) ? " - OK" : " - Error");
                // //digitalWrite(LDC_CSpin[i], HIGH);
                SR.High_all_CS();
                break;
            case /*0x32 - special case as LSB can only be programmed after msb*/ LDC1101_REGLHR_OFFSETLSB:
                /* code */
                debug.print("0x");
                debug.print(LDC1101_REGLHR_RCOUNTLSB, HEX);
                // //digitalWrite(LDC_CSpin[i], LOW);
                SR.SelectCS(LDC_CSpin[i]);
                SPI1.transfer(writeReg | LDC1101_REGLHR_RCOUNTLSB);
                SPI1.transfer(0x32); // 40
                // //digitalWrite(LDC_CSpin[i], HIGH);
                SR.High_all_CS();
                delayMicroseconds(1);
                // //digitalWrite(LDC_CSpin[i], LOW);
                SR.SelectCS(LDC_CSpin[i]);
                SPI1.transfer(readReg | LDC1101_REGLHR_RCOUNTLSB);
                // debug.print(" last");
                debug.println((SPI1.transfer(0x00) == 0x32) ? " - OK" : " - Error");
                // //digitalWrite(LDC_CSpin[i], HIGH);
                SR.High_all_CS();
                break;

            default:
                break;
            }
        }
    }
    debug.println("Configuring Sensors - TMAG5170s");
    // TMAG Sensors --- This ispending
    for (int i = 0; i < totalTMAG5170; i++)
    {
        unsigned int data, recievedData;
        for (byte RegAddress = 0x0; RegAddress < 0x15; RegAddress++)
        {
            switch (RegAddress)
            {
            case /* 0x0 */ DEVICE_CONFIG:
                /* code */
                data = CONV_AVG_1x | MAG_TEMPCO_0R12pd | OPERATING_MODE_ConfigurationMode | T_CH_EN_TempChannelDisabled | T_RATE_sameAsOtherSensors | T_HLT_EN_tempLimitCheckOff | TEMP_COMP_EN_TempCompensationEnabled;
                debug.print("Data 0x");
                debug.println(data, HEX);
                // digitalWrite(TMAG_CSpin[i], LOW);
                SR.SelectCS(TMAG_CSpin[i]);
                SPI1.transfer(writeReg | RegAddress);
                SPI1.transfer((byte)(data >> 8));
                SPI1.transfer((byte)(data & 0x00ff));
                SPI1.transfer(0x00);
                ////digitalWrite(TMAG_CSpin[i], HIGH);
                SR.High_all_CS();
                delayMicroseconds(10);
                // digitalWrite(TMAG_CSpin[i], LOW);
                SR.SelectCS(TMAG_CSpin[i]);
                SPI1.transfer(readReg | RegAddress);
                debug.print((SPI1.transfer(0x00) == (byte)(data >> 8)) ? " OK" : " Error");
                debug.println((SPI1.transfer(0x00) == (byte)(data & 0x00ff)) ? " OK" : " Error");
                SPI1.transfer(0x00);
                ////digitalWrite(TMAG_CSpin[i], HIGH);
                SR.High_all_CS();
                break;
            case /* 0x1 */ SENSOR_CONFIG:
                /* code */
                data = ANGLE_EN_NoAngleCalculation | SLEEPTIME_1ms | MAG_CH_EN_XYZenabled | Z_RANGE_100mT | Y_RANGE_100mT | X_RANGE_100mT;
                debug.print("Data 0x");
                debug.println(data, HEX);
                // digitalWrite(TMAG_CSpin[i], LOW);
                SR.SelectCS(TMAG_CSpin[i]);
                SPI1.transfer(writeReg | RegAddress);
                SPI1.transfer((byte)(data >> 8));
                SPI1.transfer((byte)(data & 0x00ff));
                SPI1.transfer(0x00);
                ////digitalWrite(TMAG_CSpin[i], HIGH);
                SR.High_all_CS();
                delayMicroseconds(10);
                // digitalWrite(TMAG_CSpin[i], LOW);
                SR.SelectCS(TMAG_CSpin[i]);
                SPI1.transfer(readReg | RegAddress);
                debug.print((SPI1.transfer(0x00) == (byte)(data >> 8)) ? " OK" : " Error");
                debug.println((SPI1.transfer(0x00) == (byte)(data & 0x00ff)) ? " OK" : " Error");
                SPI1.transfer(0x00);
                ////digitalWrite(TMAG_CSpin[i], HIGH);
                SR.High_all_CS();
                break;
            case /* 0x2 */ SYSTEM_CONFIG:
                /* code */
                data = DIAG_SEL_enabledDataPath | TRIGGER_MODE_SPI | DATA_TYPE_32bit | DIAG_EN_AFEdiagnosticsDisabled | Z_HLT_EN_ZaxisLimitCheckoff | Y_HLT_EN_YaxisLimitCheckoff | X_HLT_EN_XaxisLimitCheckoff;
                debug.print("Data 0x");
                debug.println(data, HEX);
                // digitalWrite(TMAG_CSpin[i], LOW);
                SR.SelectCS(TMAG_CSpin[i]);
                SPI1.transfer(writeReg | RegAddress);
                SPI1.transfer((byte)(data >> 8));
                SPI1.transfer((byte)(data & 0x00ff));
                SPI1.transfer(0x00);
                // //digitalWrite(TMAG_CSpin[i], HIGH);
                SR.High_all_CS();
                delayMicroseconds(10);
                // digitalWrite(TMAG_CSpin[i], LOW);
                SR.SelectCS(TMAG_CSpin[i]);
                SPI1.transfer(readReg | RegAddress);
                debug.print((SPI1.transfer(0x00) == (byte)(data >> 8)) ? " OK" : " Error");
                debug.println((SPI1.transfer(0x00) == (byte)(data & 0x00ff)) ? " OK" : " Error");
                SPI1.transfer(0x00);
                ////digitalWrite(TMAG_CSpin[i], HIGH);
                SR.High_all_CS();
                break;

            default:
                break;
            }
        }
    }

    SPI1.endTransaction();
}

void MFLSensors::putSensorsInRunMode(bool Special32BitEN)
{
    // Put LDC1101 in Run Mode
    // debug.setEnabled(1);
    SPI1.beginTransaction(SensorSPI);
    debug.println("LDC1101 Run Mode:- ");
    for (int i = 0; i < totalLDC1101; i++)
    {
        debug.print("0x");
        debug.print(LDC1101_REG_CFG_POWER_STATE, HEX);
        ////digitalWrite(LDC_CSpin[i], LOW);
        SR.SelectCS(LDC_CSpin[i]);
        SPI1.transfer(writeReg | LDC1101_REG_CFG_POWER_STATE);
        SPI1.transfer(0x00);
        // digitalWrite(LDC_CSpin[i], HIGH);
        SR.High_all_CS();
        delayMicroseconds(1);
        // digitalWrite(LDC_CSpin[i], LOW);
        SR.SelectCS(LDC_CSpin[i]);
        SPI1.transfer(readReg | LDC1101_REG_CFG_POWER_STATE);
        debug.println((SPI1.transfer(0x00) == 0x00) ? " - OK" : " - Error");
        // digitalWrite(LDC_CSpin[i], HIGH);
        SR.High_all_CS();
    }
    // Put TMAG5170 in Run Mode and in 32 Bit Special Write Mode

    for (int i = 0; i < totalTMAG5170; i++)
    {

        if (Special32BitEN) // Enable or Disable 32 Bit Special Read
        {
            debug.println("TMAG5170 Special Mode");
            unsigned int data;
            data = DIAG_SEL_enabledDataPath | TRIGGER_MODE_SPI | DATA_TYPE_12bit_XZ | DIAG_EN_AFEdiagnosticsDisabled | Z_HLT_EN_ZaxisLimitCheckoff | Y_HLT_EN_YaxisLimitCheckoff | X_HLT_EN_XaxisLimitCheckoff;
            // digitalWrite(TMAG_CSpin[i], LOW);
            SR.SelectCS(TMAG_CSpin[i]);
            SPI1.transfer(writeReg | SYSTEM_CONFIG);
            SPI1.transfer((byte)(data >> 8));
            SPI1.transfer((byte)(data & 0x00ff));
            SPI1.transfer(0x00);
            // //digitalWrite(TMAG_CSpin[i], HIGH);
            SR.High_all_CS();
            delayMicroseconds(1);
            // Check if Run Mode Enable
        }
        else
        {
            debug.println("TMAG5170 Normal Mode");
            unsigned int data;
            data = DIAG_SEL_enabledDataPath | TRIGGER_MODE_SPI | DATA_TYPE_32bit | DIAG_EN_AFEdiagnosticsDisabled | Z_HLT_EN_ZaxisLimitCheckoff | Y_HLT_EN_YaxisLimitCheckoff | X_HLT_EN_XaxisLimitCheckoff;
            // digitalWrite(TMAG_CSpin[i], LOW);
            SR.SelectCS(TMAG_CSpin[i]);
            SPI1.transfer(writeReg | SYSTEM_CONFIG);
            SPI1.transfer((byte)(data >> 8));
            SPI1.transfer((byte)(data & 0x00ff));
            SPI1.transfer(0x00);
            // digitalWrite(TMAG_CSpin[i], HIGH);
            SR.High_all_CS();
            delayMicroseconds(1);
            // Check if Run Mode Enable
            // digitalWrite(TMAG_CSpin[i], LOW);
            SR.SelectCS(TMAG_CSpin[i]);
            SPI1.transfer(readReg | SYSTEM_CONFIG);
            debug.print((SPI1.transfer(0x00) == (byte)(data >> 8)) ? " - OK" : " - Error");
            debug.println((SPI1.transfer(0x00) == (byte)(data & 0x00ff)) ? " - OK" : " - Error");
            SPI1.transfer(0x0F);
            // digitalWrite(TMAG_CSpin[i], HIGH);
            SR.High_all_CS();
        }
        // Put in Run Mode
        debug.println("TMAG5170 Run Mode:- ");
        unsigned int data;
        data = CONV_AVG_1x | MAG_TEMPCO_0R12pd | OPERATING_MODE_activeMeasureMode | T_CH_EN_TempChannelDisabled | T_RATE_sameAsOtherSensors | T_HLT_EN_tempLimitCheckOff | TEMP_COMP_EN_TempCompensationDisenabled;
        // digitalWrite(TMAG_CSpin[i], LOW);
        SR.SelectCS(TMAG_CSpin[i]);
        SPI1.transfer(writeReg | DEVICE_CONFIG);
        SPI1.transfer((byte)(data >> 8));
        SPI1.transfer((byte)(data & 0x00ff));
        SPI1.transfer(0x0F);
        // digitalWrite(TMAG_CSpin[i], HIGH);
        SR.High_all_CS();
    }
    SPI1.endTransaction();
}

void MFLSensors::putSensorsInSleep(bool Special32BitEN)
{
    // Put LDC1101 in Sleep Mode
    // debug.setEnabled(1);
    SPI1.beginTransaction(SensorSPI);
    debug.println("LDC1101 Sleep Mode:- ");
    for (int i = 0; i < totalLDC1101; i++)
    {
        debug.print("0x");
        debug.print(LDC1101_REG_CFG_POWER_STATE, HEX);
        // digitalWrite(LDC_CSpin[i], LOW);
        SR.SelectCS(TMAG_CSpin[i]);
        SPI1.transfer(writeReg | LDC1101_REG_CFG_POWER_STATE);
        SPI1.transfer(0x01);
        // digitalWrite(LDC_CSpin[i], HIGH);
        SR.High_all_CS();
        delayMicroseconds(1);
        // digitalWrite(LDC_CSpin[i], LOW);
        SR.SelectCS(TMAG_CSpin[i]);
        SPI1.transfer(readReg | LDC1101_REG_CFG_POWER_STATE);
        debug.println((SPI1.transfer(0x00) == 0x01) ? " - OK" : " - Error");
        // digitalWrite(LDC_CSpin[i], HIGH);
        SR.High_all_CS();
    }
    // Put TMAG5170 in Run Mode and in 32 Bit Special Write Mode

    for (int i = 0; i < totalTMAG5170; i++)
    {
        debug.println("TMAG5170 Normal Data Mode First");
        unsigned int data;
        data = DIAG_SEL_enabledDataPath | TRIGGER_MODE_SPI | DATA_TYPE_32bit | DIAG_EN_AFEdiagnosticsDisabled | Z_HLT_EN_ZaxisLimitCheckoff | Y_HLT_EN_YaxisLimitCheckoff | X_HLT_EN_XaxisLimitCheckoff;
        // digitalWrite(TMAG_CSpin[i], LOW);
        SR.SelectCS(TMAG_CSpin[i]);
        SPI1.transfer(writeReg | SYSTEM_CONFIG);
        SPI1.transfer((byte)(data >> 8));
        SPI1.transfer((byte)(data & 0x00ff));
        SPI1.transfer(0x00);
        // digitalWrite(TMAG_CSpin[i], HIGH);
        SR.High_all_CS();
        delayMicroseconds(1);
        // Check if Run Mode Enable
        // digitalWrite(TMAG_CSpin[i], LOW);
        SR.SelectCS(TMAG_CSpin[i]);
        SPI1.transfer(readReg | SYSTEM_CONFIG);
        debug.print((SPI1.transfer(0x00) == (byte)(data >> 8)) ? " - OK" : " - Error");
        debug.println((SPI1.transfer(0x00) == (byte)(data & 0x00ff)) ? " - OK" : " - Error");
        SPI1.transfer(0x0F);
        // digitalWrite(TMAG_CSpin[i], HIGH);
        SR.High_all_CS();
        // Put in Run Mode
        debug.println("TMAG5170 Sleep Mode:- ");
        data = CONV_AVG_1x | MAG_TEMPCO_0R12pd | OPERATING_MODE_SleepMode | T_CH_EN_TempChannelDisabled | T_RATE_sameAsOtherSensors | T_HLT_EN_tempLimitCheckOff | TEMP_COMP_EN_TempCompensationDisenabled;
        // digitalWrite(TMAG_CSpin[i], LOW);
        SR.SelectCS(TMAG_CSpin[i]);
        SPI1.transfer(writeReg | DEVICE_CONFIG);
        SPI1.transfer((byte)(data >> 8));
        SPI1.transfer((byte)(data & 0x00ff));
        SPI1.transfer(0x0F);
        // digitalWrite(TMAG_CSpin[i], HIGH);
        SR.High_all_CS();
        delayMicroseconds(1);
        // Check if Run Mode Enable
        // digitalWrite(TMAG_CSpin[i], LOW);
        SR.SelectCS(TMAG_CSpin[i]);
        SPI1.transfer(readReg | DEVICE_CONFIG);
        debug.print((SPI1.transfer(0x00) == (byte)(data >> 8)) ? " - OK" : " - Error");
        debug.println((SPI1.transfer(0x00) == (byte)(data & 0x00ff)) ? " - OK" : " - Error");
        SPI1.transfer(0x0F);
        // digitalWrite(TMAG_CSpin[i], HIGH);
        SR.High_all_CS();
    }
    SPI1.endTransaction();
}

void MFLSensors::liveReadSensors(bool Special32BitEN)
{
    // LDC Live Readout
    // debug.setEnabled(1);
    // SR.SelectCS(LDC_CSpin[Q1]);
    // unsigned long LDCValues[totalLDC1101];
    union c
    {
        uint16_t TMAGValues[8][3]; // 0 - xaxis, 1 - yaxis, 2 - zaxis
        int16_t sint[8][3];
    } c;

    byte MSB, LSB, MMSB;
    SR.StoreOneLOWbit();
    SR.ShiftCS();
    SR.ShiftCS();
    for (int i = 0; i < totalLDC1101; i++)
    {
        // delayMicroseconds(10);
        // digitalWrite(LDC_CSpin[i], LOW);
        // SR.SelectCS(LDC_CSpin[i]);
        SR.ShiftCS();
        SPI1.transfer(readReg | LDC1101_REGLHR_DATALSB);
        LSB = SPI1.transfer(0x00);
        MMSB = SPI1.transfer(0x00);
        MSB = SPI1.transfer(0x00);
        test_buff[test_buff_i] = LSB;
        test_buff_i++;
        test_buff[test_buff_i] = MMSB;
        test_buff_i++;
        test_buff[test_buff_i] = MSB;
        test_buff_i++;
        // digitalWrite(LDC_CSpin[i], HIGH);
        // SR.High_all_CS();
        LDCValues[i] = ((unsigned long)MSB << 16) | ((unsigned long)MMSB << 8) | LSB;
        // debug.print(" : ");
        debug.print(LDCValues[i]);
        debug.print("   ");
    }
    debug.print("");
    // TMAG Live Readout
    if (Special32BitEN)
    {
        for (int i = 0; i < totalTMAG5170; i++)
        {
            byte First, Second, Third, Fourth;
            // uint16_t axis1, axis2;
            //  digitalWrite(TMAG_CSpin[i], LOW);
            //  SR.SelectCS(TMAG_CSpin[i]);
            //  if (i == 2)
            //  {
            //      //SR.SelectCS(Q9);
            //      SR.ShiftCS();
            //      SR.ShiftCS();
            //  }
            SR.ShiftCS();
            // SR.SelectCS(TMAG_CSpin[i]);
            First = SPI1.transfer(readReg | X_CH_RESULT);
            Second = SPI1.transfer(0x00); // MSB
            Third = SPI1.transfer(0x00);  // LSB
            Fourth = SPI1.transfer(0xFF);
            test_buff[test_buff_i] = First;
            test_buff_i++;
            test_buff[test_buff_i] = Second;
            test_buff_i++;
            test_buff[test_buff_i] = Third;
            test_buff_i++;
            test_buff[test_buff_i] = Fourth;
            test_buff_i++;
            // digitalWrite(TMAG_CSpin[i], HIGH);
            // SR.High_all_CS();
            c.TMAGValues[i][0] = ((uint16_t)(Second & 0x0F) << 12) | ((Third & 0xF0) << 4) | (Fourth & 0xF0);
            c.TMAGValues[i][1] = ((uint16_t)(First & 0x0F) << 12) | ((Second & 0xF0) << 4) | ((Third & 0x0F) << 4);
            // Extracting the bits for Axis 2
            // TMAGValues[i][0] = (float)c.sint[i][0] * 300 / 32768;

            debug.print((float)c.sint[i][0] * 300 / 32768, 2); // 32768
            debug.print(" ");

            // TMAGValues[i][1] = (float)c.sint[i][1] * 300 / 32768;
            debug.print((float)c.sint[i][1] * 300 / 32768, 2);
            debug.print(" ");
        }
        test_buff_i = 0;
        debug.println("");
        SR.High_all_CS();
    }
    else
    {
        for (int i = 0; i < totalTMAG5170; i++)
        {
            byte MSB, LSB;
            // digitalWrite(TMAG_CSpin[i], LOW);
            SR.SelectCS(TMAG_CSpin[i]);
            SPI1.transfer(readReg | X_CH_RESULT);
            MSB = SPI1.transfer(0x00); // MSB
            LSB = SPI1.transfer(0x00); // LSB
            SPI1.transfer(0x0F);
            // digitalWrite(TMAG_CSpin[i], HIGH);
            SR.High_all_CS();
            c.TMAGValues[i][0] = (MSB << 8) | LSB; // x axis raw values
            debug.print((float)c.sint[i][0] * 300 / 32768, 2);
            debug.print(" ");
            delayMicroseconds(1);
            // digitalWrite(TMAG_CSpin[i], LOW);
            SR.SelectCS(TMAG_CSpin[i]);
            SPI1.transfer(readReg | Y_CH_RESULT);
            MSB = SPI1.transfer(0x00); // MSB
            LSB = SPI1.transfer(0x00); // LSB
            SPI1.transfer(0x0F);
            // digitalWrite(TMAG_CSpin[i], HIGH);
            SR.High_all_CS();
            c.TMAGValues[i][1] = (MSB << 8) | LSB; // y axis raw value
            debug.print((float)c.sint[i][1] * 300 / 32768, 2);
            debug.print(" ");
            delayMicroseconds(1);
            // digitalWrite(TMAG_CSpin[i], LOW);
            SR.SelectCS(TMAG_CSpin[i]);
            SPI1.transfer(readReg | Z_CH_RESULT);
            MSB = SPI1.transfer(0x00); // MSB
            LSB = SPI1.transfer(0x00); // LSB
            SPI1.transfer(0x0F);
            // digitalWrite(TMAG_CSpin[i], HIGH);
            SR.High_all_CS();
            c.TMAGValues[i][2] = (MSB << 8) | LSB; // z axis raw value
            debug.print((float)c.sint[i][2] * 300 / 32768, 2);
            debug.print(" ");
            delayMicroseconds(1);
        }
        debug.println("");
    }
    analog_val = analogRead(26);
}

void MFLSensors::UpdateSensors(bool Special32BitEN, byte *buffer, uint32_t sample_count)
{

    // size_t bufferindex=0;
    analog_val = analogRead(ANALOG_PIN);
    // buffer[bufferindex] = uint8_t(0x4D);
    //  bufferindex++;

    uint8_t startBytes[] = {0xF1, 0xE2, 0xD3, 0xC4};
    memcpy(buffer, startBytes, sizeof(startBytes));
    buffer += sizeof(startBytes);
    // *buffer = uint8_t(0xF1); // F1E2D3C4
    // buffer++;
    // *buffer = uint8_t(0xE2);
    // buffer++;
    // *buffer = uint8_t(0xD3);
    // buffer++;
    // *buffer = uint8_t(0xC4);
    // buffer++;
    SR.StoreOneLOWbit();
    SR.ShiftCS();
    SR.ShiftCS();
    for (int i = 0; i < totalLDC1101; i++)
    {
        SR.ShiftCS();
        SPI1.transfer(0xB8);
        for (int j = 0; j < 3; j++)
        {
            *buffer = SPI1.transfer(0x00);
            buffer++;
        }
        // *buffer = SPI1.transfer(0x00);
        // buffer++;
        // *buffer = SPI1.transfer(0x00);
        // buffer++;
        // *buffer = SPI1.transfer(0x00);
        // buffer++;
    }

    // TMAG Live Readout
    if (Special32BitEN)
    {
        for (int i = 0; i < totalTMAG5170; i++)
        {
            SR.ShiftCS();

            *buffer = SPI1.transfer(0x89);
            buffer++;
            *buffer = SPI1.transfer(0x00); // MSB
            buffer++;
            *buffer = SPI1.transfer(0x00); // LSB
            buffer++;
            *buffer = SPI1.transfer(0xFF);
            buffer++;
        }
    }
    else
    {
        for (int i = 0; i < totalTMAG5170; i++)
        {

            // digitalWrite(TMAG_CSpin[i], LOW);
            SR.SelectCS(TMAG_CSpin[i]);
            SPI1.transfer(readReg | X_CH_RESULT);
            //  MSB = SPI1.transfer(0x00); // MSB
            //  LSB = SPI1.transfer(0x00); // LSB
            SPI1.transfer(0x0F);
            // digitalWrite(TMAG_CSpin[i], HIGH);
            SR.High_all_CS();
            delayMicroseconds(1);
            // digitalWrite(TMAG_CSpin[i], LOW);
            SR.SelectCS(TMAG_CSpin[i]);
            SPI1.transfer(readReg | Y_CH_RESULT);
            //   MSB = SPI1.transfer(0x00); // MSB
            //   LSB = SPI1.transfer(0x00); // LSB
            SPI1.transfer(0x0F);
            // digitalWrite(TMAG_CSpin[i], HIGH);
            SR.High_all_CS();
            delayMicroseconds(1);
            // digitalWrite(TMAG_CSpin[i], LOW);
            SR.SelectCS(TMAG_CSpin[i]);
            SPI1.transfer(readReg | Z_CH_RESULT);
            //   MSB = SPI1.transfer(0x00); // MSB
            //   LSB = SPI1.transfer(0x00); // LSB
            SPI1.transfer(0x0F);
            // digitalWrite(TMAG_CSpin[i], HIGH);
            SR.High_all_CS();
            delayMicroseconds(1);
        }
        // debug.println("");
    }
    // buffer[bufferindex] = uint8_t(0x45);
    // bufferindex++;
    for (int i = 3; i >= 0; i--)
    {
        *buffer = uint8_t(sample_count >> (i * 8));
        buffer++;
    }
    // *buffer = uint8_t(sample_count >> 24);
    // buffer++;
    // *buffer = uint8_t(sample_count >> 16);
    // buffer++;
    // *buffer = uint8_t(sample_count >> 8);
    // buffer++;
    // *buffer = uint8_t(sample_count);
    // buffer++;
    *buffer = uint8_t(analog_val >> 8);
    buffer++;
    *buffer = uint8_t(analog_val);
    buffer++;
    // buffer[bufferindex] = uint8_t(0x0A);
    //  bufferindex++;
}
/*For Readout*/
/*
for (int h = 0; h < 10000; h++)
       {
           byte MSB, LSB, MMSB;
           delay(100);
           //digitalWrite(LDC_CSpin[i], LOW);
           SPI1.transfer(readReg | LDC1101_REGLHR_DATALSB);
           LSB = SPI1.transfer(0x00);
           MMSB = SPI1.transfer(0x00);
           MSB = SPI1.transfer(0x00);
           unsigned long combinedValue = ((unsigned long)MSB << 16) | ((unsigned long)MMSB << 8) | LSB;
           combinedValue = map(combinedValue, 0, 16777216, 0, 1000);
           Serial.println(combinedValue);
           //digitalWrite(LDC_CSpin[i], HIGH);
       }*/

void MFLSensors::dma_fetch_ldcdata(byte *buffer, uint32_t sample_count)
{
    // x = micros();
    uint16_t analog_val = analogRead(26);
    // Serial.println(analog_val);
    // sprintf((char*) numStr, ":%06u:%u\n", sample_count, analog_val); // Convert to string in ASCII decimal format
    /*if(sample_count != (prev_sample+1) )
    {
        Serial.println("sample missed");
    }
    prev_sample = sample_count;*/
    *buffer = uint8_t(0x4d);
    buffer++;
    *buffer = uint8_t(0x2c);
    buffer++;
    // SR.SelectCS(LDC_CSpin[Q1]);
    SR.StoreOneLOWbit();
    SR.ShiftCS();
    SR.ShiftCS();
    for (int i = 0; i < 4; i++)
    {
        // delayMicroseconds(10);
        // digitalWriteFast(LDC_CSpin[i], LOW);
        SR.ShiftCS();
        spi_dma_transfer(LDC_data_add, buffer, 3);

        /*for (int i=0;i<4;i++)
        {
         Serial.println(*buffer);
         buffer++;
        }*/
        buffer += 3;
        *buffer = uint8_t(0x2c);
        buffer++;
    }

    for (int i = 0; i < 8; i++)
    {
        if (i == 2)
        {
            SR.ShiftCS();
            SR.ShiftCS();
        }
        SR.ShiftCS();
        spi_dma_transfer(TMAG_data_add, buffer, 4);
        buffer += 4;
        *buffer = uint8_t(0x2c);
        buffer++;
    }

    *buffer = uint8_t(0x45);
    buffer++;
    *buffer = uint8_t(0x2c);
    buffer++;
    *buffer = uint8_t(sample_count >> 24);
    buffer++;
    *buffer = uint8_t(sample_count >> 16);
    buffer++;
    *buffer = uint8_t(sample_count >> 8);
    buffer++;
    *buffer = uint8_t(sample_count);
    buffer++;
    *buffer = uint8_t(0x2c);
    buffer++;
    *buffer = uint8_t(analog_val >> 8);
    buffer++;
    *buffer = uint8_t(analog_val);
    buffer++;
    *buffer = uint8_t(0x0A);
    buffer++;

    /*for (int i = 0; numStr[i] != '\0'; i++)
    {
        *buffer = uint8_t(numStr[i]);
        buffer++;
    }*/
    // y = micros()-x;
}

void MFLSensors::dma_setup()
{
    dma_tx_channel = dma_claim_unused_channel(true);
    dma_rx_channel = dma_claim_unused_channel(true);
}

void MFLSensors::spi_dma_transfer(uint8_t *tx_data, uint8_t *rx_data, size_t len)
{
    dma_channel_config tx_config = dma_channel_get_default_config(dma_tx_channel);
    channel_config_set_transfer_data_size(&tx_config, DMA_SIZE_8);
    channel_config_set_read_increment(&tx_config, true);
    channel_config_set_write_increment(&tx_config, false);
    channel_config_set_dreq(&tx_config, DREQ_SPI1_TX);

    dma_channel_configure(
        dma_tx_channel,
        &tx_config,
        &spi_get_hw(SPI_PORT)->dr, // SPI data register for transfer
        tx_data,                   // Source buffer
        len,                       // Transfer count
        false                      // Don't start yet
    );
    dma_channel_config rx_config = dma_channel_get_default_config(dma_rx_channel);
    channel_config_set_transfer_data_size(&rx_config, DMA_SIZE_8);
    channel_config_set_read_increment(&rx_config, false);
    channel_config_set_write_increment(&rx_config, true);
    channel_config_set_dreq(&rx_config, DREQ_SPI1_RX);

    dma_channel_configure(
        dma_rx_channel,
        &rx_config,
        rx_data,                   // Destination buffer
        &spi_get_hw(SPI_PORT)->dr, // SPI data register for receive
        len,                       // Transfer count
        false                      // Don't start yet
    );
    // Start SPI transaction
    // gpio_put(-1, 0);  // Select the slave
    dma_start_channel_mask((1u << dma_tx_channel) | (1u << dma_rx_channel));
    // dma_channel_set_read_addr(dma_tx_channel, rx_data, true);
    //  Wait for DMA transfer to complete
    dma_channel_wait_for_finish_blocking(dma_tx_channel);
    dma_channel_wait_for_finish_blocking(dma_rx_channel);
    // gpio_put(-1, 1);  // Deselect the slave
}
