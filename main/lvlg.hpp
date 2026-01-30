//
// Created by debilian on 29.01.2026.
//

#pragma once

#include <cassert>
#include <atomic>

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"

#include "esp_timer.h"
#include "esp_log.h"

#include "constants.hpp"
#include "il_9341.hpp"

#include <cstdio>

extern "C" {
#include "lvgl.h"
}




const lv_color_t WHITE = lv_color_make(255, 255, 255);
const lv_color_t BLACK = lv_color_make(0, 0, 0);

const lv_color_t DARK_BG = lv_color_make(0, 0, 0);

static lv_color_t rgb_fix(const lv_color_t color) {
  lv_color_t result;

  result.red = color.blue;
  result.green = color.red;
  result.blue = color.green;

  return result;
}

static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  const auto panel = static_cast<esp_lcd_panel_handle_t>(lv_display_get_user_data(disp));
  esp_lcd_panel_draw_bitmap(panel, area->x1, area->y1, area->x2 + 1, area->y2 + 1, px_map);
}

static bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t, esp_lcd_panel_io_event_data_t *, void *user_ctx) {
  const auto disp = static_cast<lv_display_t *>(user_ctx);
  lv_display_flush_ready(disp);
  return false;
}

class LVLG {
public:
  LVLG(const Display_Il9341 &display) : io_handle(display.get_io_handle()), panel(display.get_panel()) {}

  void init() {
    lv_init();

    lv_display_t *disp = lv_display_create(Display_Il9341::WIDTH, Display_Il9341::HEIGHT);
    lv_display_set_flush_cb(disp, lvgl_flush_cb);
    lv_display_set_user_data(disp, panel);

    esp_lcd_panel_io_callbacks_t cbs{};
    cbs.on_color_trans_done = notify_lvgl_flush_ready;
    ESP_ERROR_CHECK(esp_lcd_panel_io_register_event_callbacks(io_handle, &cbs, disp));

    static lv_color_t buf1[Display_Il9341::WIDTH * 40];
    static lv_color_t buf2[Display_Il9341::WIDTH * 40];
    lv_display_set_buffers(disp, buf1, buf2, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);

    lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_90);

    lv_display_set_theme(disp, lv_theme_default_init(
        disp,
        DARK_BG,
        rgb_fix(lv_color_make(255,255,255)),
        true,
        &lv_font_montserrat_14
    ));

    lv_obj_set_style_bg_color(lv_scr_act(), DARK_BG, 0);
  }

  void main_screen() {
    static bool is_initialized = false;

    if (!is_initialized) {
      lv_obj_t * frame = lv_obj_create(lv_scr_act());
      lv_obj_set_size(frame,
                      Display_Il9341::HEIGHT - 10,
                      Display_Il9341::WIDTH - 10);
      lv_obj_center(frame);

      lv_obj_set_style_bg_color(frame, rgb_fix(DARK_BG), 0);
      lv_obj_set_style_border_color(frame, rgb_fix(WHITE), 0);
      lv_obj_set_style_border_width(frame, 2, 0);

      // Horizontal layout
      lv_obj_set_flex_flow(frame, LV_FLEX_FLOW_COLUMN);
      lv_obj_set_flex_align(
          frame,
          LV_FLEX_ALIGN_START,
          LV_FLEX_ALIGN_START,
          LV_FLEX_ALIGN_START
      );


      // ================= ROW CONTENT =================
      std::array<lv_obj_t *, 5> name_labels{};
      std::array<lv_obj_t *, 5> dot_labels{};
      std::array<lv_obj_t *, 5> temp_labels{};

      for (size_t i = 0; i < 5; i++) {
        lv_obj_t * row = lv_obj_create(frame);
        lv_obj_set_width(row, lv_pct(100));
        lv_obj_set_height(row, LV_SIZE_CONTENT);

        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(
            row,
            LV_FLEX_ALIGN_START,
            LV_FLEX_ALIGN_CENTER,
            LV_FLEX_ALIGN_CENTER
        );

        lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_set_style_pad_all(row, 0, 0);
        lv_obj_set_style_pad_column(row, 6, 0);

        // NAME (left)
        name_labels[i] = lv_label_create(row);
        lv_label_set_text(name_labels[i], LABELS[i].data());
        lv_obj_set_style_text_font(name_labels[i], &lv_font_montserrat_14, 0);

        // DOTS (flex spacer)
        dot_labels[i] = lv_label_create(row);
        lv_label_set_text(dot_labels[i], "................................");
        lv_obj_set_flex_grow(dot_labels[i], 1);
        lv_label_set_long_mode(dot_labels[i], LV_LABEL_LONG_CLIP);
        lv_obj_set_style_text_font(dot_labels[i], &lv_font_montserrat_14, 0);
        lv_obj_set_style_text_color(dot_labels[i],
                                    rgb_fix({50,50,25}), 0);

        // TEMP (right)
        temp_labels[i] = lv_label_create(row);
        lv_label_set_text(temp_labels[i], "--.-°C");
        lv_obj_set_style_text_font(temp_labels[i], &lv_font_montserrat_14, 0);
        lv_obj_set_style_text_align(temp_labels[i], LV_TEXT_ALIGN_RIGHT, 0);

        is_initialized = true;
      }

      // auto readings = temperature_data.get_readings();
      constexpr auto readings = std::array<float, 5>{0};

      for (size_t i = 0; i < 5; i++) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%.1f°C", readings[i]);

        lv_label_set_text(temp_labels[i], buf);
      }
    }
  }


private:
  esp_lcd_panel_io_handle_t io_handle;
  esp_lcd_panel_handle_t panel;
};

