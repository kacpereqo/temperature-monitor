#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ds18b20.hpp"
#include "ds18b20_types.h"
#include "esp_check.h"
#include "esp_log.h"


DS18B20_Sensor::DS18B20_Sensor() : pin(GPIO_NUM_NC)
{}

DS18B20_Sensor::DS18B20_Sensor(gpio_num_t pin) : pin(pin)
{}

void DS18B20_Sensor::init()
{
	assert(pin != GPIO_NUM_NC);

	init_onewire_bus();
	init_device();

	ds18b20_set_resolution(device, DS18B20_RESOLUTION_12B);
}

float DS18B20_Sensor::read_temperature() const
{
	this->trigger_conversion();

	vTaskDelay(pdMS_TO_TICKS(750));

	float temperature = this->read_temperature_without_trigger();

	return temperature;
}

void DS18B20_Sensor::init_onewire_bus()
{
	onewire_bus_config_t     bus_config = {.bus_gpio_num = pin, .flags = {.en_pull_up = true}};
	onewire_bus_rmt_config_t rmt_config = {.max_rx_bytes = 10};
	ESP_ERROR_CHECK(onewire_new_bus_rmt(&bus_config, &rmt_config, &bus));
}

void DS18B20_Sensor::init_device()
{
	onewire_device_iter_handle_t iter;
	onewire_device_t             next_device;
	ESP_ERROR_CHECK(onewire_new_device_iter(bus, &iter));

	while (onewire_device_iter_get_next(iter, &next_device) == ESP_OK) {
		ds18b20_config_t cfg = {};
		if (ds18b20_new_device_from_enumeration(&next_device, &cfg, &device) == ESP_OK) {
			ds18b20_get_device_address(device, &address);
			break; // stop after first device on this bus
		}
	}
}
onewire_bus_handle_t DS18B20_Sensor::get_bus()
{
	return this->bus;
}
ds18b20_device_handle_t DS18B20_Sensor::get_device_handle()
{
	return this->device;
}


void DS18B20_Sensor::trigger_conversion() const
{
	assert(bus != nullptr && "Device not initialized");

	ESP_ERROR_CHECK(onewire_bus_reset(bus));

	constexpr uint8_t DS18B20_CMD_CONVERT_TEMP = 0x44;
	constexpr uint8_t ONEWIRE_CMD_SKIP_ROM     = 0xCC;

	uint8_t cmd[2] = {ONEWIRE_CMD_SKIP_ROM, DS18B20_CMD_CONVERT_TEMP};
	ESP_ERROR_CHECK(onewire_bus_write_bytes(bus, cmd, 2));
}
float DS18B20_Sensor::read_temperature_without_trigger() const
{
	assert(device != nullptr && "Device not initialized");

	float temperature = 0.0f;
	ESP_ERROR_CHECK(ds18b20_get_temperature(device, &temperature));

	ESP_LOGI(TAG, "temperature read from DS18B20[%016llX]: %.2fC", address, temperature);

	return temperature;
}
