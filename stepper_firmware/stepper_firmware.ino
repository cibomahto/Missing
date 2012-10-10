#define STEP_PIN   2
#define DIR_PIN    3
#define ENABLE_PIN 4
#define LED_PIN    11

#define POT_INPUT  0

#define MODE_RAMPUP   0
#define MODE_SPEED    1
#define MODE_RAMPDOWN 2
#define MODE_BRAKEING 3
#define MODE_STOPPED  4

int8_t mode;

int stepDelay      = 500000;

int startingDelay  = 4000;
int minDelay       = 2000;
int rampUpSpeed      = 10;
int rampDownSpeed      = 2;

void setup() {
  Serial.begin(57600);
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);

  digitalWrite(ENABLE_PIN, LOW);
  digitalWrite(STEP_PIN, LOW);
  
  stepDelay = startingDelay;
  mode = MODE_RAMPUP;
}

int targetPos = 500;

void loop() {
  int currentPos = analogRead(POT_INPUT);
  Serial.println(currentPos);
  
  if (abs(targetPos - currentPos) > 50) {
    if (targetPos > currentPos) {
      digitalWrite(DIR_PIN, LOW);
  
    }
    else {
      digitalWrite(DIR_PIN, HIGH);
    }
    
    digitalWrite(STEP_PIN, HIGH);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(stepDelay);
    delayMicroseconds(stepDelay);
    delayMicroseconds(stepDelay);
  }
}


//void loop() {
//  switch (mode) {
//    case MODE_RAMPUP:
//      stepDelay -= rampUpSpeed;
//      if (stepDelay < minDelay) {
//        stepDelay = minDelay;
//        mode = MODE_RAMPDOWN;
//      }
//      break;
//    case MODE_RAMPDOWN:
//      stepDelay += rampDownSpeed;
//      if (stepDelay > startingDelay) {
//        stepDelay = startingDelay;
//        mode = MODE_BRAKEING;
//
//      }
//      break;
//    case MODE_BRAKEING:
//      delay(500);
////      digitalWrite(ENABLE_PIN, HIGH);
//      mode=MODE_STOPPED;
//      break;      
// 
//    case MODE_STOPPED:
//      delay(2000);
//      mode=MODE_RAMPUP;
//      stepDelay = startingDelay;
//        digitalWrite(ENABLE_PIN, LOW);
//      break;
//  }
// 
//  if(mode == MODE_RAMPUP || mode == MODE_RAMPDOWN) {
//    digitalWrite(STEP_PIN, HIGH);
//    digitalWrite(STEP_PIN, LOW);
//    delayMicroseconds(stepDelay);
//  }
//  
//  Serial.println(analogRead(POT_INPUT));
//}
