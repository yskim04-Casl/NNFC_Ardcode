#pragma once
#include <stdio.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoHttpClient.h>
#include <Arduino.h>
#include <ArduinoBLE.h> 

extern bool measurementStarted;
extern bool measurementDone;
extern bool newMeasurementReceived;  // 새로 추가

extern float measuredFreq;
extern float measuredZr;
extern float measuredZi;
extern bool freqUpdated;
extern bool zrUpdated;
extern bool ziUpdated;

// HTTP
extern String res;

// Wi‑Fi 정보
extern const char* ssid;
extern const char* pass;
extern const char* host;
extern const int port;

// UART 핀
extern const int RX_PIN;
extern const int TX_PIN;

void sendEISMeasurementScript();
void parseMethodScriptLine(String line);
void parseSubPackage(String sub);
void parseSubPackages(const String & rawData);
float parseMethodScriptVariable(const String & hexStr, const String & siPrefix);
void resetDataFlags();
bool isCompleteDataSet();
bool isHexadecimalString(const String & str);
void debugPrint(String message);

class WiFiManager {
public:
    WiFiManager(const char* ssid, const char* pass, const char* host, const int port);
    void begin();
    void checkConnection();
    String generate_jsonBody(int A, int B, int C);
    String requestbody_json(String jsonbody);
private:
    const char* _ssid;
    const char* _pass;
    const char* _host;
    const int   _port;
};
