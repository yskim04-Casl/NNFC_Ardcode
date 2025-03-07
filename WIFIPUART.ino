#include "functions.h"

WiFiManager wifiManager(ssid, pass, host, port);
WiFiClientSecure wifiClient;
HttpClient client = HttpClient(wifiClient, host, port);

int i = 0;
int j = 0;
int SEND_NUM = 0;

void setup() {
  Serial.begin(115200);
  // Wi-Fi 연결
  wifiManager.begin();
  wifiClient.setInsecure(); // DEMO

  Serial1.begin(230400, SERIAL_8N1, RX_PIN, TX_PIN);
  delay(10);
  sendCAMeasurementScript();
}

void loop() {
  if (!IsParsed)
  {
    while (Serial1.available() > 0) {
      char c = Serial1.read();
      serial1Buffer += c;
      if (c == 'P') j++;
      else if (c == '\n'){
        oneLine = serial1Buffer;
        parsed_struct = parseOneLine(oneLine);
        parsedBuffer.TIME += (parsed_struct.TIME);
        parsedBuffer.TIME += "\n";
        parsedBuffer.VOLTAGE += (parsed_struct.VOLTAGE);
        parsedBuffer.VOLTAGE += "\n";
        parsedBuffer.CURRENT += (parsed_struct.CURRENT);
        parsedBuffer.CURRENT += "\n";
        serial1Buffer = "";
      }
      else if (c == '*') i++;
    }
    if (i == 1) {
      serial1Buffer.remove(0, 8);
      Serial.print(parsedBuffer.VOLTAGE);
      Serial.print("[!] 데이터 개수: ");
      Serial.print(j);
      i = 0;
      IsParsed = true;
    }
  } 
  else if (IsParsed && SEND_NUM < (j+4) ) // 파싱 작업이 완료되었을 때, 진행하도록 함
  {
    wifiManager.checkConnection(); // Wi-Fi 연결체크

    if(parsedBuffer.VOLTAGE.length() > 0 && parsedBuffer.CURRENT.length() > 0 && parsedBuffer.TIME.length() > 0)
    {
      float A;
      A = parse_value(parsedBuffer.VOLTAGE);
      parsedBuffer.VOLTAGE = remove_parse_value(parsedBuffer.VOLTAGE);
      float B;
      B = parse_value(parsedBuffer.CURRENT);
      parsedBuffer.CURRENT = remove_parse_value(parsedBuffer.CURRENT);
      float C;
      C = parse_value(parsedBuffer.TIME);
      parsedBuffer.TIME = remove_parse_value(parsedBuffer.TIME);
      // JSON 데이터 생성
      jsonBody = wifiManager.generate_jsonBody(A,B,C); // 데이터 json으로 포장하기
      String deb = "[DEBUG] " + (String)A + ";" + (String)B + ";" + (String)C;
      Serial.println(deb);
    }
    bool connected = wifiClient.connect(host, port); // 서버와 연결하기

    if (!connected) {
      Serial.println("[!] SSL 연결 실패"); // 연결 실패 시
      return;
    }
    String request = wifiManager.requestbody_json(jsonBody); // 데이터 POST 요청 준비 (양식 준비)
    wifiClient.write((const uint8_t*)request.c_str(), request.length()); // POST 요청 시작
    // 서버 응답 확인
    while (!wifiClient.available()) {
      delay(10);
    }
    wifiClient.stop();
    client.stop();

    SEND_NUM += 1;
  }
}
