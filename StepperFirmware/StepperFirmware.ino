#define STEP_PIN        2
#define DIR_PIN         3
#define ENABLE_PIN      4
#define LED_PIN         13
#define CURRENT_REF_PIN 9
#define POSITION_PIN    0

const uint8_t movingCurrent = 40, stillCurrent = 40;
const uint8_t slowStep = 90, fastStep = 30, reverseStep = 160;
const uint8_t hysteresis = 20;

uint16_t targetPosition = 512, currentPosition;
boolean lastNear = true, lastDirection = false;

void setup() {
  Serial.begin(57600);

  pinMode(LED_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);

  digitalWrite(LED_PIN, HIGH); //Turn the LED on
  digitalWrite(STEP_PIN, LOW); // Reset the step input
  digitalWrite(ENABLE_PIN, LOW); // Enable the stepper driver

  digitalWrite(DIR_PIN, LOW);

  // Set up Timer 2
  cli();
  TCNT2   = 0;
  OCR2A   = 255;
  TCCR2A  = (1<<WGM21)|(0<<WGM20);
  TCCR2B  = (0<<WGM22)|(1<<CS22)|(1<<CS21)|(1<<CS20);
  TIMSK2  = (1<<OCIE2A);
  OCR2A   = slowStep;
  sei();
}

void loop() {
  // jump to random positions
  targetPosition = random(0, 1024);
  delay(random(1, 6) * 1000);
  
  // use input bytes to indicate 0-255 => 0-1024
//  while(Serial.available()) {
//    char in = Serial.read();
//    targetPosition = 4 * in;
//  }
    
  // hit enter for a random position
//  if(Serial.available()) {
//    char in = Serial.read();
//    if(in == '\n') {
//      targetPosition = random(0, 1024);
//    }
//  }

  // print target, current, OCR2A
//  Serial.print(targetPosition);
//  Serial.print('\t');
//  Serial.print(currentPosition);
//  Serial.print('\t');
//  Serial.println(OCR2A);
//  delay(100);
}

ISR(TIMER2_COMPA_vect) {  
  currentPosition = analogRead(POSITION_PIN);
  
  boolean near = abs(currentPosition - targetPosition) < hysteresis;
  boolean curDirection = targetPosition > currentPosition;
  digitalWrite(DIR_PIN, curDirection ? LOW : HIGH);
  if(!lastNear && curDirection != lastDirection) {
    OCR2A = reverseStep;
  }
  lastDirection = curDirection;
  
  if(!near) {
    analogWrite(CURRENT_REF_PIN, movingCurrent);
    digitalWrite(STEP_PIN, HIGH);
    digitalWrite(STEP_PIN, LOW);
    if(OCR2A > fastStep) {
      OCR2A--;
    }
  } 
  else {
    analogWrite(CURRENT_REF_PIN, stillCurrent);
    OCR2A = slowStep;
  }
  lastNear = near;
}




