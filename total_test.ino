#include "functions.h"

WiFiManager wifiManager(ssid, pass, host, port);
WiFiClientSecure wifiClient;
HttpClient client = HttpClient(wifiClient, host, port);

String serial1Buffer;   // Serial1 수신 누적 버퍼

void setup() {
  Serial.begin(115200);
  Serial1.begin(230400, SERIAL_8N1, RX_PIN, TX_PIN);

  wifiManager.begin(); // Wi-Fi 연결 시작
  wifiClient.setInsecure();  // DEMO

  // 최초 측정 스크립트 전송 (한 번만)
  if (!measurementStarted) {
    sendEISMeasurementScript();
    measurementStarted = true;
  }
}

String accumulatedData = "";
unsigned long lastDataTime = 0;
const unsigned long DATA_TIMEOUT = 110000; // 110초

void loop() {
  unsigned long currentTime = millis();

  // 일정 시간 동안 새 데이터가 없으면 누적 버퍼 처리
  if (accumulatedData.length() > 0 && (currentTime - lastDataTime > DATA_TIMEOUT)) {
    Serial.println("[INFO] 데이터 타임아웃. 누적된 데이터 처리 중...");
    // 누적된 데이터에서 변수 찾기 시도
    parseSubPackages(accumulatedData);
    accumulatedData = "";
        
    // 완전한 데이터 세트가 있는지 확인
    if (isCompleteDataSet()) {
      // 데이터 전송 로직...
      resetDataFlags();
    }
  }
  // Wi‑Fi 연결 상태 확인
  wifiManager.checkConnection();

  while (Serial1.available() > 0) {
    char c = Serial1.read();
    serial1Buffer += c;

    // 개행 문자('\n')가 들어오면 한 줄 완성 – 파싱 시작
    if (c == '\n') {
      parseMethodScriptLine(serial1Buffer);
      serial1Buffer = "";

      // 새로운 측정값이 있을 때만 데이터 전송
      if (isCompleteDataSet()) {
        // JSON 데이터 생성
        String jsonBody = wifiManager.generate_jsonBody((int)measuredFreq, (int)measuredZr, (int)measuredZi);
        Serial.print("JSON Body: ");
        Serial.println(jsonBody);
                
        bool connected = wifiClient.connect(host, port);
        if (!connected) {
          Serial.println("[!] SSL 연결 실패!");
          continue;
        } else {
          String request = wifiManager.requestbody_json(jsonBody);
          wifiClient.write((const uint8_t*)request.c_str(), request.length());
          Serial.println("json 파일 전송됨");
        }
        wifiClient.flush();
        //디버그 필요
        delay(100); // 딜레이 도중 EmStat Pico 모듈에게서 데이터를 받으면 버퍼에 들어가지 않고 유실되는 문제 추정됨.
        // request 요청 후 딜레이가 없다면 response를 받지 못함 => 해결 필요
        // 데이터 잘림 -> 딜레이 이후 버퍼에 데이터가 들어왔을 것
        //
        while (wifiClient.available()) {
          char ch = wifiClient.read();
          res += ch;
        }
        Serial.println(res);
        wifiClient.stop();

        // 데이터 플래그 초기화
        resetDataFlags();
        /////////////////////////////////////////////////////////////
        //delay(1500); // 1.5초 대기 ++ 디버그 필요 +++++++++++++++++++++
        /////////////////////////////////////////////////////////////
        }
      }
    }

    // 측정이 완료되면 (measurementDone == true) 서버에 데이터 전송
    if (measurementDone) {
        // 데이터 전송 후, 다음 측정을 위해 상태 초기화 (필요에 따라 스크립트 재전송)
        measurementDone = false;
        measurementStarted = false;
        // 새 측정을 원한다면 아래와 같이 스크립트를 다시 전송할 수 있음:
        // sendEISMeasurementScript();
        // measurementStarted = true;
    }
}