#include "TMAG5170.h"

TMAG5170::TMAG5170(uint16_t SPI_CSpin)
{
  _SPI_CS = SPI_CSpin;
  pinMode(_SPI_CS, OUTPUT);

  sndSPI();
}

TMAG5170::TMAG5170(int8_t sck, int8_t miso, int8_t mosi, int8_t ss)
{
  _SPI_CS = ss;
  digitalWrite(_SPI_CS, LOW);
  // pinMode(_SPI_CS, OUTPUT);
  //  SPI.begin(sck, miso, mosi, ss);

  sndSPI();
}

void TMAG5170::sndSPI()
{
  SPI1.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  //digitalWrite(_SPI_CS, LOW);
  
  // send CSI data
  for (int i = 0; i < 3; i++)
  {

    rcvBuffer[i] = SPI1.transfer(sendBuffer[i]);
    // delayMicroseconds(100);
    // Serial.print(sendBuffer[i]);
    // Serial.println(" ");
  }
  // CS pin goes hi
  digitalWrite(_SPI_CS, HIGH);
  SPI1.endTransaction();
  delayMicroseconds(100);
  //  Serial.println();
}

void TMAG5170::initTMAG5170()
{
  unsigned int data;
  for (byte offset = 0x0; offset < 0x15; offset++)
  {
    switch (offset)
    {
    case DEVICE_CONFIG:
      data = CONV_AVG_1x | MAG_TEMPCO_0pd | OPERATING_MODE_ConfigurationMode | T_CH_EN_TempChannelEnabled | T_RATE_sameAsOtherSensors | T_HLT_EN_tempLimitCheckOff | TEMP_COMP_EN_TempCompensationDisenabled;
      sendBuffer[0] = writeReg | offset;
      sendBuffer[1] = (byte)(data >> 8);
      sendBuffer[2] = (byte)(data & 0x00ff);
      //      sendBuffer[3] = 0;
      sndSPI();
      break;
    case SENSOR_CONFIG:
      data = ANGLE_EN_NoAngleCalculation | SLEEPTIME_1ms | MAG_CH_EN_XYZenabled | Z_RANGE_100mT | Y_RANGE_100mT | X_RANGE_100mT;
      // data = ANGLE_EN_Z_X | SLEEPTIME_1ms | MAG_CH_EN_ZXenabled | Z_RANGE_25mT | Y_RANGE_25mT | X_RANGE_25mT;
      sendBuffer[0] = writeReg | offset;
      sendBuffer[1] = (byte)(data >> 8);
      sendBuffer[2] = (byte)(data & 0x00ff);
      sendBuffer[3] = 0;
      sndSPI();
      break;
    case SYSTEM_CONFIG:
      data = DIAG_SEL_AllDataPath | TRIGGER_MODE_SPI | DATA_TYPE_32bit | DIAG_EN_AFEdiagnosticsDisabled | Z_HLT_EN_ZaxisLimitCheckoff | Y_HLT_EN_YaxisLimitCheckoff | X_HLT_EN_XaxisLimitCheckoff;
      sendBuffer[0] = writeReg | offset;
      sendBuffer[1] = (byte)(data >> 8);
      sendBuffer[2] = (byte)(data & 0x00ff);
      //      sendBuffer[3] = 0;
      sndSPI();
      break;
    case TEST_CONFIG:
      data = CRC_DIS_CRCdisabled;
      sendBuffer[0] = writeReg | offset;
      sendBuffer[1] = (byte)(data >> 8);
      sendBuffer[2] = (byte)(data & 0x00ff);
      sendBuffer[3] = 0x07;
      sndSPI();
      break;
    case MAG_GAIN_CONFIG:
      data = GAIN_SELECTION_noAxisSelected;
      sendBuffer[0] = writeReg | offset;
      sendBuffer[1] = (byte)(data >> 8);
      sendBuffer[2] = (byte)(data & 0x00ff);
      //      sendBuffer[3] = 0;
      sndSPI();
      break;
    default:
      break;
    }
  }
  regConfig(writeReg, DEVICE_CONFIG, DeviceStart);
}

void TMAG5170::regConfig(unsigned char RW, unsigned char offset, unsigned int data)
{
  sendBuffer[3] = 0x0f;
  sendBuffer[0] = RW | offset;
  putSndData(data);
  sndSPI();
  // status_X = getFieldData(rcvBuffer[0], 3, 1);
  // status_Y = getFieldData(rcvBuffer[0], 2, 1);
  // status_Z = getFieldData(rcvBuffer[0], 1, 1);
  // status_T = getFieldData(rcvBuffer[0], 0, 1);
}

void TMAG5170::putSndData(unsigned int data)
{

  sendBuffer[1] = (unsigned char)(data >> 8);
  sendBuffer[2] = (unsigned char)(data & 0x00ff);
}

unsigned int TMAG5170::readRegData(byte address)
{
  unsigned int ret = 0;
  if ((address >= DEVICE_CONFIG) && (address <= MAGNITUDE_RESULT))
  {
    regConfig(readReg, address, 0x0);
  }
  else
  {
    regConfig(readReg, DEVICE_CONFIG, 0x0);
  }
  ret = (unsigned int)rcvBuffer[1] << 8 | (unsigned int)rcvBuffer[2];

  return ret;
}

unsigned long TMAG5170::readRcvData(byte address)
{

  int i;
  unsigned long ret = 0;
  if ((address >= DEVICE_CONFIG) && (address <= MAGNITUDE_RESULT))
  {
    regConfig(readReg, address, 0x0);
  }
  else
  {
    regConfig(readReg, DEVICE_CONFIG, 0x0);
  }
  //  for (i = 0; i++; i < 4)
  //  {
  //   sendbit[i] = sendBuffer[i];
  //  }
  ret = (unsigned int)rcvBuffer[0] << 24 | (unsigned int)rcvBuffer[1] << 16 | (unsigned int)rcvBuffer[2] << 8 | (unsigned int)rcvBuffer[3];

  return ret;
}

float TMAG5170::convTempdata(unsigned int data)
{
  float ret = 25 + (((float)data - 17508) / 60.1);
  return ret;
}

float TMAG5170::getTempdata()
{
  unsigned int data = readRegData(TEMP_RESULT);
  float ret = convTempdata(data);
  return ret;
}

float TMAG5170::getAngledata()
{
  //  unsigned int data = readRegData(ANGLE_RESULT);
  unsigned int data = getAngledataRaw();
  float ret = ((float)(data >> 4)) + (((float)(data & 0x000f)) / 16);
  return ret;
}

unsigned int TMAG5170::getAngledataRaw()
{
  return readRegData(ANGLE_RESULT);
}

float TMAG5170::getFluxdensity(unsigned char axis)
{
  union c
  {
    uint16_t uintval;
    int16_t sint;
  } c;
  float ret;
  uint8_t range;
  switch (axis)
  {
  case axis_X:
    c.uintval = readRegData(X_CH_RESULT);
    range = setRange[axis_X];
    break;
  case axis_Y:
    c.uintval = readRegData(Y_CH_RESULT);
    range = setRange[axis_Y] >> 2;
    break;
  case axis_Z:
    c.uintval = readRegData(Z_CH_RESULT);
    range = setRange[axis_Z] >> 4;
    break;
  default:
    break;
  }
  switch (range)
  {
  case X_RANGE_50mT:
    ret = (float)c.sint * 50 / 32768;
    break;
  case X_RANGE_25mT:
    ret = (float)c.sint * 25 / 32768;
    break;
  case X_RANGE_100mT:
    ret = (float)c.sint * 300 / 32768;
    break;
  default:
    ret = 0;
    break;
  }
  return ret;
}
