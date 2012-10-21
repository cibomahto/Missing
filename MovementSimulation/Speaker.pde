boolean debug = false;

class Speaker {
  float prevAngle, currentAngle, smoothAngle, actualAngle;
  float smoothRate = .1;
  float actualSmoothRate = .1;
  float maxSpeed = .01;
  
  boolean prevMoving = false, moving = false;
  float movingHysteresis = radians(1);
  float minHysteresis = radians(5);
  float stillHysteresis = radians(30);
  float backwardsHysteresis = PI - (PI / 8);
  float curHysteresis;
  float stillWait = .99;
  
  float x, y, orientation;
  Speaker(float x, float y, float tx, float ty) {
    this.x = x;
    this.y = y;
    this.orientation = atan2(ty - y, tx - x);
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
    PVector actual = new PVector(my - y, mx - x);
    actual.rotate(orientation);
    float realAngle = atan2(actual.x, actual.y);
    if((realAngle < -backwardsHysteresis && prevAngle > +backwardsHysteresis) ||
      (prevAngle < -backwardsHysteresis && realAngle > +backwardsHysteresis)) {
      currentAngle = prevAngle;
    } else {
      currentAngle = realAngle;
    }
    prevAngle = currentAngle;
    
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
    rotate(orientation);
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
