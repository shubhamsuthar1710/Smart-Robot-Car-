#include <Servo.h>
#include <SoftwareSerial.h>

SoftwareSerial BT(10, 11); // RX, TX for Bluetooth

// Motor pins (L298N)
const int in1 = 2;
const int in2 = 3;
const int in3 = 4;
const int in4 = 5;

// Motor speed pins
const int enA = 6;
const int enB = 7;

// Servo pin
Servo servo_motor;
const int servoPin = 12;

// Ultrasonic sensor pins
#define trig_pin 8
#define echo_pin 9
#define maximum_distance 200

// LED mode indicators
const int ledManual = A0; // Manual mode LED
const int ledAuto = A1;   // Autonomous mode LED

// Mode control
bool autonomousMode = false;
bool prevAutonomousMode = false; // Track previous mode

char lastCommand = 'S';
unsigned long lastCommandTime = 0;
const unsigned long autoStopDelay = 100; // ms

void setup() {
  // Motor pins
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);

  // LEDs
  pinMode(ledManual, OUTPUT);
  pinMode(ledAuto, OUTPUT);

  // Ultrasonic
  pinMode(trig_pin, OUTPUT);
  pinMode(echo_pin, INPUT);

  // Servo
  servo_motor.attach(servoPin);
  servo_motor.write(90); // Center servo

  // Bluetooth
  BT.begin(9600);

  stopMotors();
  updateLEDs();
}

void loop() {
  // Read Bluetooth commands
  if (BT.available()) {
    char command = BT.read();

    if (command == 'M') { // Switch to manual
      autonomousMode = false;
      updateLEDs();
    }
    else if (command == 'A') { // Switch to auto
      autonomousMode = true;
      updateLEDs();
    }
    else if (!autonomousMode) { // Manual driving
      lastCommand = command;
      lastCommandTime = millis();
      runManualCommand(command);
    }
  }

  // Detect mode change
  if (autonomousMode != prevAutonomousMode) {
    stopMotors();      // Stop immediately on mode change
    delay(300);        // Small safety pause
    prevAutonomousMode = autonomousMode;
  }

  // Manual mode auto-stop if no command
  if (!autonomousMode && millis() - lastCommandTime > autoStopDelay && lastCommand != 'S') {
    stopMotors();
    lastCommand = 'S';
  }

  // Autonomous mode driving
  if (autonomousMode) {
    autonomousDrive();
  }
}

// ---------------- LED Update ----------------
void updateLEDs() {
  digitalWrite(ledManual, !autonomousMode);
  digitalWrite(ledAuto, autonomousMode);
}

// ---------------- Manual Drive ----------------
void runManualCommand(char command) {
  switch (command) {
    case 'F': forward(); break;
    case 'B': backward(); break;
    case 'L': left(); break;
    case 'R': right(); break;
    case 'S': stopMotors(); break;
  }
}

// ---------------- Autonomous Drive ----------------
void autonomousDrive() {
  long dist = getDistance();

  if (dist < 20) {
    stopMotors();
    delay(200);

    servo_motor.write(0);
    delay(400);
    long leftDist = getDistance();

    servo_motor.write(180);
    delay(400);
    long rightDist = getDistance();

    servo_motor.write(90);
    delay(300);

    if (leftDist > rightDist) {
      left();
      delay(400);
    } else {
      right();
      delay(400);
    }
  } else {
    forward();
  }
}

// ---------------- Motor Functions ----------------
void forward() {
  analogWrite(enA, 200);
  analogWrite(enB, 200);
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

void backward() {
  analogWrite(enA, 200);
  analogWrite(enB, 200);
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
}

void left() {
  analogWrite(enA, 150);
  analogWrite(enB, 150);
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

void right() {
  analogWrite(enA, 150);
  analogWrite(enB, 150);
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
}

void stopMotors() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}

// ---------------- Ultrasonic ----------------
long getDistance() {
  digitalWrite(trig_pin, LOW);
  delayMicroseconds(2);
  digitalWrite(trig_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig_pin, LOW);

  long duration = pulseIn(echo_pin, HIGH);
  return duration * 0.034 / 2;
}