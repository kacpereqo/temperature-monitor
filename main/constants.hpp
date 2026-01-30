//
// Created by debilian on 30.01.2026.
//

#pragma once

#include <array>
#include <string_view>
#include "driver/gpio.h"

namespace GUI
{
	constexpr std::array<std::string_view, 5> LABELS = {
	  "Bufor gora",
	  "Pompa ciepla wyj",
	  "Pompa ciepla wej",
	  "Wyj kaloryfer",
	  "Wyj podloga",
	};
}

namespace Task
{
	namespace Delay
	{
		constexpr uint32_t TEMPERATURE_UPDATE = 1000;
		constexpr uint32_t GUI_UPDATE         = 1000;
	} // namespace Delay

	namespace Priority
	{
		constexpr uint32_t TEMPERATURE_UPDATE = tskIDLE_PRIORITY + 1;
		constexpr uint32_t GUI_UPDATE         = tskIDLE_PRIORITY + 1;
	} // namespace Priority

	namespace StackDepth
	{
		constexpr uint32_t TEMPERATURE_UPDATE = 4096;
		constexpr uint32_t GUI_UPDATE         = 8192;

	} // namespace StackDepth
} // namespace Task

namespace Pinout
{
	namespace IL9341
	{
		constexpr gpio_num_t MOSI = GPIO_NUM_23;
		constexpr gpio_num_t CLK  = GPIO_NUM_18;
		constexpr gpio_num_t CS   = GPIO_NUM_5;
		constexpr gpio_num_t DC   = GPIO_NUM_2;
		constexpr gpio_num_t RST  = GPIO_NUM_4;
		constexpr gpio_num_t LED  = GPIO_NUM_15;
	} // namespace IL9341

	namespace DS18B20
	{
		constexpr std::array PINS = {
		  GPIO_NUM_16,
		  GPIO_NUM_NC,
		  GPIO_NUM_NC,
		  GPIO_NUM_NC,
		  GPIO_NUM_NC,
		};
	}
} // namespace Pinout
