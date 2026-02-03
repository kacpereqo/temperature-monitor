#include "il_9341.hpp"

#include "driver/gpio.h"
#include "esp_lcd_ili9341.h"
#include "esp_log.h"

Display_Il9341::Display_Il9341(const gpio_num_t mosi,
                               const gpio_num_t clk,
                               const gpio_num_t cs,
                               const gpio_num_t dc,
                               const gpio_num_t rst,
                               const gpio_num_t bckl) :
    mosi(mosi),
    clk(clk),
    cs(cs),
    dc(dc),
    rst(rst),
    bckl(bckl)
{}

void Display_Il9341::init()
{
	this->init_backlight();
	this->init_spi_bus();
	this->init_io();
	this->init_panel();
}

esp_lcd_panel_io_handle_t Display_Il9341::get_io_handle() const
{
	return this->io_handle;
}

esp_lcd_panel_handle_t Display_Il9341::get_panel() const
{
	return this->panel;
}

void Display_Il9341::init_spi_bus()
{
	spi_bus_config_t config{};
	config.sclk_io_num     = this->clk;
	config.mosi_io_num     = this->mosi;
	config.miso_io_num     = GPIO_NUM_NC;
	config.max_transfer_sz = WIDTH * 40 * sizeof(lv_color_t);

	ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &config, SPI_DMA_CH_AUTO));
}

void Display_Il9341::init_backlight()
{
	gpio_config_t bk_gpio{};
	bk_gpio.pin_bit_mask = 1ULL << this->bckl;
	bk_gpio.mode         = GPIO_MODE_OUTPUT;

	gpio_config(&bk_gpio);
	gpio_set_level(this->bckl, true);
}

void Display_Il9341::init_io()
{
	esp_lcd_panel_io_spi_config_t config{};
	config.dc_gpio_num       = this->dc;
	config.cs_gpio_num       = this->cs;
	config.pclk_hz           = 20 * 1000 * 1000;
	config.lcd_cmd_bits      = 8;
	config.lcd_param_bits    = 8;
	config.spi_mode          = 0;
	config.trans_queue_depth = 10;

	ESP_ERROR_CHECK(
	    esp_lcd_new_panel_io_spi(LCD_HOST, &config, &this->io_handle));
}

void Display_Il9341::init_panel()
{
	esp_lcd_panel_dev_config_t config{};
	config.reset_gpio_num = this->rst;
	config.rgb_ele_order  = LCD_RGB_ELEMENT_ORDER_BGR;
	config.bits_per_pixel = 16;

	ESP_ERROR_CHECK(
	    esp_lcd_new_panel_ili9341(this->io_handle, &config, &this->panel));

	esp_lcd_panel_reset(this->panel);
	esp_lcd_panel_init(this->panel);
	esp_lcd_panel_disp_on_off(this->panel, true);
	esp_lcd_panel_set_gap(this->panel, 0, 0);
	esp_lcd_panel_swap_xy(this->panel, true);
	esp_lcd_panel_mirror(this->panel, true, true);
}
