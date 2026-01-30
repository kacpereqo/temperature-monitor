//
// Created by debilian on 30.01.2026.
//

#pragma once

#include "../constants.hpp"
#include "../il_9341.hpp"
#include "../lvlg.hpp"

namespace Task
{
	[[noreturn]] void task_update_display(void *pvParameter)
	{

		Display_Il9341 display(Pinout::IL9341::MOSI,
		                       Pinout::IL9341::CLK,
		                       Pinout::IL9341::CS,
		                       Pinout::IL9341::DC,
		                       Pinout::IL9341::RST,
		                       Pinout::IL9341::LED);

		display.init();

		LVLG lvgl(display);
		lvgl.init();

		while (true) {
			lv_tick_inc(1000);
			lv_timer_handler();
			vTaskDelay(pdMS_TO_TICKS(1000));
		}
	}
} // namespace Task
