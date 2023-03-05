#ifndef _MAIN_H_
#define _MAIN_H_

#include <stdbool.h>
#include <stdint.h>

#include "app_error.h"

static inline void SWERROR_HANDLER() {
  while (1)
    ;
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