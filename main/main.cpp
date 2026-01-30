#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "constants.hpp"
#include "tasks/task_read_temperature.hpp"
#include "tasks/task_update_display.hpp"
#include "temperature_data.hpp"

TemperatureSensorData temperature_state;

extern "C" void app_main(void)
{
	xTaskCreate(Task::task_update_display,
	            "task_update_display",
	            Task::StackDepth::GUI_UPDATE,
	            &temperature_state,
	            Task::Priority::GUI_UPDATE,
	            nullptr);

	xTaskCreate(Task::task_read_temperature_sensors,
	            "task_read_temperature_sensors",
	            Task::StackDepth::TEMPERATURE_UPDATE,
	            &temperature_state,
	            Task::Priority::TEMPERATURE_UPDATE,
	            nullptr);

	vTaskDelete(nullptr);
}
