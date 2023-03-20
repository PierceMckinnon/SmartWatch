#include "main.h"

#include "app_uart.h"
#include "epaper.h"
#include "files.h"
#include "nrf_delay.h"
#include "nrf_drv_gpiote.h"
#include "nrf_uarte.h"
#include "pinconfig.h"

// static void sendreceiveGetPush(void);

static uint8_t sendReceiveBuf[512];
static uint8_t sendReceiveCompHap;
uint8_t someBuf[1024];

app_uart_buffers_t buffers = {sendReceiveBuf, sizeof(sendReceiveBuf),
                              &sendReceiveCompHap, 1};
const app_uart_comm_params_t uartParams = {UARTRX,
                                           UARTTX,
                                           UARTRTS,
                                           UARTCTS,
                                           APP_UART_FLOW_CONTROL_DISABLED,
                                           false,
                                           NRF_UARTE_BAUDRATE_9600};

void uartHandler(app_uart_evt_t* p_event) {
  // static int y = 1;
  // nrf_gpio_pin_write(SOCONLED, y);
  // nrf_delay_ms(500);
  if (p_event->evt_type == APP_UART_FIFO_ERROR ||
      p_event->evt_type == APP_UART_COMMUNICATION_ERROR) {
    epaperDisplayError(p_event->evt_type);
    SWERROR_HANDLER();
  }

  // y ^= 1;
}

void sendreceiveInit(void) {
  uint32_t err_code =
      app_uart_init(&uartParams, &buffers, uartHandler, APP_IRQ_PRIORITY_LOW);

  if (err_code != NRF_SUCCESS) {
    epaperDisplayError(err_code);
    SWERROR_HANDLER();
  }

  // nrf_gpio_pin_write(SOCONLED, 1);
  // nrf_delay_ms(500);
}

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

void sendreceivePushToSpi(void) {
  static uint8_t* pushPtr = someBuf;
  while (app_uart_get(pushPtr) == NRF_SUCCESS) {
    pushPtr++;
  }
}