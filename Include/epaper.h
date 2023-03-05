
#ifndef _EPAPER_H_
#define _EPAPER_H_

#include "DEV_Config.h"
#include "GUI_Paint.h"
#include "bitmaps.h"
#include "calendar.h"
#include "homescreen.h"

#define EPAPERMAXPARTIALREFRESH 20

typedef enum EpaperOptions_e {
  epaperTryPartialRefresh,
  epaperFullRefresh
} EpaperOptions_e;

typedef enum EpaperState_e {
  epaperHomeScreen,
  epaperDateTime,
  epaperMusicIO,
  epaperFileIO,
  epaperStatesSize
} EpaperState_e;

typedef enum EpaperMode_e { epaperSleepMode, epaperAliveMode } EpaperMode_e;

typedef enum EpaperBlock_e { epaperBlock = 0, epaperNoBlock } EpaperBlock_e;

typedef enum EpaperSetupType_e { epaperBitmap, epaperText } EpaperSetupType_e;

typedef enum EpaperEntryCount_e {
  epaperFirstEntry,
  epaperMultiEntry
} EpaperEntryCount_e;

typedef struct EpaperPrintPosition {
  uint8_t x;
  uint8_t y;
} EpaperPrintPosition;

void epaperInit(void);
void epaperTesting(void);
void EPD_SHIFT();
void epaperSleep(void);
void epaperTurnOnDisplay(EpaperSetupType_e setupType);
EpaperState_e epaperGetState(void);
EpaperMode_e epaperGetMode(void);
EpaperBlock_e epaperGetBlockState(void);
void epaperEnableDisplayChangeSources(void);
void epaperDisplayHomescreen(const EpaperPrintPosition printPosition);
void epaperDisplayHomscreenSelect(HomescreenStates_e homescreenState);
void epaperUpdateDateTime(CalDateTime const* const dateTime,
                          CalVariables_e highlight,
                          const EpaperPrintPosition printPosition);
void epaperExitCalendar(void);
void epaperWakeFromSleep(void);

#endif
