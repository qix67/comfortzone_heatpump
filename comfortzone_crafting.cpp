#include "comfortzone_heatpump.h"
#include "comfortzone_config.h"
#include "comfortzone_frame.h"

#include "comfortzone_crafting.h"

// craft a W_CMD paquet
// input: pointer to output buffer
//		  9 byte array contaning register number
//		 16bit value (it will be automatically stored into little endian)
//		  1 CRC byte
int czcraft::craft_w_cmd(comfortzone_heatpump *czhp, byte *output_buffer, byte *reg_num, uint16_t value)
{
	W_CMD *q = (W_CMD*)output_buffer;

	memcpy(q->cz_head.source, czhp->controller_addr, 4);
	q->cz_head.unknown[0] = 0xD3;
	q->cz_head.unknown[1] = 0x5E;
	memcpy(q->cz_head.destination, czhp->heatpump_addr, 4);
	q->cz_head.packet_size = sizeof(W_CMD);		// == 0x18
	q->cz_head.cmd = 'W';
	memcpy(q->cz_head.reg_num, reg_num, 9);
	
	q->reg_value[0] = value & 0xFF;
	q->reg_value[1] = (value >> 8) & 0xFF;

	q->crc = czhp->CRC8.maxim(output_buffer, sizeof(W_CMD) - 1);

	return sizeof(W_CMD);
}

// craft a W_SMALL_CMD paquet
// input: pointer to output buffer
//		  9 byte array contaning register number
//		  8bit value
//		  1 CRC byte
int czcraft::craft_w_small_cmd(comfortzone_heatpump *czhp, byte *output_buffer, byte *reg_num, byte value)
{
	W_SMALL_CMD *q = (W_SMALL_CMD*)output_buffer;

	memcpy(q->cz_head.source, czhp->controller_addr, 4);
	q->cz_head.unknown[0] = 0xD3;
	q->cz_head.unknown[1] = 0x5E;
	memcpy(q->cz_head.destination, czhp->heatpump_addr, 4);
	q->cz_head.packet_size = sizeof(W_SMALL_CMD);		// == 0x17
	q->cz_head.cmd = 'W';
	memcpy(q->cz_head.reg_num, reg_num, 9);

	q->reg_value = value;

	q->crc = czhp->CRC8.maxim(output_buffer, sizeof(W_SMALL_CMD) - 1);

	return sizeof(W_SMALL_CMD);
}

// craft a W_REPLY packet
// input: pointer to output buffer
//		9 byte array contaning register number
//		expected 8bit value
int czcraft::craft_w_reply(comfortzone_heatpump *czhp, byte *output_buffer, byte *reg_num, byte value)
{
	W_REPLY *q = (W_REPLY*)output_buffer;

	memcpy(q->cz_head.source, czhp->heatpump_addr, 4);
	q->cz_head.unknown[0] = 0x07;
	q->cz_head.unknown[1] = 0x8A;
	memcpy(q->cz_head.destination, czhp->controller_addr, 4);
	q->cz_head.packet_size = sizeof(W_REPLY);			// == 0x17
	q->cz_head.cmd = 'w';
	memcpy(q->cz_head.reg_num, reg_num, 9);

	q->return_code = value;

	q->crc = czhp->CRC8.maxim(output_buffer, sizeof(W_REPLY) - 1);

	return sizeof(W_REPLY);
}

