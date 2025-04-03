//22343057 裴永卓
//物理学院光电信息科学与工程
//data：20250326
//https://github.com/gang521/EEE.git
//简介：舵机控制超声波探测——课程作业
//功能概述：
//  1.单击切换快速慢速模式
//  2.长按进入扫描模式
//  3.双击进入动态测量模式，再双击退出（优先级最高）
//硬件连接：
//  舵机        -> 9
//  TRIG  -> 12
//  ECHO  -> 11
//  摇杆X轴     -> A3
//  摇杆Y轴     -> A4
//  摇杆SW    -> 4

#include <Servo.h>

int Trig = 12;
int Echo = 11;
int pinServo = 9;
int pinX = A3;
int pinY = A4;
int pinSw = 4;
Servo servo;

int currentMode = 0; // 0慢速,1快速,2动态模式
bool scanning = false;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50; //防抖记录
bool buttonState = HIGH;
bool lastButtonState = HIGH;
unsigned long buttonPressStart = 0;
bool isLongPress = false;
unsigned long lastClickTime = 0; // 记录上次单击时间
float measureDistance();
void handleJoystick();
void toggleMode();
void scanMode();
void toggleMeasurementMode(); // 双击切换动态模式

int angle = 90;

void setup() {
    servo.attach(pinServo);
    servo.write(angle);

    pinMode(Trig, OUTPUT);
    pinMode(Echo, INPUT);
    pinMode(pinSw, INPUT_PULLUP);

    Serial.begin(9600);
}

float measureDistance() {
    digitalWrite(Trig, LOW);
    delayMicroseconds(2);
    digitalWrite(Trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(Trig, LOW);

    long duration = pulseIn(Echo, HIGH);
    float distance = duration * 0.034 / 2; 
    return distance;
}

// 慢速 <-> 快速
void toggleMode() {
    if (currentMode != 2) { // 仅在固定模式下切换
        currentMode = (currentMode == 0) ? 1 : 0;
        Serial.println(currentMode == 1 ? "Fast Mode (100ms interval)" : "Slow Mode (1s interval)");
    }
}

// 扫描模式
void scanMode() {
    scanning = true;
    for (int i = 0; i <= 180; i += 10) {
        servo.write(i);
        delay(500);
        float distance = measureDistance();
        Serial.print("Scan Angle: ");
        Serial.print(i);
        Serial.print(", Distance: ");
        Serial.println(distance);
    }
    servo.write(angle); // 复位
    scanning = false;
}

// 双击动态
void toggleMeasurementMode() {
    if (currentMode == 2) {
        currentMode = 0; // 退出动态
        Serial.println("Exiting Dynamic Measurement Mode, switching to Slow Mode");
    } else {
        currentMode = 2; // 进入动态模式
        Serial.println("Entering Dynamic Measurement Mode");
    }
}

void handleJoystick() {
    int xValue = analogRead(pinX);
    int yValue = analogRead(pinY);
    angle = map(xValue, 0, 1023, 0, 180);
    servo.write(angle);
}

void loop() {
    handleJoystick();

    int reading = digitalRead(pinSw);

    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading != buttonState) {
            buttonState = reading;

            if (buttonState == LOW) {
                unsigned long now = millis();
                if (now - lastClickTime < 500) { 
                    toggleMeasurementMode(); //
                }
                lastClickTime = now;
                buttonPressStart = millis();
                isLongPress = false;
            } else {
                if (!isLongPress && millis() - buttonPressStart >= 50) { // 短按
                    toggleMode(); 
                }
            }
        }

        if (buttonState == LOW && (millis() - buttonPressStart >= 2000)) {
            isLongPress = true;
            scanMode(); // 长按
        }
    }

    lastButtonState = reading;

    if (!scanning) {
        static unsigned long lastMeasureTime = 0;
        unsigned long interval;

        if (currentMode == 2) { // 动态
            int xValue = analogRead(pinX);
            int yValue = analogRead(pinY);
            int distanceFromCenter = abs(xValue - 512) + abs(yValue - 512);
            interval = map(distanceFromCenter, 0, 1023, 1000, 100); // 1s 到 100ms
        } else { 
            interval = (currentMode == 0) ? 1000 : 100; // 1s (慢)或100ms (快)
        }

        if (millis() - lastMeasureTime > interval) {
            lastMeasureTime = millis();
            float distance = measureDistance();
            Serial.print("Distance: ");
            Serial.println(distance);
        }
    }
}
