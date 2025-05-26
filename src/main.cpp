#include <Arduino.h>
#include <Servo.h>

#define CH1_PIN 7    // 조종기 PWM 입력 (전진/후진)
#define ESC_PIN 8    // ESC 제어 핀
Servo esc;

void setup() {
  Serial.begin(9600);
  pinMode(CH1_PIN, INPUT);
  esc.attach(ESC_PIN);
  esc.writeMicroseconds(1500);  // ESC 중립 (정지)
}

void loop() {
  // 조종기 PWM 신호 읽기 (pulseIn 사용, 타임아웃 25ms)
  unsigned long duration = pulseIn(CH1_PIN, HIGH, 25000);
  if (duration == 0) duration = 1500;  // 신호 없으면 중립값

  // PWM 값을 1400~1600 범위로 압축 (속도 제한)
  int mappedThrottle = map(duration, 1000, 2000, 1400, 1600);
  mappedThrottle = constrain(mappedThrottle, 1400, 1600);

  // ESC에 신호 보내기 (microseconds)
  esc.writeMicroseconds(mappedThrottle);

  Serial.print("Raw PWM: ");
  Serial.print(duration);
  Serial.print(" -> Mapped Throttle: ");
  Serial.println(mappedThrottle);

  delay(20);
}
