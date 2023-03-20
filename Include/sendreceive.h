#ifndef _SEND_RECEIVE_H_
#define _SEND_RECEIVE_H_

void sendreceiveInit(void);
void sendreceivePushToSpi(void);

extern uint8_t someBuf[500];
#endif  //_SEND_RECEIVE_H_