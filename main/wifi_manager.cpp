#include "wifi_manager.hpp"

#include <cstring>
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "lwip/ip4_addr.h"

// Define the static TAG
const char* Wifi::TAG = "WIFI";
EventGroupHandle_t wifi_event_group = nullptr;

void Wifi::event_handler(void* arg, esp_event_base_t event_base,
                         int32_t event_id, void* event_data)
{
	if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
		ESP_LOGI("WIFI", "Got IP address");
		xEventGroupSetBits(wifi_event_group, WIFI_GOT_IP_BIT);
	}

	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
		ESP_LOGW("WIFI", "Disconnected, clearing IP bit");
		xEventGroupClearBits(wifi_event_group, WIFI_GOT_IP_BIT);
		esp_wifi_connect();
	}
}
Wifi::Wifi(std::string_view ssid, std::string_view password) :ssid(ssid), password(password)
{
	wifi_event_group = xEventGroupCreate();
}

void Wifi::init() const
{
    // Robust NVS Init (Required for Wi-Fi driver)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_STA);

    // Register Event Handler
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                        &event_handler, nullptr, &instance_any_id);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                        &event_handler, nullptr, &instance_got_ip);

    // Configure Wi-Fi credentials
    wifi_config_t wifi_config = {

    };

    strncpy(reinterpret_cast<char *>(wifi_config.sta.ssid), ssid.data(), sizeof(wifi_config.sta.ssid) - 1);
    strncpy(reinterpret_cast<char *>(wifi_config.sta.password), password.data(), sizeof(wifi_config.sta.password) - 1);
    
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_LOGI(TAG, "Connecting to SSID: %s", ssid.data());

    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
    esp_wifi_connect();

    ESP_LOGI(TAG, "Wi-Fi initialization started...");
}
void Wifi::wait_for_ip()
{
	ESP_LOGI("WIFI", "Waiting for IP...");

	xEventGroupWaitBits(
		wifi_event_group,
		WIFI_GOT_IP_BIT,
		pdFALSE,
		pdTRUE,
		portMAX_DELAY
	);

	ESP_LOGI("WIFI", "IP acquired, continuing");
}
