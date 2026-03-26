#include <Arduino.h>

//// ---- ESP32 pins ---- 

// ---- Wedge motor driver ----
#define W_IN1 25    
#define W_IN2 26
#define W_PWM 27    

// ---- Left side motor ----
#define DL_IN1 16   
#define DL_IN2 17
#define DL_PWM 18

// ---- Right side motor ----
#define DR_IN1 21   
#define DR_IN2 19
#define DR_PWM 22

// ---- Sensors ----
#define IR 34         
#define US_TRIG 23
#define US_ECHO 5

//// ---- Constants ----
#define SEARCH_SPEED 80 
#define ATTACK_SPEED 150
#define TURN_SPEED 150
#define BACKUP_TIME 600 // ms
#define TURN_TIME 300
#define EDGE_DETECTED HIGH    // change to LOW if your IR outputs HIGH on black
#define DETECT_THRESHOLD 60  // cm
#define LIFT_THRESHOLD 10

// ---- Millis stuff to run multiple things at once ----
unsigned long startMillis;
const unsigned long period = 3000;

//// ---- Setup ----
void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(DL_IN1, OUTPUT);
  pinMode(DL_IN2, OUTPUT);
  pinMode(DR_IN1, OUTPUT);
  pinMode(DR_IN2, OUTPUT);

  pinMode(W_IN1, OUTPUT);
  pinMode(W_IN2, OUTPUT);

  pinMode(DL_PWM, OUTPUT);
  pinMode(DR_PWM, OUTPUT);
  pinMode(W_PWM, OUTPUT);

  pinMode(IR, INPUT);
  pinMode(US_TRIG, OUTPUT);
  pinMode(US_ECHO, INPUT);

  stopAll();
}

//// ---- Functions ----
void driveForward(int speed) {
  digitalWrite(DL_IN1, HIGH);
  digitalWrite(DL_IN2, LOW);
  analogWrite(DL_PWM, speed);

  digitalWrite(DR_IN1, HIGH);
  digitalWrite(DR_IN2, LOW);
  analogWrite(DR_PWM, speed);
}

void driveBackward(int speed) {
  digitalWrite(DL_IN1, LOW);
  digitalWrite(DL_IN2, HIGH);
  analogWrite(DL_PWM, speed);

  digitalWrite(DR_IN1, LOW);
  digitalWrite(DR_IN2, HIGH);
  analogWrite(DR_PWM, speed);
}

void pivotLeft(int speed) {
  digitalWrite(DL_IN1, LOW);
  digitalWrite(DL_IN2, HIGH);
  analogWrite(DL_PWM, speed);

  digitalWrite(DR_IN1, HIGH);
  digitalWrite(DR_IN2, LOW);
  analogWrite(DR_PWM, speed);
}

void pivotRight(int speed) {
  digitalWrite(DL_IN1, HIGH);
  digitalWrite(DL_IN2, LOW);
  analogWrite(DL_PWM, speed);

  digitalWrite(DR_IN1, LOW);
  digitalWrite(DR_IN2, HIGH);
  analogWrite(DR_PWM, speed);
}

void wedgeUp(int speed) {
  digitalWrite(W_IN1, HIGH);
  digitalWrite(W_IN2, LOW);
  analogWrite(W_PWM, speed);
}

void wedgeDown(int speed) {
  digitalWrite(W_IN1, LOW);
  digitalWrite(W_IN2, HIGH);
  analogWrite(W_PWM, speed);
}

void wedgeStop() {
  analogWrite(W_PWM, 0);
}

void stopAll() {
  analogWrite(DL_PWM, 0);
  analogWrite(DR_PWM, 0);
  analogWrite(W_PWM, 0);
}

// ---- Ultrasonic function ----
long getDistanceCM() {
  digitalWrite(US_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(US_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(US_TRIG, LOW);

  long duration = pulseIn(US_ECHO, HIGH, 25000);
  if (duration == 0) return 999;
  return duration / 58;
}

//// ---- Loop ----
void loop() {
  int edge = digitalRead(IR);
  long distance = getDistanceCM();

  //Serial.print(edge);
  //Serial.println(distance);

  // ---- IR stuff ----
  if (edge == EDGE_DETECTED) {
    stopAll();
    driveBackward(200);
    delay(BACKUP_TIME);
    stopAll();
    pivotLeft(TURN_SPEED);
    delay(TURN_TIME);
    stopAll();
    return;
  }

  // ---- Attack mode ----
  if (distance < DETECT_THRESHOLD) {
    pivotLeft(150);
    delay(60);
    stopAll();
    startMillis = millis();
    while (millis() - startMillis <= period) {
      driveForward(ATTACK_SPEED);
      if (digitalRead(IR) == EDGE_DETECTED) break;
      long distance = getDistanceCM();
      if (distance < LIFT_THRESHOLD) {
        driveForward(255);
      }
    }
  } 
  else {
    wedgeStop();
    pivotRight(SEARCH_SPEED);
  }

  delay(30);
}
