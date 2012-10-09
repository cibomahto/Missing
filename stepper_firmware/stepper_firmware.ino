#define STEP_PIN   2
#define ENABLE_PIN 3
#define LED_PIN    11

#define MODE_RAMPUP   0
#define MODE_SPEED    1
#define MODE_RAMPDOWN 2
#define MODE_BRAKEING 3
#define MODE_STOPPED  4

int8_t mode;

int stepDelay      = 0;

int startingDelay  = 4000;
int minDelay       = 2000;
int rampUpSpeed      = 10;
int rampDownSpeed      = 2;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);

  digitalWrite(ENABLE_PIN, LOW);
  digitalWrite(STEP_PIN, LOW);
  
  stepDelay = startingDelay;
  mode = MODE_RAMPUP;
}

void loop() {
  switch (mode) {
    case MODE_RAMPUP:
      stepDelay -= rampUpSpeed;
      if (stepDelay < minDelay) {
        stepDelay = minDelay;
        mode = MODE_RAMPDOWN;
      }
      break;
    case MODE_RAMPDOWN:
      stepDelay += rampDownSpeed;
      if (stepDelay > startingDelay) {
        stepDelay = startingDelay;
        mode = MODE_BRAKEING;

      }
      break;
    case MODE_BRAKEING:
      delay(500);
//      digitalWrite(ENABLE_PIN, HIGH);
      mode=MODE_STOPPED;
      break;      
 
    case MODE_STOPPED:
      delay(2000);
      mode=MODE_RAMPUP;
      stepDelay = startingDelay;
        digitalWrite(ENABLE_PIN, LOW);
      break;
  }
 
  if(mode == MODE_RAMPUP || mode == MODE_RAMPDOWN) {
    digitalWrite(STEP_PIN, HIGH);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(stepDelay);
  }
}
