#include "lvlg.hpp"

#include <array>
#include <cstdio>

#include "constants.hpp"
#include "esp_log.h"
#include "esp_timer.h"



LVLG::LVLG(const Display_Il9341 &display) :
	io_handle(display.get_io_handle()),
	panel(display.get_panel())
{}

lv_color_t LVLG::rgb_fix(const lv_color_t color)
{
	lv_color_t result;
	result.red   = color.blue;
	result.green = color.red;
	result.blue  = color.green;
	return result;
}

void LVLG::lvgl_flush_cb(lv_display_t *disp,
						 const lv_area_t *area,
						 uint8_t *px_map)
{
	const auto panel =
		static_cast<esp_lcd_panel_handle_t>(lv_display_get_user_data(disp));

	esp_lcd_panel_draw_bitmap(
		panel,
		area->x1,
		area->y1,
		area->x2 + 1,
		area->y2 + 1,
		px_map);
}

bool LVLG::notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t,
								   esp_lcd_panel_io_event_data_t *,
								   void *user_ctx)
{
	const auto disp = static_cast<lv_display_t *>(user_ctx);
	lv_display_flush_ready(disp);
	return false;
}

void LVLG::init()
{
	lv_init();

	lv_display_t *disp =
	    lv_display_create(Display_Il9341::WIDTH, Display_Il9341::HEIGHT);

	lv_display_set_flush_cb(disp, lvgl_flush_cb);
	lv_display_set_user_data(disp, this->panel);

	esp_lcd_panel_io_callbacks_t cbs{};
	cbs.on_color_trans_done = notify_lvgl_flush_ready;

	ESP_ERROR_CHECK(
	    esp_lcd_panel_io_register_event_callbacks(
	        this->io_handle,
	        &cbs,
	        disp));

	static lv_color_t buf1[Display_Il9341::WIDTH * 40];
	static lv_color_t buf2[Display_Il9341::WIDTH * 40];

	lv_display_set_buffers(
	    disp,
	    buf1,
	    buf2,
	    sizeof(buf1),
	    LV_DISPLAY_RENDER_MODE_PARTIAL);

	lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_90);

	lv_display_set_theme(
	    disp,
	    lv_theme_default_init(
	        disp,
	        DARK_BG,
	        rgb_fix(lv_color_make(255, 255, 255)),
	        true,
	        &lv_font_montserrat_14));

	lv_obj_set_style_bg_color(lv_scr_act(), DARK_BG, 0);
}


