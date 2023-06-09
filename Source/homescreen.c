#include "epaper.h"
#include "homescreen.h"
#include "nrf_drv_gpiote.h"

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
  // static int x = 1;
  // nrf_gpio_pin_write(SOCONLED, x);
  homeScreenDisplay();
  // x ^= 1;
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
  homescreenState = homescreenClock;
}

HomescreenStates_e homescreenGetState(void) {
  return homescreenState;
}

void homeScreenDisplay(void) {
  epaperDisplayHomescreen(homescreenDisplayLocations[homescreenState]);
}
