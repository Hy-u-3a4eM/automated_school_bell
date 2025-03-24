#ifndef CONFIG_H
#define CONFIG_H

#include "driver/gpio.h"

#include "constants.h"

enum /*my_gpio_num*/ {
      LED_GPIO  = GPIO_NUM_2
    , BELL_GPIO = GPIO_NUM_13       // для примера
};

enum {
      SHORT_BLINK_DURATION_MS = 200
    , BLINK_INTERVAL_MS       = 3 * SECOND
};

#endif //CONFIG_H
