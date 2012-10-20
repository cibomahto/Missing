float currentAngle, smoothAngle, actualAngle;
float smoothRate = .1;
float actualSmoothRate = .1;
float maxSpeed = .01;

boolean prevMoving = false, moving = false;
float movingHysteresis = radians(1);
float stillHysteresis = radians(30);
float curHysteresis;
float stillWait = .99;

void setup() {
  size(512, 512);
  strokeWeight(0);
  stroke(255);
  noFill();
  noSmooth();
}

void drawSpeaker() {
  pushMatrix();
  scale(10, 10);
  translate(-7.5, 0);
  beginShape();
  vertex(15, 6.5);
  vertex(0, 5);
  vertex(0, -5);
  vertex(15, -6.5);
  endShape(CLOSE);
  popMatrix();
}

void followMouse() {
  currentAngle = atan2(mouseY - width / 2, mouseX - height / 2);
  
  float nextAngle = lerp(smoothAngle, currentAngle, smoothRate);
  smoothAngle += constrain(nextAngle - smoothAngle, -maxSpeed, maxSpeed);
  
  if(abs(currentAngle - actualAngle) > curHysteresis) {
    actualAngle = lerp(actualAngle, smoothAngle, actualSmoothRate);
    curHysteresis = movingHysteresis;
    moving = true;
  } else {
    if(prevMoving) {
      curHysteresis = stillHysteresis;
    } else {
      curHysteresis = max(movingHysteresis, curHysteresis * stillWait);
    }
    moving = false;
  }
  prevMoving = moving;
}

void drawMouse() {
  ellipse(mouseX, mouseY, 8, 8);
}

void drawAngle(float angle) {
  pushMatrix();
  rotate(angle);
  line(0, 0, width, 0);
  popMatrix();
}

void draw() {
  background(0);
  followMouse();
  drawMouse();
  
  translate(width / 2, height / 2);
  
  pushMatrix();
  rotate(actualAngle);
  stroke(255, 0, 0);
  drawAngle(-curHysteresis);
  drawAngle(+curHysteresis);
  stroke(255);
  drawSpeaker();
  popMatrix();
  
  drawAngle(currentAngle);
  drawAngle(smoothAngle);
}
