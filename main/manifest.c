#include "manifest.h"
#include "config.h"

#include "http.h"
#include "cJSON.h"
#include "esp_log.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static const char *TAG = "manifest";

char *manifest_get_tiny_url(void)
{
    char url_buf[128];
    snprintf(url_buf, sizeof(url_buf), "%s/status/%s/manifest.json", CAM_BASE_URL, CAM_ID);

    uint8_t *buf;
    int len;
    if (http_get(url_buf, &buf, &len) != ESP_OK)
        return NULL;

    cJSON *json = cJSON_Parse((char *)buf);
    free(buf);

    if (!json) {
        ESP_LOGE(TAG, "JSON parse failed");
        return NULL;
    }

    cJSON *latest_tiny = cJSON_GetObjectItem(json, "latest_tiny");
    if (!latest_tiny || !cJSON_IsString(latest_tiny)) {
        ESP_LOGE(TAG, "latest_tiny not found in manifest");
        cJSON_Delete(json);
        return NULL;
    }

    size_t img_url_len = strlen(CAM_BASE_URL) + 1 + strlen(latest_tiny->valuestring) + 1;
    char *url = malloc(img_url_len);
    snprintf(url, img_url_len, "%s/%s", CAM_BASE_URL, latest_tiny->valuestring);

    cJSON_Delete(json);
    return url;
}
