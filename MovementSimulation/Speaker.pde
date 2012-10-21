boolean debug = false;

class Speaker {
  float currentAngle, smoothAngle, actualAngle;
  float smoothRate = .1;
  float actualSmoothRate = .1;
  float maxSpeed = .01;
  
  boolean prevMoving = false, moving = false;
  float movingHysteresis = radians(1);
  float minHysteresis = radians(5);
  float stillHysteresis = radians(30);
  float curHysteresis;
  float stillWait = .99;
  
  float x, y;
  Speaker(float x, float y) {
    this.x = x;
    this.y = y;
  }
  
  void drawAngle(float angle) {
    pushMatrix();
    rotate(angle);
    line(0, 0, width, 0);
    popMatrix();
  }
  
  void drawSpeaker() {
    pushMatrix();
    scale(3, 3);
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
    currentAngle = atan2(my - y, mx - x);
    
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
        curHysteresis = max(minHysteresis, curHysteresis * stillWait);
      }
      moving = false;
    }
    prevMoving = moving;
  }
  
  void update() {
    followMouse();
  }
  
  void draw() {
    pushMatrix();
    translate(x, y);
    if(debug) {
      stroke(255);
      drawAngle(currentAngle);
      drawAngle(smoothAngle);
    }
    rotate(actualAngle);
    if(debug) {
      stroke(255, 0, 0);
      drawAngle(-curHysteresis);
      drawAngle(+curHysteresis);
    }
    stroke(moving ? color(255, 0, 0) : color(255));
    drawSpeaker();
    popMatrix();
  }
};
