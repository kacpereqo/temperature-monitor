//
// Created by debilian on 30.01.2026.
//

#pragma once

#include "../constants.hpp"
#include "../il_9341.hpp"
#include "../lvlg.hpp"
#include "esp_log.h"

namespace Task
{
	inline TaskHandle_t display_task_handle = nullptr;

	[[noreturn]] void task_update_display(void *pvParameter)
	{
		ESP_LOGI("GUI", "Running on core %d", xPortGetCoreID());

		auto& state = *static_cast<TemperatureSensorData*>(pvParameter);

		Display_Il9341 display(Pinout::IL9341::MOSI,
		                       Pinout::IL9341::CLK,
		                       Pinout::IL9341::CS,
		                       Pinout::IL9341::DC,
		                       Pinout::IL9341::RST,
		                       Pinout::IL9341::LED);

		display.init();

		LVLG lvgl(display);
		lvgl.init();

		size_t current_screen = 0;
		std::array screens = {MainScreen{ state}};

		while (true) {
			xTaskNotifyWait(0,0,nullptr, portMAX_DELAY);

			screens[current_screen].update();

			lv_tick_inc(Task::Delay::TEMPERATURE_UPDATE);
			lv_timer_handler();
		}
	}
} // namespace Task
