#include "calendar.h"

const ButtonHandlerSetup calButtonHandlers = {
    .topLeftButtonPress = calHandleTopLeftPress,
    .topRightButtonPress = calHandleTopRightPress,
    .bottomLeftButtonPress = calHandleBottomLeftPress,
    .bottomRightButtonPress = calHandleBottomRightPress};

const nrf_drv_rtc_t dateTimeRTC = NRF_DRV_RTC_INSTANCE(0);

static const uint8_t calMinVals[calClear] = {MIN_DAY, MIN_MONTH, MIN_YEAR,
                                             MIN_HOUR, MIN_MINUTE};
static CalDateTime calDateTime = {31, 12, 99, 23, 59};
static CalStates_e calState = calUninitialized;
static CalVariables_e calVariableHighlight = calClear;

const EpaperPrintPosition calDisplayLocations[calClear] = {
    {.x = DAYX, .y = DATEY},
    {.x = MONTHX, .y = DATEY},
    {.x = YEARX, .y = DATEY},
    {.x = HOURX, .y = TIMEY},
    {.x = MINUTEX, .y = TIMEY}};

static void calUpdateHour();
static void calUpdateDay();
static void calUpdateMonth();
static void calUpdateYear();
static CalMaxDays_e calGetMonthMaxDay(CalMonths_e month, uint32_t year);
static void calExitEditing(void);
static void calEnterEditing(void);
static void calDisplayUpdate(CalVariables_e calNewHighlight);
static void calIncrementHighlight(void);
static void calDecrementHighlight(void);
static uint8_t calGetMaxVal(CalVariables_e item);
static uint8_t* calGetVal(CalVariables_e item);
static void calMoveHighlightRight(void);

void calUpdateMinute(DisplayDirective_e displayDirective) {
  if (calDateTime.minute + 1 > MAX_MINUTE) {
    calDateTime.minute = MIN_MINUTE;
    if (calState == calNormalClock)
      calUpdateHour();
  } else {
    calDateTime.minute++;
  }

  if (displayDirective == UpdateDisplay) {
    calDisplay();
  }
}

static void calUpdateHour() {
  if (calDateTime.hour + 1 > MAX_HOUR) {
    calDateTime.hour = MIN_HOUR;
    if (calState == calNormalClock)
      calUpdateDay();
  } else {
    calDateTime.hour++;
  }
}

static void calUpdateDay() {
  if (calDateTime.day + 1 >
      calGetMonthMaxDay((CalMonths_e)calDateTime.month, calDateTime.year)) {
    calDateTime.day = MIN_DAY;
    if (calState == calNormalClock)
      calUpdateMonth();
  } else {
    calDateTime.day++;
  }
}

static void calUpdateMonth() {
  if (calDateTime.month + 1 > MAX_MONTH) {
    calDateTime.month = MIN_MONTH;
    if (calState == calNormalClock)
      calUpdateYear();
  } else {
    calDateTime.month++;
  }
}

static void calUpdateYear() {
  if (calDateTime.year + 1 > MAX_YEAR) {
    calDateTime.year = MIN_YEAR;
  } else {
    calDateTime.year++;
  }
}

static CalMaxDays_e calGetMonthMaxDay(CalMonths_e month, uint32_t year) {
  CalMaxDays_e maxDays;
  switch (month) {
    case (calJanuary):
    case (calMarch):
    case (calMay):
    case (calJuly):
    case (calAugust):
    case (calOctober):
    case (calDecember): {
      maxDays = calMax_31;
      break;
    }
    case (calApril):
    case (calJune):
    case (calSeptember):
    case (calNovember): {
      maxDays = calMax_30;
      break;
    }
    case (calFebruary): {
      maxDays = (year % LEAP_YEAR_OCCURANCE == 0) ? calMax_29 : calMax_28;
      break;
    }
    default:  // should not hit
    {
      SWERROR_HANDLER();
    }
  }
  return maxDays;
}

void calHandleTopLeftPress(void) {
  switch (calState) {
    case (calUninitialized): {
      calEnterEditing();
      break;
    }
    case (calNormalClock): {
      calEnterEditing();
      break;
    }
    case (calEditing): {
      calExitEditing();
      break;
    }
    default:
      SWERROR_HANDLER();
  }
}

void calHandleTopRightPress() {
  switch (calState) {
    case (calUninitialized):
    case (calNormalClock):
      break;
    case (calEditing): {
      calMoveHighlightRight();
      break;
    }
    default:
      SWERROR_HANDLER();
  }
}

void calHandleBottomRightPress(void) {
  switch (calState) {
    case (calUninitialized):
    case (calNormalClock):
      break;
    case (calEditing): {
      calIncrementHighlight();
      break;
    }
    default:
      SWERROR_HANDLER();
  }
}

void calHandleBottomLeftPress(void) {
  switch (calState) {
    case (calUninitialized):
    case (calNormalClock): {
      epaperExitCalendar();
      break;
    }
    case (calEditing): {
      calDecrementHighlight();
      break;
    }
    default:
      SWERROR_HANDLER();
  }
}

static void calEnterEditing(void) {
  caldisableRTCInt();
  calDisplayUpdate(calDay);
  calState = calEditing;
}

static void calExitEditing(void) {
  calDisplayUpdate(calClear);
  calEnableRTCInt();
  calState = calNormalClock;
}

static void calDisplayUpdate(CalVariables_e calNewHighlight) {
  calVariableHighlight = calNewHighlight;
  epaperUpdateDateTime(&calDateTime, calVariableHighlight,
                       calDisplayLocations[calVariableHighlight]);
}

void calDisplay(void) {
  epaperUpdateDateTime(&calDateTime, calVariableHighlight,
                       calDisplayLocations[calVariableHighlight]);
}

static void calMoveHighlightRight(void) {
  CalVariables_e newHighlight;
  if (calVariableHighlight == calMinute)
    newHighlight = calDay;
  else
    newHighlight = calVariableHighlight + 1;

  calDisplayUpdate(newHighlight);
}

static uint8_t* calGetVal(CalVariables_e item) {
  uint8_t* getVal;
  switch (item) {
    case (calDay): {
      getVal = &(calDateTime.day);
      break;
    }
    case (calMonth): {
      getVal = &(calDateTime.month);
      break;
    }
    case (calYear): {
      getVal = &(calDateTime.year);
      break;
    }
    case (calHour): {
      getVal = &(calDateTime.hour);
      break;
    }
    case (calMinute): {
      getVal = &(calDateTime.minute);
      break;
    }
    default:
      SWERROR_HANDLER();
  }
  return getVal;  // Never reach here
}

static uint8_t calGetMaxVal(CalVariables_e item) {
  uint8_t maxVal;
  switch (item) {
    case (calDay): {
      maxVal =
          calGetMonthMaxDay((CalMonths_e)calDateTime.month, calDateTime.year);
      break;
    }
    case (calMonth): {
      maxVal = MAX_MONTH;
      break;
    }
    case (calYear): {
      maxVal = MAX_YEAR;
      break;
    }
    case (calHour): {
      maxVal = MAX_HOUR;
      break;
    }
    case (calMinute): {
      maxVal = MAX_MINUTE;
      break;
    }
    default:
      SWERROR_HANDLER();
  }
  return maxVal;
}

static void calDecrementHighlight(void) {
  uint8_t* highlightRef = calGetVal(calVariableHighlight);
  if (*highlightRef - 1 < calMinVals[calVariableHighlight])
    *highlightRef = calGetMaxVal(calVariableHighlight);
  else
    (*highlightRef)--;
  calDisplay();
}

static void calIncrementHighlight(void) {
  uint8_t* highlightRef = calGetVal(calVariableHighlight);
  if (*highlightRef + 1 > calGetMaxVal(calVariableHighlight))
    *highlightRef = calMinVals[calVariableHighlight];
  else
    (*highlightRef)++;
  calDisplay();
}
