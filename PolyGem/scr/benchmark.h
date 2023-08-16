#pragma once
#include <iostream>
#include <chrono>

#define DEBUG_LEVEL 1

template<typename T>
void Log(T message, bool endState = false) {
#if DEBUG_LEVEL == 1
	if (!endState) {
		std::cout << message;
		return;
	}
	std::cout << message << "\n";
#endif
}

template <typename I> std::string n2hexstr(I w, size_t hex_len = sizeof(I) << 1) {
	static const char* digits = "0123456789ABCDEF";
	std::string rc(hex_len, '0');
	for (size_t i = 0, j = (hex_len - 1) * 4; i < hex_len; ++i, j -= 4)
		rc[i] = digits[(w >> j) & 0x0f];
	return rc;
}

class Timer {
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> startTimepoint;
public:
	Timer() {
		startTimepoint = std::chrono::high_resolution_clock::now();
	}
	~Timer() { Stop(); }

	void Stop() {
		auto endTimepoint = std::chrono::high_resolution_clock::now();
		
		auto start = std::chrono::time_point_cast<std::chrono::microseconds>(startTimepoint).time_since_epoch().count();
		auto stop = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

		auto duration = stop - start;
		std::cout << duration << "us\n";
	}
};