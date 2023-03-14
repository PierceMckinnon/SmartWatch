#ifndef _BUTTON_CONFIG_H_
#define _BUTTON_CONFIG_H_

#include "main.h"

#include "pinconfig.h"

#define BUTTONNUM 4

#define BUTTONTOPRIGHT_INDEX 0
#define BUTTONTOPLEFT_INDEX 1
#define BUTTONBOTTOMLEFT_INDEX 2
#define BUTTONBOTTOMRIGHT_INDEX 3

typedef enum Buttons_e {
  buttonTopRight = BUTTONTOPRIGHT,
  buttonTopLeft = BUTTONTOPLEFT,
  buttonBottomLeft = BUTTONBOTTOMLEFT,
  buttonBottomRight = BUTTONBOTTOMRIGHT,
} Buttons_e;

typedef enum ButtonTypePresses_e {
  buttonTopRightPress,
  buttonTopLeftPress,
  buttonBottomLeftPress,
  buttonBottomRightPress,
} ButtonTypePresses_e;

uint8_t buttonIndex(Buttons_e button);
void buttonEnableInterrupts(void);
void buttonDisableInterrupts(void);
Buttons_e getButtonPinAtIndex(uint8_t buttonIndex);

#endif
