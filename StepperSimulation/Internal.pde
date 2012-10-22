int stepperStepCount = 180;

boolean HIGH = true;
boolean LOW = false;
boolean COUNTERCLOCKWISE = HIGH;
boolean CLOCKWISE = LOW;

int stepperPosition = 0;
boolean stepperDirection = CLOCKWISE;
boolean previousStepperValue = LOW;
void digitalWriteStep(boolean value) {
  if (value == LOW && previousStepperValue == HIGH) {
    if (stepperDirection == CLOCKWISE) {
      stepperPosition++;
    } 
    else {
      stepperPosition--;
    }
  }
  previousStepperValue = value;
}

void digitalWriteDir(boolean value) {
  stepperDirection = value;
}

int analogReadPosition() {
  return (int) map(stepperPosition, 0, stepperStepCount, 0, 1024);
}

float getStepperRotation() {
  return map(stepperPosition, 0, stepperStepCount, 0, TWO_PI);
}

void drawStepper(float radius) {
  pushMatrix();
  pushStyle();
  stroke(255);
  noFill();
  ellipse(0, 0, radius * 2, radius * 2);
  rotate(getStepperRotation());
  line(0, 0, radius, 0);

  scale(10, 10);
  translate(-7.5, 0);
  beginShape();
  vertex(15, 6.5);
  vertex(0, 5);
  vertex(0, -5);
  vertex(15, -6.5);
  endShape(CLOSE);

  popStyle();
  popMatrix();
}

void digitalWrite(int pin, boolean value) {
  if (pin == STEP_PIN) {
    digitalWriteStep(value);
  } 
  else if (pin == DIR_PIN) {
    digitalWriteDir(value);
  }
}

void analogWrite(int pin, int value) {
  if (pin == CURRENT_REF_PIN) {
    // should control torque
  }
}

int analogRead(int pin) {
  if (pin == POSITION_PIN) {
    return analogReadPosition();
  }
  return 0;
}

