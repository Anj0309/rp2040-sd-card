#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

#include <string.h>
#include "FatFsSd_C.h"
//
#include "SerialUART.h"
#include <LDC1101.h>
#include <LVC595.h>

#define printf Serial.printf
#define puts Serial.println

/* Implement library message callbacks */
void put_out_error_message(const char *s)
{
  Serial.write(s);
}
void put_out_info_message(const char *s)
{
  Serial.write(s);
}
// This will not be called unless build_flags include "-D USE_DBG_PRINTF":
// void put_out_debug_message(const char *s) {
//     Serial.write(s);
// }

/*
This example assumes the following wiring:
    | GPIO | SD Card |
    | ---- | ------- |
    | GP16 | CLK     |
    | GP17 | CMD     |
    | GP18 | D0      |
    | GP19 | D1      |
    | GP20 | D2      |
    | GP21 | D3      |
    | GP22 | DET     |
*/
static sd_sdio_if_t sdio_if = {
    /*
    Pins CLK_gpio, D1_gpio, D2_gpio, and D3_gpio are at offsets from pin D0_gpio.
    The offsets are determined by sd_driver\SDIO\rp2040_sdio.pio.
        CLK_gpio = (D0_gpio + SDIO_CLK_PIN_D0_OFFSET) % 32;
        As of this writing, SDIO_CLK_PIN_D0_OFFSET is 30,
            which is -2 in mod32 arithmetic, so:
        CLK_gpio = D0_gpio -2.
        D1_gpio = D0_gpio + 1;
        D2_gpio = D0_gpio + 2;
        D3_gpio = D0_gpio + 3;
    */
    .CMD_gpio = 3,
    .D0_gpio = 4,
    .SDIO_PIO = pio1,
    .DMA_IRQ_num = DMA_IRQ_1,
    .baud_rate = 15 * 1000 * 1000 // 15 MHz
};

// Hardware Configuration of the SD Card "objects"
static sd_card_t sd_card = {
    /* "pcName" is the FatFs "logical drive" identifier.
    (See http://elm-chan.org/fsw/ff/doc/filename.html#vol) */
    .pcName = "0:",
    .type = SD_IF_SDIO,
    .sdio_if_p = &sdio_if,
 
    // SD Card detect:
    .use_card_detect = true,
     
    .card_detect_gpio = 8,
    .card_detected_true = 0, // What the GPIO read returns when a card is present.
    .card_detect_use_pull = true,
    .card_detect_pull_hi = true,
    .fatfs = {
        .pdrv = 1, 
    }
     
    };
/*
The following functions are required by the library API.
They are how the library finds out about the configuration.
*/
extern "C" size_t sd_get_num() { return 1; }

extern "C" sd_card_t *sd_get_by_num(size_t num)
{
  if (0 == num)
  {
    return &sd_card;
  }
  else
  {
    return NULL;
  }
}

// Check the FRESULT of a library call.
//  (See http://elm-chan.org/fsw/ff/doc/rc.html.)
#define CHK_FRESULT(s, fr)                              \
  if (FR_OK != fr)                                      \
  {                                                     \
    printf("%s:%d %s error: %s (%d)\n",                 \
           __FILE__, __LINE__, s, FRESULT_str(fr), fr); \
    for (;;)                                            \
      __breakpoint();                                   \
  }

/*
cmd 'S' to change the mode into SD card reader mode for 2mint only
//control register ->
*/
#define sec 1000
#define min 60
#define change_time 60 // time for sd card reader mode is 120second
/*Pins for sd card reader IC*/
#define SDA 20
#define SCL 21
#define RST_pin 22
#define INT_pin 23
// signals from CSB
#define AI_pin 26
#define TRIGGER_pin 17
#define MODE_pin 18
#define TX_pin 16

/*Pins for sensor head*/
// #define SRCLK_pin 13
// #define SRDA_pin 14
// #define SPISCK_pin 10
// #define SPIMOSI_pin 11
// #define SPIMISO_pin 12

#define Slave_add 113 // 1110001
#define Slave_add_w 226
#define Slave_add_r 227

#define CR_Reg_add 0x00      // control register address
#define CF_Reg1_add 0x01     // configure register 1 address
#define CF_Reg2_add 0x02     // configure register 2 address
#define CF_Reg3_add 0x03     // configure register 3 address
#define IE_Reg1_add 0x04     // interrupt enable register 1 address
#define IE_Reg2_add 0x05     // interrupt enable register 2 address
#define USBVIDH_add 0x06     // USB vendor ID high byte register address
#define USBVIDL_add 0x07     // USB vendor ID low byte register address
#define USBPIDH_add 0x08     // USB product ID high byte register address
#define USBPIDL_add 0x09     // USB product ID low byte register address
#define Status_Reg1_add 0x12 // status register1 address

#define max_SPI_speed 8000000

#define null '0'

SPISettings mysetting(max_SPI_speed, LSBFIRST, SPI_MODE0);
void high_all_io();
void LOW_all_io();
void execute_cmd(char cmd);

char cmd;
bool card_reader_mode = 0;
bool led_status = 1;
bool srda_status = 0;
unsigned long previous_millis = 0;
unsigned long current_millis = millis();
unsigned long current_second = 0;
unsigned long previous_second = 0;

unsigned long x, y;

int ldcCsPins[] = {CS12, CS11, CS10, CS9};
const int numLdcSensors = 4;

bool trig_status;

int TmagCsPins[] = {CS8, CS7, CS6, CS5, CS4, CS3, CS2, CS1};
const int numTmagSensors = 8;
MFLSensors Sensors;
ShiftRegister SR;
void setup()
{
  // initialize the digital pin as an output.
  // pinMode(20, OUTPUT);
  // pinMode(21, OUTPUT);

  Serial.begin(9600);
  // pinMode(SRCLK_pin, OUTPUT);
  // pinMode(SRDA_pin, OUTPUT);
  pinMode(RST_pin, OUTPUT);
  pinMode(INT_pin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // SPI1.setTX(SPIMOSI_pin);
  // SPI1.setRX(SPIMISO_pin);
  // SPI1.setSCK(SPISCK_pin);
  // SPI1.begin();
  SR.LVC595_init();
  // initialise sensors
  Sensors.initSPI(numLdcSensors, numTmagSensors, ldcCsPins, TmagCsPins);

  Wire.setSDA(SDA);
  Wire.setSCL(SCL);
  Wire.begin();
  digitalWrite(RST_pin, HIGH);

  // digitalWrite(SRDA_pin, HIGH);
  // srda_status = 1;
  // digitalWrite(SRCLK_pin, HIGH); /*to init shift register*/
  // delayMicroseconds(1);
  // digitalWrite(SRCLK_pin, LOW);
  // delayMicroseconds(1);
  // digitalWrite(SRCLK_pin, HIGH);

  digitalWrite(LED_BUILTIN, HIGH);

  while (!Serial.available()); // Serial is via USB; wait for enumeration
  while (Serial.available())
  {
    char dump = Serial.read();
  }

  Serial.println("Running");
  printf("\033[2J\033[H"); // Clear Screen
  puts("Hello, world!");

  // See FatFs - Generic FAT Filesystem Module, "Application Interface",
  // http://elm-chan.org/fsw/ff/00index_e.html
  sd_card_t *pSD = sd_get_by_num(0);
  FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
  if(fr!= FR_OK)
  {
    Serial.println("f_mount fail");
  }
  // CHK_FRESULT("f_mount", fr);
  FIL fil;
  const char *const filename = "file.txt";
  fr = f_open(&fil, filename, FA_OPEN_APPEND | FA_WRITE);
  if(fr!= FR_OK)
  {
    Serial.println("f_open fail");
  }
  // CHK_FRESULT("f_open", fr);
  char const *const str = "Hello, world!\n";
  if (f_printf(&fil, str) < strlen(str))
  {
    printf("f_printf failed\n");
    // for (;;)
    //__breakpoint();
  }
  fr = f_close(&fil);
  // CHK_FRESULT("f_close", fr);
  fr = f_unmount(pSD->pcName);
  // CHK_FRESULT("f_unmount", fr);

  puts("Goodbye, world!");
 
}

// the loop routine runs over and over again forever:
void loop()
{
  current_millis = millis();
  if ((current_millis - previous_millis) > sec)
  {
    previous_millis = current_millis;
    current_second++;
    // digitalWrite(LED_BUILTIN, led_status);
    // led_status = !led_status;
  }

  if (Serial.available())
  {
    cmd = Serial.read();
    execute_cmd(cmd);
    while (Serial.available())
    {
      char dump = Serial.read();
    }
  }
  if(cmd == 'R')
  {
    digitalWrite(LED_BUILTIN,HIGH);
    Sensors.liveReadSensors(1);
    digitalWrite(LED_BUILTIN,LOW);
    delay(500);
  }
  

  // if (cmd == 'S')
  // {
  //   Wire.beginTransmission(Slave_add);
  //   Wire.write(CR_Reg_add);
  //   Wire.write(0x13); // cmd to enable card reader mode and disconnect sd ports from host uc
  //   Wire.endTransmission();
  //   cmd = null;
  //   card_reader_mode = 1;
  //   Serial.println("card reader mode activated");
  //   previous_second = current_second;
  // }
  if (card_reader_mode == 1)
  {
    Wire.beginTransmission(Slave_add);
    Wire.write(Status_Reg1_add);
    Wire.endTransmission(0);

    Wire.requestFrom(Slave_add, 13);
    while (Wire.available())
    {
      BYTE status_reg1 = Wire.read();
      Serial.print(status_reg1);
      Serial.print('\t');
    }
    Serial.print('\n');
    delay(100);
    if ((current_second - previous_second) > change_time)
    {
      card_reader_mode = 0;
      Wire.beginTransmission(Slave_add);
      Wire.write(CR_Reg_add);
      Wire.write(0x09); // cmd to disable card reader mode and connect sd ports from host uc
      Wire.endTransmission();
      Serial.println("card reader mode de-activated");
    }
  }
}


void execute_cmd(char cmd)
{
  switch (cmd)
  {
  case 't':
    Sensors.CheckSensors();
    break;
  case 'C':
    Sensors.enableDebugging(1);
    Sensors.ConfigureSensors();
    break;
  case 'r':
    Sensors.enableDebugging(1);
    Sensors.putSensorsInRunMode(1);
    break;
  case 'R':
  //Sensors.enableDebugging(1);
    Sensors.liveReadSensors(1);
    break;
  case 's':
    SR.ShiftCS();
    Serial.println("shifted");
    break;
  case 'l':
    SR.StoreOneLOWbit();
    Serial.println("stored");
    break;

  case 'S':
    Wire.beginTransmission(Slave_add);
    Wire.write(CR_Reg_add);
    Wire.write(0x13); // cmd to enable card reader mode and disconnect sd ports from host uc
    Wire.endTransmission();
    card_reader_mode = 1;
    Serial.println("card reader mode activated");
    previous_second = current_second;
    break;
  default:
    break;
  }
}

