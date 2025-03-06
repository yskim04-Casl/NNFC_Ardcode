/******************************************************************************
 *  Arduino 코드: MethodSCRIPT 패킷 파싱 예시
 ******************************************************************************/
const int RX_PIN = 6;   // EmStat Pico RX (TX -> 6)
const int TX_PIN = 5;   // EmStat Pico TX (RX -> 5)

String serial1Buffer = "";   // Serial1로부터 수신된 문자열 누적 버퍼
String oneLine = "";

struct ParsedBuffer {
  String TIME;
  String VOLTAGE;
  String CURRENT;
}; 
ParsedBuffer parsedBuffer = {"", "", ""};

int i = 0;
int j = 0;

struct MPACK {
  float TIME = 0.0;
  float VOLTAGE = 0.0;
  float CURRENT = 0.0;
};
MPACK parsed_struct;

void setup() {
  Serial.begin(115200);
  Serial1.begin(230400, SERIAL_8N1, RX_PIN, TX_PIN);
  sendCAMeasurementScript();
}

void loop() {
  // 2) Serial1로부터 들어오는 데이터를 누적
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
    Serial.print(parsedBuffer.TIME);
    Serial.print("[!] 데이터 개수: ");
    Serial.print(j);
    i = 2;
  }
}

MPACK parseOneLine(String line)
{
  MPACK parseData;
  if (line.substring(0, 1) != "P"){
    parseData.TIME = 0.0;
    parseData.VOLTAGE = 0.0;
    parseData.CURRENT = 0.0;
    return parseData;
  }
  line.trim();
  line.remove(0,3);
  String time_val = line.substring(0,7); line.remove(0,7);
  String time_val_prefix = line.substring(0,1); line.remove(0,1);
  parseData.TIME = parseMethodScriptVariable(time_val, time_val_prefix, 0); // TIME
  line.remove(0,3);
  String vol_val = line.substring(0,7); line.remove(0,7);
  String vol_val_prefix = line.substring(0,1);  line.remove(0,1);
  parseData.VOLTAGE = parseMethodScriptVariable(vol_val, vol_val_prefix, 1); // VOLTAGE
  line.remove(0,3);
  String current_val = line.substring(0,7);
  String current_val_prefix = line.substring(7,8);
  parseData.CURRENT = parseMethodScriptVariable(current_val, current_val_prefix, 2); // CURRENT
  return parseData;
}

/******************************************************************************
 *  EIS 측정 스크립트를 전송하는 함수
 ******************************************************************************/
void sendCAMeasurementScript() {
  Serial1.print("e\n");

  // 변수 선언
  Serial1.print("var c\n");  // 전류 데이터
  Serial1.print("var p\n");  // 전압 데이터
  Serial1.print("var t\n");  // 시간 데이터

  // 채널 설정 및 모드 변경
  Serial1.print("set_pgstat_chan 1\n");
  Serial1.print("set_pgstat_mode 0\n");

  Serial1.print("set_pgstat_chan 0\n");
  Serial1.print("set_pgstat_mode 2\n");

  // 대역폭 설정
  Serial1.print("set_max_bandwidth 40\n");

  // 전압 및 전류 범위 설정
  Serial1.print("set_range_minmax da -1 1\n");

  // 전류 범위 설정
  Serial1.print("set_range ba 1m\n");
  Serial1.print("set_autoranging ba 100p 1m\n");

  // 전압 범위 설정
  Serial1.print("set_e 500m\n"); // 0.5V

  // 측정을 위해 셀 켜기
  Serial1.print("cell_on\n");
  
  // 0초로 잡고 시작
  Serial1.print("store_var t 0 eb\n");

  // CA 측정 실행
  Serial1.print("meas_loop_ca p c 500m 50m 10000m\n");  //0.5V, 50ms, 10초

  // 패킷 전송
  Serial1.print("pck_start\n");
  Serial1.print("pck_add t\n");
  Serial1.print("pck_add p\n");
  Serial1.print("pck_add c\n");
  Serial1.print("pck_end\n");

  Serial1.print("add_var t 50m\n"); // 0.05초 간격임을 표시

  Serial1.print("endloop\n");

  // on_finished
  Serial1.print("on_finished:\n");
  Serial1.print("cell_off\n\n");
}


/******************************************************************************
 *  28bit Hex + SI Prefix 파싱
 *  1) Hex string -> 32bit 정수
 *  2) 정수 - 2^27(0x8000000)
 *  3) SI Prefix로 스케일 조정
 ******************************************************************************/
float parseMethodScriptVariable(const String& hexStr, const String& siPrefix, const int TYPE) {
  // 1) Hex -> uint32
  unsigned long rawVal = strtoul(hexStr.c_str(), NULL, 16);

  // 2) offset 보정: val = rawVal - 0x8000000
  //    MethodSCRIPT는 28bit signed offset
  const unsigned long OFFSET = 0x8000000UL; // 2^27
  long signedVal = (long)(rawVal) - (long)(OFFSET);

  // 3) SI prefix 해석
  float factor = 1.0;
  if      (siPrefix == "m") factor = 1e-3;
  else if (siPrefix == "u") factor = 1e-6;
  else if (siPrefix == "n") factor = 1e-9;
  else if (siPrefix == "p") factor = 1e-12;
  else if (siPrefix == "f") factor = 1e-15;
  else if (siPrefix == "k") factor = 1e3;
  else if (siPrefix == "M") factor = 1e6;
  else if (siPrefix == "G") factor = 1e9;

  if (TYPE == 2) // TYPE == CURRENT, 1uA ~ 1000uA
  {
    factor = 1e-6;
  }

  // float 변환
  return (float)signedVal * factor;
}
