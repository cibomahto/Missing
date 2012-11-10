
#define R_ENABLE_PIN    2
#define D_ENABLE_PIN    3

#define MS1_PIN         11
#define MS2_PIN         10
#define MS3_PIN         8
#define RESET_PIN       7
#define ENABLE_PIN      12
#define STEP_PIN        6
#define DIR_PIN         5

#define LED_PIN         4

#define CURRENT_REF_PIN 9

#define POSITION_INPUT  7

#define CLOCKWISE                  true
#define COUNTERCLOCKWISE           false

const uint8_t movingCurrent        = 35;
const uint8_t holdingCurrent       = 20;

const uint16_t minStepDelay        = 8000;
const uint16_t maxStepDelay        = 16000;
uint16_t currentStepDelay;

boolean currentDirection           = CLOCKWISE;

uint8_t stepDelayAcceleration      = 100;
uint8_t stepDelayDeceleration      = 400;
uint8_t decelerateOffset           = 20; // fudge factor, since we aren't doing real trapezoidal acceleration.

const uint8_t stopHysteresis       = 2;  // How close we have to be from the target to stop moving
const uint8_t startHysteresis      = 30;  // How far from the target we have to be to start moving
boolean moving                     = false;

int16_t targetPosition             = 512;
int16_t currentPosition;

void setup() {
  // Turn on the LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  // Set up the RS485 interface
  digitalWrite(R_ENABLE_PIN, HIGH);  //Note: output mode.
  digitalWrite(D_ENABLE_PIN, HIGH);
  pinMode(R_ENABLE_PIN, OUTPUT);
  pinMode(D_ENABLE_PIN, OUTPUT);
  Serial.begin(57600);
  
  // Disable the stepper driver, configure it, then turn it back on.
  digitalWrite(ENABLE_PIN, HIGH);
  pinMode(ENABLE_PIN, OUTPUT);
  
  pinMode(RESET_PIN, OUTPUT);
  pinMode(MS1_PIN, OUTPUT);
  pinMode(MS2_PIN, OUTPUT);
  pinMode(MS3_PIN, OUTPUT);
  digitalWrite(RESET_PIN, HIGH);
  digitalWrite(MS1_PIN, HIGH);
  digitalWrite(MS2_PIN, HIGH);
  digitalWrite(MS3_PIN, LOW);

  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  digitalWrite(STEP_PIN, LOW);
  digitalWrite(DIR_PIN, LOW);
  
  analogWrite(CURRENT_REF_PIN, holdingCurrent);
    
  digitalWrite(ENABLE_PIN, LOW);

  currentStepDelay = maxStepDelay;
  
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
//  if(Serial.available()) {
//    char a = Serial.read();
//    if(a >= '1' && a <= '9') {
//      targetPosition = (a - '1')*100;
//    }
//  }
  
  // jump to random positions
  targetPosition = random(50, 1024-50);
//  delay(random(1, 6) * 1000);
   
  uint8_t d = random(200,200);  
  for(uint8_t i = 0; i < d; i++) {
    Serial.print(currentPosition);
    Serial.print(", ");
    Serial.print(targetPosition);
    Serial.print(", ");
    Serial.print(currentStepDelay);
    Serial.print(", ");
    Serial.print(currentStepDelay/100);
    Serial.print(", ");
    Serial.print(currentDirection);
    Serial.print(", ");
    Serial.print(moving);
    Serial.print(", ");
    Serial.println(abs(targetPosition - currentPosition));
    delay(50);
  }
}

ISR(TIMER2_COMPA_vect) {
  currentPosition = analogRead(POSITION_INPUT);
  
  // If we should be moving
  if (abs(targetPosition - currentPosition) > startHysteresis || moving == true) {
    analogWrite(CURRENT_REF_PIN, movingCurrent);
    digitalWrite(LED_PIN, HIGH);
    
    // Now, there are three states we could be in. If currentStepDelay is equal to
    // maxStepDelay, then we just started a move, and should be sure to reset the direction pin

    // If we are at a stop, set the new direction and make a move
    if (currentStepDelay == maxStepDelay) {
      if (targetPosition > currentPosition) {
        currentDirection = CLOCKWISE; // todo: backwards?
        digitalWrite(DIR_PIN, LOW);
      }
      else {
        currentDirection = COUNTERCLOCKWISE;
        digitalWrite(DIR_PIN, HIGH);
      }
    }
    
    digitalWrite(STEP_PIN, HIGH);
    digitalWrite(STEP_PIN, LOW);

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
    else {
      moving = false;
    }
  }
  else {
    // We are not moving.
    analogWrite(CURRENT_REF_PIN, holdingCurrent);
    digitalWrite(LED_PIN, LOW);
    
    currentStepDelay = maxStepDelay;
  }
  
  OCR2A = currentStepDelay/100;
}





