#include "st7789.h"
#include "wifi.h"
#include "manifest.h"
#include "status.h"
#include "jpeg.h"
#include "config.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static const char *TAG = "main";

static void draw_stats(const cam_status_t *s)
{
    st7789_fill_rect(STATS_X, 0, STATS_W, ST7789_HEIGHT, COLOR_BLACK);

    char buf[16];

    int cam_x = STATS_X + (STATS_W - 8 * (int)strlen(CAM_ID)) / 2;
    st7789_draw_string(cam_x, STATS_CAM_Y, CAM_ID, COLOR_WHITE, COLOR_BLACK);

    int cx = STATS_X + (STATS_W - 8 * 8) / 2;  // centre 8-char string in panel
    st7789_draw_string(cx, STATS_TIME_Y, s->time_str, COLOR_WHITE, COLOR_BLACK);

    snprintf(buf, sizeof(buf), "%.1fv", s->battery_voltage);
    int volt_x = STATS_X + (STATS_W - 8 * (int)strlen(buf)) / 2;
    st7789_draw_string(volt_x, STATS_VOLT_Y, buf, COLOR_WHITE, COLOR_BLACK);

}

void app_main(void)
{
    nvs_flash_init();
    wifi_init();
    wifi_connect();
    st7789_init();
    st7789_fill(COLOR_BLACK);

    while (1) {
        char *url = manifest_get_tiny_url();
        cam_status_t status = {0};
        bool have_status = status_get(&status);

        if (url) {
            jpeg_draw(url, IMG_X, IMG_Y);
            free(url);
        } else {
            ESP_LOGW(TAG, "no image URL");
        }

        if (have_status)
            draw_stats(&status);

        vTaskDelay(pdMS_TO_TICKS(30000));
    }
}
