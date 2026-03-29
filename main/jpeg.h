#pragma once

#include <stdbool.h>

// Fetch JPEG from url, decode and draw at (x, y) on screen.
bool jpeg_draw(const char *url, int x, int y);
