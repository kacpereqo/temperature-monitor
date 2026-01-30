//
// Created by debilian on 30.01.2026.
//

#pragma once

#include "temperature_data.hpp"
#include "ds18b20.hpp"

[[noreturn]] inline void task_read_temperature_sensors(void *pvParameter) {
  TemperatureSensorData& temperature_state= *static_cast<TemperatureSensorData*>(pvParameter);

  DS18B20_Sensor sensor(GPIO_NUM_16);
  sensor.init();

  while (true) {
    const float temp = sensor.read_temperature();

    std::array<float, 5> new_readings{};
    for (size_t i = 0; i < 5; i++) {
      new_readings[i] = temp + std::rand() % 100 / 1000.0f;
    }

    temperature_state.set_readings(new_readings);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}