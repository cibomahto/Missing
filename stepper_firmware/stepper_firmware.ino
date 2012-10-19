
// Digital outputs
#define STEP_PIN        2
#define DIR_PIN         3
#define ENABLE_PIN      4
#define LED_PIN         13
#define CURRENT_REF_PIN 9

// Analog input
#define POSITION_PIN    0

#define CLOCKWISE        false
#define COUNTERCLOCKWISE true


const uint8_t movingCurrent   = 25;  // current to apply when moving, in counts
const uint8_t holdingCurrent  = 10;  // current to apply when holding, in counts

uint8_t hysteresis            = 20;  // overshoot amount

boolean currentDirection      = CLOCKWISE;  // Only valid if currentStepDelay < maxStepDelay
uint8_t currentStepDelay      = 140;
const uint8_t minStepDelay    = 40;
const uint8_t maxStepDelay    = 140;

uint8_t stepDelayAcceleration = 2;
uint8_t stepDelayDeceleration = 2;
int     decelerateOffset      = 80;  // fudge factor, since we aren't doing real trapezoidal acceleration.

uint16_t lastPos;
uint16_t targetPos;


void setup() {
  Serial.begin(57600);
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);
  
  digitalWrite(LED_PIN, HIGH);      // Turn the LED on
  digitalWrite(STEP_PIN, LOW);      // Reset the step input
  digitalWrite(ENABLE_PIN, LOW);    // Enable the stepper driver

  analogWrite(CURRENT_REF_PIN, 20); // Set the current reference to a low value
  
  lastPos = analogRead(POSITION_PIN);
  targetPos = 600;
  
  // Set up Timer 2 to generate interrupts on overflow, and start it.
  // The display is updated in the interrupt routine
  cli();
  TCNT2   = 0;
  OCR2A   = 255;
  TCCR2A  = (1<<WGM21)|(0<<WGM20);
  TCCR2B  = (0<<WGM22)|(1<<CS22)|(1<<CS21)|(1<<CS20);
  TIMSK2  = (1<<OCIE2A);
  sei();
}


void loop() {

  targetPos = random(1,10)*100;
  delay(random(1,6)*1000);
  
//  if(Serial.available()) {
//    char in = Serial.read();
//    if(in >= '0' && in <= '9') {
//      cli();
//      targetPos = (in - '0' + 1)*100;
//      sei();
//    }
//  }
  
//  Serial.print(currentPos);
//  Serial.print(' ');
//  Serial.print(targetPos);
//  Serial.print(' ');
//  Serial.print(dir);
//  Serial.print(' ');
//  Serial.println(moving);
}



// Servo loop
ISR(TIMER2_COMPA_vect)
{  
  uint16_t currentPos = analogRead(POSITION_PIN);
  
  // If we should be moving
  if (abs(targetPos - currentPos) > hysteresis) {
    analogWrite(CURRENT_REF_PIN, movingCurrent);

    // Now, there are three states we could be in. If currentStepDelay is equal to
    // maxStepDelay, then we just started a move, and should be sure to reset the direction pin

    // If we are at a stop, set the new direction and make a move
    if (currentStepDelay >= maxStepDelay) {
      if (targetPos > currentPos) {
        currentDirection = CLOCKWISE;  // todo: backwards?
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
    if ((targetPos < currentPos) == currentDirection) {
        if (currentStepDelay < maxStepDelay) {
          currentStepDelay += stepDelayDeceleration;
        }
    }
    // Otherwise, do the normal trapezoid thing here.
    else {
      // Now, determine if we should be speeding up/slowing down
      // note we are not considering directioN!!
      if(abs(targetPos - currentPos) > decelerateOffset) {
  
        // try to accelerate
        if (currentStepDelay > minStepDelay) {
          currentStepDelay -= stepDelayAcceleration;
        }
      }
      else {
        // try to accelerate
        if (currentStepDelay < maxStepDelay) {
          currentStepDelay += stepDelayDeceleration;
        }
      }
    }
  }
  else {
    // We are not moving.
    analogWrite(CURRENT_REF_PIN, holdingCurrent);
    
    currentStepDelay = maxStepDelay;
  }
  
  OCR2A = currentStepDelay;
}
