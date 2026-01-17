#include <Arduino.h>

int Q1 = 3;
int Q2 = 5;
int Q3 = 6;
int Q4 = 9;

void setup() {
  pinMode(Q1, OUTPUT);
  pinMode(Q2, OUTPUT);
  pinMode(Q3, OUTPUT);
  pinMode(Q4, OUTPUT);

  pinMode(A0, INPUT);

  Serial.begin(9600);


  // initial state set to freewheeling
  digitalWrite(Q1, HIGH);
  digitalWrite(Q2, LOW); 
  digitalWrite(Q3, HIGH);
  digitalWrite(Q4, LOW);

}

char cmd = ' ';
int duty = 255;

void loop() {
  int Q1pwm = 0;
  int Q2pwm = 0;
  int Q3pwm = 0;  
  int Q4pwm = 0;

  // read cmd
  if (Serial.available() > 0) {
    cmd = Serial.read();
  }

  duty = analogRead(A0) / 4;

  if (cmd == 'f') {
      Serial.println("Forward");
      Q1pwm = 0; 
      Q2pwm = 0;
      Q3pwm = duty;
      Q4pwm = duty;

      delay(100);

  } else if (cmd == 'b') {
      Serial.println("Backward");
      Q1pwm = duty;
      Q2pwm = duty;
      Q3pwm = 0;
      Q4pwm = 0;
      
      delay(100);
      
  } else if (cmd == 's') {
      Serial.println("Stop");
      Q1pwm = 0;
      Q2pwm = 0;
      Q3pwm = 0;
      Q4pwm = 0;

      delay(100);
      
  } else if (cmd == 't') {
      Serial.println("Test");
      analogWrite(Q1, 0);
      analogWrite(Q2, 0);
      analogWrite(Q3, 255);
      analogWrite(Q4, 255);
      delay(2000);
      analogWrite(Q1, 255);
      analogWrite(Q2, 255);
      analogWrite(Q3, 0);
      analogWrite(Q4, 0);
      delay(1000);
  }

  // write pwm to motors
  analogWrite(Q1, Q1pwm);
  analogWrite(Q2, Q2pwm);
  analogWrite(Q3, Q3pwm);
  analogWrite(Q4, Q4pwm);

}