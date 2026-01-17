#include <Arduino.h>
#include <Wire.h>
#include <math.h>


float AX, AY, AZ, Tmp, GX, GY, GZ;
long previousMillis = 0;
float angleX = 0;
float angleY = 0;
float angleZ = 0;

float rollG = 0.0;
float Proll = 0.0; // error covariance for roll
float Qroll = 0.5; // estimate error for roll
float Rroll = 30; // measurement error for roll

float pitchG = 0.0;
float Ppitch = 0.0; // error covariance for pitch
float Qpitch = 0.5; // estimate error for pitch
float Rpitch = 30; // measurement error for pitch


void setup() {
  Serial.begin(9600);
  Wire.begin();

  delay(100);

  // gyro config
  Wire.beginTransmission(0x68);
  Wire.write(27);
  Wire.write(0b00010000); 
  Wire.endTransmission();

  // accel config
  Wire.beginTransmission(0x68);
  Wire.write(28);
  Wire.write(0b00010000);
  Wire.endTransmission();

}

void loop () {
  // read sensor data
  Wire.beginTransmission(0x68);
  Wire.write(59);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 14);
  if (Wire.available() >= 14) {
    AX = Wire.read() << 8 | Wire.read(); 
    AY = Wire.read() << 8 | Wire.read();
    AZ = Wire.read() << 8 | Wire.read();
    Tmp = Wire.read() << 8 | Wire.read();
    GX = Wire.read() << 8 | Wire.read();
    GY = Wire.read() << 8 | Wire.read();
    GZ = Wire.read() << 8 | Wire.read();
  }

  // convert to G
  AX = AX / 4096.0;
  AY = AY / 4096.0;
  AZ = AZ / 4096.0;

  // convert to deg C
  Tmp = (Tmp / 340.00) + 36.53;

  // convert to deg/s
  GX = GX / 32.8;
  GY = GY / 32.8;
  GZ = GZ / 32.8;

  // calculate acc angles
  float roll = atan2(AY, AZ) * 180 / 3.1415;
  float pitch = atan2(-AX, AZ) * 180 / 3.1415;

  // calculate delta time
  long currentMillis = millis();
  float deltaTime = (currentMillis - previousMillis) / 1000.0;

  // integrate gyro angles
  angleX += GX * deltaTime;
  angleY += GY * deltaTime;
  angleZ += GZ * deltaTime;
  previousMillis = currentMillis;

  // kalman
  float thetaG = angleX;

  // roll calculation 
  Proll += Qroll;
  float Kroll = Proll / (Proll + Rroll);
  rollG += Kroll * (roll - rollG);
  Proll = Proll - Kroll * Proll;

  // pitch calculation
  Ppitch += Qpitch;
  float Kpitch = Ppitch / (Ppitch + Rpitch);
  pitchG += Kpitch * (pitch - pitchG);
  Ppitch = Ppitch - Kpitch * Ppitch;


  Serial.print(rollG);
  Serial.print(",");
  Serial.print(angleX);
  Serial.print(",");
  Serial.print(pitchG);
  Serial.print(",");
  Serial.println(angleY);

}
