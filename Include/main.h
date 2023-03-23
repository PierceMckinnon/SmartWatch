#ifndef _MAIN_H_
#define _MAIN_H_

#include <stdbool.h>
#include <stdint.h>
#include "app_error.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "pinconfig.h"

static inline void SWERROR_HANDLER() {
  int x = 1;
  while (1) {
    nrf_gpio_pin_write(SOCONLED, x);
    nrf_delay_ms(100);
    x ^= 1;
  }
}

typedef void (*buttonHandlers)(void);

typedef struct ButtonHandlerSetup {
  buttonHandlers topLeftButtonPress;
  buttonHandlers topRightButtonPress;
  buttonHandlers bottomLeftButtonPress;
  buttonHandlers bottomRightButtonPress;
} ButtonHandlerSetup;

typedef enum DisplayDirective_e { NoAction, UpdateDisplay } DisplayDirective_e;

static inline void emptyFunction(void) {}

#endif