idf_component_register(SRCS "main.cpp"
        PRIV_REQUIRES spi_flash esp_driver_gpio esp_driver_spi esp_lcd esp_lcd_ili9341 lvgl esp_timer
        INCLUDE_DIRS ".")