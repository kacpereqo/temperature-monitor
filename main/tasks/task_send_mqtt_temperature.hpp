//
// Created by debilian on 4.02.2026.
//

#pragma once

#include "constants.hpp"
#include "wifi_manager.hpp"
#include "mqtt_manager.hpp"

namespace  Task
{
	[[noreturn]] void task_send_mqtt_temperature(void *pvParameters)
	{
		auto& state = *static_cast<TemperatureSensorData*>(pvParameters);

		Wifi wifi(Network::Wifi::SSID, Network::Wifi::PASS);
		wifi.init();

		wifi.wait_for_ip();

		MqttManager mqtt(Network::MQTT::BROKER_URI);
		mqtt.init();

		while (true) {

			mqtt.publish_temperatures(state.get_readings());

			vTaskDelay(pdMS_TO_TICKS(10000));
		}
	}
};
