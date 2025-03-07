#include "functions.h"

bool measurementStarted = false;
bool measurementDone    = false;
bool newMeasurementReceived = false;  // 새로 추가

bool freqUpdated = false;
bool zrUpdated = false;
bool ziUpdated = false;

// Parsed measurement
float measuredFreq = 0.0;
float measuredZr   = 0.0;
float measuredZi   = 0.0;

// HTTP
String res = "";

// Wi-Fi
const char* ssid = "Ardtest";
const char* pass = "39819477";
const char* host = "testweb1-y5nj.onrender.com";
const int port = 443;

// UART
const int RX_PIN = 6;
const int TX_PIN = 5;