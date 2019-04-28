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

const int northPedestrianButtonPin = 1;

const int northPedestrianLightPin = 0;

//East-bound pins
const int eastCarSensorTriggerPin = 5;
const int eastCarSensorEchoPin = 3;

const int eastGreenLightPin = 11;
const int eastYellowLightPin = 12;
const int eastRedLightPin = 13;

const int eastPedestrianButtonPin = 9;

const int eastPedestrianLightPin = 10;

//North-bound flags
bool northCarSensorFlag = false;
bool northPedestrianButtonFlag = false;

//East-bound flags
bool eastCarSensorFlag = false;
bool eastPedestrianButtonFlag = false;

//Global states
bool switchingDirections = false;
enum cross_direction { NORTH, EAST };
cross_direction crossingDirection = NORTH;

//Light states
enum light_color { GREEN, YELLOW, RED };

//Timer compare interrupt for pedestrian light blinking
ISR(TIMER1_COMPA_vect) {
  if (!switchingDirections) {
    if (crossingDirection == NORTH && northPedestrianButtonFlag == true) {
      digitalWrite(northPedestrianLightPin, !digitalRead(northPedestrianLightPin));
    } else if (crossingDirection == EAST && eastPedestrianButtonFlag == true) {
      digitalWrite(eastPedestrianLightPin, !digitalRead(eastPedestrianLightPin));
    }    
  } else {
    digitalWrite(northPedestrianLightPin, LOW);
    digitalWrite(eastPedestrianLightPin, LOW);
  }
}

//North-bound button pin-change interrupt ISR
ISR(PCINT2_vect) {
  if (PIND & (1 << PD1)) {
    northPedestrianButtonFlag = true;
  }
}

//East-bound button pin-change interrupt ISR
ISR(PCINT0_vect) {
  if (PINB & (1 << PB1)) {
    eastPedestrianButtonFlag = true;
  }
}

//North-bound sensor interrupt function
void northCarSensorISR() {
    static unsigned long northStartTime;

    if (digitalRead(northCarSensorEchoPin)) {
      northStartTime = micros();
    } else if ((((micros() - northStartTime) / 2) / 29.1) < 5.0) {
      northCarSensorFlag = true;
    }
}

//East-bound sensor interrupt function
void eastCarSensorISR() {
    static unsigned long eastStartTime;

    if (digitalRead(eastCarSensorEchoPin)) {
      eastStartTime = micros();
    } else if ((((micros() - eastStartTime) / 2) / 29.1) < 5.0) {
      //eastCarSensorFlag = true;
    }
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
    setActiveLight(EAST, GREEN);
    
    crossingDirection = EAST;
    eastCarSensorFlag = false;
  } else if (crossingDirection == EAST) {
    eastPedestrianButtonFlag = false;
    
    setActiveLight(EAST, YELLOW);
    delay(2000);
    setActiveLight(EAST, RED);
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
  } else if (targetDirection == EAST) {
    switch (targetColor) {
      case GREEN:
        digitalWrite(eastGreenLightPin, HIGH);
        digitalWrite(eastYellowLightPin, LOW);
        digitalWrite(eastRedLightPin, LOW);
        break;
      case YELLOW:
        digitalWrite(eastGreenLightPin, LOW);
        digitalWrite(eastYellowLightPin, HIGH);
        digitalWrite(eastRedLightPin, LOW);
        break;
      case RED:
        digitalWrite(eastGreenLightPin, LOW);
        digitalWrite(eastYellowLightPin, LOW);
        digitalWrite(eastRedLightPin, HIGH);
        break;
    }
  }
}

//Setup
void setup() {
  noInterrupts();
  //1 sec timer
  TCCR1A = 0; 
  TCCR1B = 0;
  
  OCR1A = 15625;
  
  TCCR1B = (1 << WGM12);
  TCCR1B |= ((1 << CS12) | (1 << CS10));
  
  TCNT1 = 0;
  
  TIMSK1 = (1 << OCIE1A);
  
  //North-bound button pin change interrupt setup
  PCMSK2 |= (1 << PCINT17);
  PCICR  |= (1 << PCIE2);

  //East-bound button pin change interrupt setup
  PCMSK0 |= (1 << PCINT1);
  PCICR  |= (1 << PCIE0);
  interrupts();

  //Initialize start time
  cycleStartTime = millis();

  //LED pin modes
  pinMode(northGreenLightPin, OUTPUT);
  pinMode(northYellowLightPin, OUTPUT);
  pinMode(northRedLightPin, OUTPUT);

  pinMode(northPedestrianLightPin, OUTPUT);
  
  pinMode(eastGreenLightPin, OUTPUT);
  pinMode(eastYellowLightPin, OUTPUT);
  pinMode(eastRedLightPin, OUTPUT);
  
  pinMode(eastPedestrianLightPin, OUTPUT);
  
  //Button pin modes and pull-up resistor
  pinMode(northPedestrianButtonPin, INPUT_PULLUP);
  pinMode(eastPedestrianButtonPin, INPUT_PULLUP);
  
  //Ultrasonic sensor pin modes
  pinMode(northCarSensorTriggerPin, OUTPUT);
  pinMode(northCarSensorEchoPin, INPUT);
  
  pinMode(eastCarSensorTriggerPin, OUTPUT);
  pinMode(eastCarSensorEchoPin, INPUT);
  
  //Attach car sensors to input interrupts
  attachInterrupt(0, northCarSensorISR, CHANGE);
  attachInterrupt(1, eastCarSensorISR, CHANGE);

  //Initial state
  setActiveLight(NORTH, GREEN);
  setActiveLight(EAST, RED);
}

//Loop
void loop() {
  //Ultrasonic sensor edge triggers
  digitalWrite(northCarSensorTriggerPin, LOW);
  digitalWrite(eastCarSensorTriggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(northCarSensorTriggerPin, HIGH);
  digitalWrite(eastCarSensorTriggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(northCarSensorTriggerPin, LOW);
  digitalWrite(eastCarSensorTriggerPin, LOW);

  //Main update function to detect when to trigger a light switch
  if (!switchingDirections) {
    if (crossingDirection == NORTH) {
      if (((millis() >= (cycleStartTime + halfCycleTime)) && (eastCarSensorFlag || eastPedestrianButtonFlag)) || (millis() >= (cycleStartTime + fullCycleTime))) {
        switchDirections();
      }
    } else if (crossingDirection == EAST) {
      if (((millis() >= (cycleStartTime + halfCycleTime)) && (northCarSensorFlag || northPedestrianButtonFlag)) || (millis() >= (cycleStartTime + fullCycleTime))) {
        switchDirections();
      }
    }
  }
}
