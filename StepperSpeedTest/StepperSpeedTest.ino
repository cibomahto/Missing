#define STEP_PIN        2
#define DIR_PIN         3
#define ENABLE_PIN      4
#define LED_PIN         13
#define CURRENT_REF_PIN 9

uint8_t movingCurrent = 40, stillCurrent = 10;
uint8_t slowStep = 90, fastStep = 30;
boolean pause = false;

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
  OCR2A = slowStep;
  sei();
}

uint16_t totalSteps = 0;
boolean lastPause = false;

void loop() {  
  if(Serial.available()) {
    char in = Serial.read();
    if(in == ' ') {
      cli();
      OCR2A = slowStep;
      sei();
    }
    if(in == '0') {
      pause = true;
    }
    if(in == '1') {
      OCR2A = slowStep;
      pause = false;
    }
  }
  Serial.println(OCR2A);
  delay(100);
}

ISR(TIMER2_COMPA_vect) {  
  if(!pause) {
    analogWrite(CURRENT_REF_PIN, movingCurrent);

    digitalWrite(STEP_PIN, HIGH);
    digitalWrite(STEP_PIN, LOW);

    if(OCR2A > fastStep) {
      OCR2A--;
    }
  } 
  else {
    if(pause != lastPause) {
      analogWrite(CURRENT_REF_PIN, stillCurrent);
    } 
    if(OCR2A < slowStep) {
      OCR2A--;
      digitalWrite(STEP_PIN, HIGH);
      digitalWrite(STEP_PIN, LOW);
    } 
    else {
      OCR2A = slowStep;
    }
  }
  lastPause = pause;

  if(OCR2A == slowStep) {
    analogWrite(CURRENT_REF_PIN, stillCurrent);
  }
}




