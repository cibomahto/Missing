
// Digital outputs
#define STEP_PIN        2
#define DIR_PIN         3
#define ENABLE_PIN      4
#define LED_PIN         11
//#define CURRENT_REF_PIN 9

// Analog input
#define POSITION_PIN    0

uint8_t hysteresis    = 50;

int targetPos;
boolean dir;
boolean moving;

void setup() {
  Serial.begin(57600);
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);
  
  digitalWrite(LED_PIN, HIGH);      // Turn the LED on
  digitalWrite(STEP_PIN, LOW);      // Reset the step input
  digitalWrite(ENABLE_PIN, LOW);    // Enable the stepper driver

//  analogWrite(CURRENT_REF_PIN, 25); // Set the current reference to a low value
  
  targetPos = 600;
  dir = false;
  moving = false;
  
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

  if(Serial.available()) {
    char in = Serial.read();
    if(in >= '0' && in <= '9') {
      cli();
      targetPos = (in - '0' + 1)*100;
      sei();
    }
  }
  
//  Serial.print(currentPos);
//  Serial.print(' ');
//  Serial.print(targetPos);
//  Serial.print(' ');
//  Serial.print(dir);
//  Serial.print(' ');
//  Serial.println(moving);
}

uint8_t currentStepDelay = 200;
uint8_t minStepDelay = 60;
uint8_t maxStepDelay = 255;

uint8_t stepDelayAcceleration = 2;

// Servo loop
ISR(TIMER2_COMPA_vect)
{  
  int currentPos = analogRead(POSITION_PIN);
  
  if (abs(targetPos - currentPos) > hysteresis) {
    // If we should be moving, make a move
    
    if (targetPos > currentPos) {
      dir = false;
      digitalWrite(DIR_PIN, LOW);
    }
    else {
      dir = true;
      digitalWrite(DIR_PIN, HIGH);
    }
    
    moving = true;
    digitalWrite(STEP_PIN, HIGH);
    digitalWrite(STEP_PIN, LOW);
    
    // Now, determine if we should be speeding up/slowing down
    // note we are not considering directioN!!
    if(abs(targetPos - currentPos) > 100) {
      // try to accelerate
      if (currentStepDelay > minStepDelay) {
        currentStepDelay -= stepDelayAcceleration;
      }
    }
    else {    
      // try to accelerate
      if (currentStepDelay < maxStepDelay) {
        currentStepDelay += stepDelayAcceleration;
      }
    }
  }
  else {
    // We are not mobing.
      
    moving = false;
    currentStepDelay = maxStepDelay;
  }
  
  OCR2A = currentStepDelay;
}
