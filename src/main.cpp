#include "PinChangeInterrupt.h" // 핀 상태 변화(상승/하강 엣지)를 감지하기 위한 라이브러리로, 외부 인터럽트가 아닌 핀 변경 인터럽트를 지원
#include <math.h> // 수학 함수(hsv → rgb 변환 등)에 필요한 표준 라이브러리 포함

// 중립 스로틀 값 정의 (RC 수신기의 중간값으로 보통 1500μs)
#define NEUTRAL_THROTTLE 1500

// RC 수신기 채널에 연결된 아날로그 핀 정의
#define pinRC1 A2    // CH1: 조이스틱 수평 방향 → RGB 색상 제어 (Hue 값 변화용)
#define pinRC3 A0    // CH3: 조이스틱 수직 방향 → LED 밝기 조절용
#define pinRC5 A1    // CH5: 스위치 역할 → LED ON/OFF 기능 제어

// 출력 핀 정의
#define pinLED_PWM 9         // 밝기 조절을 위한 일반 LED (PWM 핀)
#define pinLED_ONOFF 6       // LED 시스템 전체 ON/OFF 상태 표시용 디지털 핀

// RGB LED 핀 정의
#define pinLED_R 3           // 빨간색 LED PWM 제어
#define pinLED_G 5           // 초록색 LED PWM 제어
#define pinLED_B 10          // 파란색 LED PWM 제어

// RC 수신기의 각 채널에서 읽은 펄스폭(μs)을 저장할 변수들 (초기값은 중립 상태)
volatile int nRC1PulseWidth = 1500;
volatile int nRC3PulseWidth = NEUTRAL_THROTTLE;
volatile int nRC5PulseWidth = 1000;

// HIGH 신호가 시작된 시간 기록용 변수들
volatile unsigned long ulRC1StartHigh = 0;
volatile unsigned long ulRC3StartHigh = 0;
volatile unsigned long ulRC5StartHigh = 0;

// 새로운 펄스가 들어왔는지 여부를 표시하는 플래그 변수
volatile boolean bNewRC1Pulse = false;
volatile boolean bNewRC3Pulse = false;
volatile boolean bNewRC5Pulse = false;

// 이전 밝기 값을 저장하여 변화량이 작을 경우 출력하지 않도록 함
int prevRC3PulseWidth = NEUTRAL_THROTTLE;

// LED 전체 시스템이 현재 켜져 있는지를 나타내는 플래그
bool ledEnabled = false;

// 각 채널별 인터럽트 핸들러 및 HSV→RGB 변환 함수 선언
void pwmRC1();
void pwmRC3();
void pwmRC5();
void hsvToRgb(float h, float s, float v, int& r, int& g, int& b);

void setup() {
  // RC 채널 입력핀을 풀업 저항을 사용하여 설정
  pinMode(pinRC1, INPUT_PULLUP);
  pinMode(pinRC3, INPUT_PULLUP);
  pinMode(pinRC5, INPUT_PULLUP);

  // 핀 변경 인터럽트를 등록하여 상승/하강 엣지 모두 감지하도록 설정
  attachPCINT(digitalPinToPCINT(pinRC1), pwmRC1, CHANGE);
  attachPCINT(digitalPinToPCINT(pinRC3), pwmRC3, CHANGE);
  attachPCINT(digitalPinToPCINT(pinRC5), pwmRC5, CHANGE);

  // LED 관련 핀을 출력으로 설정
  pinMode(pinLED_PWM, OUTPUT);
  pinMode(pinLED_ONOFF, OUTPUT);
  pinMode(pinLED_R, OUTPUT);
  pinMode(pinLED_G, OUTPUT);
  pinMode(pinLED_B, OUTPUT);

  // 초기값으로 모든 LED 꺼짐 상태로 설정
  analogWrite(pinLED_PWM, 0);          // 밝기 0
  digitalWrite(pinLED_ONOFF, LOW);     // ON/OFF 표시 LED 꺼짐
  digitalWrite(pinLED_R, LOW);
  digitalWrite(pinLED_G, LOW);
  digitalWrite(pinLED_B, LOW);

  // 시리얼 통신 시작 (모니터링용)
  Serial.begin(9600);
  Serial.println("CH1(A2): RGB 색상 | CH3(A0): 밝기 | CH5(A1): LED ON/OFF");
}

void loop() {
  // CH5 스위치 상태가 변경되었을 때 실행
  if (bNewRC5Pulse) {
    if (nRC5PulseWidth > 1500) {  // 스위치가 ON 위치일 경우
      ledEnabled = true;
      digitalWrite(pinLED_ONOFF, HIGH);  // ON/OFF 상태 표시 LED 켜기

      // 밝기 복원: RC3의 현재 값을 사용하여 LED 밝기를 설정
      int brightness = map(nRC3PulseWidth, 1000, 2000, 255, 0);
      brightness = constrain(brightness, 0, 255);
      analogWrite(pinLED_PWM, brightness);

      Serial.println("CH5 A Switch: ON → LEDs ENABLED (Brightness Restored)");
    } else {
      // 스위치가 OFF일 경우: 모든 LED 끄기
      ledEnabled = false;
      digitalWrite(pinLED_ONOFF, LOW);
      analogWrite(pinLED_PWM, 0);
      digitalWrite(pinLED_R, LOW);
      digitalWrite(pinLED_G, LOW);
      digitalWrite(pinLED_B, LOW);
    }
    bNewRC5Pulse = false; // 플래그 초기화
  }

  // CH3 밝기 조정 (조이스틱 위/아래)
  if (ledEnabled && bNewRC3Pulse) {
    if (abs(prevRC3PulseWidth - nRC3PulseWidth) > 10) { // 변화가 일정 수준 이상일 경우만 적용
      int brightness = map(nRC3PulseWidth, 1000, 2000, 255, 0); // RC 값 → 밝기로 매핑
      brightness = constrain(brightness, 0, 255); // 0~255 범위로 제한
      analogWrite(pinLED_PWM, brightness); // 밝기 적용

      Serial.print("CH3 Brightness: ");
      Serial.println(brightness);

      prevRC3PulseWidth = nRC3PulseWidth;
    }
    bNewRC3Pulse = false;
  }

  // CH1 색상 제어 (조이스틱 좌/우)
  if (ledEnabled && bNewRC1Pulse) {
    float hue = map(nRC1PulseWidth, 1000, 2000, 0, 360); // RC 신호 → Hue 값으로 변환
    int r, g, b;
    hsvToRgb(hue, 1.0, 1.0, r, g, b); // HSV → RGB 변환

    // RGB 핀에 색상 적용 (밝기는 항상 100%)
    analogWrite(pinLED_R, r);
    analogWrite(pinLED_G, g);
    analogWrite(pinLED_B, b);

    Serial.print("CH1 Hue: ");
    Serial.print(hue);
    Serial.print(" → RGB(");
    Serial.print(r); Serial.print(", ");
    Serial.print(g); Serial.print(", ");
    Serial.print(b); Serial.println(")");
    
    bNewRC1Pulse = false;
  }
}

// CH1 핀에서 펄스폭 측정
void pwmRC1() {
  if (digitalRead(pinRC1) == HIGH) {
    ulRC1StartHigh = micros(); // 상승 엣지: 시작 시간 기록
  } else {
    if (ulRC1StartHigh && !bNewRC1Pulse) {
      nRC1PulseWidth = (int)(micros() - ulRC1StartHigh); // 펄스폭 계산
      ulRC1StartHigh = 0;
      bNewRC1Pulse = true; // 새로운 값 수신 표시
    }
  }
}

// CH3 핀에서 펄스폭 측정 (밝기 제어용)
void pwmRC3() {
  if (digitalRead(pinRC3) == HIGH) {
    ulRC3StartHigh = micros(); // 상승 엣지: 시작 시간 기록
  } else {
    if (ulRC3StartHigh && !bNewRC3Pulse) {
      nRC3PulseWidth = (int)(micros() - ulRC3StartHigh);
      ulRC3StartHigh = 0;
      bNewRC3Pulse = true;
    }
  }
}

// CH5 핀에서 펄스폭 측정 (ON/OFF 스위치용)
void pwmRC5() {
  if (digitalRead(pinRC5) == HIGH) {
    ulRC5StartHigh = micros(); // 상승 엣지
  } else {
    if (ulRC5StartHigh && !bNewRC5Pulse) {
      nRC5PulseWidth = (int)(micros() - ulRC5StartHigh);
      ulRC5StartHigh = 0;
      bNewRC5Pulse = true;
    }
  }
}

// HSV 색상 모델 값을 RGB 모델 값으로 변환
void hsvToRgb(float h, float s, float v, int& r, int& g, int& b) {
  float c = v * s; // 채도 및 밝기에 따른 색상 강도
  float x = c * (1 - fabs(fmod(h / 60.0, 2) - 1)); // 색상 구간 내 보정 값
  float m = v - c; // 밝기 보정 값
  float r1, g1, b1;

  // HSV 색상 범위에 따라 RGB 비율 설정
  if (h < 60) {
    r1 = c; g1 = x; b1 = 0;
  } else if (h < 120) {
    r1 = x; g1 = c; b1 = 0;
  } else if (h < 180) {
    r1 = 0; g1 = c; b1 = x;
  } else if (h < 240) {
    r1 = 0; g1 = x; b1 = c;
  } else if (h < 300) {
    r1 = x; g1 = 0; b1 = c;
  } else {
    r1 = c; g1 = 0; b1 = x;
  }

  // 0~1 범위의 RGB 값 → 0~255로 변환
  r = (r1 + m) * 255;
  g = (g1 + m) * 255;
  b = (b1 + m) * 255;
}
