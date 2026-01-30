//
// Created by debilian on 30.01.2026.
//

#pragma once

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"

#include "esp_lcd_ili9341.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"

extern "C"
{
#include "lvgl.h"
}


class Display_Il9341
{
public:
	Display_Il9341(const gpio_num_t mosi, const gpio_num_t clk, const gpio_num_t cs, const gpio_num_t dc,
	               const gpio_num_t rst, const gpio_num_t bckl) :
	    mosi(mosi), clk(clk), cs(cs), dc(dc), rst(rst), bckl(bckl)
	{}

	void init()
	{
		this->init_backlight();
		this->init_spi_bus();
		this->init_io();
		this->init_panel();
	}

	[[nodiscard]] esp_lcd_panel_io_handle_t get_io_handle() const
	{
		return this->io_handle;
	}

	[[nodiscard]] esp_lcd_panel_handle_t get_panel() const
	{
		return this->panel;
	}

	static constexpr uint32_t WIDTH  = 240;
	static constexpr uint32_t HEIGHT = 320;

private:
	static constexpr auto LCD_HOST = SPI2_HOST;

	const gpio_num_t mosi;
	const gpio_num_t clk;
	const gpio_num_t cs;
	const gpio_num_t dc;
	const gpio_num_t rst;
	const gpio_num_t bckl;

	esp_lcd_panel_io_handle_t io_handle = nullptr;
	esp_lcd_panel_handle_t    panel     = nullptr;


	void init_spi_bus()
	{
		spi_bus_config_t config{};
		config.sclk_io_num = this->clk;
		config.mosi_io_num = this->mosi;
		config.miso_io_num = GPIO_NUM_NC config.max_transfer_sz = WIDTH * 40 * sizeof(lv_color_t);
		ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &config, SPI_DMA_CH_AUTO));
	}

	void init_backlight()
	{
		gpio_config_t bk_gpio{};
		bk_gpio.pin_bit_mask = 1ULL << this->bckl;
		bk_gpio.mode         = GPIO_MODE_OUTPUT;
		gpio_config(&bk_gpio);
		gpio_set_level(this->bckl, true);
	}

	void init_io()
	{
		esp_lcd_panel_io_spi_config_t config{};
		config.dc_gpio_num       = this->dc;
		config.cs_gpio_num       = this->cs;
		config.pclk_hz           = 20 * 1000 * 1000;
		config.lcd_cmd_bits      = 8;
		config.lcd_param_bits    = 8;
		config.spi_mode          = 0;
		config.trans_queue_depth = 10;

		ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(LCD_HOST, &config, &this->io_handle));
	}

	void init_panel()
	{
		esp_lcd_panel_dev_config_t config{};
		config.reset_gpio_num = this->rst;
		config.rgb_ele_order  = LCD_RGB_ELEMENT_ORDER_BGR;
		config.bits_per_pixel = 16;
		ESP_ERROR_CHECK(esp_lcd_new_panel_ili9341(io_handle, &config, &panel));

		esp_lcd_panel_reset(panel);
		esp_lcd_panel_init(panel);
		esp_lcd_panel_disp_on_off(panel, true);
		esp_lcd_panel_set_gap(panel, 0, 0);
		esp_lcd_panel_swap_xy(panel, true);
		esp_lcd_panel_mirror(panel, true, true);
	}
};
