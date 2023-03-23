#include "main.h"

#include <stdio.h>
#include "app_uart.h"
#include "buttonconfig.h"
#include "epaper.h"
#include "files.h"
#include "nrf_delay.h"
#include "nrf_drv_gpiote.h"
#include "nrf_uarte.h"
#include "pinconfig.h"
#include "sendreceive.h"
#include "timerconfig.h"
#include "vibration.h"

// static void sendreceiveGetPush(void);
static void sendReceiveBuildFilename(uint8_t character);

typedef enum SendReceiveStates_e {
  sendreceiveInit,
  sendreceiveLookForTitle,
  sendreceivePushText,
  sendreceiveDone,
  sendreceiveTimeout
} SendReceiveStates_e;

SendReceiveStates_e sendreceiveState = sendreceiveInit;

static uint8_t sendReceiveBuf[2048];
static uint8_t sendReceiveCompHap;

app_uart_buffers_t buffers = {sendReceiveBuf, sizeof(sendReceiveBuf),
                              &sendReceiveCompHap, 1};
const app_uart_comm_params_t uartParams = {UARTRX,
                                           UARTTX,
                                           UARTRTS,
                                           UARTCTS,
                                           APP_UART_FLOW_CONTROL_DISABLED,
                                           false,
                                           NRF_UARTE_BAUDRATE_115200};

void uartHandler(app_uart_evt_t* p_event) {
  if (p_event->evt_type == APP_UART_FIFO_ERROR ||
      p_event->evt_type == APP_UART_COMMUNICATION_ERROR) {
    epaperDisplayError(p_event->evt_type);
    SWERROR_HANDLER();
  }
}

// void sendreceiveInit(void) {
//   uint32_t err_code =
//       app_uart_init(&uartParams, &buffers, uartHandler,
//       APP_IRQ_PRIORITY_LOW);

//   if (err_code != NRF_SUCCESS) {
//     epaperDisplayError(err_code);
//     SWERROR_HANDLER();
//   }

//   // nrf_gpio_pin_write(SOCONLED, 1);
//   // nrf_delay_ms(500);
// }

// void sendreceiveGetPush(void) {
//   static int j = 0;
//   // static int x = 0;
//   // nrf_gpio_pin_write(SOCONLED, x);
//   static uint8_t* pushPtr = someBuf;
//   uint32_t err_code = app_uart_get(pushPtr);
//   if (err_code != NRF_SUCCESS) {
//     epaperDisplayError(3005);
//     SWERROR_HANDLER();
//   }
//   // if (*pushPtr != 0) {
//   //   epaperDisplayError(*pushPtr);
//   // }

//   pushPtr++;
//   j++;

//   // if (j == 300) {
//   //   filesDisplayLength(j);

//   //   j = 0;
//   // }
//   // x ^= 1;
// }

void sendReceiveStartUart(void) {
  sendreceiveState = sendreceiveInit;
  buttonDisableInterrupts();
  filesDisplayUartProcessing();
  app_uart_init(&uartParams, &buffers, uartHandler, APP_IRQ_PRIORITY_HIGHEST);
  timerEnableUart();
}

void sendReceiveStopUart(void) {
  timerDisableUart();
  app_uart_flush();
  app_uart_close();
  sendreceiveState = sendreceiveInit;
}

void sendReceiveGetData(void) {
  uint8_t getCharacter;
  static uint32_t timeoutCount = 0;
  bool incTimeout = true;

  while (app_uart_get(&getCharacter) == NRF_SUCCESS) {
    incTimeout = false;
    switch (sendreceiveState) {
      case (sendreceiveInit): {
        sendreceiveState = sendreceiveLookForTitle;
      }
      case (sendreceiveLookForTitle): {
        sendReceiveBuildFilename(getCharacter);
        break;
      }
      case (sendreceivePushText): {
        filesFileWrite(&getCharacter, sizeof(getCharacter));
        break;
      }
      case (sendreceiveDone):
      case (sendreceiveTimeout): {
        buttonDisableInterrupts();
        filesDisplayUartProcessing();
        sendReceiveBuildFilename(getCharacter);
        sendreceiveState = sendreceiveLookForTitle;
        break;
      }
    }
  }

  if (incTimeout)
    timeoutCount++;
  else
    timeoutCount = 0;

  if (timeoutCount == SENDRECEIVETIMEOUTCOUNT) {
    if (sendreceiveState == sendreceiveInit) {
      buttonEnableInterrupts();
    }
    app_uart_flush();
    filesDisplayUartNothingSent();

    sendreceiveState = sendreceiveTimeout;
    timeoutCount = 0;
  } else if (timeoutCount == SENDRECEIVECLOSECOUNT &&
             (sendreceiveState == sendreceivePushText)) {
    filesFileClose();
    app_uart_flush();
    vibrateStart();
    filesDisplayUartDone();
    buttonEnableInterrupts();
    sendreceiveState = sendreceiveDone;
  }
}

char filenameBuffer[FILESMAXTITLESIZE];
char filenameCopyBuffer[FILESMAXTITLESIZE];
static void sendReceiveBuildFilename(uint8_t character) {
  static int index = 0;
  if (character == 0x0A) {
    if (filenameBuffer[index - 1] == 0x0D) {
      snprintf(filenameCopyBuffer, index, "%s", filenameBuffer);
      filesFileOpenForWrite(filenameCopyBuffer);  // fifo is emptying
      sendreceiveState = sendreceivePushText;
      index = 0;
      return;
    } else {
      SWERROR_HANDLER();
    }
  }
  filenameBuffer[index] = character;
  index++;
}