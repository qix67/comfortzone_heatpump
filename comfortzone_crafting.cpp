#include "comfortzone_heatpump.h"
#include "comfortzone_config.h"
#include "comfortzone_frame.h"

#include "comfortzone_crafting.h"

// craft a W_CMD paquet
// input: pointer to output buffer
//		  9 byte array contaning register number
//		 16bit value (it will be automatically stored into little endian)
//		  1 CRC byte
void czcraft::craft_w_cmd(comfortzone_heatpump *czhp, W_CMD *q, byte *reg_num, uint16_t value)
{
	byte comp1_dest[4];

	memcpy(q->cz_head.destination, czhp->heatpump_addr, 4);
	q->cz_head.destination_crc = czhp->CRC8.maxim(q->cz_head.destination, 4);

	comp1_dest[0] = q->cz_head.destination[0] ^ 0xFF;
  	comp1_dest[1] = q->cz_head.destination[1] ^ 0xFF;
  	comp1_dest[2] = q->cz_head.destination[2] ^ 0xFF;
  	comp1_dest[3] = q->cz_head.destination[3] ^ 0xFF;

	q->cz_head.comp1_destination_crc = czhp->CRC8.maxim(comp1_dest, 4);

	memcpy(q->cz_head.source, czhp->controller_addr, 4);
	q->cz_head.packet_size = sizeof(W_CMD);		// == 0x18
	q->cz_head.cmd = 'W';
	memcpy(q->cz_head.reg_num, reg_num, 9);
	
	q->reg_value[0] = value & 0xFF;
	q->reg_value[1] = (value >> 8) & 0xFF;

	q->crc = czhp->CRC8.maxim((byte*)q, sizeof(W_CMD) - 1);
}

// craft a W_SMALL_CMD paquet
// input: pointer to output buffer
//		  9 byte array contaning register number
//		  8bit value
//		  1 CRC byte
void czcraft::craft_w_small_cmd(comfortzone_heatpump *czhp, W_SMALL_CMD *q, byte *reg_num, byte value)
{
	byte comp1_dest[4];

	memcpy(q->cz_head.destination, czhp->heatpump_addr, 4);

	memcpy(q->cz_head.destination, czhp->heatpump_addr, 4);
	q->cz_head.destination_crc = czhp->CRC8.maxim(q->cz_head.destination, 4);

	comp1_dest[0] = q->cz_head.destination[0] ^ 0xFF;
  	comp1_dest[1] = q->cz_head.destination[1] ^ 0xFF;
  	comp1_dest[2] = q->cz_head.destination[2] ^ 0xFF;
  	comp1_dest[3] = q->cz_head.destination[3] ^ 0xFF;

	q->cz_head.comp1_destination_crc = czhp->CRC8.maxim(comp1_dest, 4);

	memcpy(q->cz_head.source, czhp->controller_addr, 4);
	q->cz_head.packet_size = sizeof(W_SMALL_CMD);		// == 0x17
	q->cz_head.cmd = 'W';
	memcpy(q->cz_head.reg_num, reg_num, 9);

	q->reg_value = value;

	q->crc = czhp->CRC8.maxim((byte*)q, sizeof(W_SMALL_CMD) - 1);
}

// craft a W_REPLY packet
// input: pointer to output buffer
//		9 byte array contaning register number
//		expected 8bit value
void czcraft::craft_w_reply(comfortzone_heatpump *czhp, W_REPLY *q, byte *reg_num, byte value)
{
	byte comp1_dest[4];

	memcpy(q->cz_head.destination, czhp->heatpump_addr, 4);

	memcpy(q->cz_head.destination, czhp->controller_addr, 4);
	q->cz_head.destination_crc = czhp->CRC8.maxim(q->cz_head.destination, 4);

	comp1_dest[0] = q->cz_head.destination[0] ^ 0xFF;
  	comp1_dest[1] = q->cz_head.destination[1] ^ 0xFF;
  	comp1_dest[2] = q->cz_head.destination[2] ^ 0xFF;
  	comp1_dest[3] = q->cz_head.destination[3] ^ 0xFF;

	q->cz_head.comp1_destination_crc = czhp->CRC8.maxim(comp1_dest, 4);

	memcpy(q->cz_head.source, czhp->heatpump_addr, 4);
	q->cz_head.packet_size = sizeof(W_REPLY);			// == 0x17
	q->cz_head.cmd = 'w';
	memcpy(q->cz_head.reg_num, reg_num, 9);

	q->return_code = value;

	q->crc = czhp->CRC8.maxim((byte*)q, sizeof(W_REPLY) - 1);
}

