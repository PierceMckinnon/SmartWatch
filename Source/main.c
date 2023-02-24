#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "app_error.h"
#include "bsp.h"
#include "nordic_common.h"
#include "nrf_drv_clock.h"
#include "sdk_errors.h"
#include "task.h"
#include "timers.h"

#include "EPD_1in54_V2.h"
#include "EPD_Test.h"

#define TASK_DELAY 200 /**< Task delay. Delays a LED2 task for 200 ms */

TaskHandle_t
    led_toggle_task_handle; /**< Reference to LED2 toggling FreeRTOS task. */

static void led_toggle_task_function(void* pvParameter) {
  UNUSED_PARAMETER(pvParameter);

  while (true) {
    bsp_board_led_invert(BSP_BOARD_LED_2);
    vTaskDelay(TASK_DELAY);
  }
}

int main(void) {
  // ret_code_t err_code;

  /* Initialize clock driver for better time accuracy in FREERTOS */
  // err_code = nrf_drv_clock_init();
  // APP_ERROR_CHECK(err_code);
  nrf_drv_clock_init();
  bsp_board_init(BSP_INIT_LEDS);

  EPD_1IN54_V2_Cfg_GPIO();
  epdSpiPins_s spiPins = {.sck = 11, .mosi = 13};
  EPD_INIT_SPI(spiPins);  // change once adding other spi devices

  EPD_test();
  /* Create task for LED2 blinking with priority set to 2 */
  UNUSED_VARIABLE(xTaskCreate(led_toggle_task_function, "LED2",
                              configMINIMAL_STACK_SIZE + 200, NULL, 2,
                              &led_toggle_task_handle));

  /* Activate deep sleep mode */
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

  /* Start FreeRTOS scheduler. */
  vTaskStartScheduler();

  while (true) {
    /* FreeRTOS should not be here... FreeRTOS goes back to the start of stack
     * in vTaskStartScheduler function. */
  }
}