//
// Created by debilian on 30.01.2026.
//

#pragma once

#include "../constants.hpp"
#include "../ds18b20.hpp"
#include "../temperature_data.hpp"
#include "esp_log.h"
#include "task_update_display.hpp"

namespace Task
{
	[[noreturn]] inline void task_read_temperature_sensors(void *pvParameter)
	{
		ESP_LOGI("READ", "Running on core %d", xPortGetCoreID());

		TemperatureSensorData &temperature_state = *static_cast<TemperatureSensorData *>(pvParameter);

		std::array sensors{DS18B20_Sensor(Pinout::DS18B20::PINS[0]), DS18B20_Sensor(Pinout::DS18B20::PINS[1])};

		for (auto &temperature_sensor : sensors)
			temperature_sensor.init();


		while (true) {
			std::array<float, 5> new_readings{};

			// trigger temperature conversion
			for (auto &sensor : sensors)
				sensor.trigger_conversion();


			// wait for temperature conversion to complete
			vTaskDelay(pdMS_TO_TICKS(750));

			for (size_t i = 0; i < sensors.size(); i++)
				new_readings[i] = sensors[i].read_temperature_without_trigger();

			temperature_state.set_readings(new_readings);
			xTaskNotify(Task::display_task_handle, 0, eNoAction);
		}
	}
} // namespace Task
