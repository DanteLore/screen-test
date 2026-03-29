#include "status.h"
#include "config.h"

#include "http.h"
#include "cJSON.h"
#include "esp_log.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static const char *TAG = "status";

bool status_get(cam_status_t *out)
{
    char url[128];
    snprintf(url, sizeof(url), "%s/status/%s/status.json", CAM_BASE_URL, CAM_ID);

    uint8_t *buf;
    int len;
    if (http_get(url, &buf, &len) != ESP_OK)
        return false;

    cJSON *json = cJSON_Parse((char *)buf);
    free(buf);

    if (!json) {
        ESP_LOGE(TAG, "JSON parse failed");
        return false;
    }

    cJSON *voltage    = cJSON_GetObjectItem(json, "battery_voltage");
    cJSON *uptime     = cJSON_GetObjectItem(json, "uptime");
    cJSON *recorded_at = cJSON_GetObjectItem(json, "recorded_at");

    if (!voltage || !uptime || !recorded_at) {
        ESP_LOGE(TAG, "missing fields");
        cJSON_Delete(json);
        return false;
    }

    out->battery_voltage = (float)cJSON_GetNumberValue(voltage);
    out->uptime          = (int)cJSON_GetNumberValue(uptime);

    // recorded_at format: "2025-03-15T14:30:22.123456+00:00"
    // Extract HH:MM:SS starting at index 11
    const char *ts = recorded_at->valuestring;
    if (strlen(ts) >= 19)
        snprintf(out->time_str, sizeof(out->time_str), "%.8s", ts + 11);
    else
        strncpy(out->time_str, "??:??:??", sizeof(out->time_str));

    cJSON_Delete(json);
    return true;
}
