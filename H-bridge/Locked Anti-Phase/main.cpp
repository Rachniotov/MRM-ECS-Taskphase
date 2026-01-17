#include <Arduino.h>
char cmd;
void setup() {
  Serial.begin(9600);
  pinMode(A0, INPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);

  TCCR1A = 0b10110011; // 9 is noninv, 10 is inv, fast pwm
  TCCR1B = 0b00001001; // prescaler 1, fast pwm
  
}

void loop() {
  int p = analogRead(A0);

  // use only half of the pot for speed control
  if (p < 512) {
    p = 512;
  }

  // read cmd
  if(Serial.available() > 0) {
    cmd = Serial.read();
    delay(100);
  } 
  
  
  if(cmd == 'f') {
    p = p; // set duty cycle as p
    Serial.println("f");
    delay(10);

  } else if(cmd == 'b') {
    p = 1023 - p; // invert duty cycle for backward
    Serial.println("b");
    delay(10);

  } else if (cmd == 's') {
    p = 512; // set duty cycle to half for stop
    Serial.println("s");
    delay(10);

  }

  // write pwm to registers
  OCR1A = p;
  OCR1B = p;
  
}