#ifndef _SEND_RECEIVE_H_
#define _SEND_RECEIVE_H_

#define SENDRECEIVETIMEOUTCOUNT 60000
#define SENDRECEIVECLOSECOUNT 50

// void sendreceiveInit(void);
void sendReceiveStartUart(void);
void sendReceiveStopUart(void);
void sendReceiveGetData(void);

extern uint8_t someBuf[500];

#endif  //_SEND_RECEIVE_H_