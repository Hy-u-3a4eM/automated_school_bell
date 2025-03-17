#include <stdatomic.h>

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE // Важно для VERBOSE!!!
#include "esp_log.h"

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_sntp.h"

#include ".env.h"
#include "constants.h"
#include "schedule.h"
#include "structures.h"

#define countof(a) (sizeof(a) / sizeof((a)[0]))

static EventGroupHandle_t s_event_group;

static SemaphoreHandle_t s_led_xMutex;

static time_t now;
static struct tm timeinfo;

atomic_uchar s_errno = MY_SNTP_FAIL_BIT;

[[noreturn]] static void blink_err_bits_task(void *pvParameters) {
    while (true) {
        if (xSemaphoreTake(s_led_xMutex, portMAX_DELAY) == pdTRUE) {
            const unsigned char bits = atomic_load(&s_errno);
            ESP_LOGV(TAG, "blink_err_bits_task: %d", bits);

            for (unsigned char i = 0; i < bits; i++) {
                gpio_set_level(LED_GPIO, LED_ON);
                vTaskDelay(pdMS_TO_TICKS(SHORT_BLINK_DURATION_MS));
                gpio_set_level(LED_GPIO, LED_OFF);
                vTaskDelay(pdMS_TO_TICKS(SHORT_BLINK_DURATION_MS));
            }

            if (xSemaphoreGive(s_led_xMutex) != pdTRUE) {
                ESP_LOGW(TAG, "Не удалось освободить мьютекс светодиода для мигания ошибок");
            }
        } else {
            ESP_LOGW(TAG, "Не удалось захватить мьютекс светодиода для мигания ошибок");
        }

        vTaskDelay(pdMS_TO_TICKS(BLINK_INTERVAL_MS - 2 * SHORT_BLINK_DURATION_MS));
    }
}

static void configure_led(void) {
    //ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
    ESP_LOGV(TAG, "Инициализация светодиода");
    gpio_reset_pin(LED_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
}

// Заглушка
static void configure_bell(void) {
    ESP_LOGV(TAG, "Инициализация (реле) звонка");
    //gpio_reset_pin(BELL_GPIO);
    //gpio_set_direction(BELL_GPIO, GPIO_MODE_OUTPUT);
}

static void wifi_event_handler([[maybe_unused]] void *arg,
    const esp_event_base_t event_base,
    const int32_t event_id,
    // ReSharper disable once CppParameterMayBeConstPtrOrRef
    void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG,"Connect to the AP fail. Retry to connect to the AP");
        xEventGroupSetBits(s_event_group, STA_FAIL_BIT);
        atomic_fetch_or(&s_errno, MY_STA_FAIL_BIT);

        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(s_event_group, STA_CONNECTED_BIT);
        atomic_fetch_and(&s_errno, ~MY_STA_FAIL_BIT);

        const ip_event_got_ip_t *event = event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    }
}

void sta_init(void) {
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(  WIFI_EVENT
                                                        , ESP_EVENT_ANY_ID
                                                        , &wifi_event_handler
                                                        , nullptr
                                                        , nullptr));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(  IP_EVENT
                                                        , IP_EVENT_STA_GOT_IP
                                                        , &wifi_event_handler
                                                        , nullptr
                                                        , nullptr));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = STA_SSID,
            .password = STA_PASSWORD,
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (password len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
             * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            //.threshold.authmode = WIFI_AUTH_WEP,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "sta_init finished.");

    EventBits_t bits = xEventGroupWaitBits(  s_event_group
                                           , STA_FAIL_BIT | STA_CONNECTED_BIT
                                           , pdFALSE
                                           , pdFALSE                // TODO: А что здесь ставить-то?
                                           , portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits == STA_CONNECTED_BIT) {
        ESP_LOGI(  TAG
                 , "connected to ap SSID:%s password:%s"
                 , STA_SSID
                 , STA_PASSWORD);
    } else if (bits & STA_FAIL_BIT) {
        ESP_LOGI(  TAG
                 , "Failed to connect to SSID:%s, password:%s"
                 , STA_SSID
                 , STA_PASSWORD);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}

// Функция для вывода информации о времени при получении уведомления от SNTP
void time_sync_notification_cb(struct timeval *tv) {
    ESP_LOGI(TAG, "Уведомление о событии синхронизации времени");
}

// Инициализация SNTP
static void initialize_sntp(void) {
    ESP_LOGI(TAG, "Инициализация SNTP");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    esp_sntp_init();
}

static void update_time(void) {
    time(&now);
    localtime_r(&now, &timeinfo);
}

// Функция для получения и вывода времени
static void obtain_time(void) {
    // Ожидание синхронизации времени

    while (timeinfo.tm_year < 2025 - 1900) {
        ESP_LOGV(TAG, "Ожидание синхронизации времени...");
        //vTaskDelay(pdMS_TO_TICKS(MINUTE));
        update_time();

        vTaskDelay(pdMS_TO_TICKS(SECOND));
    }

    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "Синхронизация завершена. Текущее время: %s", strftime_buf);

    ESP_LOGV(TAG, "Убираем бит ошибки SNTP...");
    atomic_fetch_and(&s_errno, ~MY_SNTP_FAIL_BIT);
}

// ReSharper disable once CppDFAConstantParameter
static void bell_set_level([[maybe_unused]] const gpio_num_t bell_gpio, const uint32_t level) {
    gpio_set_level(LED_GPIO, level);
}

[[noreturn]] static void schedule_blink_task(void *pvParameter) {
    while (true) {
        // Проверяем, активна ли запись расписания для текущей минуты
        bool blink_now = false;
        for (int i = 0; i < BELLS_PER_DAY; i++) {
            if ((    timeinfo.tm_wday == MONDAY
                 &&  timeinfo.tm_hour == schedule_monday[i].hour
                 &&  timeinfo.tm_min  == schedule_monday[i].minute)
                 || (timeinfo.tm_wday != MONDAY
                 &&  timeinfo.tm_hour == schedule_other_day[i].hour
                 &&  timeinfo.tm_min  == schedule_other_day[i].minute)) {
                ESP_LOGV(TAG, "Время мигать!");
                blink_now = true;
                break;
            }
        }

        if (blink_now) {
            if (xSemaphoreTake(s_led_xMutex, portMAX_DELAY) == pdTRUE) {
                ESP_LOGV(TAG, "Звонок звенит");
                bell_set_level(BELL_GPIO, RING_BELL);

                vTaskDelay(pdMS_TO_TICKS(MINUTE));

                ESP_LOGV(TAG, "Звонок больше не звенит");
                bell_set_level(BELL_GPIO, STOP_BELL);

                if (xSemaphoreGive(s_led_xMutex) != pdTRUE) {
                    ESP_LOGW(TAG, "Не удалось освободить мьютекс светодиода для звонка");
                }
            } else {
                ESP_LOGW(TAG, "Не удалось захватить мьютекс светодиода для звонка");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(SECOND));
    }
}

[[noreturn]] static void update_time_task(void *pvParameter) {
    while (true) {
        update_time();

        char strftime_buf[64];
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        ESP_LOGV(TAG, "Текущее время: %s", strftime_buf);

        vTaskDelay(pdMS_TO_TICKS(SECOND));
    }
}

void app_main() {
    esp_log_level_set("*", ESP_LOG_VERBOSE);        // Это тоже нужно

    setenv("TZ", "UTC-5", 1);
    tzset();

    vTaskDelay(pdMS_TO_TICKS(3 * SECOND));

    s_event_group = xEventGroupCreate();
    if (s_event_group == nullptr) {
        ESP_LOGW(TAG, "Не удалось создать s_event_group");
    }

    s_led_xMutex = xSemaphoreCreateMutex();
    if (s_led_xMutex == nullptr) {
        ESP_LOGW(TAG, "Не удалось создать мьютекс светодиода");
    }

    xTaskCreate(blink_err_bits_task
        , "blink_err_bits_task"
        , 2048
        , nullptr
        , 5
        , nullptr);

    configure_led();
    gpio_set_level(LED_GPIO, LED_OFF);

    configure_bell();
    bell_set_level(BELL_GPIO, STOP_BELL);

    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    sta_init();

    initialize_sntp();
    xTaskCreate(update_time_task
        , "say_time_task"
        , 2048
        , nullptr
        , 5
        , nullptr);
    obtain_time();

    xTaskCreate(schedule_blink_task
        , "schedule_blink_task"
        , 2048
        , nullptr
        , 5
        , nullptr);

    // TODO: рефакторинг магических чисел
    // TODO: обработка ошибок, включая NTP

    // TODO: logv
    // TODO: исправить время
    // TODO: изменить функцию на подать_ток_на_реле()
}
