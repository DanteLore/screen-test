#pragma once

#include "esp_err.h"
#include <stdint.h>

// GET url into a malloc'd buffer. Caller must free *out_buf.
esp_err_t http_get(const char *url, uint8_t **out_buf, int *out_len);
