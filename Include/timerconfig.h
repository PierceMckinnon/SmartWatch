#ifndef _TIMER_CONFIG_H_
#define _TIMER_CONFIG_H_

#include "nrf_drv_timer.h"

#define TIMER_MS_EPAPER_INACTIVE 300000
#define TIMER_MS_BUTTON_INT_DELAY 100
#define TIMER_MS_UART_INTERVAL 1

extern const nrf_drv_timer_t timerEpaperSleep;
extern const nrf_drv_timer_t timerEpaperRefreshDelay;
extern const nrf_drv_timer_t timerButtonIntDelay;
extern const nrf_drv_timer_t timerUart;

static inline void timerEnableEpaperRefresh(uint32_t refreshDelayMs) {
  uint32_t time_ticks =
      nrf_drv_timer_ms_to_ticks(&timerEpaperRefreshDelay, refreshDelayMs);
  nrf_drv_timer_extended_compare(
      &timerEpaperRefreshDelay, NRF_TIMER_CC_CHANNEL1, time_ticks,
      NRF_TIMER_SHORT_COMPARE1_STOP_MASK | NRF_TIMER_SHORT_COMPARE1_CLEAR_MASK,
      true);
  nrf_drv_timer_enable(&timerEpaperRefreshDelay);
}

static inline void timerEnableUart(void) {
  uint32_t time_ticks =
      nrf_drv_timer_ms_to_ticks(&timerUart, TIMER_MS_UART_INTERVAL);
  nrf_drv_timer_extended_compare(&timerUart, NRF_TIMER_CC_CHANNEL3, time_ticks,
                                 NRF_TIMER_SHORT_COMPARE3_CLEAR_MASK, true);
  nrf_drv_timer_enable(&timerUart);
}

static inline void timerDisableUart(void) {
  nrf_drv_timer_disable(&timerUart);
}

#endif