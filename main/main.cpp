#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "constants.hpp"
#include "tasks/task_read_temperature.hpp"
#include "tasks/task_update_display.hpp"
#include "tasks/task_send_mqtt_temperature.hpp"
#include "temperature_data.hpp"

TemperatureSensorData temperature_state;

extern "C" void app_main(void)
{
	xTaskCreatePinnedToCore(Task::task_update_display,
	            "task_update_display",
	            Task::StackDepth::GUI_UPDATE,
	            &temperature_state,
	            Task::Priority::GUI_UPDATE,
	            &Task::display_task_handle,
	            1);

	xTaskCreatePinnedToCore(Task::task_read_temperature_sensors,
	            "task_read_temperature_sensors",
	            Task::StackDepth::TEMPERATURE_UPDATE,
	            &temperature_state,
	            Task::Priority::TEMPERATURE_UPDATE,
	            nullptr,
	            0);

	xTaskCreatePinnedToCore(Task::task_send_mqtt_temperature,
			"task_send_mqtt_temperature",
			Task::StackDepth::MQTT_SEND,
			&temperature_state,
			Task::Priority::MQTT_SEND,
			nullptr,
			0);

	vTaskDelete(nullptr);
}
