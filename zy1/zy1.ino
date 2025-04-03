#include <Servo.h>

int Trig = 12, Echo = 11;    // 超声波模块引脚
int pinServo = 9;            // 舵机引脚
int pinX = A3, pinY = A4;    // 摇杆X、Y
int pinSw = 4;               // 摇杆按钮

bool fastMode = true;   // 100ms模式 (true) / 1s模式 (false)
bool scanMode = false;  // 扫描模式
unsigned long pressStart = 0;  // 记录按键时间
Servo myServo;

// **超声波测距函数**
int ceJuli() {
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);
  return pulseIn(Echo, HIGH) / 58; // 计算距离
}

void setup() {
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);
  pinMode(pinSw, INPUT_PULLUP);
  Serial.begin(9600);
  
  myServo.attach(pinServo);
  myServo.write(90);
}

void loop() {
  int resultX = analogRead(pinX) - 512;
  int resultY = analogRead(pinY) - 512;
  int buttonState = digitalRead(pinSw);
  
  // **计算角度**
  float angle = atan2(resultY, resultX) * 180.0 / PI;
  angle = map(angle, -180, 180, 0, 180);

  if (angle >= 0 && angle <= 180) {
    myServo.write(angle);
    Serial.print("角度: ");
    Serial.println(angle);
  } else {
    Serial.println("角度超出范围");
  }

  // **检测按钮状态**
  if (buttonState == LOW) {  
    if (pressStart == 0) pressStart = millis(); // 记录按下时间

    if (millis() - pressStart >= 2000) {  // **长按 ≥ 2s 进入扫描模式**
      scanMode = true;
      Serial.println("进入扫描模式");
      pressStart = 0;
    }
  } else {  
    if (pressStart > 0 && millis() - pressStart < 2000) {  // **短按切换模式**
      fastMode = !fastMode;
      Serial.print("切换测量模式: ");
      Serial.println(fastMode ? "快速 (100ms)" : "慢速 (1s)");
    }
    pressStart = 0;
  }

  // **扫描模式**
  if (scanMode) {
    for (int scanAngle = 0; scanAngle <= 180; scanAngle += 15) {
      myServo.write(scanAngle);
      delay(500);
      int distance = ceJuli();
      Serial.print("扫描角度: ");
      Serial.print(scanAngle);
      Serial.print("°, 距离: ");
      Serial.print(distance);
      Serial.println(" cm");
    }
    Serial.println("扫描结束，返回原始位置");
    myServo.write(angle);
    scanMode = false;
  } else {
    // **普通模式**
    int distance = ceJuli();
    Serial.print("当前角度: ");
    Serial.print(angle);
    Serial.print("°, 距离: ");
    Serial.print(distance);
    Serial.println(" cm");

    delay(fastMode ? 100 : 1000);
  }
}
