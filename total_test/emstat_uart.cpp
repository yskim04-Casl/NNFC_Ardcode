#include "functions.h"

void sendEISMeasurementScript() {
    Serial1.print("e\n");
    Serial1.print("var h\n");  // 주파수 (Frequency)
    Serial1.print("var r\n");  // 실수부 (Z-real)
    Serial1.print("var j\n");  // 허수부 (Z-imag)
    Serial1.print("set_pgstat_chan 1\n");
    Serial1.print("set_pgstat_mode 0\n");
    Serial1.print("set_pgstat_chan 0\n");
    Serial1.print("set_pgstat_mode 3\n");
    Serial1.print("set_max_bandwidth 50k\n");
    Serial1.print("set_range_minmax da 0 0\n");
    Serial1.print("set_range ba 2950u\n");
    Serial1.print("set_autoranging ba 1u 5m\n");
    Serial1.print("set_range ab 4200m\n");
    Serial1.print("set_autoranging ab 4200m 4200m\n");
    Serial1.print("set_e 0\n");
    Serial1.print("cell_on\n");
    Serial1.print("meas_loop_eis h r j 10m 50k 5 41 0\n");
    Serial1.print("pck_start\n");
    Serial1.print("pck_add h\n");
    Serial1.print("pck_add r\n");
    Serial1.print("pck_add j\n");
    Serial1.print("pck_end\n");
    Serial1.print("endloop\n");
    Serial1.print("on_finished:\n");
    Serial1.print("cell_off\n\n");
}

void parseMethodScriptLine(String line) {
    line.trim();  // 앞뒤 공백/개행 제거

    debugPrint("라인 처리 중: " + line);

    // 측정 완료(*) 감지
    if (line == "*") {
        measurementDone = true;
        Serial.println("[!] 측정 완료 ('*' 수신).");
        return;
    }

    // 측정 데이터 패킷인 경우 (예: "Pdc8F1432Bu;cc8A12428m,10,289;cd7FAD9EBm,10,203")
    if (line.startsWith("P")) {
        String rawData = line.substring(1);
        rawData.trim();
        parseSubPackages(rawData);
        newMeasurementReceived = true;  // 유효한 측정 데이터가 파싱되었음을 표시
    } 
    // P로 시작하지 않지만 변수 데이터를 포함하는 라인 파싱 시도
    else if (line.indexOf("dc") != -1 || line.indexOf("cc") != -1 || line.indexOf("cd") != -1) {
        parseSubPackages(line);
        newMeasurementReceived = true;
    }
    else {
        Serial.println("[DEBUG] " + line);
    }
}

void parseSubPackage(String sub) {
    sub.trim();
    if (sub.length() < 2) return;
  
    int varTypeStart = sub.indexOf("dc");
    if (varTypeStart == -1) varTypeStart = sub.indexOf("cc");
    if (varTypeStart == -1) varTypeStart = sub.indexOf("cd");
    if (varTypeStart == -1) return; // 인식된 변수 유형이 없음
    
    String varType = sub.substring(varTypeStart, varTypeStart + 2);
    String rest = sub.substring(varTypeStart + 2);
    
    int commaPos = rest.indexOf(',');
    String valuePart;
    String metadata;
    if (commaPos == -1) {
        valuePart = rest;
    } else {
        valuePart = rest.substring(0, commaPos);
        metadata = rest.substring(commaPos + 1);
    }
  
    char prefixChar = valuePart[valuePart.length() - 1];
    String prefixList = "munkfaE ";
    bool hasPrefix = prefixList.indexOf(prefixChar) != -1;
    String hexStr, siPrefix = "";
    if (hasPrefix) {
        siPrefix = String(prefixChar);
        hexStr = valuePart.substring(0, valuePart.length() - 1);
    } else {
        hexStr = valuePart; 
    }
  
    float numericValue = parseMethodScriptVariable(hexStr, siPrefix);
  
    if (varType == "dc") {
        measuredFreq = numericValue;
        freqUpdated = true;  // 업데이트 플래그 설정
        Serial.print("Freq=");
        Serial.print(numericValue);
        Serial.print("Hz  ");
    } else if (varType == "cc") {
        measuredZr = numericValue;
        zrUpdated = true;  // 업데이트 플래그 설정
        Serial.print("Zr=");
        Serial.print(numericValue);
        Serial.print("Ω  ");
    } else if (varType == "cd") {
        measuredZi = numericValue;
        ziUpdated = true;  // 업데이트 플래그 설정
        Serial.print("Zi=");
        Serial.print(numericValue);
        Serial.print("Ω  ");
    } else {
        Serial.print(varType);
        Serial.print("=");
        Serial.print(numericValue);
        Serial.print("  ");
    }
  
    if (metadata.length() > 0) {
        Serial.print("(meta:");
        Serial.print(metadata);
        Serial.print(")  ");
    }
}

void parseSubPackages(const String & rawData) {
    int start = 0;
    while (true) {
      int sepPos = rawData.indexOf(';', start);
      String token;
      if (sepPos == -1) {
        token = rawData.substring(start);
        parseSubPackage(token);
        break;
      } else {
        token = rawData.substring(start, sepPos);
        parseSubPackage(token);
        start = sepPos + 1;
      }
    }
    Serial.println(); // 줄바꿈
}

bool isHexadecimalString(const String & str) {
  if (str.length() == 0) return false;
    
  for (unsigned int i = 0; i < str.length(); i++) {
    char c = str.charAt(i);
    if (!isxdigit(c)) return false;
  }
  return true;
}

float parseMethodScriptVariable(const String & hexStr, const String & siPrefix) {
  // 유효성 검사 추가
  if (!isHexadecimalString(hexStr)) {
    Serial.println("[오류] 유효하지 않은 16진수 문자열: " + hexStr);
    return 0.0; // 오류의 경우 기본값 반환
  }

  unsigned long rawVal = strtoul(hexStr.c_str(), NULL, 16);
  const unsigned long OFFSET = 0x8000000UL;
  long signedVal = (long)rawVal - (long)OFFSET;
  float factor = 1.0;
  if      (siPrefix == "m") factor = 1e-3;
  else if (siPrefix == "u") factor = 1e-6;
  else if (siPrefix == "n") factor = 1e-9;
  else if (siPrefix == "p") factor = 1e-12;
  else if (siPrefix == "f") factor = 1e-15;
  else if (siPrefix == "k") factor = 1e3;
  else if (siPrefix == "M") factor = 1e6;
  else if (siPrefix == "G") factor = 1e9;
  return (float)signedVal * factor;
}

void debugPrint(String message) {
    #if DEBUG_MODE
    Serial.print("[DEBUG] ");
    Serial.println(message);
    #endif
}

bool isCompleteDataSet() {
    return freqUpdated && zrUpdated && ziUpdated;
}

void resetDataFlags() {
    freqUpdated = false;
    zrUpdated = false;
    ziUpdated = false;
    newMeasurementReceived = false;
}