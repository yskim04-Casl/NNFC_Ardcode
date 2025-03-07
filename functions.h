#pragma once
#include <stdio.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoHttpClient.h>  // Arduino HTTP Client
#include <Arduino.h>
#include <ArduinoBLE.h>

float parseMethodScriptVariable(const String& hexStr, const String& siPrefix, const int TYPE);
void sendCAMeasurementScript();
struct MPACK parseOneLine(String line);
float parse_value(String line);
String remove_parse_value(String line);

class WiFiManager {
public:
	WiFiManager(const char* ssid, const char* pass, const char* host, const int port);
	void	begin();
	void	checkConnection();
	String	generate_jsonBody(float A, float B, float C);
	String	requestbody_json(String jsonbody);

private:
	const char* _ssid;
	const char* _pass;
	const char* _host;
	const int	_port;
};

// Wi-Fi 정보
extern const char* ssid;
extern const char* pass;
extern const char* host;
extern const int port;  

extern const int RX_PIN;   // EmStat Pico RX (TX -> 6)
extern const int TX_PIN;   // EmStat Pico TX (RX -> 5)

extern bool IsParsed;

extern String jsonBody;

/////////////////////////////////////////////////////////////////////////////////
// EmStat Pico SETTING VAR                                                     //
/////////////////////////////////////////////////////////////////////////////////
struct ParsedBuffer {
  String TIME;
  String VOLTAGE;
  String CURRENT;
};
extern ParsedBuffer parsedBuffer;

struct MPACK {
  float TIME;
  float VOLTAGE;
  float CURRENT;
};
extern MPACK parsed_struct;

extern String serial1Buffer;
extern String oneLine;

extern String BANDWIDTH;
extern String M_LOW_VOLTAGE;
extern String M_HIGH_VOLTAGE;
extern String M_LOW_CURRENT_AUTORANGING;
extern String M_HIGH_CURRENT_AUTORANGING;
extern String VOLTAGE_OFFSET;
extern String SAMPLING_TIME;
extern String CURRENT_RANGE;
extern String MEAS_TIME;