#include "st7789.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "st7789";

#define LCD_SPI_HOST    SPI2_HOST
#define SPI_CLOCK       26000000

// ST7789V2 frame buffer is 240x320; the 170-wide panel is offset by 35 columns
#define COL_OFFSET  35
#define ROW_OFFSET   0

// ST7789 commands
#define CMD_SWRESET 0x01
#define CMD_SLPOUT  0x11
#define CMD_COLMOD  0x3A
#define CMD_MADCTL  0x36
#define CMD_INVON   0x21
#define CMD_CASET   0x2A
#define CMD_RASET   0x2B
#define CMD_RAMWR   0x2C
#define CMD_DISPON  0x29

static spi_device_handle_t spi;

static void write_cmd(uint8_t cmd)
{
    gpio_set_level(ST7789_PIN_DC, 0);
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &cmd,
    };
    spi_device_polling_transmit(spi, &t);
}

static void write_data(const uint8_t *data, size_t len)
{
    if (len == 0) return;
    gpio_set_level(ST7789_PIN_DC, 1);
    spi_transaction_t t = {
        .length = len * 8,
        .tx_buffer = data,
    };
    spi_device_polling_transmit(spi, &t);
}

static void write_data_byte(uint8_t b)
{
    write_data(&b, 1);
}

static void set_window(int x, int y, int x2, int y2)
{
    uint8_t buf[4];
    int cx = x + COL_OFFSET, cx2 = x2 + COL_OFFSET;
    int ry = y + ROW_OFFSET, ry2 = y2 + ROW_OFFSET;

    write_cmd(CMD_CASET);
    buf[0] = cx >> 8; buf[1] = cx & 0xFF;
    buf[2] = cx2 >> 8; buf[3] = cx2 & 0xFF;
    write_data(buf, 4);

    write_cmd(CMD_RASET);
    buf[0] = ry >> 8; buf[1] = ry & 0xFF;
    buf[2] = ry2 >> 8; buf[3] = ry2 & 0xFF;
    write_data(buf, 4);

    write_cmd(CMD_RAMWR);
}

void st7789_init(void)
{
    // Backlight on
    gpio_set_direction(ST7789_PIN_BL, GPIO_MODE_OUTPUT);
    gpio_set_level(ST7789_PIN_BL, 1);

    // DC and RST as outputs
    gpio_set_direction(ST7789_PIN_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(ST7789_PIN_RST, GPIO_MODE_OUTPUT);

    // Hardware reset
    gpio_set_level(ST7789_PIN_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(ST7789_PIN_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(120));

    // SPI bus
    spi_bus_config_t bus = {
        .mosi_io_num   = ST7789_PIN_MOSI,
        .miso_io_num   = -1,
        .sclk_io_num   = ST7789_PIN_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };
    esp_err_t ret = spi_bus_initialize(LCD_SPI_HOST, &bus, SPI_DMA_CH_AUTO);
    ESP_LOGI(TAG, "spi_bus_initialize: %s", esp_err_to_name(ret));

    // SPI device
    spi_device_interface_config_t dev = {
        .clock_speed_hz = SPI_CLOCK,
        .mode           = 0,
        .spics_io_num   = ST7789_PIN_CS,
        .queue_size     = 1,
        .flags          = SPI_DEVICE_HALFDUPLEX,
    };
    ret = spi_bus_add_device(LCD_SPI_HOST, &dev, &spi);
    ESP_LOGI(TAG, "spi_bus_add_device: %s", esp_err_to_name(ret));

    // Initialisation sequence
    write_cmd(CMD_SWRESET);
    vTaskDelay(pdMS_TO_TICKS(150));

    write_cmd(CMD_SLPOUT);
    vTaskDelay(pdMS_TO_TICKS(10));

    write_cmd(CMD_COLMOD);
    write_data_byte(0x55);   // RGB565

    write_cmd(CMD_MADCTL);
    write_data_byte(0x00);

    write_cmd(CMD_INVON);    // inversion required for this panel

    write_cmd(CMD_DISPON);
    vTaskDelay(pdMS_TO_TICKS(10));
}

void st7789_fill(uint16_t colour)
{
    st7789_fill_rect(0, 0, ST7789_WIDTH, ST7789_HEIGHT, colour);
}

void st7789_fill_rect(int x, int y, int w, int h, uint16_t colour)
{
    set_window(x, y, x + w - 1, y + h - 1);

    // Send pixels in chunks using DMA buffer
    uint8_t hi = colour >> 8;
    uint8_t lo = colour & 0xFF;

    // 128-pixel chunk, two bytes each
    static uint8_t chunk[256];
    for (int i = 0; i < (int)sizeof(chunk); i += 2) {
        chunk[i]     = hi;
        chunk[i + 1] = lo;
    }

    gpio_set_level(ST7789_PIN_DC, 1);

    int total = w * h;
    int remaining = total;
    while (remaining > 0) {
        int pixels = remaining < 128 ? remaining : 128;
        spi_transaction_t t = {
            .length    = pixels * 16,
            .tx_buffer = chunk,
        };
        spi_device_polling_transmit(spi, &t);
        remaining -= pixels;
    }
}
