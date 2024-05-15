#pragma once

#ifdef ESP_PLATFORM
#ifndef ARDUINO
#include "esp_timer.h"
inline unsigned long millis() {
	return esp_timer_get_time() / 1000;
}
#endif
#endif

#ifdef _WIN32
#include <windows.h>
#include <stdint.h>

inline unsigned long millis() {
    // FILETIME represents the number of 100-nanosecond intervals since January 1, 1601 (UTC)
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);

    // Convert the FILETIME to a ULARGE_INTEGER for arithmetic purposes
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;

    // Convert the time to milliseconds from 100-nanosecond intervals
    uint64_t time_in_ms = uli.QuadPart / 10000;

    // The Unix epoch starts on Jan 1 1970. To convert the FILETIME to Unix epoch time,
    // subtract the number of milliseconds from Jan 1 1601 to Jan 1 1970
    const uint64_t UNIX_TIME_START = 11644473600000ULL;

    // Finally, convert the time to Unix epoch in milliseconds
    uint64_t epoch_time_in_ms = time_in_ms - UNIX_TIME_START;

    return epoch_time_in_ms;
}
#endif

#ifndef NULL
#define NULL 0
#endif

typedef unsigned char byte;
