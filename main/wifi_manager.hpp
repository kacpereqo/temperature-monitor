#pragma once

#include <string_view>


#include "esp_event.h"

class Wifi
{
private:
	// Declaration of the log tag
	static const char* TAG;
	std::string_view ssid;
	std::string_view  password;

	static constexpr EventBits_t WIFI_GOT_IP_BIT = BIT0;

	/**
	 * @brief Static event handler for Wi-Fi events (Got IP, Disconnect, etc.)
	 */
	static void event_handler(void* arg, esp_event_base_t event_base,
							  int32_t event_id, void* event_data);
public:
	Wifi(std::string_view ssid, std::string_view password);
	/**
	 * @brief Initialize Wi-Fi in Station (STA) mode
	 *
	 * @param ssid      The Wi-Fi Network Name
	 * @param password  The Wi-Fi Password
	 */
	void wait_for_ip();

	void init() const;
};