#include "functions.h"

WiFiManager::WiFiManager(const char* ssid, const char* pass, const char* host, const int port)
	: _ssid(ssid), _pass(pass), _host(host), _port(port) { }

void WiFiManager::begin() {
	Serial.print("Wi-Fi에 연결 중...");
	WiFi.begin(_ssid, _pass);
	delay(10);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("\nWi-Fi 연결 완료!");
}

void WiFiManager::checkConnection() {
	if (WiFi.status() != WL_CONNECTED) {
		Serial.println("Wi-Fi 끊김. 재연결 중...");
		WiFi.begin(_ssid, _pass);
		delay(5000);
		return;
	}
}

String WiFiManager::generate_jsonBody(int A, int B, int C) {
	String jsonBody = "{\"A\":" + String(A) + ",\"B\":" + String(B) + ",\"C\":" + String(C) + "}";
	Serial.print("JSON Body: ");
	Serial.println(jsonBody);
	return jsonBody;
}

String WiFiManager::requestbody_json(String jsonbody) {
	String request = "";
	request += "POST /api/signal HTTP/1.1\r\n";
	request += "Host: " + String(_host) + "\r\n";
	request += "Content-Type: application/json\r\n";
	request += "Accept: application/json\r\n";
	request += "Connection: close\r\n";
	request += "Content-Length: " + String(jsonbody.length()) + "\r\n";
	request += "\r\n";
	request += jsonbody;

	return request;
}