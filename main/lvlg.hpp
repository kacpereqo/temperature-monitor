//
// Created by debilian on 29.01.2026.
//

#pragma once

#include <cstdio>
#include "constants.hpp"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"

#include <array>
#include "il_9341.hpp"
#include "temperature_data.hpp"

static lv_color_t WHITE   = lv_color_make(255, 255, 255);
static lv_color_t BLACK   = lv_color_make(0, 0, 0);
static lv_color_t DARK_BG = lv_color_make(0, 0, 0);

extern "C"
{
#include "lvgl.h"
}

class LVLG
{
public:
	explicit LVLG(const Display_Il9341 &display);

	void init();

	static lv_color_t rgb_fix(lv_color_t color);

private:
	esp_lcd_panel_io_handle_t io_handle;
	esp_lcd_panel_handle_t    panel;


	static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);

	static bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t io, esp_lcd_panel_io_event_data_t *edata,
	                                    void *user_ctx);
};

class Screen
{
public:
	void update()
	{
		if (!is_initialized) {
			this->init();
			is_initialized = true;
		}
		this->_update();
	}


private:
	bool is_initialized    = false;
	void virtual _update() = 0;
	void virtual init()    = 0;
};

class MainScreen : public Screen
{
private:
	TemperatureSensorData &state;

	lv_obj_t                 *frame{};
	std::array<lv_obj_t *, 5> name_labels{};
	std::array<lv_obj_t *, 5> dot_labels{};
	std::array<lv_obj_t *, 5> temp_labels{};

public:
	MainScreen(TemperatureSensorData &state) : state(state)
	{}

	void _update() override
	{
		auto readings = state.get_readings();

		for (size_t i = 0; i < 5; i++) {
			char buf[16];
			std::snprintf(buf, sizeof(buf), "%.1f°C", readings[i]);
			lv_label_set_text(temp_labels[i], buf);
		}
	}

	void init()
	{
		frame = lv_obj_create(lv_scr_act());
		lv_obj_set_size(frame, Display_Il9341::HEIGHT - 10, Display_Il9341::WIDTH - 10);

		lv_obj_center(frame);

		lv_obj_set_style_bg_color(frame, LVLG::rgb_fix(DARK_BG), 0);
		lv_obj_set_style_border_color(frame, LVLG::rgb_fix(WHITE), 0);
		lv_obj_set_style_border_width(frame, 2, 0);

		lv_obj_set_flex_flow(frame, LV_FLEX_FLOW_COLUMN);
		lv_obj_set_flex_align(frame, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

		for (size_t i = 0; i < 5; i++) {
			lv_obj_t *row = lv_obj_create(frame);
			lv_obj_set_width(row, lv_pct(100));
			lv_obj_set_height(row, LV_SIZE_CONTENT);

			lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
			lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

			lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
			lv_obj_set_style_border_width(row, 0, 0);
			lv_obj_set_style_pad_all(row, 0, 0);
			lv_obj_set_style_pad_column(row, 6, 0);

			name_labels[i] = lv_label_create(row);
			lv_label_set_text(name_labels[i], GUI::LABELS[i].data());
			lv_obj_set_style_text_font(name_labels[i], &lv_font_montserrat_14, 0);

			dot_labels[i] = lv_label_create(row);
			lv_label_set_text(dot_labels[i], "................................");
			lv_obj_set_flex_grow(dot_labels[i], 1);
			lv_label_set_long_mode(dot_labels[i], LV_LABEL_LONG_CLIP);
			lv_obj_set_style_text_font(dot_labels[i], &lv_font_montserrat_14, 0);
			lv_obj_set_style_text_color(dot_labels[i], LVLG::rgb_fix({50, 50, 25}), 0);

			temp_labels[i] = lv_label_create(row);
			lv_label_set_text(temp_labels[i], "--.-°C");
			lv_obj_set_style_text_font(temp_labels[i], &lv_font_montserrat_14, 0);
			lv_obj_set_style_text_align(temp_labels[i], LV_TEXT_ALIGN_RIGHT, 0);
		}
	}
};
