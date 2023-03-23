#include "main.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "bsp.h"
#include "nordic_common.h"
#include "nrf_delay.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_gpiote.h"
#include "nrf_drv_rtc.h"
#include "sdk_errors.h"
#include "task.h"
#include "timers.h"

#include "EPD_1in54_V2.h"
#include "buttonconfig.h"
#include "epaper.h"
#include "files.h"
#include "homescreen.h"
#include "pinconfig.h"
#include "sendreceive.h"
#include "spiconfig.h"
#include "timerconfig.h"
#include "vibration.h"
#include "w25qxx.h"

// SETUPS
static void setupGlobal(void);
static void setupButtonInterrupts(void);
static void setupEpaper(void);
static void setupTimers(void);
static void setupRTC(void);
static void setupSPI(void);
static void setupFlash(void);
// ENABLES
static void enableTimers(void);
// TASKS
static void buttonTopRightPushedTask(void* pvParameter);
static void buttonTopLeftPushedTask(void* pvParameter);
static void buttonBottomLeftPushedTask(void* pvParameter);
static void buttonBottomRightPushedTask(void* pvParameter);
static void epaperInactiveTimerTask(void* pvParameter);
// static void epaperRefreshDelayTask(void* pvParameter);
static void buttonIntDelayTask(void* pvParameter);
static void dateTimeRTCTask(void* pvParameter);
static void fileUartTask(void* pvParameter);
// INT HANDLERS
static void inputPinIntHandler(nrf_drv_gpiote_pin_t pin,
                               nrf_gpiote_polarity_t action);
static void timerIntHandler(nrf_timer_event_t event_type, void* p_context);
static void minuteRTCHandler(nrf_drv_rtc_int_type_t int_type);
// EXTRA
static void handleButtonPress(ButtonTypePresses_e buttonPressed);
static void isrGiveSemaphore(SemaphoreHandle_t* semaphore);

static const ButtonHandlerSetup* buttonHandlerFuncs[epaperStatesSize] = {
    &homescreenButtonHandlers, &calButtonHandlers, &filesButtonHandlers,
    &filesButtonHandlers};

static SemaphoreHandle_t buttonSemaphore[BUTTONNUM] = {};
static SemaphoreHandle_t epaperInactiveSemaphore = NULL;
static SemaphoreHandle_t epaperRefreshDelaySemaphore = NULL;
static SemaphoreHandle_t buttonIntDelaySemaphore = NULL;
static SemaphoreHandle_t uartSemaphore = NULL;
static SemaphoreHandle_t rtcSemaphore = NULL;

const nrf_drv_timer_t timerEpaperSleep = NRF_DRV_TIMER_INSTANCE(0);
const nrf_drv_timer_t timerEpaperRefreshDelay = NRF_DRV_TIMER_INSTANCE(1);
const nrf_drv_timer_t timerButtonIntDelay = NRF_DRV_TIMER_INSTANCE(2);
const nrf_drv_timer_t timerUart = NRF_DRV_TIMER_INSTANCE(3);

void vApplicationStackOverflowHook(TaskHandle_t* pxTask,
                                   signed char* pcTaskName) {
  while (1) {
    // bsp_board_led_invert(BSP_BOARD_LED_3);
    epaperDisplayTestingString("STACKO");
    SWERROR_HANDLER();
  }
}

void vApplicationMallocFailedHook(void) {
  while (1) {
    epaperDisplayTestingString("MALLOCO");
    // bsp_board_led_invert(BSP_BOARD_LED_2);
    SWERROR_HANDLER();
  }
}

// void vApplicationIdleHook(void) {
//   // can place in low power
//   // bsp_board_led_invert(BSP_BOARD_LED_1);
// }

int main(void) {
#ifdef PCB
  nrf_gpio_cfg_output(SOCONLED);
#endif

  setupGlobal();
  setupSPI();
  setupButtonInterrupts();
  setupEpaper();
  setupTimers();
  setupRTC();
  setupFlash();
  vibrateInit();

  // filesTest();

  // while (1)
  //   ;

  xTaskCreate(buttonTopRightPushedTask, "ButtonTopRightTask",
              configMINIMAL_STACK_SIZE + 150, (void*)NULL, 1, NULL);
  xTaskCreate(buttonTopLeftPushedTask, "ButtonTopLeftTask",
              configMINIMAL_STACK_SIZE + 150, (void*)NULL, 1, NULL);
  xTaskCreate(buttonBottomLeftPushedTask, "ButtonBottomLeftTask",
              configMINIMAL_STACK_SIZE + 150, (void*)NULL, 1, NULL);
  xTaskCreate(buttonBottomRightPushedTask, "ButtonBottomRightTask",
              configMINIMAL_STACK_SIZE + 150, (void*)NULL, 1, NULL);
  xTaskCreate(epaperInactiveTimerTask, "EpaperInactiveTask",
              configMINIMAL_STACK_SIZE, (void*)NULL, 0, NULL);
  // xTaskCreate(epaperRefreshDelayTask, "EpaperRefreshDelayTask",
  //             configMINIMAL_STACK_SIZE + 50, (void*)NULL, 1, NULL);
  xTaskCreate(buttonIntDelayTask, "ButtonIntDelayTask",
              configMINIMAL_STACK_SIZE, (void*)NULL, 1, NULL);
  xTaskCreate(fileUartTask, "fileUartTask", configMINIMAL_STACK_SIZE + 200,
              (void*)NULL, 1, NULL);
  xTaskCreate(dateTimeRTCTask, "RTCTask", configMINIMAL_STACK_SIZE + 100,
              (void*)NULL, 2, NULL);

  buttonFirstEnableInterrupts();

  homeScreenDisplay();

  // Activate

  enableTimers();

  /* Activate deep sleep mode */
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;  // review

  /* Start FreeRTOS scheduler. */
  vTaskStartScheduler();

  while (true) {
    /* should not end up here */
  }
}

// SETUPS
static void setupGlobal(void) {
  ret_code_t err_code;

  /* Initialize clock driver for better time accuracy in FREERTOS */
  err_code = nrf_drv_clock_init();
  APP_ERROR_CHECK(err_code);
  // bsp_board_init(BSP_INIT_LEDS);
}

static void setupSPI(void) {
  spiInitAll();
  EPD_1IN54_V2_SET_SPI();
  W25qxx_Set_Spi();
}

static void setupButtonInterrupts(void) {
  // initalize buttons
  ret_code_t err_code;
  err_code = nrf_drv_gpiote_init();
  APP_ERROR_CHECK(err_code);
  nrf_drv_gpiote_in_config_t pullDownConfig =
      GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);

  for (int i = 0; i < BUTTONNUM; i++) {
    err_code = nrf_drv_gpiote_in_init((int)getButtonPinAtIndex(i),
                                      &pullDownConfig, inputPinIntHandler);
    APP_ERROR_CHECK(err_code);
    vSemaphoreCreateBinary(buttonSemaphore[i]);
  }
}

static void setupEpaper(void) {
  EPD_1IN54_V2_CFG_GPIO();
  epaperInit();
}

static void setupTimers(void) {
  nrf_drv_timer_config_t timerConfig = NRF_DRV_TIMER_DEFAULT_CONFIG;
  uint32_t err_code =
      nrf_drv_timer_init(&timerEpaperSleep, &timerConfig, timerIntHandler);
  APP_ERROR_CHECK(err_code);

  uint32_t time_ticks =
      nrf_drv_timer_ms_to_ticks(&timerEpaperSleep, TIMER_MS_EPAPER_INACTIVE);

  nrf_drv_timer_extended_compare(&timerEpaperSleep, NRF_TIMER_CC_CHANNEL0,
                                 time_ticks, NRF_TIMER_SHORT_COMPARE0_STOP_MASK,
                                 true);
  vSemaphoreCreateBinary(epaperInactiveSemaphore);

  err_code = nrf_drv_timer_init(&timerEpaperRefreshDelay, &timerConfig,
                                timerIntHandler);
  APP_ERROR_CHECK(err_code);
  vSemaphoreCreateBinary(epaperRefreshDelaySemaphore);

  err_code =
      nrf_drv_timer_init(&timerButtonIntDelay, &timerConfig, timerIntHandler);
  APP_ERROR_CHECK(err_code);
  vSemaphoreCreateBinary(buttonIntDelaySemaphore);

  err_code = nrf_drv_timer_init(&timerUart, &timerConfig, timerIntHandler);
  APP_ERROR_CHECK(err_code);
  vSemaphoreCreateBinary(uartSemaphore);
}

static void setupRTC(void) {
  ret_code_t err_code;
  nrf_drv_rtc_config_t rtcConfig = NRF_DRV_RTC_DEFAULT_CONFIG;
  err_code = nrf_drv_rtc_init(&dateTimeRTC, &rtcConfig, minuteRTCHandler);
  APP_ERROR_CHECK(err_code);
  vSemaphoreCreateBinary(rtcSemaphore);
}

static void setupFlash(void) {
  W25qxx_Cfg_GPIO();
  if (!W25qxx_Init()) {
    SWERROR_HANDLER();
  }
  // for (uint32_t i = 0; i < 256; i++) {
  //   W25qxx_EraseBlock(i);
  // }
  // // W25qxx_EraseChip();
  // W25qxx_EraseSector(0);
  filesFSInit();
}

// ENABLES
static void enableTimers(void) {
  nrf_drv_timer_enable(&timerEpaperSleep);
  // nrf_drv_timer_enable(&timerEpaperRefreshDelay);
}

// TASKS
static void buttonTopRightPushedTask(void* pvParameter) {
  UNUSED_PARAMETER(pvParameter);
  int buttonInd = buttonIndex(buttonTopRight);
  // dont block
  xSemaphoreTake(buttonSemaphore[buttonInd], 0);

  while (true) {
    xSemaphoreTake(buttonSemaphore[buttonInd], portMAX_DELAY);
    handleButtonPress(buttonTopRightPress);
  }
}

static void buttonTopLeftPushedTask(void* pvParameter) {
  UNUSED_PARAMETER(pvParameter);
  int buttonInd = buttonIndex(buttonTopLeft);
  // dont block
  xSemaphoreTake(buttonSemaphore[buttonInd], 0);

  while (true) {
    xSemaphoreTake(buttonSemaphore[buttonInd], portMAX_DELAY);
    // filesDisplay();
    handleButtonPress(buttonTopLeftPress);
  }
}

static void buttonBottomLeftPushedTask(void* pvParameter) {
  UNUSED_PARAMETER(pvParameter);
  int buttonInd = buttonIndex(buttonBottomLeft);
  // dont block
  xSemaphoreTake(buttonSemaphore[buttonInd], 0);

  while (true) {
    xSemaphoreTake(buttonSemaphore[buttonInd], portMAX_DELAY);
    handleButtonPress(buttonBottomLeftPress);
  }
}

static void buttonBottomRightPushedTask(void* pvParameter) {
  UNUSED_PARAMETER(pvParameter);
  int buttonInd = buttonIndex(buttonBottomRight);
  // dont block
  xSemaphoreTake(buttonSemaphore[buttonInd], 0);

  while (true) {
    xSemaphoreTake(buttonSemaphore[buttonInd], portMAX_DELAY);
    handleButtonPress(buttonBottomRightPress);
  }
}

static void epaperInactiveTimerTask(void* pvParameter) {
  UNUSED_PARAMETER(pvParameter);
  // dont block
  xSemaphoreTake(epaperInactiveSemaphore, 0);

  while (true) {
    xSemaphoreTake(epaperInactiveSemaphore, portMAX_DELAY);
    epaperSleep();
  }
}

// static void epaperRefreshDelayTask(void* pvParameter) {
//   UNUSED_PARAMETER(pvParameter);
//   // dont block
//   xSemaphoreTake(epaperRefreshDelaySemaphore, 0);

//   while (true) {
//     xSemaphoreTake(epaperRefreshDelaySemaphore, portMAX_DELAY);
//     nrf_drv_timer_disable(&timerEpaperRefreshDelay);
//     vibrateStop();
//   }
// }

static void buttonIntDelayTask(void* pvParameter) {
  UNUSED_PARAMETER(pvParameter);
  // dont block
  xSemaphoreTake(buttonIntDelaySemaphore, 0);

  while (true) {
    xSemaphoreTake(buttonIntDelaySemaphore, portMAX_DELAY);
    nrf_drv_timer_disable(&timerButtonIntDelay);  // change back
    buttonEnableInterrupts();
  }
}

static void dateTimeRTCTask(void* pvParameter) {
  UNUSED_PARAMETER(pvParameter);

  xSemaphoreTake(rtcSemaphore, 0);
  while (true) {
    xSemaphoreTake(rtcSemaphore, portMAX_DELAY);
    calUpdateMinute(
        ((epaperGetState() == epaperDateTime) && epaperGetBlockState())
            ? UpdateDisplay
            : NoAction);
  }
}

static void fileUartTask(void* pvParameter) {
  UNUSED_PARAMETER(pvParameter);

  xSemaphoreTake(uartSemaphore, 0);
  // int x = 1;
  while (true) {
    xSemaphoreTake(uartSemaphore, portMAX_DELAY);
    sendReceiveGetData();
  }
}

// INTERUPT HANDLERS
static void inputPinIntHandler(nrf_drv_gpiote_pin_t pin,
                               nrf_gpiote_polarity_t action) {
  if (action != NRF_GPIOTE_POLARITY_LOTOHI)
    return;
  // static int x = 1;
  // nrf_gpio_pin_write(SOCONLED, x);
  buttonDisableInterrupts();
  uint32_t time_ticks = nrf_drv_timer_ms_to_ticks(&timerButtonIntDelay,
                                                  TIMER_MS_BUTTON_INT_DELAY);
  nrf_drv_timer_extended_compare(
      &timerButtonIntDelay, NRF_TIMER_CC_CHANNEL2, time_ticks,
      NRF_TIMER_SHORT_COMPARE2_STOP_MASK | NRF_TIMER_SHORT_COMPARE2_CLEAR_MASK,
      true);
  nrf_drv_timer_enable(&timerButtonIntDelay);

  isrGiveSemaphore(&(buttonSemaphore[buttonIndex((Buttons_e)pin)]));
  // x ^= 1;
}

static void timerIntHandler(nrf_timer_event_t event_type, void* p_context) {
  switch (event_type) {
    case NRF_TIMER_EVENT_COMPARE0: {
      isrGiveSemaphore(&epaperInactiveSemaphore);
      break;
    }
    case NRF_TIMER_EVENT_COMPARE1: {
      isrGiveSemaphore(&epaperRefreshDelaySemaphore);
      break;
    }
    case NRF_TIMER_EVENT_COMPARE2: {
      isrGiveSemaphore(&buttonIntDelaySemaphore);
      break;
    }
    case NRF_TIMER_EVENT_COMPARE3: {
      isrGiveSemaphore(&uartSemaphore);
      break;
    }
    default:
      break;
  }
}

static void minuteRTCHandler(nrf_drv_rtc_int_type_t int_type) {
  calUpdateRTCInt();
  isrGiveSemaphore(&rtcSemaphore);
}

// EXTRA
static void handleButtonPress(ButtonTypePresses_e buttonPressed) {
  if (epaperGetMode() == epaperSleepMode) {
    epaperWakeFromSleep();
    return;
  }

  const ButtonHandlerSetup* handlerStruct =
      buttonHandlerFuncs[epaperGetState()];
  switch (buttonPressed) {
    case (buttonTopLeftPress): {
      (*(handlerStruct->topLeftButtonPress))();
      break;
    }
    case (buttonTopRightPress): {
      (*(handlerStruct->topRightButtonPress))();
      break;
    }
    case (buttonBottomLeftPress): {
      (*(handlerStruct->bottomLeftButtonPress))();
      break;
    }
    case (buttonBottomRightPress): {
      (*(handlerStruct->bottomRightButtonPress))();
      break;
    }
    default:
      SWERROR_HANDLER();
  }
}

static void isrGiveSemaphore(SemaphoreHandle_t* semaphore) {
  long lHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(*semaphore, &lHigherPriorityTaskWoken);
  portEND_SWITCHING_ISR(lHigherPriorityTaskWoken);
}
