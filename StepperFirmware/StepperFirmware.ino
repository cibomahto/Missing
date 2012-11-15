#include <EEPROM.h>

#include "defines.h"
#include "PololuStepper.h"
#include "RS485.h"
#include "Protocol.h"
#include "parameters.h"

const uint8_t movingCurrent        = 35;
const uint8_t decelerationCurrent  = 35;
const uint8_t holdingCurrent       = 35;

uint16_t minStepDelay        = 3800;
uint16_t maxStepDelay        = 10000;
uint16_t currentStepDelay;

boolean currentDirection           = CLOCKWISE;

uint8_t stepDelayAcceleration      = 100;
uint8_t stepDelayDeceleration      = 500;
uint8_t decelerateOffset           = 15; ///< fudge factor, since we aren't doing real trapezoidal acceleration.

uint8_t stopHysteresis       = 2;  ///< How close we have to be from the target to stop moving
uint8_t startHysteresis      = 10;  ///< How far from the target we have to be to start moving
boolean moving                     = false;

int16_t targetPosition             = 512;
int16_t currentPosition;

int32_t currentStepPosition;   ///< The current step position we are on; use with microstepping to ensure we always stop on a whole step.

uint8_t postDivider;
#define POSTDIVIDER_SCALE 0

PololuStepper stepper;
RS485 rs485;
Protocol protocol;

#define NOISE_LENGTH 64
#define NOISE_MAX 2
uint8_t noise[NOISE_LENGTH];
uint8_t noisePhase;

void setup() {
  // Turn on the LED
  pinMode(LED_PIN, OUTPUT);
//  digitalWrite(LED_PIN, HIGH);
//  delay(2000);
//  digitalWrite(LED_PIN, LOW);  
  
  initParameter();

  rs485.init(57600, MODE_RECEIVE);
  
  protocol.init(readUint8Parameter(PARAMETER_ADDRESS));
  
  // Disable the stepper driver, configure it, then turn it back on.
  stepper.init(SIXTEENTH_STEP, holdingCurrent, readUint8Parameter(PARAMETER_REVERSED));

  currentStepDelay = maxStepDelay;
  currentStepPosition = 0;
  
  // build a table of pseudrorandom data
  for(uint8_t i = 0; i < NOISE_LENGTH; i++) {
    noise[i] = random(0,NOISE_MAX);
  }
  noisePhase = 0;
  
  
  // Set up Timer 2
  cli();
  TCNT2   = 0;
  TCCR2A  = (1<<WGM21)|(0<<WGM20);
  TCCR2B  = (0<<WGM22)|(1<<CS22)|(1<<CS21)|(1<<CS20);
  TIMSK2  = (1<<OCIE2A);
  OCR2A   = minStepDelay;
  sei();
}



void loop() {
  
//************************************************************
// Protocol input
//************************************************************

  if(Serial.available()) {
    char a = Serial.read();
    protocol.readByte(a, targetPosition, minStepDelay, maxStepDelay, stopHysteresis, startHysteresis);
  }

//************************************************************
// ASCII input
//************************************************************

//  if(Serial.available()) {
//    char a = Serial.read();
//    
//    if(a >= '0' && a <= '9') {
//      targetPosition = (a - '0')*100;
//    }
//  }

  
//************************************************************
// Debug input
//************************************************************
  
//  if(Serial.available()) {
//    char a = Serial.read();
//    
//    if(a >= '0' && a <= '9') {
//      stepper.setReferenceCurrent((a - '0')*10);
//    }
//
//    else if(a == 'q') {
//      stepper.setMicrostepMode(FULL_STEP);
//      stepper.reset();
//    }
//    else if(a == 'w') {
//      stepper.setMicrostepMode(HALF_STEP);
//      stepper.reset();
//    }
//    else if(a == 'e') {
//      stepper.setMicrostepMode(QUARTER_STEP);
//      stepper.reset();
//    }
//    else if(a == 'r') {
//      stepper.setMicrostepMode(EIGHTH_STEP);
//      stepper.reset();
//    }
//    else if(a == 't') {
//      stepper.setMicrostepMode(SIXTEENTH_STEP);
//      stepper.reset();
//    }
//    
//    else if(a == 'z') {
//      stepper.step();
//    }
//    else if(a == 'x') {
//      for(uint8_t i=0; i < 2; i++) {
//        stepper.step();
//        delay(200);
//      }
//    }
//    else if(a == 'c') {
//      for(uint8_t i=0; i < 4; i++) {
//        stepper.step();
//        delay(200);
//      }
//    }
//    else if(a == 'v') {
//      for(uint8_t i=0; i < 8; i++) {
//        stepper.step();
//        delay(200);
//      }
//    }
//    else if(a == 'b') {
//      for(uint8_t i=0; i < 16; i++) {
//        stepper.step();
//        delay(200);
//      }
//    }
//    
//    else if(a == 'R') {
//      stepper.reset();
//    }
//    else if(a == 'D') {
//      stepper.disable();
//    } U
//    else if(a == 'E') {
//      stepper.enable();
//    }
//  }
  
//************************************************************
// jump to random positions
//************************************************************

//  targetPosition = random(MIN_POSITION, MAX_POSITION);
//  delay(random(1, 6) * 1000);


//************************************************************
// cycle from end to end
//************************************************************

//
//if(targetPosition == MAX_POSITION) {
//    targetPosition = MIN_POSITION;
//  }
//  else {
//    targetPosition = MAX_POSITION;
//  }
//  delay(12000);



//************************************************************
// Output debug data
//************************************************************

//  uint8_t d = random(200,200);  
//  for(uint8_t i = 0; i < d; i++) {
//    Serial.print(currentPosition);
//    Serial.print(", ");
//    Serial.print(targetPosition);
//    Serial.print(", ");
//    Serial.print(currentStepDelay);
//    Serial.print(", ");
//    Serial.print(currentStepDelay/100);
//    Serial.print(", ");
//    Serial.print(currentDirection);
//    Serial.print(", ");
//    Serial.print(moving);
//    Serial.print(", ");
//    Serial.println(abs(targetPosition - currentPosition));
//    delay(50);
//  }
}

ISR(TIMER2_COMPA_vect) {
//  postDivider = (postDivider + 1)%POSTDIVIDER_SCALE;
//  
//  if(postDivider == 0) {
  
    currentPosition = analogRead(POSITION_INPUT);
    
    // If we should be moving
    if (abs(targetPosition - currentPosition) > startHysteresis || moving == true) {
      stepper.setReferenceCurrent(movingCurrent);
      
      // Now, there are three states we could be in. If currentStepDelay is equal to
      // maxStepDelay, then we just started a move, and should be sure to reset the direction pin
  
      // If we are at a stop, set the new direction and make a move
      if (currentStepDelay == maxStepDelay) {
        if (targetPosition > currentPosition) {
          currentDirection = CLOCKWISE; // todo: backwards
          stepper.setDirection(CLOCKWISE);
        }
        else {
          currentDirection = COUNTERCLOCKWISE;
          stepper.setDirection(COUNTERCLOCKWISE);
        }
      }
      
      stepper.step();
      if(currentDirection == CLOCKWISE) {
        currentStepPosition += 1;
      }
      else {
        currentStepPosition -= 1;
      }
  
      // Now, let's figure out acceleration or deceleration for the next move.
      // If we have velocity in the wrong direction, just burn some of it off
      if ((targetPosition < currentPosition) == currentDirection) {
          if (currentStepDelay < maxStepDelay) {
            currentStepDelay += stepDelayAcceleration;
          }
          if (currentStepDelay > maxStepDelay) {
            currentStepDelay = maxStepDelay;
          }
      }
      // Otherwise, do the normal trapezoid thing here.
      else {
        // Now, determine if we should be speeding up/slowing down
        // note we are not considering directioN!!
        if(abs(targetPosition - currentPosition) > decelerateOffset) {
          // try to accelerate
          if (currentStepDelay > minStepDelay) {
            currentStepDelay -= stepDelayAcceleration;
          }
          if (currentStepDelay < minStepDelay) {
            currentStepDelay = minStepDelay;
          }
        }
        else {
          stepper.setReferenceCurrent(decelerationCurrent);
          // try to decelerate
          if (currentStepDelay < maxStepDelay) {
            currentStepDelay += stepDelayDeceleration;
          }
          if (currentStepDelay > maxStepDelay) {
            currentStepDelay = maxStepDelay;
          }
        }
      }
      
      // If we got within the stop hysterisis band, mark as not moving.
      if (abs(targetPosition - currentPosition) > stopHysteresis) {
        moving = true;
      }
//      // Unless we are't on a step point yet.
//      else if(currentStepPosition % 16 != 4) {
//        digitalWrite(LED_PIN, HIGH);
//        moving = true;
//      }
      else {
//        digitalWrite(LED_PIN, LOW);
        moving = false;
      }
    }
    else {
      // We are not moving.
      stepper.setReferenceCurrent(holdingCurrent);
      
      currentStepDelay = maxStepDelay;
    }
    
//  }
  
  OCR2A = currentStepDelay/100 + noise[noisePhase];
  noisePhase = (noisePhase + 1)%NOISE_LENGTH;
}





