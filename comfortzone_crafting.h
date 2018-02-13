#ifndef _COMFORTZONE_CRAFTING_H
#define _COMFORTZONE_CRAFTING_H

#include "comfortzone_heatpump.h"
#include "comfortzone_frame.h"

// craft a W_CMD paquet
// input: pointer to output buffer
//        9 byte array contaning register number
//       16bit value (it will be automatically stored into little endian)
//        1 CRC byte
int cz_craft_w_cmd(byte *output_buffer, byte *reg_num, uint16_t value, byte crc);

// craft a W_SMALL_CMD paquet
// input: pointer to output buffer
//        9 byte array contaning register number
//        8bit value
//        1 CRC byte
int cz_craft_w_small_cmd(byte *output_buffer, byte *reg_num, byte value, byte crc);

#endif
