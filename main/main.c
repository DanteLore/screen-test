#include "st7789.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    st7789_init();

    st7789_fill(COLOR_BLACK);

    // Colour bars
    st7789_fill_rect(10, 20, 150, 50, COLOR_RED);
    st7789_fill_rect(10, 80, 150, 50, COLOR_GREEN);
    st7789_fill_rect(10, 140, 150, 50, COLOR_BLUE);
    st7789_fill_rect(10, 200, 150, 50, COLOR_CYAN);
    st7789_fill_rect(10, 260, 150, 50, COLOR_YELLOW);
}
