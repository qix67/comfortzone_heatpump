#if 0
#include "comfortzone_heatpump.h"
#include "comfortzone_config.h"
#include "comfortzone_frame.h"

#include "comfortzone_crafting.h"

extern "C"
{
	static W_CMD empty_w_cmd_packet =
										{
											.header = { 0x65, 0x6F, 0xDE },
											.frame_type = 0x02,
											.unknown1 = { 0xD3, 0x5E },
											.addr1 = { 0x41, 0x44, 0x44, 0x52 },
											.packet_size = 0x18,
											.cmd = 'W',
											.reg_num = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 
											.reg_value = { 0x00, 0x00 }, 
											.crc = 0x00,
											.addr2 = { 0x41, 0x44, 0x44, 0x52 },
											.unknown2 = { 0x07, 0x8A }
										};
}

// craft a W_CMD paquet
// input: pointer to output buffer
//        9 byte array contaning register number
//       16bit value (it will be automatically stored into little endian)
//        1 CRC byte
int cz_craft_w_cmd(byte *output_buffer, byte *reg_num, uint16_t value, byte crc)
{
	W_CMD *q = (W_CMD*)output_buffer;

	memcpy(output_buffer, (byte*)&empty_w_cmd_packet, sizeof(W_CMD));

	memcpy(q->reg_num, reg_num, 9);
	q->reg_value[0] = value & 0xFF;
	q->reg_value[1] = (value >> 8) & 0xFF;
	q->crc = crc;
	return sizeof(W_CMD);
}


extern "C"
{
	static W_SMALL_CMD empty_w_small_cmd_packet =
										{
											.header = { 0x65, 0x6F, 0xDE },
											.frame_type = 0x02,
											.unknown1 = { 0xD3, 0x5E },
											.addr1 = { 0x41, 0x44, 0x44, 0x52 },
											.packet_size = 0x17,
											.cmd = 'W',
											.reg_num = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 
											.reg_value = 0x00,
											.crc = 0x00,
											.addr2 = { 0x41, 0x44, 0x44, 0x52 },
											.unknown2 = { 0x07, 0x8A }
										};
}

// craft a W_SMALL_CMD paquet
// input: pointer to output buffer
//        9 byte array contaning register number
//        8bit value
//        1 CRC byte
int cz_craft_w_small_cmd(byte *output_buffer, byte *reg_num, byte value, byte crc)
{
	W_SMALL_CMD *q = (W_SMALL_CMD*)output_buffer;

	memcpy(output_buffer, (byte*)&empty_w_small_cmd_packet, sizeof(W_SMALL_CMD));

	memcpy(q->reg_num, reg_num, 9);
	q->reg_value = value;
	q->crc = crc;
	return sizeof(W_SMALL_CMD);
}

#endif
