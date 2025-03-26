//22343057 裴永卓 物理学院光电信息科学与工程
//舵机控制超声波探测
#include <Servo.h>

int Trig = 12
int Echo = 11; 
int pinServo = 9; 
int pinX = A3
int pinY = A4;
int pinSw = 4;

bool fastMode = true;        // 快速测量模式 (true) / 慢速测量模式 (false)
bool scanMode = false;       // 扫描模式状态
bool dynamicMode = false;    // 动态频率模式状态
unsigned long pressStart = 0;// 按键按下时间记录变量
unsigned long lastDebounceTime = 0; // 上次去抖时间
unsigned long lastClickTime = 0;    // 上次点击时间
bool waitingForDoubleClick = false; // 是否在等待双击
Servo myServo;               // 创建舵机对象

// 超声波距离测量函数
int measureDistance() {
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);
  return pulseIn(Echo, HIGH) / 58; // 计算并返回距离值
}

void setup() {
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);
  pinMode(pinSw, INPUT_PULLUP);
  Serial.begin(9600);
  
  myServo.attach(pinServo);
  myServo.write(90); // 初始化舵机角度为90度
}

void loop() {
  int resultX = analogRead(pinX) - 512; // 读取摇杆X轴偏移量
  int resultY = analogRead(pinY) - 512; // 读取摇杆Y轴偏移量
  int buttonState = digitalRead(pinSw); // 读取摇杆按钮状态
  
  // 计算摇杆控制的角度值
  float angle = atan2(resultY, resultX) * 180.0 / PI;
  angle = map(angle, -180, 180, 0, 180);

  if (angle >= 0 && angle <= 180) {
    myServo.write(angle); // 根据计算角度控制舵机
    Serial.print("当前角度: ");
    Serial.println(angle);
  } else {
    Serial.println("角度计算超出有效范围");
  }

  // 按钮去抖动检测
  if (buttonState == LOW) {
    if (millis() - lastDebounceTime > 50) { // 去抖动时间间隔
      if (pressStart == 0) pressStart = millis(); // 记录按钮按下时间
      
      if (millis() - pressStart >= 2000) {  // 长按操作，进入扫描模式
        scanMode = true;
        Serial.println("已进入扫描模式");
        pressStart = 0;
      }
      
      lastDebounceTime = millis();
    }
  } else {  
    if (pressStart > 0) {
      if (millis() - pressStart < 2000) {  // 短按操作
        if (waitingForDoubleClick) {
          if (millis() - lastClickTime < 300) { // 双击时间间隔
            dynamicMode = !dynamicMode;
            Serial.print("切换到 ");
            Serial.println(dynamicMode ? "动态频率模式" : "固定频率模式");
            waitingForDoubleClick = false;
          } else {
            waitingForDoubleClick = false;
          }
        } else {
          // 单击切换测量模式
          fastMode = !fastMode;
          Serial.print("当前测量模式: ");
          Serial.println(fastMode ? "快速模式 (100ms)" : "慢速模式 (1s)");
          lastClickTime = millis();
          waitingForDoubleClick = true;
        }
      }
      pressStart = 0;
    }
  }

  // 扫描模式运行逻辑
  if (scanMode) {
    for (int scanAngle = 0; scanAngle <= 180; scanAngle += 15) {
      myServo.write(scanAngle); // 调整舵机到扫描角度
      delay(500); // 短暂延时稳定测量
      int distance = measureDistance(); // 获取当前角度下的距离
      Serial.print("扫描角度: ");
      Serial.print(scanAngle);
      Serial.print("°, 测量距离: ");
      Serial.print(distance);
      Serial.println(" cm");
    }
    Serial.println("扫描完成，恢复初始位置");
    myServo.write(angle); // 扫描结束后恢复到之前角度
    scanMode = false;
  } else {
    // 根据摇杆偏移量计算动态频率
    int offset = sqrt(resultX * resultX + resultY * resultY);
    int delayTime = map(offset, 0, 512, 100, 1000); // 偏移越大，延迟越小
    delayTime = constrain(delayTime, 100, 1000); // 限制在合理范围
    
    // 动态模式下使用计算的延迟时间，否则使用固定模式
    if (dynamicMode) {
      delay(delayTime);
    } else {
      delay(fastMode ? 100 : 1000);
    }
    
    int distance = measureDistance(); // 获取当前距离
    Serial.print("当前角度: ");
    Serial.print(angle);
    Serial.print("°, 测量距离: ");
    Serial.print(distance);
    Serial.println(" cm");
  }
}