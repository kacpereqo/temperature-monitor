//
// Created by debilian on 30.01.2026.
//

#pragma once

#include "driver/spi_master.h"

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"

extern "C"
{
#include "lvgl.h"
}

class Display_Il9341
{
public:
	Display_Il9341(gpio_num_t mosi,
				   gpio_num_t clk,
				   gpio_num_t cs,
				   gpio_num_t dc,
				   gpio_num_t rst,
				   gpio_num_t bckl);

	void init();

	[[nodiscard]] esp_lcd_panel_io_handle_t get_io_handle() const;
	[[nodiscard]] esp_lcd_panel_handle_t    get_panel() const;

	static constexpr uint32_t WIDTH  = 240;
	static constexpr uint32_t HEIGHT = 320;

private:
	static constexpr spi_host_device_t LCD_HOST = SPI2_HOST;


	const gpio_num_t mosi;
	const gpio_num_t clk;
	const gpio_num_t cs;
	const gpio_num_t dc;
	const gpio_num_t rst;
	const gpio_num_t bckl;

	esp_lcd_panel_io_handle_t io_handle = nullptr;
	esp_lcd_panel_handle_t    panel     = nullptr;

	void init_spi_bus();
	void init_backlight();
	void init_io();
	void init_panel();
};
