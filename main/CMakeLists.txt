idf_component_register(
        SRCS
        "main.cpp"
        "lvlg.cpp"
        "il_9341.cpp"
        PRIV_REQUIRES
        spi_flash
        esp_driver_gpio
        esp_driver_spi
        esp_lcd
        esp_lcd_ili9341
        lvgl
        esp_timer
        ds18b20
        espressif__onewire_bus
        INCLUDE_DIRS "./"
)
