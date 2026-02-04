#pragma once

#include <array>
#include <string>
#include <cstdio>
#include "mqtt_client.h"
#include "esp_log.h"

class MqttManager
{
private:
	esp_mqtt_client_handle_t client = nullptr;

	static const char* TAG;
	std::string_view broker_uri;

	/**
	 * @brief Static event handler for MQTT events
	 */
	static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

	/**
	 * @brief Initialize the MQTT client
	 * @param uri The broker URI (e.g., "mqtt://test.mosquitto.org")
	 */
public:
	MqttManager(std::string_view broker_uri);
	void init();

	/**
	 * @brief Publishes an array of temperatures as a JSON string.
	 * Note: This must remain in the header because it is a template.
	 */

	void publish_temperatures(const std::array<float, 5>& temperatures);
};