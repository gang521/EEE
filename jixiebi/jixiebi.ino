#include <Servo.h>

int resultX, resultY;
int resultButton;
const int pinButton = 2;
const int pinLED = 13;
const int pinSw = 4;
const int pinWan = 11;
const int pinServo1 = 9;
const int pinServo2 = 10;
int buttonState;
int angleX, angleY;
int lastAngleX = 90, lastAngleY = 90;  // 记录上次角度，初始为90度
const int threshold = 5;  // **角度变化阈值**
Servo myservo1, myservo2, myservo3;

void setup() {
  pinMode(pinButton, INPUT);
  pinMode(pinLED, OUTPUT);
  pinMode(pinSw, INPUT);
  Serial.begin(9600);
  
  myservo1.attach(pinServo1);
  myservo2.attach(pinServo2);
  myservo3.attach(pinWan);
  
  myservo1.write(90);
  myservo2.write(90);
  myservo3.write(0);
}

void loop() {
  resultButton = digitalRead(pinButton);
  buttonState = digitalRead(pinSw);
  digitalWrite(pinLED, resultButton);

  resultX = analogRead(A4);
  resultY = analogRead(A3);

  int newAngleY = map(resultY, 0, 1023, 30, 150);
  int newAngleX = map(resultX, 0, 1023, 30, 150);

  // **只有当角度变化超过阈值时才更新**
  if (abs(newAngleY - lastAngleY) > threshold) {
    myservo1.write(newAngleY);
    lastAngleY = newAngleY;  // 记录当前角度
  }
  if (abs(newAngleX - lastAngleX) > threshold) {
    myservo2.write(newAngleX);
    lastAngleX = newAngleX;  // 记录当前角度
  }

  // **控制夹爪**
  if (buttonState == HIGH) {
    myservo3.write(180);  
  } else {
    myservo3.write(0);  
  }

  Serial.print("X: ");
  Serial.print(resultX);
  Serial.print(", Y: ");
  Serial.println(resultY);
}
