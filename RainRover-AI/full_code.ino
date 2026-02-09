#include <Wire.h>
#include <PCF8574.h>
#include <Servo.h>

// ==================== PCF8574 GPIO EXTENDER ====================
PCF8574 pcf8574(0x20);

// ==================== ULTRASONIC SENSORS ====================
// Sensor 1 (Left)
#define trigPin1 12
#define echoPin1 13

// Sensor 2 (Right) 
#define echoPin2 A0
#define trig2_PCF 0  // P0 on extender

// Sensor 3 (Front) 
#define echoPin3 A1
#define trig3_PCF 1  // P1 on extender

// ==================== MOTOR DRIVER PINS ====================
const int Enable1Pin = 11;  
const int Enable2Pin = 6;   
const int In1pin = 10;       
const int In2pin = 9;        
const int In3pin = 8;        
const int In4pin = 7;       

// ==================== IR SENSORS (LINE FOLLOWER) ====================
const int IRSensorLeft = 2;
const int IRSensorRight = 3;

// ==================== RAIN SENSOR & SERVO ====================
int rainSensorPin = 5;
Servo roofServo;
const int servoPin = 4;

// ==================== OPERATIONAL MODES ====================
enum Mode {
  LINE_FOLLOWING,
  PARKING,
  PARKED
};

Mode currentMode = LINE_FOLLOWING;

// ==================== PARKING VARIABLES ====================
bool wall = false;
long duration1, duration2, duration3;
long distance1 = 0, distance2 = 0, distance3 = 0;
double old_distance1 = 0;
const int depth = 30;
double delta_speed = 0;
const long speed_avg = 75;

// ==================== LINE FOLLOWER SPEEDS ====================
#define BASE_SPEED 100
#define TURN_SPEED 130

// ==================== ROOF STATE ====================
bool roofClosed = false;

void setup() {
  Serial.begin(9600);
  Wire.begin();


  pcf8574.begin();
  
  // Set PCF8574 pins LOW initially
  pcf8574.write(trig2_PCF, LOW);
  pcf8574.write(trig3_PCF, LOW);

  // Motor pins
  pinMode(In1pin, OUTPUT);
  pinMode(In2pin, OUTPUT);
  pinMode(Enable1Pin, OUTPUT);
  pinMode(In3pin, OUTPUT);
  pinMode(In4pin, OUTPUT);
  pinMode(Enable2Pin, OUTPUT);

  // Ultrasonic Sensor-1 pins (Left)
  pinMode(echoPin1, INPUT);
  pinMode(trigPin1, OUTPUT);
  digitalWrite(trigPin1, LOW);

  // Ultrasonic Sensor-2/3 ECHO pins
  pinMode(echoPin2, INPUT);
  pinMode(echoPin3, INPUT);

  // IR Sensors for line following
  pinMode(IRSensorLeft, INPUT);
  pinMode(IRSensorRight, INPUT);

  // Rain sensor and servo
  pinMode(rainSensorPin, INPUT);
  roofServo.attach(servoPin);
  roofServo.write(0);  // Start with roof open

  Serial.println("System Initialized");
  Serial.println("Mode: LINE_FOLLOWING");
}

void loop() {
  // Always check roof status
  checkRoof();

  // Execute based on current mode
  switch (currentMode) {
    case LINE_FOLLOWING:
      lineFollowingMode();
      break;
    
    case PARKING:
      parkingMode();
      break;
    
    case PARKED:
      parkedMode();
      break;
  }
}

// ==================== ROOF CONTROL ====================
void checkRoof() {
  int sensorState = digitalRead(rainSensorPin);
  
  if (sensorState == HIGH && !roofClosed) {
    Serial.println("Rain detected! Closing roof...");
    roofServo.write(120);
    roofClosed = true;
  } else if (sensorState == LOW && roofClosed) {
    Serial.println("No rain. Opening roof...");
    roofServo.write(0);
    roofClosed = false;
  }
}

// ==================== LINE FOLLOWING MODE ====================
void lineFollowingMode() {
  bool leftSensor = digitalRead(IRSensorLeft);
  bool rightSensor = digitalRead(IRSensorRight);
  
  // Check for parking spot (detect gap on left side)
  readUltrasonicSensors();
  if (walldetect(distance1)) {
    Serial.println("Parking spot detected! Switching to PARKING mode");
    currentMode = PARKING;
    stopMotors();
    delay(500);
    return;
  }

  // Line following logic
  if (leftSensor == LOW && rightSensor == LOW) {
    moveForwardLine();
  } else if (leftSensor == LOW && rightSensor == HIGH) {
    turnRight();
  } else if (leftSensor == HIGH && rightSensor == LOW) {
    turnLeft();
  } else {
    stopMotors();
  }
}

// ==================== PARKING MODE ====================
void parkingMode() {
  readUltrasonicSensors();
  
  wall = walldetect(distance1);

  if (!wall) {
    // Still detecting the gap, adjust alignment
    long h = distance1 - distance2;

    if (h <= 10) {
      delta_speed -= 0.1 * h;
      Serial.println("Aligning for parking...");

      double speed1 = speed_avg - delta_speed;
      double speed2 = speed_avg + delta_speed;

      setspeed1(speed1);
      setspeed2(speed2);
      delay(30);
    }
    old_distance1 = distance1;
  } else {
    // Gap ended, execute parking maneuver
    Serial.println("Executing parking maneuver...");
    
    stopMotors();
    delay(10);

    setspeed1(75);
    setspeed2(75);
    delay(75);

    stopMotors();
    delay(10);

    // Turn into parking spot
    setspeed1(85);
    delay(900);

    // Move forward until close to obstacle
    while (distance3 > 7) {
      readFrontSensor();
      setspeed1(95);
      setspeed2(95);
      delay(50);
    }

    stopMotors();
    Serial.println("Parked successfully!");
    currentMode = PARKED;
  }
}

// ==================== PARKED MODE ====================
void parkedMode() {
  // Car is parked, only check roof
  stopMotors();
  delay(1000);
  // Could add logic here to resume driving after certain condition
}

// ==================== ULTRASONIC SENSOR FUNCTIONS ====================
void readUltrasonicSensors() {
  // LEFT SENSOR (Sensor-1)
  digitalWrite(trigPin1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin1, LOW);
  duration1 = pulseIn(echoPin1, HIGH, 30000);
  distance1 = (duration1 / 2) / 29.15;

  // RIGHT SENSOR (Sensor-2) via PCF8574
  pcf8574.write(trig2_PCF, HIGH);
  delayMicroseconds(10);
  pcf8574.write(trig2_PCF, LOW);
  duration2 = pulseIn(echoPin2, HIGH, 30000);
  distance2 = (duration2 / 2) / 29.15;

  // FRONT SENSOR (Sensor-3)
  readFrontSensor();
}

void readFrontSensor() {
  pcf8574.write(trig3_PCF, HIGH);
  delayMicroseconds(10);
  pcf8574.write(trig3_PCF, LOW);
  duration3 = pulseIn(echoPin3, HIGH, 30000);
  distance3 = (duration3 / 2) / 29.15;
}

bool walldetect(int side_distance) {
  return (side_distance > depth);
}

// ==================== MOTOR CONTROL FUNCTIONS ====================
void setspeed1(double speed) {
  digitalWrite(In1pin, HIGH);
  digitalWrite(In2pin, LOW);
  analogWrite(Enable1Pin, speed);
}

void setspeed2(double speed) {
  digitalWrite(In3pin, HIGH);
  digitalWrite(In4pin, LOW);
  analogWrite(Enable2Pin, speed);
}

void moveForwardLine() {
  digitalWrite(In1pin, HIGH);
  digitalWrite(In2pin, LOW);
  analogWrite(Enable1Pin, BASE_SPEED);

  digitalWrite(In3pin, HIGH);
  digitalWrite(In4pin, LOW);
  analogWrite(Enable2Pin, BASE_SPEED);
}

void turnRight() {
  digitalWrite(In1pin, LOW);
  digitalWrite(In2pin, HIGH);
  analogWrite(Enable1Pin, TURN_SPEED);

  digitalWrite(In3pin, HIGH);
  digitalWrite(In4pin, LOW);
  analogWrite(Enable2Pin, TURN_SPEED);
}

void turnLeft() {
  digitalWrite(In1pin, HIGH);
  digitalWrite(In2pin, LOW);
  analogWrite(Enable1Pin, TURN_SPEED);

  digitalWrite(In3pin, LOW);
  digitalWrite(In4pin, HIGH);
  analogWrite(Enable2Pin, TURN_SPEED);
}

void stopMotors() {
  digitalWrite(In1pin, LOW);
  digitalWrite(In2pin, LOW);
  analogWrite(Enable1Pin, 0);

  digitalWrite(In3pin, LOW);
  digitalWrite(In4pin, LOW);
  analogWrite(Enable2Pin, 0);
}