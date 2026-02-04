#pragma once

#include "driver/gpio.h"
#include "ds18b20.h"
#include "onewire_bus.h"

class DS18B20_Sensor
{
public:
	DS18B20_Sensor();
	explicit DS18B20_Sensor(gpio_num_t pin);

	void init();
	[[nodiscard]] float read_temperature() const;

	void trigger_conversion() const;
	[[nodiscard]] float read_temperature_without_trigger() const;

	onewire_bus_handle_t get_bus();
	ds18b20_device_handle_t get_device_handle();

private:
	static constexpr const char *TAG = "[DS18B20]";

	void init_onewire_bus();
	void init_device();

	onewire_bus_handle_t    bus    = nullptr;
	ds18b20_device_handle_t device = nullptr;
	gpio_num_t              pin{};
	onewire_device_address_t address{};

};
