#include "Speaker.h"

#include "ofxAssimpModelLoader.h"
#include "Conversion.h"
#include "MiniFont.h"

ofVec2f play3Orientation = ofVec2f(-1, 0);
float mountOffset = centimetersToMillimeters(14);

ofVboMesh Speaker::play3;
int Speaker::count = 0;
void Speaker::setupMesh() {
	ofxAssimpModelLoader model;
	model.loadModel("play3.obj");
	play3 = model.getMesh(0);
	scale(play3, centimetersToMillimeters(1));
}

ofVec3f referencePoint;
bool distanceToReference(const ofVec3f& a, const ofVec3f& b) {
	return a.distance(referencePoint) < b.distance(referencePoint);
}

float movingHysteresis = 1;
float minHysteresis = 5;
float stillHysteresis = 30;
float backwardsHysteresis = 150;
float maximumAngle = 170;
float curHysteresis;
float stillWait = .99;
float smoothRate = .1;
float actualSmoothRate = .1;
float maxSpeed = .5;

void Speaker::setup(ofVec3f position, ofMesh& wiresCloud, int posMin, int posCenter, int posMax) {
	this->position = position;
	this->posMin = posMin;
	this->posCenter = posCenter;
	this->posMax = posMax;
	float crimpPosition = position.z + mountOffset;
	label = ofToString(count++) + "/" + ofToString((int) millimetersToInches(crimpPosition)) + "in";
	
	vector<ofVec3f> centers = wiresCloud.getVertices();
	referencePoint = position;
	ofSort(centers, distanceToReference);
	ofVec2f a = centers[0], b = centers[1], c = centers[2];
	wires.addVertex(a);
	wires.addVertex(b);
	wires.addVertex(c);
	// could also determine the real orientation from wires, but it's more work
	orientation = -position;
	orientation.normalize();
	baseRotation = play3Orientation.angle(orientation);
	
	prevMoving = false;
	moving = false;
	prevAngle = 0;
	currentAngle = 0;
	smoothAngle = 0;
	actualAngle = 0;
}

void drawAngle(float angle, ofColor color) {
	ofPushMatrix();
	ofPushStyle();
	ofSetColor(color);
	ofRotateZ(angle);
	ofLine(ofVec2f(0, 0), play3Orientation * feetToMillimeters(1));
	ofPopStyle();
	ofPopMatrix();
}

void Speaker::draw(bool showLabel) {
	ofPushMatrix();
	ofLine(ofVec2f(position), ofVec2f(position) + orientation * feetToMillimeters(1));
	
	wires.drawWireframe();
	ofTranslate(0, 0, position.z + mountOffset);
	wires.drawWireframe();
	ofPopMatrix();

	ofPushMatrix();
	
	ofPushStyle();
	ofSetColor(255);
	if(showLabel) {
		ofDrawBitmapString(label, position.x, position.y);
	}
	ofPopStyle();
	
	ofTranslate(position);
	ofRotateZ(baseRotation);

	ofLine(ofVec2f(0, 0), play3Orientation * feetToMillimeters(1));
	
	drawAngle(actualAngle, ofColor::fromHex(0xffee00));
	drawAngle(smoothAngle, ofColor::fromHex(0x00abec));
	drawAngle(currentAngle, ofColor::fromHex(0xec008c));
	
	ofRotateZ(actualAngle);
	play3.draw();

	ofPopMatrix();
}

float Speaker::getAngle() const {
	return actualAngle;
}

float Speaker::getPosMin() const {
	return posMin;
}

float Speaker::getPosCenter() const {
	return posCenter;
}

float Speaker::getPosMax() const {
	return posMax;
}


ofVec2f getClosestPoint(vector<ofVec2f>& points, ofVec2f target) {
	float minimum;
	ofVec2f closest;
	for(int i = 0; i < points.size(); i++) {
		float cur = points[i].distance(target);
		if(i == 0 || cur < minimum) {
			minimum = cur;
			closest = points[i];
		}
	}
	return closest;
}

void Speaker::update(vector<ofVec2f>& listeners) {
	ofVec2f closest = getClosestPoint(listeners, position);
	ofVec2f actual = closest - position;
	float realAngle = orientation.angle(actual);
	
	if((realAngle < -backwardsHysteresis && prevAngle > +backwardsHysteresis) ||
		(prevAngle < -backwardsHysteresis && realAngle > +backwardsHysteresis)) {
		currentAngle = prevAngle;
	} else {
		currentAngle = realAngle;
	}
	currentAngle = ofClamp(currentAngle, -maximumAngle, +maximumAngle);
	prevAngle = currentAngle;
	
	float nextAngle = ofLerp(smoothAngle, currentAngle, smoothRate);
	smoothAngle += ofClamp(nextAngle - smoothAngle, -maxSpeed, maxSpeed);
	
	if(fabsf(currentAngle - actualAngle) > curHysteresis) {
		actualAngle = ofLerp(actualAngle, smoothAngle, actualSmoothRate);
		curHysteresis = movingHysteresis;
		moving = true;
	} else {
		if(prevMoving) {
			curHysteresis = stillHysteresis;
		} else {
			curHysteresis = MAX(minHysteresis, curHysteresis * stillWait);
		}
		moving = false;
	}
	prevMoving = moving;
}
