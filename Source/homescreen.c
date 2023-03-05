#include "epaper.h"
#include "homescreen.h"

const ButtonHandlerSetup homescreenButtonHandlers = {
    .topLeftButtonPress = homescreenHandleTopLeftPress,
    .topRightButtonPress = homescreenHandleTopRightPress,
    .bottomLeftButtonPress = homescreenHandleBottomLeftPress,
    .bottomRightButtonPress = homescreenHandleBottomRightPress};

static HomescreenStates_e homescreenState = homescreenClock;

const EpaperPrintPosition homescreenDisplayLocations[homescreenSize] = {
    {.x = CLOCKX, .y = CLOCKY},
    {.x = FILESX, .y = FILESY},
    {.x = MUSICX, .y = MUSICY}};

static void homeScreenMoveUp(void);
static void homeScreenMoveDown(void);
static void homeScreenSelect(void);

void homescreenHandleTopLeftPress(void) {
  homeScreenMoveUp();
}

void homescreenHandleTopRightPress(void) {
  homeScreenMoveDown();
}

void homescreenHandleBottomRightPress(void) {
  homeScreenSelect();
}

void homescreenHandleBottomLeftPress(void) {
  homeScreenDisplay();
}

static void homeScreenMoveUp(void) {
  if (homescreenState == homescreenClock)
    homescreenState = homescreenMusic;
  else
    homescreenState--;

  epaperDisplayHomescreen(homescreenDisplayLocations[homescreenState]);
}

static void homeScreenMoveDown(void) {
  if (homescreenState == homescreenMusic)
    homescreenState = homescreenClock;
  else
    homescreenState++;

  epaperDisplayHomescreen(homescreenDisplayLocations[homescreenState]);
}

static void homeScreenSelect(void) {
  epaperDisplayHomscreenSelect(homescreenState);
}

HomescreenStates_e homescreenGetState(void) {
  return homescreenState;
}

void homeScreenDisplay(void) {
  epaperDisplayHomescreen(homescreenDisplayLocations[homescreenState]);
}
