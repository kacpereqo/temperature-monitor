//
// Created by debilian on 30.01.2026.
//

#include "driver/gpio.h"
#include "ds18b20.h"
#include "esp_log.h"
#include "onewire_bus.h"

#pragma once

class DS18B20_Sensor
{
public:
	DS18B20_Sensor() : pin(GPIO_NUM_NC)
	{}
	explicit DS18B20_Sensor(const gpio_num_t pin) : pin(pin)
	{}

	void init()
	{
		assert(this->pin != GPIO_NUM_NC);

		init_onewire_bus();
		init_device();
	}

	[[nodiscard]] float read_temperature() const
	{
		assert(device != nullptr && "Device not initialized");

		float temperature = 0.0f;

		ESP_ERROR_CHECK(ds18b20_trigger_temperature_conversion_for_all(bus));
		ESP_ERROR_CHECK(ds18b20_get_temperature(device, &temperature));
		ESP_LOGI(TAG, "temperature read from DS18B20: %.2fC", temperature);

		return temperature;
	}

private:
	constexpr static const char *TAG = "[DS18B20]";

	onewire_bus_handle_t    bus    = nullptr;
	ds18b20_device_handle_t device = nullptr;
	const gpio_num_t        pin;

	void init_onewire_bus()
	{
		const onewire_bus_config_t bus_config = {.bus_gpio_num = this->pin,
		                                         .flags        = {
		                                                  .en_pull_up = true,
                                                 }};

		constexpr onewire_bus_rmt_config_t rmt_config = {
		  .max_rx_bytes = 10,
		};

		ESP_ERROR_CHECK(onewire_new_bus_rmt(&bus_config, &rmt_config, &bus));
	}

	void init_device()
	{
		onewire_device_iter_handle_t iter = nullptr;
		onewire_device_t             next_onewire_device;
		esp_err_t                    search_result = ESP_OK;

		ESP_ERROR_CHECK(onewire_new_device_iter(bus, &iter));
		ESP_LOGI(TAG, "Device iterator created, start searching...");
		do {
			search_result = onewire_device_iter_get_next(iter, &next_onewire_device);
			if (search_result == ESP_OK) {
				ds18b20_config_t         ds_cfg = {};
				onewire_device_address_t address;

				if (ds18b20_new_device_from_enumeration(&next_onewire_device, &ds_cfg, &device) == ESP_OK) {
					ds18b20_get_device_address(device, &address);
					ESP_LOGI(TAG, "Found a DS18B20[%d], address: %016llX", 1, address);
				} else {
					ESP_LOGI(TAG, "Found an unknown device, address: %016llX", next_onewire_device.address);
				}
			}
		} while (search_result != ESP_ERR_NOT_FOUND);
	}
};
