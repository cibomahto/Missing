#include "PololuStepper.h"
#include "defines.h"

void PololuStepper::init(uint8_t steppingMode, uint8_t referenceCurrent, boolean reversed) {
  disable();
  pinMode(ENABLE_PIN, OUTPUT);
  
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, HIGH);  
  
  pinMode(MS1_PIN, OUTPUT);
  pinMode(MS2_PIN, OUTPUT);
  pinMode(MS3_PIN, OUTPUT);
  setMicrostepMode(steppingMode);

  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  digitalWrite(STEP_PIN, LOW);
  digitalWrite(DIR_PIN, LOW);
  
  setReferenceCurrent(referenceCurrent);
  
  m_reversed = reversed;
    
  enable();
}


void PololuStepper::enable() {
  digitalWrite(ENABLE_PIN, LOW);
}

void PololuStepper::disable() {
  digitalWrite(ENABLE_PIN, HIGH);
}

void PololuStepper::reset() {
  digitalWrite(RESET_PIN, LOW);
  delay(200);
  digitalWrite(RESET_PIN, HIGH);
}

void PololuStepper::setDirection(boolean direction) {
  uint8_t low = LOW;
  uint8_t high = HIGH;
  
  if(m_reversed) {
    low = HIGH;
    high = LOW;
  }
    
  
  if(direction == CLOCKWISE) {
    digitalWrite(DIR_PIN, low);
  }
  else {
    digitalWrite(DIR_PIN, high);
  }
}

void PololuStepper::step() {
  digitalWrite(STEP_PIN, HIGH);
  digitalWrite(STEP_PIN, LOW);
}

void PololuStepper::setReferenceCurrent(uint8_t current) {
  // Determined experimentally:
  //  0: 38ma  (  0 mV)
  // 10: 60ma  (150 mV)
  // 20: 88ma  (200 mV)
  // 30: 112ma (520 mV)
  // 40: 120ma (700 mV)
  
  analogWrite(CURRENT_REF_PIN, current);
}

void PololuStepper::setMicrostepMode(uint8_t mode) {
  switch(mode) {
    case FULL_STEP:
      digitalWrite(MS1_PIN, LOW);
      digitalWrite(MS2_PIN, LOW);
      digitalWrite(MS3_PIN, LOW);
      break;
    case HALF_STEP:
      digitalWrite(MS1_PIN, HIGH);
      digitalWrite(MS2_PIN, LOW);
      digitalWrite(MS3_PIN, LOW);
      break;
    case QUARTER_STEP:
      digitalWrite(MS1_PIN, LOW);
      digitalWrite(MS2_PIN, HIGH);
      digitalWrite(MS3_PIN, LOW);
      break;
    case EIGHTH_STEP:
      digitalWrite(MS1_PIN, HIGH);
      digitalWrite(MS2_PIN, HIGH);
      digitalWrite(MS3_PIN, LOW);
      break;
    case SIXTEENTH_STEP:
      digitalWrite(MS1_PIN, HIGH);
      digitalWrite(MS2_PIN, HIGH);
      digitalWrite(MS3_PIN, HIGH);
      break;
  }
}
