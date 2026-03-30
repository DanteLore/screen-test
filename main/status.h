#pragma once

#include <stdbool.h>

typedef struct {
    float battery_voltage;
    char  time_str[12];  // "HH:MM:SS"
} cam_status_t;

// Fetch and parse status. Returns true on success.
bool status_get(cam_status_t *out);
