// Comfortzone heat pump to Home Assistant over MQTT (protocol 2.50).
//
// - Snoops the RS485 bus and publishes every decoded value to <prefix>/state/<name> (retained, on change,
//   full refresh every 5 minutes).
// - Accepts commands on <prefix>/set/<name> and calls the matching library setter.
// - Announces all entities through Home Assistant MQTT discovery, grouped under one device.
// - <prefix>/availability is "online" while frames arrive, "offline" otherwise (and as last will).
// - <prefix>/result/last carries the outcome of the last command (OK or FAILED plus the library's trace).
//
// Needs: PubSubClient (Nick O'Leary), the ESP32 core, and a config.h next to this file (see config.h.example).
// Keep DEBUG off in comfortzone_config.h: the frame dump would starve the loop.

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>

#include "comfortzone_heatpump.h"
#include "rs485_interface.h"
#include "config.h"

#define SKETCH_VERSION "0.1"

#define FULL_REFRESH_MS      300000UL	// republish everything every 5 minutes
#define BUS_DEAD_MS          30000UL	// no frame for this long = heat pump offline
#define WIFI_RETRY_MS        15000UL
#define MQTT_RETRY_MS        5000UL

comfortzone_heatpump heatpump(new ArduinoRS485Interface(RS485SER, RS485_DE_PIN));
WiFiClient wifi_client;
PubSubClient mqtt(wifi_client);

static COMFORTZONE_STATUS &st = heatpump.comfortzone_status;

// ---------------------------------------------------------------------------------------------------------------
// Entity table
// ---------------------------------------------------------------------------------------------------------------

typedef void (*state_fn)(char *buf, size_t len);
typedef bool (*cmd_fn)(const char *payload);

struct Entity
{
	const char *component;		// sensor, binary_sensor, switch, number, select, button
	const char *id;				// object id, also the last topic segment
	const char *name;
	const char *unit;				// or NULL
	const char *device_class;	// or NULL
	const char *state_class;	// or NULL
	const char *icon;				// or NULL
	const char *extra;			// extra JSON fields for the discovery payload, or NULL
	const char *category;		// entity category: NULL, "diagnostic" or "config"
	bool slow;						// publish only on the periodic full refresh (values that change constantly)
	state_fn state;				// or NULL (buttons)
	cmd_fn command;				// or NULL (read only)
	char last[24];					// last published state
};

static void fmt_temp(char *b, size_t n, int16_t v) { snprintf(b, n, "%.1f", v / 10.0); }
static void fmt_int(char *b, size_t n, long v) { snprintf(b, n, "%ld", v); }
static void fmt_bool(char *b, size_t n, bool v) { strncpy(b, v ? "ON" : "OFF", n); }

static char result_text[COMFORTZONE_HEATPUMP_LAST_MESSAGE_BUFFER_SIZE + 48];

static bool cmd_bool(const char *p, bool (comfortzone_heatpump::*setter)(bool, int))
{
	bool on = (strcasecmp(p, "ON") == 0) || (strcmp(p, "1") == 0) || (strcasecmp(p, "true") == 0);
	return (heatpump.*setter)(on, 5);
}

static Entity entities[] =
{
	// --- sensors: temperatures
	{ "sensor", "te0_outdoor_temp",   "Outdoor temperature",      "°C", "temperature", "measurement", NULL, NULL, NULL, false, [](char *b, size_t n){ fmt_temp(b, n, st.sensors_te0_outdoor_temp); }, NULL, "" },
	{ "sensor", "te1_flow_water",     "Flow water temperature",   "°C", "temperature", "measurement", NULL, NULL, NULL, false, [](char *b, size_t n){ fmt_temp(b, n, st.sensors_te1_flow_water); }, NULL, "" },
	{ "sensor", "te2_return_water",   "Return water temperature", "°C", "temperature", "measurement", NULL, NULL, NULL, false, [](char *b, size_t n){ fmt_temp(b, n, st.sensors_te2_return_water); }, NULL, "" },
	{ "sensor", "te3_indoor_temp",    "Indoor temperature",       "°C", "temperature", "measurement", NULL, NULL, NULL, false, [](char *b, size_t n){ fmt_temp(b, n, st.sensors_te3_indoor_temp); }, NULL, "" },
	{ "sensor", "te4_hot_gas_temp",   "Hot gas temperature",      "°C", "temperature", "measurement", NULL, NULL, NULL, false, [](char *b, size_t n){ fmt_temp(b, n, st.sensors_te4_hot_gas_temp); }, NULL, "" },
	{ "sensor", "te5_exchanger_out",  "Exchanger out temperature","°C", "temperature", "measurement", NULL, NULL, NULL, false, [](char *b, size_t n){ fmt_temp(b, n, st.sensors_te5_exchanger_out); }, NULL, "" },
	{ "sensor", "te7_exhaust_air",    "Exhaust air temperature",  "°C", "temperature", "measurement", NULL, NULL, NULL, false, [](char *b, size_t n){ fmt_temp(b, n, st.sensors_te7_exhaust_air); }, NULL, "" },
	{ "sensor", "te24_hot_water_temp","Hot water temperature",    "°C", "temperature", "measurement", NULL, NULL, NULL, false, [](char *b, size_t n){ fmt_temp(b, n, st.sensors_te24_hot_water_temp); }, NULL, "" },

	// --- sensors: compressor, power, energy
	{ "sensor", "compressor_frequency",      "Compressor frequency",      "Hz",  "frequency", "measurement", "mdi:sine-wave", NULL, NULL, false, [](char *b, size_t n){ fmt_temp(b, n, st.heatpump_current_compressor_frequency); }, NULL, "" },
	{ "sensor", "compressor_power",          "Compressor heat output",    "W",   "power", "measurement", NULL, NULL, NULL, false, [](char *b, size_t n){ fmt_int(b, n, st.heatpump_current_compressor_power); }, NULL, "" },
	{ "sensor", "additional_power",          "Additional heater power",   "W",   "power", "measurement", NULL, NULL, NULL, false, [](char *b, size_t n){ fmt_int(b, n, st.heatpump_current_add_power); }, NULL, "" },
	{ "sensor", "total_power",               "Total heat output",         "W",   "power", "measurement", NULL, NULL, NULL, false, [](char *b, size_t n){ fmt_int(b, n, st.heatpump_current_total_power); }, NULL, "" },
	{ "sensor", "compressor_input_power",    "Compressor input power",    "W",   "power", "measurement", NULL, NULL, NULL, false, [](char *b, size_t n){ fmt_int(b, n, st.heatpump_current_compressor_input_power); }, NULL, "" },
	{ "sensor", "compressor_energy",         "Compressor energy",         "kWh", "energy", "total_increasing", NULL, NULL, NULL, false, [](char *b, size_t n){ snprintf(b, n, "%.2f", st.compressor_energy / 100.0); }, NULL, "" },
	{ "sensor", "additional_energy",         "Additional heater energy",  "kWh", "energy", "total_increasing", NULL, NULL, NULL, false, [](char *b, size_t n){ snprintf(b, n, "%.2f", st.add_energy / 100.0); }, NULL, "" },
	{ "sensor", "hot_water_energy",          "Hot water energy",          "kWh", "energy", "total_increasing", NULL, NULL, NULL, false, [](char *b, size_t n){ snprintf(b, n, "%.2f", st.hot_water_energy / 100.0); }, NULL, "" },
	{ "sensor", "compressor_runtime",        "Compressor runtime",        "h",   "duration", "total_increasing", NULL, NULL, "diagnostic", false, [](char *b, size_t n){ snprintf(b, n, "%.1f", st.compressor_runtime / 60.0); }, NULL, "" },
	{ "sensor", "total_runtime",             "Total runtime",             "h",   "duration", "total_increasing", NULL, NULL, "diagnostic", false, [](char *b, size_t n){ snprintf(b, n, "%.1f", st.total_runtime / 60.0); }, NULL, "" },
	{ "sensor", "filter_days_left",          "Filter days left",          "d",   NULL, "measurement", "mdi:air-filter", NULL, NULL, false, [](char *b, size_t n){ fmt_int(b, n, st.fan_time_to_filter_change); }, NULL, "" },
	{ "sensor", "room_heating_setting",      "Heating calculated setting","°C",  "temperature", "measurement", NULL, NULL, NULL, false, [](char *b, size_t n){ fmt_temp(b, n, st.room_heating_setting); }, NULL, "" },

	// --- binary sensors
	{ "binary_sensor", "compressor_running", "Compressor",           NULL, "running", NULL, NULL, NULL, NULL, false, [](char *b, size_t n){ fmt_bool(b, n, st.compressor_activity == CZCMP_RUNNING); }, NULL, "" },
	{ "binary_sensor", "additional_heater",  "Additional heater",    NULL, "heat",    NULL, NULL, NULL, NULL, false, [](char *b, size_t n){ fmt_bool(b, n, st.additional_power_enabled); }, NULL, "" },
	{ "binary_sensor", "filter_alarm",       "Filter alarm",         NULL, "problem", NULL, NULL, NULL, NULL, false, [](char *b, size_t n){ fmt_bool(b, n, st.filter_alarm); }, NULL, "" },
	{ "binary_sensor", "general_alarm",      "Alarm",                NULL, "problem", NULL, NULL, NULL, NULL, false, [](char *b, size_t n){ fmt_bool(b, n, st.general_alarm); }, NULL, "" },

	// --- switches
	{ "switch", "extra_hot_water", "Extra hot water",            NULL, NULL, NULL, "mdi:water-boiler", NULL, NULL, false, [](char *b, size_t n){ fmt_bool(b, n, st.extra_hot_water_setting); }, [](const char *p){ return cmd_bool(p, &comfortzone_heatpump::set_extra_hot_water); }, "" },
	{ "switch", "cooling_mode",    "Cooling mode",               NULL, NULL, NULL, "mdi:snowflake", NULL, NULL, false, [](char *b, size_t n){ fmt_bool(b, n, st.cooling_enabled); }, [](const char *p){ return cmd_bool(p, &comfortzone_heatpump::set_cooling_mode); }, "" },
	{ "switch", "fireplace_mode",  "Fireplace mode",             NULL, NULL, NULL, "mdi:fireplace", NULL, NULL, false, [](char *b, size_t n){ fmt_bool(b, n, st.fireplace_mode_enabled); }, [](const char *p){ return cmd_bool(p, &comfortzone_heatpump::set_fireplace_mode); }, "" },
	{ "switch", "auto_daylight_saving", "Automatic daylight saving", NULL, NULL, NULL, "mdi:clock-outline", NULL, "config", false, [](char *b, size_t n){ fmt_bool(b, n, st.automatic_daylight_saving); }, [](const char *p){ return cmd_bool(p, &comfortzone_heatpump::set_automatic_daylight_saving); }, "" },

	// --- numbers
	{ "number", "hot_water_setpoint", "Hot water setpoint", "°C", "temperature", NULL, NULL, "\"min\":10,\"max\":60,\"step\":0.5,\"mode\":\"box\"", NULL, false,
		[](char *b, size_t n){ fmt_temp(b, n, st.hot_water_setting); },
		[](const char *p){ return heatpump.set_hot_water_temperature(strtof(p, NULL), 5); }, "" },
	{ "number", "heating_curve", "Heating curve", NULL, NULL, NULL, "mdi:chart-line", "\"min\":0,\"max\":25.5,\"step\":0.1,\"mode\":\"box\"", NULL, false,
		[](char *b, size_t n){ snprintf(b, n, "%.1f", st.heating_curve / 10.0); },
		[](const char *p){ return heatpump.set_heating_curve(strtof(p, NULL), 5); }, "" },
	{ "number", "filter_interval", "Filter interval", "d", NULL, NULL, "mdi:air-filter", "\"min\":1,\"max\":365,\"step\":1,\"mode\":\"box\"", "config", false,
		[](char *b, size_t n){ fmt_int(b, n, st.filter_interval); },
		[](const char *p){ return heatpump.set_filter_interval((uint16_t)atoi(p), 5); }, "" },

	// --- select
	{ "select", "hot_water_priority", "Hot water priority", NULL, NULL, NULL, "mdi:priority-high", "\"options\":[\"low\",\"normal\",\"high\"]", NULL, false,
		[](char *b, size_t n){ const char *s = st.hot_water_priority_setting == 1 ? "low" : st.hot_water_priority_setting == 3 ? "high" : "normal"; strncpy(b, s, n); },
		[](const char *p){ uint8_t v = strcasecmp(p, "low") == 0 ? 1 : strcasecmp(p, "high") == 0 ? 3 : 2; return heatpump.set_hot_water_priority(v, 5); }, "" },

	// --- buttons
	{ "button", "acknowledge_alarms", "Acknowledge alarms", NULL, NULL, NULL, "mdi:bell-check", NULL, NULL, false, NULL,
		[](const char *p){ uint16_t m = st.alarm_pending_mask; if(m == 0) { strncpy(heatpump.last_message, "no pending alarm", sizeof(heatpump.last_message)); return false; } return heatpump.acknowledge_alarm(m, 5); }, "" },
	{ "button", "reset_filter", "Filter replaced", NULL, NULL, NULL, "mdi:air-filter", NULL, NULL, false, NULL,
		[](const char *p){ uint16_t d = st.filter_interval; if(d == 0) { strncpy(heatpump.last_message, "filter interval not received yet", sizeof(heatpump.last_message)); return false; } return heatpump.set_filter_days_left(d, 5); }, "" },

	// --- diagnostics
	{ "sensor", "bus_frame_age", "Bus frame age", "s", NULL, "measurement", "mdi:lan-connect", NULL, "diagnostic", false, [](char *b, size_t n){ fmt_int(b, n, heatpump.last_frame_age() / 1000); }, NULL, "" },
	{ "sensor", "wifi_rssi",     "WiFi signal",   "dBm", "signal_strength", "measurement", NULL, NULL, "diagnostic", true, [](char *b, size_t n){ fmt_int(b, n, WiFi.RSSI()); }, NULL, "" },
};

#define N_ENTITIES (sizeof(entities) / sizeof(entities[0]))

// ---------------------------------------------------------------------------------------------------------------
// MQTT helpers
// ---------------------------------------------------------------------------------------------------------------

static char topic_buf[96];
static char payload_buf[640];

static const char *topic(const char *kind, const char *id)
{
	snprintf(topic_buf, sizeof(topic_buf), "%s/%s/%s", MQTT_PREFIX, kind, id);
	return topic_buf;
}

static void publish_discovery(Entity &e)
{
	char cfg_topic[96];
	int n;

	snprintf(cfg_topic, sizeof(cfg_topic), "homeassistant/%s/%s/%s/config", e.component, DEVICE_ID, e.id);

	n = snprintf(payload_buf, sizeof(payload_buf),
		"{\"name\":\"%s\",\"uniq_id\":\"%s_%s\",\"avty_t\":\"%s/availability\"",
		e.name, DEVICE_ID, e.id, MQTT_PREFIX);

	if(e.state)
		n += snprintf(payload_buf + n, sizeof(payload_buf) - n, ",\"stat_t\":\"%s/state/%s\"", MQTT_PREFIX, e.id);
	if(e.command)
		n += snprintf(payload_buf + n, sizeof(payload_buf) - n, ",\"cmd_t\":\"%s/set/%s\"", MQTT_PREFIX, e.id);
	if(e.unit)
		n += snprintf(payload_buf + n, sizeof(payload_buf) - n, ",\"unit_of_meas\":\"%s\"", e.unit);
	if(e.device_class)
		n += snprintf(payload_buf + n, sizeof(payload_buf) - n, ",\"dev_cla\":\"%s\"", e.device_class);
	if(e.state_class)
		n += snprintf(payload_buf + n, sizeof(payload_buf) - n, ",\"stat_cla\":\"%s\"", e.state_class);
	if(e.icon)
		n += snprintf(payload_buf + n, sizeof(payload_buf) - n, ",\"icon\":\"%s\"", e.icon);
	if(e.category)
		n += snprintf(payload_buf + n, sizeof(payload_buf) - n, ",\"ent_cat\":\"%s\"", e.category);
	if(e.extra)
		n += snprintf(payload_buf + n, sizeof(payload_buf) - n, ",%s", e.extra);

	if((strcmp(e.component, "switch") == 0) || (strcmp(e.component, "binary_sensor") == 0))
		n += snprintf(payload_buf + n, sizeof(payload_buf) - n, ",\"pl_on\":\"ON\",\"pl_off\":\"OFF\"");
	if(strcmp(e.component, "button") == 0)
		n += snprintf(payload_buf + n, sizeof(payload_buf) - n, ",\"pl_prs\":\"PRESS\"");

	n += snprintf(payload_buf + n, sizeof(payload_buf) - n,
		",\"dev\":{\"ids\":[\"%s\"],\"name\":\"%s\",\"mf\":\"Inventum\",\"mdl\":\"Air Blue (panel protocol 2.50)\",\"sw\":\"heatpump_mqtt %s\"}}",
		DEVICE_ID, DEVICE_NAME, SKETCH_VERSION);

	mqtt.publish(cfg_topic, payload_buf, true);
}

static void publish_state(Entity &e, bool force)
{
	char buf[24];

	if(!e.state)
		return;

	if(e.slow && !force)
		return;

	e.state(buf, sizeof(buf));

	if(!force && (strcmp(buf, e.last) == 0))
		return;

	if(mqtt.publish(topic("state", e.id), buf, true))
		strncpy(e.last, buf, sizeof(e.last));
}

static void publish_result(const char *id, bool ok)
{
	snprintf(result_text, sizeof(result_text), "%s: %s%s%s", id, ok ? "OK" : "FAILED", heatpump.last_message[0] ? " " : "", heatpump.last_message);
	mqtt.publish(topic("result", "last"), result_text, false);
	Serial.println(result_text);
}

static void on_message(char *t, byte *payload, unsigned int length)
{
	char p[32];
	const char *set_prefix = MQTT_PREFIX "/set/";

	if(strncmp(t, set_prefix, strlen(set_prefix)) != 0)
		return;

	if(length >= sizeof(p))
		length = sizeof(p) - 1;
	memcpy(p, payload, length);
	p[length] = '\0';

	const char *id = t + strlen(set_prefix);

	for(size_t i = 0; i < N_ENTITIES; i++)
	{
		Entity &e = entities[i];

		if(!e.command || strcmp(e.id, id) != 0)
			continue;

		Serial.print("command "); Serial.print(id); Serial.print(" = "); Serial.println(p);

		bool ok = e.command(p);		// blocks for up to 5 s while waiting for a bus slot and the reply
		publish_result(id, ok);
		publish_state(e, true);		// setters update the status cache on success
		return;
	}
}

static bool bus_online = false;

static void set_availability(bool online, bool force = false)
{
	if((online == bus_online) && !force)
		return;
	bus_online = online;
	mqtt.publish(MQTT_PREFIX "/availability", online ? "online" : "offline", true);
	Serial.println(online ? "heat pump online" : "heat pump offline (no frames)");
}

static bool mqtt_connect()
{
	Serial.print("MQTT connecting to " MQTT_HOST " ... ");

	if(!mqtt.connect(DEVICE_ID, MQTT_USER, MQTT_PASSWORD, MQTT_PREFIX "/availability", 0, true, "offline"))
	{
		Serial.print("failed, rc="); Serial.println(mqtt.state());
		return false;
	}

	Serial.println("connected");

	mqtt.subscribe(topic("set", "+"));

	for(size_t i = 0; i < N_ENTITIES; i++)
	{
		publish_discovery(entities[i]);
		entities[i].last[0] = '\0';		// force a state publish
		mqtt.loop();
	}

	set_availability(heatpump.last_frame_age() < BUS_DEAD_MS, true);
	return true;
}

// ---------------------------------------------------------------------------------------------------------------

void setup()
{
	// receiver mode before anything else: the DE pin has a pull-up during reset and would jam the bus
	pinMode(RS485_DE_PIN, OUTPUT);
	digitalWrite(RS485_DE_PIN, LOW);

	Serial.begin(115200);
	delay(1000);
	Serial.println("heatpump_mqtt " SKETCH_VERSION);

	RS485SER.setRxBufferSize(8192);		// ~4 s of bus traffic, covers a blocking network call (MQTT socket timeout is 3 s)
	RS485SER.begin(19200, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
	heatpump.begin();
	heatpump.enable_debug_mode(true);	// setters fill last_message with a trace, published in the command result

	WiFi.setHostname(DEVICE_ID);			// before mode() on the ESP32 core
	WiFi.mode(WIFI_STA);
	WiFi.setAutoReconnect(true);
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

	mqtt.setServer(MQTT_HOST, MQTT_PORT);
	mqtt.setCallback(on_message);
	mqtt.setBufferSize(1024);				// discovery payloads are ~500 bytes
	mqtt.setKeepAlive(60);					// a command can block the loop for 5 s
	mqtt.setSocketTimeout(3);

	if(strlen(OTA_PASSWORD) > 0)
	{
		ArduinoOTA.setHostname(DEVICE_ID);
		ArduinoOTA.setPassword(OTA_PASSWORD);
		ArduinoOTA.begin();
	}
}

void loop()
{
	static unsigned long last_wifi_try = 0, last_mqtt_try = 0, last_full = 0, last_publish = 0;
	static bool wifi_was_up = false;
	unsigned long now = millis();

	heatpump.process();

	// --- WiFi
	bool wifi_up = (WiFi.status() == WL_CONNECTED);

	if(wifi_up != wifi_was_up)
	{
		wifi_was_up = wifi_up;
		if(wifi_up) { Serial.print("WiFi up, IP "); Serial.println(WiFi.localIP()); }
		else Serial.println("WiFi down");
	}

	if(!wifi_up)
	{
		if(now - last_wifi_try > WIFI_RETRY_MS)
		{
			last_wifi_try = now;
			WiFi.reconnect();
		}
		return;
	}

	if(strlen(OTA_PASSWORD) > 0)
		ArduinoOTA.handle();

	// --- MQTT
	if(!mqtt.connected())
	{
		if(now - last_mqtt_try > MQTT_RETRY_MS)
		{
			last_mqtt_try = now;
			if(mqtt_connect())
				last_full = 0;
		}
		return;
	}

	mqtt.loop();

	// --- availability follows the bus
	set_availability(heatpump.last_frame_age() < BUS_DEAD_MS);

	// --- states: on change every second, everything every FULL_REFRESH_MS
	if(now - last_publish >= 1000)
	{
		last_publish = now;
		bool force = (now - last_full >= FULL_REFRESH_MS);
		if(force)
			last_full = now;

		for(size_t i = 0; i < N_ENTITIES; i++)
		{
			publish_state(entities[i], force);
			heatpump.process();		// keep the bus serviced between publishes
		}
	}
}
