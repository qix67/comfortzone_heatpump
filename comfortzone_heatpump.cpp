#include "comfortzone_heatpump.h"
#include "comfortzone_config.h"

#include "comfortzone_frame.h"
#include "comfortzone_decoder.h"

#include "comfortzone_crafting.h"

void comfortzone_heatpump::begin(HardwareSerial &rs485_serial, int de_pin)
{
	rs485 = rs485_serial;
	rs485_de_pin = de_pin;

	rs485.begin(19200, SERIAL_8N1);

	pinMode(rs485_de_pin, OUTPUT);
	digitalWrite(rs485_de_pin, LOW);		// enable RS485 receive mode
}

comfortzone_heatpump::PROCESSED_FRAME_TYPE comfortzone_heatpump::process()
{
	PROCESSED_FRAME_TYPE pft;

	// there is no frame header. First we must collect 21 bytes (sizeof (CZ_PACKET_HEADER)).
	// To find start of frame, it is possible to check if the duration between 2 bytes is not too long
	// (= > 8 bits sent @19.2Kbit/s) which is not possible here as the library does not receive data itself
	// and moreover receiver may (should) have a buffer hidding data stream pause.
	// The 2nd solution is to check if 
	//  - unknown variable (byte[5 & 6]) is either {0xD3, 0x5E} (command) or {0x07, 0x8A} (reply)
	//  - and if cmd byte is 'R' or 'W' (in command case) or 'r' or 'w' (in reply case)
	//
	// if not, first byte is discarded

	while(rs485.available())
	{
		cz_buf[cz_size++] = rs485.read();

		if(cz_size == sizeof(cz_buf))
		{	// something goes wrong. packet size is store in a single byte, how can it goes above 255 ???
			// disable_cz_buf_clear_on_completion = true ?
			cz_size = 0;
			continue;
		}

		if(cz_size < sizeof(CZ_PACKET_HEADER))
			continue;

		if(cz_size == sizeof(CZ_PACKET_HEADER))
		{
			CZ_PACKET_HEADER *czph = (CZ_PACKET_HEADER *)cz_buf;
			byte comp1_dest[4];

			comp1_dest[0] = czph->destination[0] ^ 0xFF;
			comp1_dest[1] = czph->destination[1] ^ 0xFF;
			comp1_dest[2] = czph->destination[2] ^ 0xFF;
			comp1_dest[3] = czph->destination[3] ^ 0xFF;

			if(
				(czph->destination_crc == CRC8.maxim(czph->destination, 4))
				&& (czph->comp1_destination_crc == CRC8.maxim(comp1_dest, 4))
				&& (
						(czph->cmd == 'W') || (czph->cmd == 'R') || (czph->cmd == 'w') || (czph->cmd == 'r')
					)
				)
			{
				cz_full_frame_size = czph->packet_size;
			}
			else
			{
				memcpy(cz_buf, cz_buf + 1, sizeof(CZ_PACKET_HEADER) - 1);
				cz_size--;
				continue;
			}
		}

		if(cz_size == cz_full_frame_size)
			break;
	}

	// not enough data received to be a header
	if(cz_size < sizeof(CZ_PACKET_HEADER))
		return PFT_NONE;

	// not enough data received to be a packet (header+data)
	if(cz_size != cz_full_frame_size)
		return PFT_NONE;

	if(cz_size == sizeof(cz_buf))
	{	// something goes wrong. packet size is store in a single byte, how can it goes above 255 ???
		// disable_cz_buf_clear_on_completion = true ?
		cz_size = 0;
		return PFT_NONE;
	}

	// check frame CRC (last byte of buffer is CRC
	if(CRC8.maxim(cz_buf, cz_size - 1) == cz_buf[cz_size - 1])
	{
		pft = czdec::process_frame(this, (CZ_PACKET_HEADER *)cz_buf);
	}
	else
	{
		pft = PFT_CORRUPTED;
	}

	if(grab_buffer)
	{
		if(cz_size > grab_buffer_size)
		{
			// frame is too big for grab buffer => notify empty frame
			*grab_buffer_frame_size = 0;
		}
		else
		{
			memcpy(grab_buffer, cz_buf, cz_size);
			*grab_buffer_frame_size = cz_size;
		}
	}

	if(!disable_cz_buf_clear_on_completion)
		cz_size = 0;

	last_frame_timestamp = millis();

	if(pft == comfortzone_heatpump::PFT_REPLY)
		last_reply_frame_timestamp = last_frame_timestamp;

	return pft;
}

// for debug purpose, it can be useful to get full frame
// input: pointer on buffer where last full frame will be copied
//        max size of buffer
//        size of the last full frame received
// If buffer is set to NULL, frame grabber is disabled
// If buffer is not NULL, each time comfortzone_receive() reply is not PFT_NONE, the received frame will
// be copied into buffer and *frame_size will be updated
// recommended buffer_size is 256 bytes
void comfortzone_heatpump::set_grab_buffer(byte *buffer, uint16_t buffer_size, uint16_t *frame_size)
{
	if(buffer == NULL)
	{
		grab_buffer = NULL;
		grab_buffer_size = 0;
		grab_buffer_frame_size = NULL;
	}
	else
	{
		grab_buffer = buffer;
		grab_buffer_size = buffer_size;
		grab_buffer_frame_size = frame_size;
	}
}

// Functions to modify heatpump settings
// timeout (in second) is the maximum duration before giving up (RS485 bus always busy)
// output: true = ok, false = failed to process

// from logical analyzer, after a command, there is roughly 50ms-60ms before the reply
// except for status 19 where reply comes ~200ms later
// after status 08 reply, there is a pause of ~1.1s.
// between a replay and the next command, there is a variable gap between 2 and 10 ms

// algorithm: 
// 1) craft command packet and expected reply
// 2) we assume there is only 3 devices on the bus, the heatpump, its control panel and us.
//    This means after a reply, we can immediatly send a command without collision
//    and we expect the reply within 200ms
// Another solution is to wait for status 08 reply and use the large pause. However, the pause
// appears once only every 5 seconds. This is the solution used here because it generates no side effect

#define RETURN_MESSAGE(msg)	{ strcpy(last_message, msg); last_message_size = strlen(last_message); }

// fan speed: 1 = low, 2 = normal, 3 = fast
bool comfortzone_heatpump::set_fan_speed(uint8_t fan_speed, int timeout)
{
	W_SMALL_CMD cmd;
	W_REPLY expected_reply;
	czdec::KNOWN_REGISTER *kr;
	bool push_result;

	if((fan_speed < 1) || (fan_speed > 3))
	{
		RETURN_MESSAGE("Invalid value, must be between 1 and 3");
		return false;
	}

	kr = czdec::kr_craft_name_to_index(czcraft::KR_FAN_SPEED);

	if(kr == NULL)
	{
		RETURN_MESSAGE("czcraft::KR_FAN_SPEED not found");
		return false;
	}

	czcraft::craft_w_small_cmd(this, &cmd, kr->reg_num, fan_speed);
	czcraft::craft_w_reply(this, &expected_reply, kr->reg_num, fan_speed);

	push_result = push_settings((byte*)&cmd, sizeof(cmd), (byte*)&expected_reply, sizeof(expected_reply), timeout);

	if(push_result == true)
	{
		// on success, immediatly update status cache. Without this, if status cache is sent to client
		// before receiving update from heatpump, an incorrect value is returned
		comfortzone_status.fan_speed = fan_speed;
	}

	return push_result;
}

// room temperature temperature in °C: 10.0°->50.0°, step 0.1°
bool comfortzone_heatpump::set_room_temperature(float room_temp, int timeout)
{
	W_CMD cmd;
	W_REPLY expected_reply;
	czdec::KNOWN_REGISTER *kr;
	uint16_t int_value;
	bool push_result;

	if((room_temp < 10.0) || (room_temp > 50.0))
	{
		RETURN_MESSAGE("Invalid value, must be between 10.0 and 50.0");
		return false;
	}

	kr = czdec::kr_craft_name_to_index(czcraft::KR_ROOM_HEATING_TEMP);

	if(kr == NULL)
	{
		RETURN_MESSAGE("czcraft::KR_ROOM_HEATING_TEMP not found");
		return false;
	}

	int_value = (room_temp * 10.0 + 0.5);		// +0.5 to round to closest integer number

	czcraft::craft_w_cmd(this, &cmd, kr->reg_num, int_value);
	czcraft::craft_w_reply(this, &expected_reply, kr->reg_num, 0);		// reply value is always 0 on success

	push_result = push_settings((byte*)&cmd, sizeof(cmd), (byte*)&expected_reply, sizeof(expected_reply), timeout);

	if(push_result == true)
	{
		// on success, immediatly update status cache. Without this, if status cache is sent to client
		// before receiving update from heatpump, an incorrect value is returned
		comfortzone_status.room_heating_setting = int_value;
	}

	return push_result;
}

// hot water temperature in °C: 10.0°-60.0°, step 0.1°
bool comfortzone_heatpump::set_hot_water_temperature(float hot_water_temp, int timeout)
{
	W_CMD cmd;
	W_REPLY expected_reply;
	czdec::KNOWN_REGISTER *kr;
	uint16_t int_value;
	bool push_result;

	// WARNING: technically, heatpump (or at least control panel) accepts 0.0° as minimum value but by security, I limit it to 10.0°
	if((hot_water_temp < 10.0) || (hot_water_temp > 60.0))
	{
		RETURN_MESSAGE("Invalid value, must be between 10.0 and 60.0");
		return false;
	}

	kr = czdec::kr_craft_name_to_index(czcraft::KR_HOT_WATER_TEMP);

	if(kr == NULL)
	{
		RETURN_MESSAGE("czcraft::KR_ROOM_HEATING_TEMP not found");
		return false;
	}

	int_value = (hot_water_temp * 10.0 + 0.5);		// +0.5 to round to closest integer number

	czcraft::craft_w_cmd(this, &cmd, kr->reg_num, int_value);
	czcraft::craft_w_reply(this, &expected_reply, kr->reg_num, 1);		// reply value is always 1 on success

	push_result = push_settings((byte*)&cmd, sizeof(cmd), (byte*)&expected_reply, sizeof(expected_reply), timeout);

	if(push_result == true)
	{
		// on success, immediatly update status cache. Without this, if status cache is sent to client
		// before receiving update from heatpump, an incorrect value is returned
		comfortzone_status.hot_water_setting = int_value;
	}

	return push_result;
}

// led level: 0 = off -> 6 = highest level
bool comfortzone_heatpump::set_led_luminosity(uint8_t led_level, int timeout)
{
	W_SMALL_CMD cmd;
	W_REPLY expected_reply;
	czdec::KNOWN_REGISTER *kr;
	bool push_result;

	if(led_level > 6)
	{
		RETURN_MESSAGE("Invalid value, must be between 0 and 6");
		return false;
	}

	kr = czdec::kr_craft_name_to_index(czcraft::KR_LED_LUMINOSITY);

	if(kr == NULL)
	{
		RETURN_MESSAGE("czcraft::KR_LED_LUMINOSITY not found");
		return false;
	}

	czcraft::craft_w_small_cmd(this, &cmd, kr->reg_num, led_level);
	czcraft::craft_w_reply(this, &expected_reply, kr->reg_num, led_level);

	push_result = push_settings((byte*)&cmd, sizeof(cmd), (byte*)&expected_reply, sizeof(expected_reply), timeout);

	if(push_result == true)
	{
		// on success, immediatly update status cache. Without this, if status cache is sent to client
		// before receiving update from heatpump, an incorrect value is returned
		comfortzone_status.led_luminosity_setting = led_level;
	}

	return push_result;
}

// hour: 0-23
bool comfortzone_heatpump::set_hour(uint8_t hour, int timeout)
{
	W_SMALL_CMD cmd;
	W_REPLY expected_reply;
	czdec::KNOWN_REGISTER *kr;
	bool push_result;

	if(hour > 23)
	{
		RETURN_MESSAGE("Invalid value, must be between 0 and 23");
		return false;
	}

	kr = czdec::kr_craft_name_to_index(czcraft::KR_HOUR);

	if(kr == NULL)
	{
		RETURN_MESSAGE("czcraft::KR_HOUR not found");
		return false;
	}

	czcraft::craft_w_small_cmd(this, &cmd, kr->reg_num, hour);
	czcraft::craft_w_reply(this, &expected_reply, kr->reg_num, hour);

	push_result = push_settings((byte*)&cmd, sizeof(cmd), (byte*)&expected_reply, sizeof(expected_reply), timeout);

	if(push_result == true)
	{
		// on success, immediatly update status cache. Without this, if status cache is sent to client
		// before receiving update from heatpump, an incorrect value is returned
		comfortzone_status.hour = hour;
	}

	return push_result;
}

// minute: 0-59
bool comfortzone_heatpump::set_minute(uint8_t minute, int timeout)
{
	W_SMALL_CMD cmd;
	W_REPLY expected_reply;
	czdec::KNOWN_REGISTER *kr;
	bool push_result;

	if(minute > 59)
	{
		RETURN_MESSAGE("Invalid value, must be between 0 and 59");
		return false;
	}

	kr = czdec::kr_craft_name_to_index(czcraft::KR_MINUTE);

	if(kr == NULL)
	{
		RETURN_MESSAGE("czcraft::KR_MINUTE not found");
		return false;
	}

	czcraft::craft_w_small_cmd(this, &cmd, kr->reg_num, minute);
	czcraft::craft_w_reply(this, &expected_reply, kr->reg_num, minute);

	push_result = push_settings((byte*)&cmd, sizeof(cmd), (byte*)&expected_reply, sizeof(expected_reply), timeout);

	if(push_result == true)
	{
		// on success, immediatly update status cache. Without this, if status cache is sent to client
		// before receiving update from heatpump, an incorrect value is returned
		comfortzone_status.minute = minute;
	}

	return push_result;
}

// day: 1-31
bool comfortzone_heatpump::set_day(uint8_t day, int timeout)
{
	W_SMALL_CMD cmd;
	W_REPLY expected_reply;
	czdec::KNOWN_REGISTER *kr;
	bool push_result;

	if((day < 1) || (day > 31))
	{
		RETURN_MESSAGE("Invalid value, must be between 1 and 31");
		return false;
	}

	kr = czdec::kr_craft_name_to_index(czcraft::KR_DAY);

	if(kr == NULL)
	{
		RETURN_MESSAGE("czcraft::KR_DAY not found");
		return false;
	}

	czcraft::craft_w_small_cmd(this, &cmd, kr->reg_num, day);
	czcraft::craft_w_reply(this, &expected_reply, kr->reg_num, day);

	push_result = push_settings((byte*)&cmd, sizeof(cmd), (byte*)&expected_reply, sizeof(expected_reply), timeout);

	if(push_result == true)
	{
		// on success, immediatly update status cache. Without this, if status cache is sent to client
		// before receiving update from heatpump, an incorrect value is returned
		comfortzone_status.day = day;
	}

	return push_result;
}

// month: 1-12
bool comfortzone_heatpump::set_month(uint8_t month, int timeout)
{
	W_SMALL_CMD cmd;
	W_REPLY expected_reply;
	czdec::KNOWN_REGISTER *kr;
	bool push_result;

	if((month < 1) || (month > 12))
	{
		RETURN_MESSAGE("Invalid value, must be between 1 and 12");
		return false;
	}

	kr = czdec::kr_craft_name_to_index(czcraft::KR_MONTH);

	if(kr == NULL)
	{
		RETURN_MESSAGE("czcraft::KR_MONTH not found");
		return false;
	}

	czcraft::craft_w_small_cmd(this, &cmd, kr->reg_num, month);
	czcraft::craft_w_reply(this, &expected_reply, kr->reg_num, month);

	push_result = push_settings((byte*)&cmd, sizeof(cmd), (byte*)&expected_reply, sizeof(expected_reply), timeout);

	if(push_result == true)
	{
		// on success, immediatly update status cache. Without this, if status cache is sent to client
		// before receiving update from heatpump, an incorrect value is returned
		comfortzone_status.month = month;
	}

	return push_result;
}

// year: 2000-2255
bool comfortzone_heatpump::set_year(uint16_t year, int timeout)
{
	W_SMALL_CMD cmd;
	W_REPLY expected_reply;
	czdec::KNOWN_REGISTER *kr;
	bool push_result;

	if((year < 2000) || (year > 2255))
	{
		RETURN_MESSAGE("Invalid value, must be between 2000 and 2255");
		return false;
	}

	kr = czdec::kr_craft_name_to_index(czcraft::KR_YEAR);

	if(kr == NULL)
	{
		RETURN_MESSAGE("czcraft::KR_YEAR not found");
		return false;
	}

	year -= 2000;

	czcraft::craft_w_small_cmd(this, &cmd, kr->reg_num, year);
	czcraft::craft_w_reply(this, &expected_reply, kr->reg_num, year);

	push_result = push_settings((byte*)&cmd, sizeof(cmd), (byte*)&expected_reply, sizeof(expected_reply), timeout);

	if(push_result == true)
	{
		// on success, immediatly update status cache. Without this, if status cache is sent to client
		// before receiving update from heatpump, an incorrect value is returned
		comfortzone_status.year = year;
	}

	return push_result;
}

// true = enable, false = disable
bool comfortzone_heatpump::set_extra_hot_water(bool enable, int timeout)
{
	W_CMD cmd;
	W_REPLY expected_reply;
	czdec::KNOWN_REGISTER *kr;
	uint16_t cmd_value;
	byte reply_value;
	bool push_result;

	// Don't know why this setting requires 2 register (???)
	if(enable)
	{
		kr = czdec::kr_craft_name_to_index(czcraft::KR_EXTRA_HOT_WATER_ON);
		cmd_value = 0x0001;
		reply_value = 0x08;
	}
	else
	{
		kr = czdec::kr_craft_name_to_index(czcraft::KR_EXTRA_HOT_WATER_OFF);
		cmd_value = 0xFFFE;
		reply_value = 0x0C;
	}

	if(kr == NULL)
	{
		RETURN_MESSAGE("nczcraft::KR_EXTRA_HOT_WATER_* Not found");
		return false;
	}

	czcraft::craft_w_cmd(this, &cmd, kr->reg_num, cmd_value);
	czcraft::craft_w_reply(this, &expected_reply, kr->reg_num, reply_value);

	push_result = push_settings((byte*)&cmd, sizeof(cmd), (byte*)&expected_reply, sizeof(expected_reply), timeout);

	if(push_result == true)
	{
		// on success, immediatly update status cache. Without this, if status cache is sent to client
		// before receiving update from heatpump, an incorrect value is returned
		comfortzone_status.extra_hot_water_setting = enable;
	}

	return push_result;
}

// true = enable, false = disable
bool comfortzone_heatpump::set_automatic_daylight_saving(bool enable, int timeout)
{
	W_CMD cmd;
	W_REPLY expected_reply;
	czdec::KNOWN_REGISTER *kr;
	uint16_t cmd_value;
	byte reply_value;
	bool push_result;

	// Don't know why this setting requires 2 register (???)
	if(enable)
	{
		kr = czdec::kr_craft_name_to_index(czcraft::KR_AUTO_DAYLIGHT_SAVING_ON);
		cmd_value = 0x0040;
		reply_value = 0x8E;
	}
	else
	{
		kr = czdec::kr_craft_name_to_index(czcraft::KR_AUTO_DAYLIGHT_SAVING_OFF);
		cmd_value = 0xFFBF;
		reply_value = 0x8E;
	}

	if(kr == NULL)
	{
		RETURN_MESSAGE("nczcraft::KR_AUTO_DAYLIGHT_SAVING_* Not found");
		return false;
	}

	czcraft::craft_w_cmd(this, &cmd, kr->reg_num, cmd_value);
	czcraft::craft_w_reply(this, &expected_reply, kr->reg_num, reply_value);

	push_result = push_settings((byte*)&cmd, sizeof(cmd), (byte*)&expected_reply, sizeof(expected_reply), timeout);

	if(push_result == true)
	{
		// on success, immediatly update status cache. Without this, if status cache is sent to client
		// before receiving update from heatpump, an incorrect value is returned
		//comfortzone_status.fan_speed = fan_speed;
		// (never stored in cache)
	}

	return push_result;
}

// Sensor num: 0 -> 7
// Sensor offset: -10.0 -> 10.0
bool comfortzone_heatpump::set_sensor_offset(uint16_t sensor_num, float temp_offset, int timeout)
{
	W_SMALL_CMD cmd;
	W_REPLY expected_reply;
	czdec::KNOWN_REGISTER *kr;
	czcraft::KNOWN_REGISTER_CRAFT_NAME czname;
	uint16_t int_temp_offset;

	bool push_result;

	switch(sensor_num)
	{
		case 0:
					czname = czcraft::KR_TEMP_OFFSET_SENSOR0; break;

		case 1:
					czname = czcraft::KR_TEMP_OFFSET_SENSOR1; break;

		case 2:
					czname = czcraft::KR_TEMP_OFFSET_SENSOR2; break;

		case 3:
					czname = czcraft::KR_TEMP_OFFSET_SENSOR3; break;

		case 4:
					czname = czcraft::KR_TEMP_OFFSET_SENSOR4; break;

		case 5:
					czname = czcraft::KR_TEMP_OFFSET_SENSOR5; break;

		case 6:
					czname = czcraft::KR_TEMP_OFFSET_SENSOR6; break;

		case 7:
					czname = czcraft::KR_TEMP_OFFSET_SENSOR7; break;

		default:
					RETURN_MESSAGE("Invalid sensor number, must be between 0 and 7");
					return false;
	}

	if((temp_offset < -10.0) || (temp_offset > 10.0))
	{
		RETURN_MESSAGE("Invalid value, must be between -10.0 and 10.0");
		return false;
	}

	kr = czdec::kr_craft_name_to_index(czname);

	if(kr == NULL)
	{
		RETURN_MESSAGE("czcraft::KR_TEMP_OFFSET_SENSORx not found");
		return false;
	}

	int_temp_offset = (int)(temp_offset * 10.0);

	czcraft::craft_w_small_cmd(this, &cmd, kr->reg_num, int_temp_offset);
	czcraft::craft_w_reply(this, &expected_reply, kr->reg_num, int_temp_offset);

	push_result = push_settings((byte*)&cmd, sizeof(cmd), (byte*)&expected_reply, sizeof(expected_reply), timeout);

	if(push_result == true)
	{
		// on success, immediatly update status cache. Without this, if status cache is sent to client
		// before receiving update from heatpump, an incorrect value is returned
		//comfortzone_status.xxx = led_level;
		// (never stored in cache)
	}
	return push_result;
}

// send a command to the heatpump and wait for the given reply
// on error, several retries may occur and the command may take up to "timeout" seconds
// if reply_header_check_only is false, reply must be exactly equal to expected_reply
// if reply_header_check_only is true, only reply header (CZ_PACKET_HEADER) must match expected_reply header
bool comfortzone_heatpump::push_settings(byte *cmd, int cmd_length, byte *expected_reply, int expected_reply_length, int timeout, bool header_check_only)
{
	unsigned long now;
	unsigned long timeout_time;
	unsigned long min_time_after_reply;
	unsigned long reply_frame_time;
	unsigned long reply_timeout;
	PROCESSED_FRAME_TYPE pft;
	int i;

	now = millis();
	timeout_time = now + timeout * 1000;

	last_message[0] = '\0';
	last_message_size = 0;

	if(debug_mode)
	{
		for(i=0; i < expected_reply_length; i ++)
		{
			if(last_message_size >= (COMFORTZONE_HEATPUMP_LAST_MESSAGE_BUFFER_SIZE - 4))
				break;

			sprintf(last_message + last_message_size, "%02X ", (int)(expected_reply[i]));
	
			last_message_size += 3;
		}
	}

	while(now < timeout_time)
	{
		if(last_frame_timestamp == last_reply_frame_timestamp)
		{
			if( (debug_mode) && (last_message_size < (COMFORTZONE_HEATPUMP_LAST_MESSAGE_BUFFER_SIZE - 2)) )
				last_message[last_message_size++] = 'a';
			
			// last received frame was a reply frame
			min_time_after_reply = now + 40;		// during test, there is always less than 10ms after a reply and controller next command
			reply_frame_time = last_reply_frame_timestamp;

			// wait a little time to see if the controller has not strated to send a new command
			while(now < min_time_after_reply)
			{
				pft = process();

				if(pft != comfortzone_heatpump::PFT_NONE)
					break;

				now = millis();
			}

			// no new frame or reply frame and incoming frame buffer is empty, we have a go
			if(
				(last_frame_timestamp == last_reply_frame_timestamp)
				&& (reply_frame_time == last_reply_frame_timestamp)
				&& (cz_size == 0))
			{
				if( (debug_mode) && (last_message_size < (COMFORTZONE_HEATPUMP_LAST_MESSAGE_BUFFER_SIZE - 2)) )
					last_message[last_message_size++] = 'b';

				digitalWrite(rs485_de_pin, HIGH);	// enable send mode
				disable_cz_buf_clear_on_completion = true;
				rs485.write(cmd, cmd_length);
				rs485.flush();
				digitalWrite(rs485_de_pin, LOW);		// enable receive mode

				// now, wait for a frame at most 100ms
				now = millis();
				reply_timeout = now + 200;

				while(now < reply_timeout)
				{
					pft = process();

					if(pft != comfortzone_heatpump::PFT_NONE)
						break;

					now = millis();
				}
				
				// if we have a reply frame with the correct size and content, command was successfully processed
				if( (pft == comfortzone_heatpump::PFT_REPLY)
					 && (cz_size == expected_reply_length)
					 &&	(
								( (header_check_only == false) && (!memcmp(cz_buf, expected_reply, expected_reply_length)) )
								||
								( (header_check_only == true) && (!memcmp(cz_buf, expected_reply, sizeof(CZ_PACKET_HEADER))) )
							)
					)
				{
					if( (debug_mode) && (last_message_size < (COMFORTZONE_HEATPUMP_LAST_MESSAGE_BUFFER_SIZE - 2)) )
						last_message[last_message_size++] = 'c';

					// clear input buffer and restart normal frame processing
					disable_cz_buf_clear_on_completion = false;
					cz_size = 0;

					last_message[last_message_size] = '\0';
					return true;
				}

				// no correct reply received, retry
				if( (debug_mode) && (last_message_size < (COMFORTZONE_HEATPUMP_LAST_MESSAGE_BUFFER_SIZE - 34)) )
				{
					last_message[last_message_size++] = 'd';
					sprintf(last_message + last_message_size, "%d-(%02X) %02X %02X %02X %c-%d=%d ", (int)pft, 
												((CZ_PACKET_HEADER*)cz_buf)->comp1_destination_crc,
												((CZ_PACKET_HEADER*)cz_buf)->reg_num[6],
												((CZ_PACKET_HEADER*)cz_buf)->reg_num[7],
												((CZ_PACKET_HEADER*)cz_buf)->reg_num[8],
												((CZ_PACKET_HEADER*)cz_buf)->cmd, cz_size, expected_reply_length);
					last_message_size = strlen(last_message);

					if(pft != comfortzone_heatpump::PFT_NONE)
					{
						if(last_message_size < (COMFORTZONE_HEATPUMP_LAST_MESSAGE_BUFFER_SIZE - 2))
							last_message[last_message_size++] = '\n';

						for(i=0; i < cz_size; i ++)
						{
							if(last_message_size >= (COMFORTZONE_HEATPUMP_LAST_MESSAGE_BUFFER_SIZE - 4))
								break;
					
							sprintf(last_message + last_message_size, "%02X ", (int)(cz_buf[i]));
					
							last_message_size = strlen(last_message);
						}
					}
				}

				// clear input buffer and restart normal frame processing
				disable_cz_buf_clear_on_completion = false;
				cz_size = 0;

				if(pft != comfortzone_heatpump::PFT_NONE)
				{
					last_message[last_message_size] = '\0';
					return false;
				}
			}
		}

		process();
		now = millis();
	}

	last_message[last_message_size] = '\0';
	return false;
}

void comfortzone_heatpump::enable_debug_mode(bool debug_flag)
{
	debug_mode = debug_flag;

	last_message[0] = '\0';
	last_message_size = 0;
}

void comfortzone_heatpump::set_heatpump_addr(byte new_heatpump_addr[4])
{
	heatpump_addr[0] = new_heatpump_addr[0];
	heatpump_addr[1] = new_heatpump_addr[1];
	heatpump_addr[2] = new_heatpump_addr[2];
	heatpump_addr[3] = new_heatpump_addr[3];
}

// try to guess heatpump address from query packet sent by control panel
bool comfortzone_heatpump::guess_heatpump_addr(byte guessed_addr[4], int timeout)
{
	unsigned long now;
	unsigned long timeout_time;
	PROCESSED_FRAME_TYPE pft;

	now = millis();
	timeout_time = now + timeout * 1000;

	disable_cz_buf_clear_on_completion = true;

	while(now < timeout_time)
	{
		pft = process();

		if(pft != comfortzone_heatpump::PFT_NONE)
		{
			if(pft == comfortzone_heatpump::PFT_QUERY)
			{
				guessed_addr[0] = ((CZ_PACKET_HEADER*)cz_buf)->destination[0];
				guessed_addr[1] = ((CZ_PACKET_HEADER*)cz_buf)->destination[1];
				guessed_addr[2] = ((CZ_PACKET_HEADER*)cz_buf)->destination[2];
				guessed_addr[3] = ((CZ_PACKET_HEADER*)cz_buf)->destination[3];

				disable_cz_buf_clear_on_completion = false;
				cz_size = 0;

				return true;
			}

			// manually clear buffer
			cz_size = 0;
		}

		now = millis();
	}

	disable_cz_buf_clear_on_completion = false;
	cz_size = 0;

	return false;
}

