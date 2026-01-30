//
// Created by debilian on 30.01.2026.
//

#pragma once

#include<array>
#include<string_view>
#include"driver/gpio.h"

constexpr std::array<std::string_view, 5> LABELS = {
  "Bufor gora",
  "Pompa ciepla wyj",
  "Pompa ciepla wej",
  "Wyj kaloryfer",
  "Wyj podloga",
};

constexpr gpio_num_t PIN_NUM_MOSI = GPIO_NUM_23;
constexpr gpio_num_t PIN_NUM_CLK = GPIO_NUM_18;
constexpr gpio_num_t PIN_NUM_CS = GPIO_NUM_5;
constexpr gpio_num_t PIN_NUM_DC = GPIO_NUM_2;
constexpr gpio_num_t PIN_NUM_RST = GPIO_NUM_4;
constexpr gpio_num_t PIN_NUM_BCKL = GPIO_NUM_15;