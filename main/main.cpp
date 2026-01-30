#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "temperature_data.hpp"
#include "task_update_display.hpp"
#include "task_read_temperature.hpp"


TemperatureSensorData temperature_state;

extern "C" void app_main(void) {
    xTaskCreate(task_update_display, "task_update_display", 8192, &temperature_state, tskIDLE_PRIORITY + 1, nullptr);
    xTaskCreate(task_read_temperature_sensors, "task_read_temperature_sensors", 4096, &temperature_state, tskIDLE_PRIORITY + 1, nullptr);

    vTaskDelete(nullptr);
}
