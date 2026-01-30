//
// Created by debilian on 30.01.2026.
//

#pragma once

#include "constants.hpp"
#include "il_9341.hpp"
#include "lvlg.hpp"

[[noreturn]] void task_update_display(void *pvParameter) {

  Display_Il9341 display(PIN_NUM_MOSI, PIN_NUM_CLK, PIN_NUM_CS, PIN_NUM_DC, PIN_NUM_RST, PIN_NUM_BCKL);
  display.init();

  LVLG lvgl(display);
  lvgl.init();

  while (true) {
    lv_tick_inc(1000);
    lv_timer_handler();
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
