#include "EPD_Test.h"
#include "nrf_delay.h"
#include "EPD_1in54_V2.h"
#include "nrf_gpio.h"
#include "sdk_config.h"
#include "boards.h"


int main(void)
{
  EPD_1IN54_V2_Cfg_GPIO();
  bsp_board_init(BSP_INIT_LEDS);
  epdSpiPins_s spiPins = { .sck = 11, .mosi = 13};
  EPD_INIT_SPI(spiPins); //change once adding other spi devices
  EPD_test();

    while (1) {
      bsp_board_led_invert(2);
      nrf_delay_ms(500);
    }

}
