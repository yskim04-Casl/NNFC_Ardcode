#include "functions.h"

// Wi-Fi 정보
const char* ssid = "Ardtest";
const char* pass = "39819477";
const char* host = "testweb1-9gn3.onrender.com";
const int port = 443;  

const int RX_PIN = 6;   // EmStat Pico RX (TX -> 6)
const int TX_PIN = 5;   // EmStat Pico TX (RX -> 5)

bool IsParsed = false;

String serial1Buffer = "";
String oneLine = "";
String jsonBody = "";