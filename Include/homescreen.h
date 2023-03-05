#ifndef _HOMESCREEN_H_
#define _HOMESCREEN_H_

#include "main.h"

#define CLOCKX 45
#define CLOCKY 20
#define FILESX 45
#define FILESY 77
#define MUSICX 45
#define MUSICY 134

#define HOMSESCREENRECWIDTH 110
#define HOMESCREENRECHEIGHT 45

typedef enum HomescreenStates_e {
  homescreenClock,
  homescreeenFiles,
  homescreenMusic,
  homescreenSize
} HomescreenStates_e;

extern const ButtonHandlerSetup homescreenButtonHandlers;

void homescreenHandleTopLeftPress(void);
void homescreenHandleTopRightPress(void);
void homescreenHandleBottomRightPress(void);
HomescreenStates_e homescreenGetState(void);
void homeScreenDisplay(void);
#endif