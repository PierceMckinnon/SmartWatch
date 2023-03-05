#ifndef _TIMER_CONFIG_H_
#define _TIMER_CONFIG_H_

#include "nrf_drv_timer.h"

#define TIME_MS_EPD_INACTIVE 300000

extern const nrf_drv_timer_t timerEpaperSleep;
extern const nrf_drv_timer_t timerEpaperRefreshDelay;

static inline void timerEnableEpaperRefresh(uint32_t refreshDelayMs) {
  uint32_t time_ticks =
      nrf_drv_timer_ms_to_ticks(&timerEpaperRefreshDelay, refreshDelayMs);
  nrf_drv_timer_extended_compare(
      &timerEpaperRefreshDelay, NRF_TIMER_CC_CHANNEL1, time_ticks,
      NRF_TIMER_SHORT_COMPARE1_STOP_MASK | NRF_TIMER_SHORT_COMPARE1_CLEAR_MASK,
      true);
  nrf_drv_timer_enable(&timerEpaperRefreshDelay);
}

#endif