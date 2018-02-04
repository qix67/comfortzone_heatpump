#ifndef _COMFORTZONE_FRAME_H
#define _COMFORTZONE_FRAME_H

#include <Arduino.h>

// This file contains the description of RS485 frames

#define FRAME_TYPE_01 0x01
#define FRAME_TYPE_02 0x02
#define FRAME_TYPE_02_CMD_p1 0x4302
#define FRAME_TYPE_02_CMD_p2 0x6302
#define FRAME_TYPE_02_REPLY 0x5202
#define FRAME_TYPE_15 0x15

// r cmd is
// 65 6F DE 02 D3 5E 41 44 44 52 17 52 xx xx xx xx xx xx xx xx xx yy cc 41 44 44 52 07 8A
// xx is register num
// yy is ?? (wanted reply size ?)
// cc = crc
typedef struct __attribute__ ((packed))
{
	byte header[3];		// 65 6F DE
	byte frame_type	;	// 02
	byte unkown1[2];		// D3 5E
	byte addr1[4];			// 41 44 44 52 (=ADDR)
	byte packet_size;		// cz_size-6  = 0x17  (BCD?)
	byte cmd;				// 52 (=r)
	byte reg_num[9];
	byte wanted_reply_size;
	byte crc;
	byte addr2[4];			// 41 44 44 52 (=ADDR)
	byte unknown2[2];		// 07 8A
} R_CMD;

// w cmd is
// 65 6F DE 02 D3 5E 41 44 44 52 18 57 xx xx xx xx xx xx xx xx xx yy yy cc 41 44 44 52 07 8A
// xx is register num
// yy is reg value (little endian)
// cc = crc
typedef struct __attribute__ ((packed))
{
	byte header[3];		// 65 6F DE
	byte frame_type	;	// 02
	byte unkown1[2];		// D3 5E
	byte addr1[4];			// 41 44 44 52 (=ADDR)
	byte packet_size;		// cz_size-6  = 0x18  (BCD?)
	byte cmd;				// 57 (=W)
	byte reg_num[9];
	byte reg_value[2];
	byte crc;
	byte addr2[4];			// 41 44 44 52 (=ADDR)
	byte unknown2[2];		// 07 8A
} W_CMD;

// w cmd is
// 65 6F DE 02 D3 5E 41 44 44 52 17 57 xx xx xx xx xx xx xx xx xx yy yy cc 41 44 44 52 07 8A
// xx is register num
// yy is reg value (little endian)
// cc = crc
typedef struct __attribute__ ((packed))
{
	byte header[3];		// 65 6F DE
	byte frame_type	;	// 02
	byte unkown1[2];		// D3 5E
	byte addr1[4];			// 41 44 44 52 (=ADDR)
	byte packet_size;		// cz_size-6  = 0x17  (BCD?)
	byte cmd;				// 57 (=W)
	byte reg_num[9];
	byte reg_value;
	byte crc;
	byte addr2[4];			// 41 44 44 52 (=ADDR)
	byte unknown2[2];		// 07 8A
} W_SMALL_CMD;

// r reply is
// 65 6F DE 02 18 52 xx xx xx xx xx xx xx xx xx yy yy cc
// xx is register num
// yy is reg value (little endian)
// cc = crc
typedef struct __attribute__ ((packed))
{
	byte header[3];		// 65 6F DE
	byte frame_type	;	// 02
	byte packet_size;		// cz_size+6  = 0x18
	byte cmd;				// 52 (=r)
	byte reg_num[9];
	byte reg_value[2];
	byte crc;
} R_REPLY;

// r reply is
// 65 6F DE 02 17 52 xx xx xx xx xx xx xx xx xx yy cc
// xx is register num
// yy is reg value (little endian)
// cc = crc
typedef struct __attribute__ ((packed))
{
	byte header[3];		// 65 6F DE
	byte frame_type	;	// 02
	byte packet_size;		// cz_size+6  = 0x17
	byte cmd;				// 52 (=r)
	byte reg_num[9];
	byte reg_value;
	byte crc;
} R_SMALL_REPLY;

// w reply is
// 65 6F DE 02 17 77 xx xx xx xx xx xx xx xx xx yy cc
// xx is register num
// yy = status code: 00 = ok
// cc = crc
typedef struct __attribute__ ((packed))
{
	byte header[3];		// 65 6F DE
	byte frame_type	;	// 02
	byte packet_size;		// cz_size+6  = 0x17
	byte cmd;				// 77 (=w)
	byte reg_num[9];
	byte return_code;
	byte crc;
} W_REPLY;

#endif
