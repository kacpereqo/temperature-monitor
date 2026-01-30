//
// Created by debilian on 30.01.2026.
//

#pragma once

#include <array>
#include <mutex>

class TemperatureSensorData
{
public:
	static constexpr uint32_t PIPE_1 = 0;
	static constexpr uint32_t PIPE_2 = 1;
	static constexpr uint32_t PIPE_3 = 2;
	static constexpr uint32_t PIPE_4 = 3;
	static constexpr uint32_t PIPE_5 = 4;

	void set_readings(const std::array<float, 5> &new_readings)
	{
		std::lock_guard lock(this->semaphore);
		this->readings = new_readings;
	}

	std::array<float, 5> get_readings()
	{
		std::lock_guard lock(this->semaphore);
		return this->readings;
	}

private:
	std::mutex           semaphore;
	std::array<float, 5> readings{};
};
