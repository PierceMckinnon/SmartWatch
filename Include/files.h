#ifndef _FILES_H_
#define _FILES_H_

#include "main.h"

extern const ButtonHandlerSetup filesButtonHandlers;
void filesHandleTopLeftPress(void);
void filesHandleTopRightPress(void);
void filesHandleBottomLeftPress(void);
void filesHandleBottomRightPress(void);

void filesDisplay(void);
void filesDisplayLength(uint32_t textLength);
void filesEmptyTheBuffer(void);
#endif  //_FILES_H_