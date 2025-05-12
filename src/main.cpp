#include "PinChangeInterrupt.h"  // PinChangeInterrupt 라이브러리 임포트

#define NEUTRAL_THROTTLE 1500     // 기본 스로틀 값 (중립)
#define pinRC3 A0                 // CH3 입력 (스로틀)
#define pinRC5 A1                 // CH5 입력 (A 스위치)
#define pinRC6 A2                 // CH6 입력 (RGB 스위치 - G로 설정됨)
#define pinLED_PWM 9              // LED 1 (PWM 밝기 조절용)
#define pinLED_ONOFF 6            // LED 2 (ON/OFF 제어용)

// RGB LED 핀 정의
#define pinLED_R 3                // 빨간색 LED 핀
#define pinLED_G 5                // 초록색 LED 핀
#define pinLED_B 10               // 파란색 LED 핀

// 변수 정의
volatile int nRC3PulseWidth = NEUTRAL_THROTTLE;  // CH3 PWM 신호의 길이
volatile int nRC5PulseWidth = 1000;              // CH5 PWM 신호의 길이 (A 스위치)
volatile int nRC6PulseWidth = 1000;              // CH6 PWM 신호의 길이 (RGB 스위치)
volatile unsigned long ulRC3StartHigh = 0;       // CH3 신호 시작 시간
volatile unsigned long ulRC5StartHigh = 0;       // CH5 신호 시작 시간
volatile unsigned long ulRC6StartHigh = 0;       // CH6 신호 시작 시간
volatile boolean bNewRC3Pulse = false;           // CH3 신호 새로 들어옴
volatile boolean bNewRC5Pulse = false;           // CH5 신호 새로 들어옴
volatile boolean bNewRC6Pulse = false;           // CH6 신호 새로 들어옴

int prevRC3PulseWidth = NEUTRAL_THROTTLE;        // 이전 CH3 PWM 값 (변경 여부 체크용)
bool ledEnabled = false;                         // 전체 LED ON/OFF 상태
bool rgbLedEnabled = false;                     // RGB LED ON/OFF 상태

// 함수 선언
void pwmRC3();
void pwmRC5();
void pwmRC6();

void setup() {
  pinMode(pinRC3, INPUT_PULLUP);                 // CH3 핀을 입력으로 설정 (풀업 저항 사용)
  pinMode(pinRC5, INPUT_PULLUP);                 // CH5 핀을 입력으로 설정 (풀업 저항 사용)
  pinMode(pinRC6, INPUT_PULLUP);                 // CH6 핀을 입력으로 설정 (풀업 저항 사용)
  
  // 핀 변화 인터럽트 설정
  attachPCINT(digitalPinToPCINT(pinRC3), pwmRC3, CHANGE);  // CH3에서 변화가 있을 때 pwmRC3 함수 호출
  attachPCINT(digitalPinToPCINT(pinRC5), pwmRC5, CHANGE);  // CH5에서 변화가 있을 때 pwmRC5 함수 호출
  attachPCINT(digitalPinToPCINT(pinRC6), pwmRC6, CHANGE);  // CH6에서 변화가 있을 때 pwmRC6 함수 호출

  pinMode(pinLED_PWM, OUTPUT);                  // PWM LED 핀을 출력으로 설정
  pinMode(pinLED_ONOFF, OUTPUT);                // ON/OFF LED 핀을 출력으로 설정
  
  // RGB LED 핀을 출력으로 설정
  pinMode(pinLED_R, OUTPUT);                    
  pinMode(pinLED_G, OUTPUT);                    
  pinMode(pinLED_B, OUTPUT);                    

  // 초기 상태로 LED 끄기
  analogWrite(pinLED_PWM, 0);                   // PWM LED 끄기
  digitalWrite(pinLED_ONOFF, LOW);              // ON/OFF LED 끄기
  digitalWrite(pinLED_R, LOW);                  // 빨강 LED 끄기
  digitalWrite(pinLED_G, LOW);                  // 초록 LED 끄기
  digitalWrite(pinLED_B, LOW);                  // 파랑 LED 끄기

  Serial.begin(9600);                           // 시리얼 통신 시작
  Serial.println("CH3: Brightness | CH5(A Switch): ON/OFF Control | CH6(RGB Switch): Color Change");
}

void pwmRC3() {
  if (digitalRead(pinRC3) == HIGH) {            // CH3 핀이 HIGH일 경우
    ulRC3StartHigh = micros();                  // 신호의 시작 시간 기록
  } else {                                      // CH3 핀이 LOW일 경우
    if (ulRC3StartHigh && !bNewRC3Pulse) {      // 시작 시간이 기록되고 새로운 펄스가 아닐 경우
      nRC3PulseWidth = (int)(micros() - ulRC3StartHigh); // 펄스 길이 계산
      ulRC3StartHigh = 0;                       // 시작 시간 초기화
      bNewRC3Pulse = true;                      // 새로운 펄스 있음
    }
  }
}

void pwmRC5() {
  if (digitalRead(pinRC5) == HIGH) {            // CH5 핀이 HIGH일 경우
    ulRC5StartHigh = micros();                  // 신호의 시작 시간 기록
  } else {                                      // CH5 핀이 LOW일 경우
    if (ulRC5StartHigh && !bNewRC5Pulse) {      // 시작 시간이 기록되고 새로운 펄스가 아닐 경우
      nRC5PulseWidth = (int)(micros() - ulRC5StartHigh); // 펄스 길이 계산
      ulRC5StartHigh = 0;                       // 시작 시간 초기화
      bNewRC5Pulse = true;                      // 새로운 펄스 있음
    }
  }
}

void pwmRC6() {
  if (digitalRead(pinRC6) == HIGH) {            // CH6 핀이 HIGH일 경우
    ulRC6StartHigh = micros();                  // 신호의 시작 시간 기록
  } else {                                      // CH6 핀이 LOW일 경우
    if (ulRC6StartHigh && !bNewRC6Pulse) {      // 시작 시간이 기록되고 새로운 펄스가 아닐 경우
      nRC6PulseWidth = (int)(micros() - ulRC6StartHigh); // 펄스 길이 계산
      ulRC6StartHigh = 0;                       // 시작 시간 초기화
      bNewRC6Pulse = true;                      // 새로운 펄스 있음
    }
  }
}

void loop() {
  // A 스위치(ON/OFF)
  if (bNewRC5Pulse) {                           // CH5 신호가 새로 들어왔을 때
    if (nRC5PulseWidth > 1500) {                // PWM 값이 1500보다 크면
      ledEnabled = true;                        // 전체 LED ON
      rgbLedEnabled = true;                     // RGB LED도 켬
      digitalWrite(pinLED_ONOFF, HIGH);         // ON/OFF LED 켬
      Serial.println("CH5 A Switch: ON → LEDs ENABLED");
    } else {                                    // PWM 값이 1500 이하이면
      ledEnabled = false;                       // 전체 LED OFF
      rgbLedEnabled = false;                    // RGB LED 끔
      digitalWrite(pinLED_ONOFF, LOW);          // ON/OFF LED 끔
      analogWrite(pinLED_PWM, 0);               // PWM LED 끄기
      digitalWrite(pinLED_R, LOW);              // 빨강 LED 끄기
      digitalWrite(pinLED_G, LOW);              // 초록 LED 끄기
      digitalWrite(pinLED_B, LOW);              // 파랑 LED 끄기
    }
    bNewRC5Pulse = false;                       // CH5 신호 처리 완료
  }

  // RGB LED 스위치 (CH6)
  if (rgbLedEnabled && bNewRC6Pulse) {          // RGB LED가 활성화되고 CH6 신호가 새로 들어왔을 때
    if (nRC6PulseWidth < 1200) {                // PWM 값이 1200 미만이면
      digitalWrite(pinLED_R, HIGH);             // 빨강 LED 켬
      digitalWrite(pinLED_G, LOW);              // 초록 LED 끔
      digitalWrite(pinLED_B, LOW);              // 파랑 LED 끔
      Serial.println("RGB LED: RED");
    } else if (nRC6PulseWidth > 1800) {         // PWM 값이 1800 이상이면
      digitalWrite(pinLED_R, LOW);              // 빨강 LED 끔
      digitalWrite(pinLED_G, LOW);              // 초록 LED 끔
      digitalWrite(pinLED_B, HIGH);             // 파랑 LED 켬
      Serial.println("RGB LED: BLUE");
    } else {                                    // PWM 값이 1200과 1800 사이이면
      digitalWrite(pinLED_R, LOW);              // 빨강 LED 끔
      digitalWrite(pinLED_G, HIGH);             // 초록 LED 켬
      digitalWrite(pinLED_B, LOW);              // 파랑 LED 끔
      Serial.println("RGB LED: GREEN");
    }
    bNewRC6Pulse = false;                       // CH6 신호 처리 완료
  }

  // 스위치 ON 상태일 때만 밝기 반영
  if (ledEnabled && bNewRC3Pulse) {             // LED가 켜져 있고 CH3 신호가 새로 들어왔을 때
    if (abs(prevRC3PulseWidth - nRC3PulseWidth) > 10) {  // 이전과 현재 PWM 값 차이가 10 이상일 때만 처리
      int brightness = map(nRC3PulseWidth, 1000, 2000, 255, 0); // PWM 값을 밝기로 매핑
      brightness = constrain(brightness, 0, 255);              // 밝기 범위 제한

      analogWrite(pinLED_PWM, brightness);      // PWM LED 밝기 조정

      Serial.print("CH3 PWM: ");                // Serial에 CH3 PWM: ____ PWM값 출력
      Serial.print(nRC3PulseWidth);
      Serial.print(" → Brightness: ");          // Serial에 -> Brightness: ____ 밝기값 출력
      Serial.println(brightness);

      prevRC3PulseWidth = nRC3PulseWidth;     // 이전 PWM 값 업데이트
    }
    bNewRC3Pulse = false;                      // CH3 신호 처리 완료
  }
}
