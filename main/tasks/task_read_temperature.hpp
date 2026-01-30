//
// Created by debilian on 30.01.2026.
//

#pragma once

#include "../constants.hpp"
#include "../ds18b20.hpp"
#include "../temperature_data.hpp"

namespace Task
{
	[[noreturn]] inline void task_read_temperature_sensors(void *pvParameter)
	{
		TemperatureSensorData &temperature_state = *static_cast<TemperatureSensorData *>(pvParameter);

		DS18B20_Sensor sensor(GPIO_NUM_16);
		sensor.init();

		const std::array sensors{DS18B20_Sensor(Pinout::DS18B20::PINS[0]),
		                         DS18B20_Sensor(Pinout::DS18B20::PINS[1]),
		                         DS18B20_Sensor(Pinout::DS18B20::PINS[2]),
		                         DS18B20_Sensor(Pinout::DS18B20::PINS[3]),
		                         DS18B20_Sensor(Pinout::DS18B20::PINS[4])};

		for (auto temperature_sensor : sensors)
			temperature_sensor.init();


		while (true) {
			std::array<float, 5> new_readings{};

			for (size_t i = 0; i < new_readings.size(); i++)
				new_readings[i] = sensors[i].read_temperature();

			temperature_state.set_readings(new_readings);
			vTaskDelay(pdMS_TO_TICKS(1000));
		}
	}
} // namespace Task
