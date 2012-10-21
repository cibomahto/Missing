// possible problem: what if we are sending more steps than the motor can handle?
// how does it respond if we tell it to step 10 times but in the process it can only do 8?
// would also make sense to change the current as the motor is accelerating/decelerating?

PFont font;
int fontSize = 16;
float[] OCR2AHistory;
int OCR2At = 0;
void setup() {
  size(512, 512);
  noSmooth();
  font = createFont("", fontSize);
  textFont(font, fontSize);
  textAlign(LEFT, TOP);
  strokeWeight(0);
  OCR2AHistory = new float[width];
}

long lastISR = 0;
int OCR2A = 0;
void draw() {
  int curISR = millis();
  boolean didISR = false;
  if(curISR - lastISR > OCR2A) {
    ISR();
    didISR = true;
    lastISR = curISR;
  }
  OCR2AHistory[OCR2At] = OCR2A;
  OCR2At = (OCR2At + 1) % OCR2AHistory.length;
  
  background(0);
  
  pushMatrix();
  translate(20, 20);
  text("OCR2A: " + OCR2A + " " + (didISR ? "ISR" : ""), 0, 0);
  int y = 0;
  text("targetPos: " + targetPos, 0, y+=fontSize);
  text("currentPos: " + currentPos, 0, y+=fontSize);
  text("hysteresis: " + hysteresis, 0, y+=fontSize);
  text("moving: " + moving, 0, y+=fontSize);
  popMatrix();
  
  pushMatrix();
  translate(width / 2, height / 2);
  drawStepper(128);
  popMatrix();
  
  translate(0, height - maxStepDelay - 10);
  noFill();
  stroke(128);
  line(0, minStepDelay, width, minStepDelay);
  line(0, maxStepDelay, width, maxStepDelay);
  stroke(255);
  beginShape();
  for(int i = 0; i < OCR2AHistory.length; i++) {
    vertex(i, OCR2AHistory[i]);
  }
  endShape();
}

void keyPressed() {
  if(key == ' ') {
    targetPos = (int)random(0,1024);
  }
}

// - - -- essentially arduino code below here

// #define
int STEP_PIN = 2;
int DIR_PIN = 3;
int POSITION_PIN = 0;
int CURRENT_REF_PIN = 9;

// const
int movingCurrent = 25;
int holdingCurrent = 10;
int stepDelayAcceleration = 2;
int stepDelayDeceleration = 2;
int decelerateOffset = 80;
int currentStepDelay = 200;
int minStepDelay = 40;
int maxStepDelay = 140;

// variable
int targetPos = 0;
int currentPos = 0;
int hysteresis = 20;
boolean moving = false;

void ISR() {
  currentPos = analogRead(POSITION_PIN);

  if (abs(targetPos - currentPos) > hysteresis) {
    // If we should be moving, make a move    
    analogWrite(CURRENT_REF_PIN, movingCurrent);

    if (targetPos > currentPos) {
      digitalWrite(DIR_PIN, LOW);
    }
    else {
      digitalWrite(DIR_PIN, HIGH);
    }

    moving = true;
    digitalWrite(STEP_PIN, HIGH);
    digitalWrite(STEP_PIN, LOW);

    // Now, determine if we should be speeding up/slowing down
    // note we are not considering directioN!!
    if (abs(targetPos - currentPos) > decelerateOffset) {

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
  else {
    // We are not moving.
    analogWrite(CURRENT_REF_PIN, holdingCurrent);

    moving = false;
    currentStepDelay = maxStepDelay;
  }

  OCR2A = currentStepDelay;
}

