#include "functions.h"

MPACK parseOneLine(String line)
{
  MPACK parseData;
  if (line.substring(0, 1) != "P"){
    parseData.TIME = -1.0;
    parseData.VOLTAGE = -1.0;
    parseData.CURRENT = -1.0;
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

  // 대역폭 설정 40Hz
  Serial1.print("set_max_bandwidth ");
  Serial1.print(BANDWIDTH);
  Serial1.print("\n");

  // 전압 및 전류 범위 설정 -1V~1V
  Serial1.print("set_range_minmax da ");
  Serial1.print(M_LOW_VOLTAGE);
  Serial1.print(" ");
  Serial1.print(M_HIGH_VOLTAGE);
  Serial1.print("\n");

  // 전류 범위 설정 1000uA, 0.1uA~1000uA
  Serial1.print("set_range ba ");
  Serial1.print(CURRENT_RANGE);
  Serial1.print("\n");
  Serial1.print("set_autoranging ba ");
  Serial1.print(M_LOW_CURRENT_AUTORANGING);
  Serial1.print(" ");
  Serial1.print(M_HIGH_CURRENT_AUTORANGING);
  Serial1.print("\n");

  // 전압 오프셋 설정
  Serial1.print("set_e "); // 0.5V
  Serial1.print(VOLTAGE_OFFSET);
  Serial1.print("\n");

  // 측정을 위해 셀 켜기
  Serial1.print("cell_on\n");
  
  // 0초로 잡고 시작
  Serial1.print("store_var t 0 eb\n");

  // CA 측정 실행
  Serial1.print("meas_loop_ca p c ");  // 0.5V, 50ms, 10초
  Serial1.print(VOLTAGE_OFFSET);
  Serial1.print(" ");
  Serial1.print(SAMPLING_TIME);
  Serial1.print(" ");
  Serial1.print(MEAS_TIME);
  Serial1.print("\n");

  // 패킷 전송
  Serial1.print("pck_start\n");
  Serial1.print("pck_add t\n");
  Serial1.print("pck_add p\n");
  Serial1.print("pck_add c\n");
  Serial1.print("pck_end\n");

  Serial1.print("add_var t "); // 0.05초 간격임을 표시
  Serial1.print(SAMPLING_TIME);
  Serial1.print("\n");

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
  
float parse_value(String line)
{
  if (line.length() > 0)
  {
    String value = "";
    float val;
    
    int endLineIndex = line.indexOf("\n");
    value = line.substring(0,endLineIndex);
    val = line.toFloat();
    return val;
  }
  else return -32.0;
}

String remove_parse_value(String line)
{
  if (line.length() > 0)
  {
    String value = "";
    
    int endLineIndex = line.indexOf("\n");
    line.remove(0,endLineIndex+1);
    return line;
  }
  else return "";
}