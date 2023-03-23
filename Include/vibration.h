#ifndef _VIBRATION_H_
#define _VIBRATION_H_

#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "pinconfig.h"
#include "timerconfig.h"

static inline void vibrateStop() {
  nrf_gpio_pin_write(VIBRATION, 0);
}
static inline void vibrateStart(void) {
  nrf_gpio_pin_write(VIBRATION, 1);
  nrf_delay_ms(1000);
  vibrateStop();
  //   timerEnableEpaperRefresh(500);
}
static inline void vibrateInit(void) {
  nrf_gpio_cfg_output(VIBRATION);
  vibrateStop();
}

#endif  //_VIBRATION_H_