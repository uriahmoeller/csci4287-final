//Uriah Moeller, Dalton Burke
//Final Project - Traffic Light
//Simulation of a one-way intersection with a one-way pedestrian crossing

//csci4287-final.ino
//main project file

#include "avr/io.h"
#include "avr/interrupt.h"

//Timer
long cycleStartTime = 0;

//Constants
const long fullCycleTime = 30000;
const long halfCycleTime = 15000;

//North-bound pins
const int northCarSensorTriggerPin = 4;
const int northCarSensorEchoPin = 2;

const int northGreenLightPin = 6;
const int northYellowLightPin = 7;
const int northRedLightPin = 8;

const int northPedestrianButtonPin = 9;

const int northPedestrianLightPin = 10;

//South-bound pins
const int southCarSensorTriggerPin = 5;
const int southCarSensorEchoPin = 3;

const int southGreenLightPin = 11;
const int southYellowLightPin = 12;
const int southRedLightPin = 13;

const int southPedestrianButtonPin = 0;

const int southPedestrianLightPin = 1;

//North-bound flags
bool northCarSensorFlag = false;
bool northPedestrianButtonFlag = false;

//South-bound flags
bool southCarSensorFlag = false;
bool southPedestrianButtonFlag = false;

//Global states
bool switchingDirections = false;
enum cross_direction { NORTH, SOUTH };
cross_direction crossingDirection = NORTH;

//Light states
enum light_color { GREEN, YELLOW, RED };

//Timer compare interrupt
ISR(TIMER1_COMPA_vect) {
  if (!switchingDirections) {
    if (crossingDirection == NORTH && northPedestrianButtonFlag == true) {
      digitalWrite(northPedestrianLightPin, !digitalRead(northPedestrianLightPin));
    } else if (crossingDirection == SOUTH && southPedestrianButtonFlag == true) {
      digitalWrite(southPedestrianLightPin, !digitalRead(southPedestrianLightPin));
    }    
  } else {
    digitalWrite(northPedestrianLightPin, LOW);
    digitalWrite(southPedestrianLightPin, LOW);
  }
}

//North-bound input interrupt functions
void northCarSensorISR() {
    static unsigned long northStartTime;

    if (digitalRead(2)) {
      northStartTime = micros();
    } else if ((((micros() - northStartTime) / 2) / 29.1) < 5.0) {
      northCarSensorFlag = true;
    }
}

void northPedestrianButtonISR() {
  northPedestrianButtonFlag = true;
}

//South-bound input interrupt functions
void southCarSensorISR() {
    static unsigned long southStartTime;

    if (digitalRead(2)) {
      southStartTime = micros();
    } else if ((((micros() - southStartTime) / 2) / 29.1) < 5.0) {
      southCarSensorFlag = true;
    }
}

void southPedestrianButtonISR() {
  southPedestrianButtonFlag = true;
}

//Direction switching sequence
void switchDirections() {
  switchingDirections = true;
  if (crossingDirection == NORTH) {
    northPedestrianButtonFlag = false;
    
    setActiveLight(NORTH, YELLOW);
    delay(2000);
    setActiveLight(NORTH, RED);
    delay(5000);
    setActiveLight(SOUTH, GREEN);
    
    crossingDirection = SOUTH;
    southCarSensorFlag = false;
  } else if (crossingDirection == SOUTH) {
    southPedestrianButtonFlag = false;
    
    setActiveLight(SOUTH, YELLOW);
    delay(2000);
    setActiveLight(SOUTH, RED);
    delay(5000);
    setActiveLight(NORTH, GREEN);
    
    crossingDirection = NORTH;
    northCarSensorFlag = false;
  }
  switchingDirections = false;
  cycleStartTime = millis();
}

//Light LED color setter
void setActiveLight (cross_direction targetDirection, light_color targetColor) {
  if (targetDirection == NORTH) {
    switch (targetColor) {
      case GREEN:
        digitalWrite(northGreenLightPin, HIGH);
        digitalWrite(northYellowLightPin, LOW);
        digitalWrite(northRedLightPin, LOW);
        break;
      case YELLOW:
        digitalWrite(northGreenLightPin, LOW);
        digitalWrite(northYellowLightPin, HIGH);
        digitalWrite(northRedLightPin, LOW);
        break;
      case RED:
        digitalWrite(northGreenLightPin, LOW);
        digitalWrite(northYellowLightPin, LOW);
        digitalWrite(northRedLightPin, HIGH);
        break;
    }
  } else if (targetDirection == SOUTH) {
    switch (targetColor) {
      case GREEN:
        digitalWrite(southGreenLightPin, HIGH);
        digitalWrite(southYellowLightPin, LOW);
        digitalWrite(southRedLightPin, LOW);
        break;
      case YELLOW:
        digitalWrite(southGreenLightPin, LOW);
        digitalWrite(southYellowLightPin, HIGH);
        digitalWrite(southRedLightPin, LOW);
        break;
      case RED:
        digitalWrite(southGreenLightPin, LOW);
        digitalWrite(southYellowLightPin, LOW);
        digitalWrite(southRedLightPin, HIGH);
        break;
    }
  }
}

//Setup
void setup() {
  //1 sec timer
  noInterrupts();
  TCCR1A = 0; 
  TCCR1B = 0;
  
  OCR1A = 15625;
  
  TCCR1B = (1 << WGM12);
  TCCR1B |= ((1 << CS12) | (1 << CS10));
  
  TCNT1 = 0;
  
  TIMSK1 = (1 << OCIE1A);
  interrupts();

  //Initialize start time
  cycleStartTime = millis();

  //LED pin modes
  pinMode(northGreenLightPin, OUTPUT);
  pinMode(northYellowLightPin, OUTPUT);
  pinMode(northRedLightPin, OUTPUT);

  pinMode(northPedestrianLightPin, OUTPUT);
  
  pinMode(southGreenLightPin, OUTPUT);
  pinMode(southYellowLightPin, OUTPUT);
  pinMode(southRedLightPin, OUTPUT);
  
  pinMode(southPedestrianLightPin, OUTPUT);

  //Ultrasonic sensor pin modes
  pinMode(northCarSensorTriggerPin, OUTPUT);
  pinMode(northCarSensorEchoPin, INPUT);
  
  pinMode(southCarSensorTriggerPin, OUTPUT);
  pinMode(southCarSensorEchoPin, INPUT);
  
  //Attach car sensors to input interrupts
  attachInterrupt(0, northCarSensorISR, CHANGE);
  attachInterrupt(1, southCarSensorISR, CHANGE);

  //Attach pedestrian crossing buttons to input interrupts
  attachInterrupt(digitalPinToInterrupt(northPedestrianButtonPin), northPedestrianButtonISR, CHANGE); 
  attachInterrupt(digitalPinToInterrupt(southPedestrianButtonPin), southPedestrianButtonISR, CHANGE); 

  //Initial state
  setActiveLight(NORTH, GREEN);
  setActiveLight(SOUTH, RED);
}

//Loop
void loop() {
  //Ultrasonic sensor edge triggers
  digitalWrite(northCarSensorTriggerPin, LOW);
  digitalWrite(southCarSensorTriggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(northCarSensorTriggerPin, HIGH);
  digitalWrite(southCarSensorTriggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(northCarSensorTriggerPin, LOW);
  digitalWrite(southCarSensorTriggerPin, LOW);

  //Main update function to detect when to trigger a light switch
  if (!switchingDirections) {
    if (crossingDirection == NORTH) {
      if (((millis() >= (cycleStartTime + halfCycleTime)) && (southCarSensorFlag || southPedestrianButtonFlag)) || (millis() >= (cycleStartTime + fullCycleTime))) {
        switchDirections();
      }
    } else if (crossingDirection == SOUTH) {
      if (((millis() >= (cycleStartTime + halfCycleTime)) && (northCarSensorFlag || northPedestrianButtonFlag)) || (millis() >= (cycleStartTime + fullCycleTime))) {
        switchDirections();
      }
    }
  }
}
