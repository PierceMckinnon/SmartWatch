#include "buttonconfig.h"
#include "nrf_drv_gpiote.h"

const Buttons_e buttonsPinList[BUTTONNUM] = {
    buttonTopRight, buttonTopLeft, buttonBottomLeft, buttonBottomRight};
// const Buttons_e buttonsPinList[BUTTONNUM] = {buttonTopLeft};
static int buttonInterruptsState = 0;

uint8_t buttonIndex(Buttons_e button) {
  uint8_t buttonIndex;
  switch (button) {
    case buttonTopRight: {
      buttonIndex = BUTTONTOPLEFT_INDEX;
      break;
    }
    case buttonTopLeft: {
      buttonIndex = BUTTONTOPRIGHT_INDEX;
      break;
    }
    case buttonBottomLeft: {
      buttonIndex = BUTTONBOTTOMLEFT_INDEX;
      break;
    }
    case buttonBottomRight: {
      buttonIndex = BUTTONBOTTOMRIGHT_INDEX;
      break;
    }
    default:
      SWERROR_HANDLER();
  }
  return buttonIndex;
}

void buttonFirstEnableInterrupts(void) {
  for (int i = 0; i < BUTTONNUM; i++) {
    nrf_drv_gpiote_in_event_enable((int)buttonsPinList[i], true);
  }
}

void buttonEnableInterrupts(void) {
  buttonInterruptsState++;
  if (buttonInterruptsState == 0) {
    nrf_gpio_pin_write(SOCONLED, 1);
    for (int i = 0; i < BUTTONNUM; i++) {
      nrf_drv_gpiote_in_event_enable((int)buttonsPinList[i], true);
    }
  }
}

void buttonDisableInterrupts(void) {
  nrf_gpio_pin_write(SOCONLED, 0);
  buttonInterruptsState--;
  for (int i = 0; i < BUTTONNUM; i++) {
    nrf_drv_gpiote_in_event_disable((int)buttonsPinList[i]);
  }
}

Buttons_e getButtonPinAtIndex(uint8_t buttonIndex) {
  return buttonsPinList[buttonIndex];
}