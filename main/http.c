#include "http.h"

#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_log.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG = "http";

esp_err_t http_get(const char *url, uint8_t **out_buf, int *out_len)
{
    esp_http_client_config_t config = {
        .url               = url,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "open failed: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return err;
    }

    int content_len = esp_http_client_fetch_headers(client);
    // Fall back to a generous cap if server doesn't send Content-Length
    int buf_size = (content_len > 0) ? content_len : 65536;

    uint8_t *buf = malloc(buf_size + 1);  // +1 for null terminator
    if (!buf) {
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return ESP_ERR_NO_MEM;
    }

    int total = 0, r;
    while ((r = esp_http_client_read(client, (char *)(buf + total), buf_size - total)) > 0)
        total += r;

    buf[total] = '\0';

    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    ESP_LOGI(TAG, "GET %s → %d bytes", url, total);
    *out_buf = buf;
    *out_len = total;
    return ESP_OK;
}
