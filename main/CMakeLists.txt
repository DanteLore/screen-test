idf_component_register(
    SRCS "jpeg.c" "status.c" "manifest.c" "http.c" "wifi.c" "st7789.c" "main.c"
    INCLUDE_DIRS "."
    REQUIRES esp_wifi esp_netif esp_event nvs_flash esp_http_client esp-tls espressif__cjson esp_rom esp_driver_spi esp_driver_gpio
)
