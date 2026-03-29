#include "jpeg.h"

#include "http.h"
#include "st7789.h"
#include "esp32/rom/tjpgd.h"
#include "esp_log.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG = "jpeg";

// Work buffer for TJpgDec (minimum 3100 bytes)
#define TJPGD_WORK_SIZE 3500

typedef struct {
    const uint8_t *data;
    int            len;
    int            pos;
    int            x;
    int            y;
} decode_ctx_t;

static unsigned jpeg_input(JDEC *jdec, uint8_t *buf, unsigned nd)
{
    decode_ctx_t *ctx = (decode_ctx_t *)jdec->device;
    int remaining = ctx->len - ctx->pos;
    if ((int)nd > remaining) nd = (unsigned)remaining;
    if (buf)
        memcpy(buf, ctx->data + ctx->pos, nd);
    ctx->pos += nd;
    return nd;
}

static unsigned int jpeg_output(JDEC *jdec, void *bitmap, JRECT *rect)
{
    decode_ctx_t *ctx = (decode_ctx_t *)jdec->device;
    int w = rect->right  - rect->left + 1;
    int h = rect->bottom - rect->top  + 1;
    int n = w * h;

    // Convert RGB888 → RGB565 big-endian
    uint8_t  *src  = (uint8_t *)bitmap;
    uint16_t *px   = malloc(n * 2);
    if (!px) return 0;

    for (int i = 0; i < n; i++) {
        uint8_t r = src[i * 3 + 0];
        uint8_t g = src[i * 3 + 1];
        uint8_t b = src[i * 3 + 2];
        uint16_t c = RGB565(r, g, b);
        // big-endian for SPI
        px[i] = (c >> 8) | (c << 8);
    }

    st7789_draw_pixels(ctx->x + rect->left, ctx->y + rect->top, w, h, px);
    free(px);
    return 1;
}

bool jpeg_draw(const char *url, int x, int y)
{
    uint8_t *buf;
    int len;
    if (http_get(url, &buf, &len) != ESP_OK)
        return false;

    void *work = malloc(TJPGD_WORK_SIZE);
    if (!work) {
        free(buf);
        return false;
    }

    decode_ctx_t ctx = { .data = buf, .len = len, .pos = 0, .x = x, .y = y };

    JDEC jdec;
    JRESULT rc = jd_prepare(&jdec, jpeg_input, work, TJPGD_WORK_SIZE, &ctx);
    if (rc != JDR_OK) {
        ESP_LOGE(TAG, "jd_prepare failed: %d", rc);
        free(work);
        free(buf);
        return false;
    }

    ESP_LOGI(TAG, "JPEG %ux%u", jdec.width, jdec.height);

    rc = jd_decomp(&jdec, jpeg_output, 0);
    if (rc != JDR_OK)
        ESP_LOGE(TAG, "jd_decomp failed: %d", rc);

    free(work);
    free(buf);
    return rc == JDR_OK;
}
