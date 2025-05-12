# 아두이노 RC 스위치 기반 RGB LED 컨트롤러

![프로젝트 이미지](images/project-demo.jpg)  

---

## 🎥 데모 영상

[![시연 영상 보기](https://img.youtube.com/vi/YOUTUBE_VIDEO_ID/0.jpg)](https://www.youtube.com/watch?v=YOUTUBE_VIDEO_ID)  

---

## 📘 프로젝트 개요

이 프로젝트는 RC 수신기(CH3, CH5, CH6)로부터 입력되는 PWM 신호를 읽어:

- 일반 LED의 **밝기 조절** (CH3)
- 전체 LED의 **ON/OFF 제어** (CH5)
- 삼색 LED(RGB)의 **색상 선택** (CH6)

을 수행합니다. 아두이노는 `PinChangeInterrupt` 라이브러리를 사용하여 PWM 신호의 폭을 읽고, `analogWrite()`를 통해 LED를 제어합니다.

---

## 💾 소프트웨어 구성

| 구성 요소             | 설명                                        |
|----------------------|---------------------------------------------|
| 아두이노 스케치       | PWM 신호 측정 및 LED 제어를 위한 메인 코드 |
| PinChangeInterrupt   | 아날로그 핀의 인터럽트를 처리하는 라이브러리 |
| 시리얼 모니터         | PWM 신호 및 상태 디버깅용 출력              |

### ✅ 주요 기능
- `micros()`를 활용한 PWM 펄스폭 측정  
- CH3: PWM 값을 밝기로 매핑 (1000~2000μs → 0~255)  
- CH5: LED 전체 ON/OFF 제어  
- CH6: RGB 색상 선택 (빨강, 초록, 파랑)  
- RGB LED도 CH5의 ON/OFF 상태를 따라감

---

## 🔌 하드웨어 구성

| 부품                 | 역할 및 사양                             |
|----------------------|-------------------------------------------|
| 아두이노 (UNO/Nano 등) | 전체 로직 처리용 MCU                   |
| 일반 LED (PWM 제어용) | D9 핀에 연결, 밝기 조절 가능            |
| 삼색 LED (공통 음극)   | R: D3 / G: D5 / B: D10 핀에 연결        |
| RC 수신기             | CH3: A0 / CH5: A1 / CH6(G 스위치): A2 핀 연결 |
| 저항들               | LED 전류 제한용                         |
| 전원 공급            | USB 또는 외부 5V 사용                   |

---

## 🔧 회로도 (선택 사항)

> ![프로젝트 이미지](images/project-demo.jpg)  

