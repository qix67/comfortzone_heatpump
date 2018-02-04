#ifndef _COMFORTZONE_TOOLS_H
#define _COMFORTZONE_TOOLS_H

#include <Arduino.h>

static inline uint16_t get_uint16(byte *ptr)
{
	uint16_t v;

	v = (ptr[1] << 8) | ptr[0];

	return v;
}

static inline int16_t get_int16(byte *ptr)
{
	int16_t v;

	v = (ptr[1] << 8) | ptr[0];

	return v;
}

static inline uint32_t get_uint32(byte *ptr)
{
	uint32_t v;

	v = (ptr[3] << 24) | (ptr[2] << 16) | (ptr[1] << 8) | ptr[0];

	return v;
}

#endif
