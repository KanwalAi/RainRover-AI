# RainRover: Autonomous Self-Parking & Weather-Responsive Vehicle

**Course:** Programming for AI & Intro to DS (AI & DS 2001) 

**Department:** National University of Computer and Emerging Sciences, Islamabad

**Project Category:** Physical AI / Hardware-Software Integration 

## ## Project Overview

RainRover is an integrated Physical AI project featuring a mobile robot base equipped with three high-complexity features: **Autonomous Line Following**, **Ultrasonic Self-Parking**, and a **Weather-Responsive Convertible Rooftop**. The system processes real-time sensor data through an Arduino microcontroller to execute precise physical movements.

## ## Core Features

* **Autonomous Navigation:** Uses an IR reflectance sensor array to perform line following via a PID-ready control loop.

* **Intelligent Self-Parking:** Implements an ultrasonic-based distance algorithm to detect parking spaces and execute maneuvering routines.

* **Weather-Responsive Actuation:** A dedicated raindrop sensor triggers a servo-motor mechanism to automatically open or close the convertible roof based on environmental conditions.

---

## ## Technical Specifications & Bill of Materials

As required by the project guidelines, this system integrates specific hardware with modular software components:

### Hardware Components 

* 
**Microcontroller:** Arduino (UNO/Mega compatible).


* **Drivetrain:** 2× Gearbox DC Motors + L298N/TB6612 Motor Driver.
* **Sensors:** * 3-5× TCRT5000 IR Line Sensors.
* HC-SR04 Ultrasonic Sensor (Distance detection).
* Raindrop Sensor Module (Analog weather detection).


* **Actuators:** SG90/MG90S Servo Motor for rooftop mechanics.
* **Power:** External battery pack with regulated 5V supply for logic.

### Software Modules 

The firmware is designed with high **modularity** to ensure maintainability:

* `supervisor.ino`: The main state machine handling mode transitions and safety.


* `line_follow.ino`: Dedicated navigation logic.
* `autopark.ino`: Precision distance-based parking routine.
* `roof_control.ino`: Sensor-triggered actuation for the rooftop.

---

## Implementation Phases 

### Part A: Simulation 

* **Circuit Diagrams:** Comprehensive wiring layouts created in Tinkercad/Wokwi.

* **Code Validation:** All software components tested in a simulated environment before hardware deployment.


### Part B: Hardware Integration 

* **Assembly:** Full physical construction of the mobile robot.


* **Functional Testing:** Real-world validation of the sensor-to-motor output loop.

---

## ## Operation & Calibration

1. **IR Calibration:** Thresholds must be tuned based on surface reflectance to ensure stable line tracking.
2. **Ultrasonic Debouncing:** Software implements averaging to filter noise in distance readings during parking maneuvers.
3. **Servo Limits:** Minimum and maximum angles are hard-coded to prevent mechanical strain on the convertible roof structure.

---

## Group Information 

**Team Size:** 5 Members.

**Submitted To:** Ms. Umarah Qaseem / Ms. Mariam bint Imran 
