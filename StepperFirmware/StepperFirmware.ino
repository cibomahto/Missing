
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


const uint8_t movingCurrent = 20;
const uint8_t stillCurrent  = 5;
const uint8_t slowStep = 90;
const uint8_t fastStep = 30;
const uint8_t reverseStep = 160;

//const uint8_t slowStep = 240;
//const uint8_t fastStep = 200;
//const uint8_t reverseStep = 240;

const uint8_t hysteresis = 20;

uint16_t targetPosition = 512;
uint16_t currentPosition;
boolean lastNear = true;
boolean lastDirection = false;


void setup() {
  // Turn on the LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  // Set up the RS485 interface
  digitalWrite(R_ENABLE_PIN, LOW);
  digitalWrite(D_ENABLE_PIN, LOW);
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
  digitalWrite(MS3_PIN, HIGH);

  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  digitalWrite(STEP_PIN, LOW);
  digitalWrite(DIR_PIN, LOW);
  
  digitalWrite(ENABLE_PIN, LOW);

  // Set up Timer 2
  cli();
  TCNT2   = 0;
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
  currentPosition = analogRead(POSITION_INPUT);
  
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
    digitalWrite(LED_PIN, HIGH);
  } 
  else {
    analogWrite(CURRENT_REF_PIN, stillCurrent);
    OCR2A = slowStep;
    
    digitalWrite(LED_PIN, LOW);
  }
  lastNear = near;
}





