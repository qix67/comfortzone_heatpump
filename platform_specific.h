#pragma once

#ifdef ESP_PLATFORM
#include "esp_timer.h"
inline unsigned long millis() {
	return esp_timer_get_time() / 1000;
}
#endif

#ifndef NULL
#define NULL 0
#endif

typedef unsigned char byte;
