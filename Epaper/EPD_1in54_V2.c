/*****************************************************************************
* | File      	:   EPD_1in54_V2.c
* | Author      :   Waveshare team
* | Function    :   1.54inch e-paper V2
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2019-06-11
* | Info        :
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#define SPI_TRANSFER(buffer) \
  nrf_drv_spi_transfer(&spi, buffer, sizeof(uint8_t), NULL, 0)

#include "EPD_1in54_V2.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "nrf_drv_spi.h"
#include "nrf_gpio.h"

static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(0);
nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;

void EPD_1IN54_V2_Cfg_GPIO() {
  nrf_gpio_cfg_output(EPD_CS_PIN);
  nrf_gpio_cfg_output(EPD_RST_PIN);
  nrf_gpio_cfg_output(EPD_DC_PIN);

  nrf_gpio_cfg_input(EPD_BUSY_PIN, NRF_GPIO_PIN_NOPULL);
}

void EPD_INIT_SPI(epdSpiPins_s spiPins) {
  spi_config.ss_pin = NRF_DRV_SPI_PIN_NOT_USED;
  spi_config.sck_pin = spiPins.sck;
  spi_config.mosi_pin = spiPins.mosi;
  spi_config.frequency = NRF_SPI_FREQ_500K;
  APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, NULL, NULL));
}

// waveform full refresh
unsigned char WF_Full_1IN54[159] = {
    0x80, 0x48, 0x40, 0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,  0x0,  0x40,
    0x48, 0x80, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,  0x80, 0x48,
    0x40, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x40, 0x48, 0x80,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0xA, 0x0, 0x0,  0x0,  0x0,
    0x0,  0x0,  0x8,  0x1,  0x0,  0x8,  0x1,  0x0, 0x2, 0xA, 0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,  0x0,  0x0,
    0x0,  0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x0, 0x0, 0x0, 0x22, 0x17, 0x41,
    0x0,  0x32, 0x20};

// waveform partial refresh(fast)
unsigned char WF_PARTIAL_1IN54_0[159] = {
    0x0,  0x40, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,  0x0,  0x80,
    0x80, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,  0x40, 0x40,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,  0x80, 0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0xF, 0x0, 0x0,  0x0,  0x0,
    0x0,  0x0,  0x1,  0x1,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,  0x0,  0x0,
    0x0,  0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x0, 0x0, 0x0, 0x02, 0x17, 0x41,
    0xB0, 0x32, 0x28,
};

/******************************************************************************
function :	Software reset
parameter:
******************************************************************************/
static void EPD_1IN54_V2_Reset(void) {
  nrf_gpio_pin_write(EPD_RST_PIN, 1);
  nrf_delay_ms(20);
  nrf_gpio_pin_write(EPD_RST_PIN, 0);
  nrf_delay_ms(5);
  nrf_gpio_pin_write(EPD_RST_PIN, 1);
  nrf_delay_ms(20);
}

/******************************************************************************
function :	send command
parameter:
     Reg : Command register
******************************************************************************/
static void EPD_1IN54_V2_SendCommand(uint8_t Reg) {
  nrf_gpio_pin_write(EPD_DC_PIN, 0);
  nrf_gpio_pin_write(EPD_CS_PIN, 0);
  SPI_TRANSFER(&Reg);
  nrf_gpio_pin_write(EPD_CS_PIN, 1);
}

/******************************************************************************
function :	send data
parameter:
    Data : Write data
******************************************************************************/
static void EPD_1IN54_V2_SendData(uint8_t Data) {
  nrf_gpio_pin_write(EPD_DC_PIN, 1);
  nrf_gpio_pin_write(EPD_CS_PIN, 0);
  SPI_TRANSFER(&Data);
  nrf_gpio_pin_write(EPD_CS_PIN, 1);
}

/******************************************************************************
function :	Wait until the busy_pin goes LOW
parameter:
******************************************************************************/
static void EPD_1IN54_V2_ReadBusy(void) {
  while (nrf_gpio_pin_read(EPD_BUSY_PIN) == 1) {  // LOW: idle, HIGH: busy
    nrf_delay_ms(100);
  }
  nrf_delay_ms(200);
}

/******************************************************************************
function :	Turn On Display full
parameter:
******************************************************************************/
static void EPD_1IN54_V2_TurnOnDisplay(void) {
  EPD_1IN54_V2_SendCommand(0x22);
  EPD_1IN54_V2_SendData(0xc7);
  EPD_1IN54_V2_SendCommand(0x20);
  EPD_1IN54_V2_ReadBusy();
}

/******************************************************************************
function :	Turn On Display part
parameter:
******************************************************************************/
void EPD_1IN54_V2_TurnOnDisplayPart(void) {
  EPD_1IN54_V2_SendCommand(0x22);
  EPD_1IN54_V2_SendData(0xcF);
  EPD_1IN54_V2_SendCommand(0x20);
  EPD_1IN54_V2_ReadBusy();
}

static void EPD_1IN54_V2_Lut(uint8_t* lut) {
  EPD_1IN54_V2_SendCommand(0x32);
  for (uint8_t i = 0; i < 153; i++)
    EPD_1IN54_V2_SendData(lut[i]);
  EPD_1IN54_V2_ReadBusy();
}

static void EPD_1IN54_V2_SetLut(uint8_t* lut) {
  EPD_1IN54_V2_Lut(lut);

  EPD_1IN54_V2_SendCommand(0x3f);
  EPD_1IN54_V2_SendData(lut[153]);

  EPD_1IN54_V2_SendCommand(0x03);
  EPD_1IN54_V2_SendData(lut[154]);

  EPD_1IN54_V2_SendCommand(0x04);
  EPD_1IN54_V2_SendData(lut[155]);
  EPD_1IN54_V2_SendData(lut[156]);
  EPD_1IN54_V2_SendData(lut[157]);

  EPD_1IN54_V2_SendCommand(0x2c);
  EPD_1IN54_V2_SendData(lut[158]);
}

// static void EPD_1IN54_V2_SetWindows(uint16_t Xstart, uint16_t Ystart,
// uint16_t Xend, uint16_t Yend)
// {
//     EPD_1IN54_V2_SendCommand(0x44); // SET_RAM_X_ADDRESS_START_END_POSITION
//     EPD_1IN54_V2_SendData((Xstart>>3) & 0xFF);
//     EPD_1IN54_V2_SendData((Xend>>3) & 0xFF);

//     EPD_1IN54_V2_SendCommand(0x45); // SET_RAM_Y_ADDRESS_START_END_POSITION
//     EPD_1IN54_V2_SendData(Ystart & 0xFF);
//     EPD_1IN54_V2_SendData((Ystart >> 8) & 0xFF);
//     EPD_1IN54_V2_SendData(Yend & 0xFF);
//     EPD_1IN54_V2_SendData((Yend >> 8) & 0xFF);
// }

// static void EPD_1IN54_V2_SetCursor(uint16_t Xstart, uint16_t Ystart)
// {
//     EPD_1IN54_V2_SendCommand(0x4E); // SET_RAM_X_ADDRESS_COUNTER
//     EPD_1IN54_V2_SendData(Xstart & 0xFF);

//     EPD_1IN54_V2_SendCommand(0x4F); // SET_RAM_Y_ADDRESS_COUNTER
//     EPD_1IN54_V2_SendData(Ystart & 0xFF);
//     EPD_1IN54_V2_SendData((Ystart >> 8) & 0xFF);
// }

/******************************************************************************
function :	Initialize the e-Paper register
parameter:
******************************************************************************/
void EPD_1IN54_V2_Text_Init(void) {
  EPD_1IN54_V2_Reset();

  EPD_1IN54_V2_ReadBusy();
  EPD_1IN54_V2_SendCommand(0x12);  // SWRESET
  EPD_1IN54_V2_ReadBusy();

  EPD_1IN54_V2_SendCommand(0x01);  // Driver output control
  EPD_1IN54_V2_SendData(0xC7);
  EPD_1IN54_V2_SendData(0x00);
  // EPD_1IN54_V2_SendData(0x01);
  EPD_1IN54_V2_SendData(0x00);  // ldir

  EPD_1IN54_V2_SendCommand(0x11);  // data entry mode
  EPD_1IN54_V2_SendData(0x03);

  // temp start
  EPD_1IN54_V2_SendCommand(0x44);  // set Ram-X address start/end position
  EPD_1IN54_V2_SendData(0x00);     // 0x18-->(24+1)*8=200
  EPD_1IN54_V2_SendData(0x18);

  EPD_1IN54_V2_SendCommand(0x45);  // set Ram-Y address start/end position
  EPD_1IN54_V2_SendData(0x00);
  EPD_1IN54_V2_SendData(0x00);
  EPD_1IN54_V2_SendData(0xC7);  // 0xC7-->(199+1)=200
  EPD_1IN54_V2_SendData(0x00);

  EPD_1IN54_V2_SendCommand(0x3C);  // BorderWavefrom
  EPD_1IN54_V2_SendData(0x01);

  EPD_1IN54_V2_SendCommand(0x18);  // Read built-in temperature sensor
  EPD_1IN54_V2_SendData(0x80);

  EPD_1IN54_V2_SendCommand(0x22);  // //Load Temperature and waveform setting.
  EPD_1IN54_V2_SendData(0xB1);
  EPD_1IN54_V2_SendCommand(0x20);

  EPD_1IN54_V2_SendCommand(0x4E);  // set RAM x address count to 0;
  EPD_1IN54_V2_SendData(0x00);
  EPD_1IN54_V2_SendCommand(0x4F);  // set RAM y address count to 0X199;
  EPD_1IN54_V2_SendData(0xC7);
  EPD_1IN54_V2_SendData(0x00);
  EPD_1IN54_V2_ReadBusy();

  EPD_1IN54_V2_SetLut(WF_Full_1IN54);
}

void EPD_1IN54_V2_BMP_Init(void) {
  EPD_1IN54_V2_Reset();

  EPD_1IN54_V2_ReadBusy();
  EPD_1IN54_V2_SendCommand(0x12);  // SWRESET
  EPD_1IN54_V2_ReadBusy();

  EPD_1IN54_V2_SendCommand(0x01);  // Driver output control
  EPD_1IN54_V2_SendData(0xC7);
  EPD_1IN54_V2_SendData(0x00);
  EPD_1IN54_V2_SendData(0x01);
  // EPD_1IN54_V2_SendData(0x00);

  EPD_1IN54_V2_SendCommand(0x11);  // data entry mode
  EPD_1IN54_V2_SendData(0x03);

  // temp start
  EPD_1IN54_V2_SendCommand(0x44);  // set Ram-X address start/end position
  EPD_1IN54_V2_SendData(0x00);     // 0x18-->(24+1)*8=200
  EPD_1IN54_V2_SendData(0x18);

  EPD_1IN54_V2_SendCommand(0x45);  // set Ram-Y address start/end position
  EPD_1IN54_V2_SendData(0x00);
  EPD_1IN54_V2_SendData(0x00);
  EPD_1IN54_V2_SendData(0xC7);  // 0xC7-->(199+1)=200
  EPD_1IN54_V2_SendData(0x00);

  EPD_1IN54_V2_SendCommand(0x3C);  // BorderWavefrom
  EPD_1IN54_V2_SendData(0x01);

  EPD_1IN54_V2_SendCommand(0x18);  // Read built-in temperature sensor
  EPD_1IN54_V2_SendData(0x80);

  EPD_1IN54_V2_SendCommand(0x22);  // //Load Temperature and waveform setting.
  EPD_1IN54_V2_SendData(0xB1);
  EPD_1IN54_V2_SendCommand(0x20);

  EPD_1IN54_V2_SendCommand(0x4E);  // set RAM x address count to 0;
  EPD_1IN54_V2_SendData(0x00);
  EPD_1IN54_V2_SendCommand(0x4F);  // set RAM y address count to 0X199;
  EPD_1IN54_V2_SendData(0xC7);
  EPD_1IN54_V2_SendData(0x00);
  EPD_1IN54_V2_ReadBusy();

  EPD_1IN54_V2_SetLut(WF_Full_1IN54);
}
/******************************************************************************
function :	Initialize the e-Paper register (Partial display)
parameter:
******************************************************************************/
void EPD_1IN54_V2_Init_Partial(void) {
  EPD_1IN54_V2_Reset();
  EPD_1IN54_V2_ReadBusy();

  EPD_1IN54_V2_SetLut(WF_PARTIAL_1IN54_0);
  EPD_1IN54_V2_SendCommand(0x37);
  EPD_1IN54_V2_SendData(0x00);
  EPD_1IN54_V2_SendData(0x00);
  EPD_1IN54_V2_SendData(0x00);
  EPD_1IN54_V2_SendData(0x00);
  EPD_1IN54_V2_SendData(0x00);
  EPD_1IN54_V2_SendData(0x40);
  EPD_1IN54_V2_SendData(0x00);
  EPD_1IN54_V2_SendData(0x00);
  EPD_1IN54_V2_SendData(0x00);
  EPD_1IN54_V2_SendData(0x00);

  EPD_1IN54_V2_SendCommand(0x3C);  // BorderWavefrom
  EPD_1IN54_V2_SendData(0x80);

  EPD_1IN54_V2_SendCommand(0x22);
  EPD_1IN54_V2_SendData(0xc0);
  EPD_1IN54_V2_SendCommand(0x20);
  EPD_1IN54_V2_ReadBusy();
}

/******************************************************************************
function :	Clear screen
parameter:
******************************************************************************/
void EPD_1IN54_V2_Clear(void) {
  uint16_t Width, Height;
  Width = (EPD_1IN54_V2_WIDTH % 8 == 0) ? (EPD_1IN54_V2_WIDTH / 8)
                                        : (EPD_1IN54_V2_WIDTH / 8 + 1);
  Height = EPD_1IN54_V2_HEIGHT;

  EPD_1IN54_V2_SendCommand(0x24);
  for (uint16_t j = 0; j < Height; j++) {
    for (uint16_t i = 0; i < Width; i++) {
      EPD_1IN54_V2_SendData(0xFF);
    }
  }
  EPD_1IN54_V2_TurnOnDisplay();
}

void EPD_1IN54_V2_Clear_Part(void) {
  uint16_t Width, Height;
  Width = (EPD_1IN54_V2_WIDTH % 8 == 0) ? (EPD_1IN54_V2_WIDTH / 8)
                                        : (EPD_1IN54_V2_WIDTH / 8 + 1);
  Height = EPD_1IN54_V2_HEIGHT;

  EPD_1IN54_V2_SendCommand(0x24);
  for (uint16_t j = 0; j < Height; j++) {
    for (uint16_t i = 0; i < Width; i++) {
      EPD_1IN54_V2_SendData(0xFF);
    }
  }
  EPD_1IN54_V2_TurnOnDisplayPart();
}

// EPD_1IN54_V2_SendCommand(0x26);
// for (uint16_t j = 0; j < Height; j++) {
//     for (uint16_t i = 0; i < Width; i++) {
//         EPD_1IN54_V2_SendData(0Xaa);
//     }
// }

/******************************************************************************
function :	Sends the image buffer in RAM to e-Paper and displays
parameter:
******************************************************************************/
void EPD_1IN54_V2_Display(const uint8_t* image) {
  uint16_t Width, Height;
  Width = (EPD_1IN54_V2_WIDTH % 8 == 0) ? (EPD_1IN54_V2_WIDTH / 8)
                                        : (EPD_1IN54_V2_WIDTH / 8 + 1);
  Height = EPD_1IN54_V2_HEIGHT;
  EPD_1IN54_V2_SendCommand(0x3C);  // BorderWavefrom
  EPD_1IN54_V2_SendData(0x01);

  EPD_1IN54_V2_SendCommand(0x24);
  for (uint16_t j = 0; j < Height; j++) {
    for (uint16_t i = 0; i < Width; i++) {
      EPD_1IN54_V2_SendData(image[i + j * Width]);
      // EPD_1IN54_V2_SendData(0xFF);
    }
  }
  EPD_1IN54_V2_TurnOnDisplay();
}

/******************************************************************************
function :	 The image of the previous frame must be uploaded, otherwise the
                         first few seconds will display an exception.
parameter:
******************************************************************************/
void EPD_1IN54_V2_DisplayPartBaseimage(uint8_t* image) {
  uint16_t Width, Height;
  Width = (EPD_1IN54_V2_WIDTH % 8 == 0) ? (EPD_1IN54_V2_WIDTH / 8)
                                        : (EPD_1IN54_V2_WIDTH / 8 + 1);
  Height = EPD_1IN54_V2_HEIGHT;

  uint32_t Addr = 0;
  EPD_1IN54_V2_SendCommand(0x24);
  for (uint16_t j = 0; j < Height; j++) {
    for (uint16_t i = 0; i < Width; i++) {
      Addr = i + j * Width;
      EPD_1IN54_V2_SendData(image[Addr]);
    }
  }
  EPD_1IN54_V2_TurnOnDisplayPart();
}

/******************************************************************************
function :	Sends the image buffer in RAM to e-Paper and displays
parameter:
******************************************************************************/
void EPD_1IN54_V2_DisplayPart(uint8_t* image) {
  uint16_t Width, Height;
  Width = (EPD_1IN54_V2_WIDTH % 8 == 0) ? (EPD_1IN54_V2_WIDTH / 8)
                                        : (EPD_1IN54_V2_WIDTH / 8 + 1);
  Height = EPD_1IN54_V2_HEIGHT;

  uint32_t Addr = 0;
  EPD_1IN54_V2_SendCommand(0x24);
  for (uint16_t j = 0; j < Height; j++) {
    for (uint16_t i = 0; i < Width; i++) {
      Addr = i + j * Width;
      EPD_1IN54_V2_SendData(image[Addr]);
    }
  }
  EPD_1IN54_V2_TurnOnDisplayPart();
}

/******************************************************************************
function :	Enter sleep mode
parameter:
******************************************************************************/
void EPD_1IN54_V2_Sleep(void) {
  EPD_1IN54_V2_SendCommand(0x10);  // enter deep sleep
  EPD_1IN54_V2_SendData(0x01);
  nrf_delay_ms(100);
}

void EPD_SetFrameMemoryPartial(const unsigned char* image_buffer,
                               int x,
                               int y,
                               int image_width,
                               int image_height) {
  int x_end;
  int y_end;

  nrf_gpio_pin_write(EPD_RST_PIN, 0);  // module reset
  nrf_delay_ms(2);
  nrf_gpio_pin_write(EPD_RST_PIN, 1);
  nrf_delay_ms(2);

  EPD_1IN54_V2_SetLut(WF_PARTIAL_1IN54_0);
  EPD_1IN54_V2_SendCommand(0x37);
  EPD_1IN54_V2_SendData(0x00);
  EPD_1IN54_V2_SendData(0x00);
  EPD_1IN54_V2_SendData(0x00);
  EPD_1IN54_V2_SendData(0x00);
  EPD_1IN54_V2_SendData(0x00);
  EPD_1IN54_V2_SendData(0x40);
  EPD_1IN54_V2_SendData(0x00);
  EPD_1IN54_V2_SendData(0x00);
  EPD_1IN54_V2_SendData(0x00);
  EPD_1IN54_V2_SendData(0x00);

  EPD_1IN54_V2_SendCommand(0x3c);
  EPD_1IN54_V2_SendData(0x80);

  EPD_1IN54_V2_SendCommand(0x22);
  EPD_1IN54_V2_SendData(0xc0);
  EPD_1IN54_V2_SendCommand(0x20);
  EPD_1IN54_V2_ReadBusy();

  if (image_buffer == NULL || x < 0 || image_width < 0 || y < 0 ||
      image_height < 0) {
    return;
  }
  /* x point must be the multiple of 8 or the last 3 bits will be ignored */
  x &= 0xF8;
  image_width &= 0xF8;
  if (x + image_width >= 200) {
    x_end = 200 - 1;
  } else {
    x_end = x + image_width - 1;
  }
  if (y + image_height >= 200) {
    y_end = 200 - 1;
  } else {
    y_end = y + image_height - 1;
  }
  EPD_SetMemoryArea(x, y, x_end, y_end);
  EPD_SetMemoryPointer(x, y);
  EPD_1IN54_V2_SendCommand(0x24);
  /* send the image data */
  for (int j = 0; j < y_end - y + 1; j++) {
    for (int i = 0; i < (x_end - x + 1) / 8; i++) {
      EPD_1IN54_V2_SendData(image_buffer[i + j * (image_width / 8)]);
    }
  }
}

void EPD_SetMemoryArea(int x_start, int y_start, int x_end, int y_end) {
  EPD_1IN54_V2_SendCommand(0x44);
  /* x point must be the multiple of 8 or the last 3 bits will be ignored */
  EPD_1IN54_V2_SendData((x_start >> 3) & 0xFF);
  EPD_1IN54_V2_SendData((x_end >> 3) & 0xFF);
  EPD_1IN54_V2_SendCommand(0x45);
  EPD_1IN54_V2_SendData(y_start & 0xFF);
  EPD_1IN54_V2_SendData((y_start >> 8) & 0xFF);
  EPD_1IN54_V2_SendData(y_end & 0xFF);
  EPD_1IN54_V2_SendData((y_end >> 8) & 0xFF);
}

void EPD_SetMemoryPointer(int x, int y) {
  EPD_1IN54_V2_SendCommand(0x4e);
  /* x point must be the multiple of 8 or the last 3 bits will be ignored */
  EPD_1IN54_V2_SendData((x >> 3) & 0xFF);
  EPD_1IN54_V2_SendCommand(0x4F);
  EPD_1IN54_V2_SendData(y & 0xFF);
  EPD_1IN54_V2_SendData((y >> 8) & 0xFF);
  EPD_1IN54_V2_ReadBusy();
}