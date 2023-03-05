#ifndef _CALENDAR_H_
#define _CALENDAR_H_

#include "main.h"

#include "buttonconfig.h"
#include "nrf_drv_rtc.h"

#define DAYS_31 31
#define DAYS_30 30
#define DAYS_29 29
#define DAYS_28 28
#define MIN_DAY 1

#define MAX_YEAR 99
#define MIN_YEAR 0
#define MAX_MONTH 12
#define MIN_MONTH 1
#define MAX_HOUR 23
#define MIN_HOUR 0
#define MAX_MINUTE 59
#define MIN_MINUTE 0

#define DAYX 9
#define MONTHX 69
#define YEARX 130
#define DATEY 38

#define DATERECWIDTH 48
#define DATERECHEIGHT 34

#define HOURX 18
#define MINUTEX 100
#define TIMEY 118

#define TIMERECWIDTH 66
#define TIMERECHEIGHT 54

#define LEAP_YEAR_OCCURANCE 4

typedef enum CalMonths_e {
  calJanuary = 1,
  calFebruary,
  calMarch,
  calApril,
  calMay,
  calJune,
  calJuly,
  calAugust,
  calSeptember,
  calOctober,
  calNovember,
  calDecember
} CalMonths_e;

typedef enum CalMaxDays_e {
  calMax_31 = DAYS_31,
  calMax_30 = DAYS_30,
  calMax_29 = DAYS_29,
  calMax_28 = DAYS_28,
} CalMaxDays_e;

typedef enum CalStates_e {
  calUninitialized,
  calNormalClock,
  calEditing
} CalStates_e;

typedef enum CalVariables_e {
  calDay,
  calMonth,
  calYear,
  calHour,
  calMinute,
  calClear
} CalVariables_e;

typedef struct CalDateTime {
  uint8_t day;
  uint8_t month;
  uint8_t year;
  uint8_t hour;
  uint8_t minute;
} CalDateTime;

// RTC
#define CAl_RTC_CHANNEL_0 0
#define RTC_MINUTE_TICKS \
  (RTC_US_TO_TICKS(60000000ULL, RTC_DEFAULT_CONFIG_FREQUENCY))

extern const nrf_drv_rtc_t dateTimeRTC;

static inline void calUpdateRTCInt(void) {
  ret_code_t err_code;
  err_code =
      nrf_drv_rtc_cc_set(&dateTimeRTC, CAl_RTC_CHANNEL_0,
                         (nrf_rtc_cc_get(dateTimeRTC.p_reg, CAl_RTC_CHANNEL_0) +
                          RTC_MINUTE_TICKS) &
                             RTC_COUNTER_COUNTER_Msk,
                         true);
  APP_ERROR_CHECK(err_code);
}
static inline void calEnableRTCInt(void) {
  nrfx_rtc_counter_clear(&dateTimeRTC);
  nrf_drv_rtc_cc_set(&dateTimeRTC, CAl_RTC_CHANNEL_0, RTC_MINUTE_TICKS, true);
  nrf_drv_rtc_enable(&dateTimeRTC);
}

static inline void caldisableRTCInt(void) {
  nrfx_rtc_cc_disable(&dateTimeRTC, CAl_RTC_CHANNEL_0);
}

#include "epaper.h"

void calDisplay(void);
void calUpdateMinute(DisplayDirective_e displayDirective);
void calHandleTopLeftPress(void);
void calHandleTopRightPress(void);
void calHandleBottomLeftPress(void);
void calHandleBottomRightPress(void);

extern const ButtonHandlerSetup calButtonHandlers;

#endif