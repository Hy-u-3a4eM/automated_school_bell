#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdint.h>

#include "esp_bit_defs.h"

enum {
      LESSONS_PER_DAY = 13
    , BELLS_PER_DAY   = 2 * LESSONS_PER_DAY
};

enum /*led_level*/ : uint32_t {
      LED_OFF = 0
    , LED_ON  = 1
};

enum /*bell_level*/ : uint32_t {
      STOP_BELL = 0
    , RING_BELL = 1
};

enum /*sta_bit*/ : uint32_t {
      STA_CONNECTED_BIT = BIT0
    , STA_FAIL_BIT      = BIT1
};

enum /*my_fail_bit*/ : uint32_t {
      MY_OK_BIT        = 0
    , MY_STA_FAIL_BIT  = BIT0
    , MY_SNTP_FAIL_BIT = BIT1
};

enum /*weekday*/ : uint8_t {
      SUNDAY
    , MONDAY
    , TUESDAY
    , WEDNESDAY
    , THURSDAY
    , FRIDAY
    , SATURDAY
    , MAX_WEEKDAY
};

enum /*unit_time*/ {
      MILLISECOND = 1
    , SECOND      = 1000 * MILLISECOND
    , MINUTE      = 60 * SECOND
};

extern const char *TAG;

#endif //CONSTANTS_H
