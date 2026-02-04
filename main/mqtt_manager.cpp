#include "mqtt_manager.hpp"
#include "esp_event.h"
#include <cstring> // for strerror

#include "constants.hpp"

// Define the static TAG
const char* MqttManager::TAG = "MQTT";

void MqttManager::mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    auto event = static_cast<esp_mqtt_event_handle_t>(event_data);
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            // Publish status
            esp_mqtt_client_publish(client, "my_device/status", "online", 0, 1, 0);
            // Subscribe to commands
            esp_mqtt_client_subscribe(client, "my_device/commands", 0);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
            }
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
}

void MqttManager::init()
{
    esp_mqtt_client_config_t mqtt_cfg = {};

    mqtt_cfg.broker.address.uri = broker_uri.data();
    
    client = esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event(client, static_cast<esp_mqtt_event_id_t>(ESP_EVENT_ANY_ID), mqtt_event_handler, nullptr);

    esp_mqtt_client_start(client);
}

MqttManager::MqttManager(std::string_view broker_uri) : broker_uri(broker_uri)
{}

void MqttManager::publish_temperatures(const std::array<float, 5> &temperatures)
{
		if (client == nullptr) {
			ESP_LOGW(TAG, "Client not initialized, cannot publish");
			return;
		}

		std::string json_str = "{\"sensors\":[";

		char buffer[16];
		for (size_t i = 0; i < temperatures.size(); ++i) {
			snprintf(buffer, sizeof(buffer), "%.1f", temperatures[i]);
			json_str += buffer;

			if (i < temperatures.size() - 1) {
				json_str += ",";
			}
		}

		json_str += "]}";

		esp_mqtt_client_publish(client, Network::MQTT::TOPIC.data(), json_str.c_str(), 0, 1, 0);

		ESP_LOGI(TAG, "Sent: %s", json_str.c_str());
}
