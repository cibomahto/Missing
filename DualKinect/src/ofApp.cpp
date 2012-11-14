#include "ofApp.h"

void ofApp::setup() {
	ofSetLogLevel(OF_LOG_VERBOSE);
	
	kinect1.init();
	kinect2.init();
	
	kinect1.setDepthClipping(1000, 6000);
	kinect2.setDepthClipping(1000, 6000);
	
	kinect1.open();
	kinect2.open();
}

void ofApp::update() {
	kinect1.update();
	kinect2.update();
	
	if(kinect1.isFrameNew()) {
		ofVec3f down1 = kinect1.getRawAccel();
		if(smoothDown1 == ofVec3f()) {
			smoothDown1 = down1;
		} else {
			smoothDown1 = down1.interpolate(smoothDown1, .999);
		}
	}
	
	if(kinect2.isFrameNew()) {
		ofVec3f down2 = kinect2.getRawAccel();
		if(smoothDown2 == ofVec3f()) {
			smoothDown2 = down2;
		} else {
			smoothDown2 = down2.interpolate(smoothDown2, .999);
		}
	}
}

void ofApp::draw() {
	kinect1.drawDepth(0, 0);
	kinect1.draw(0, 480);
	kinect2.drawDepth(640, 0);
	kinect2.draw(640, 480);
	
	ofDrawBitmapString(ofToString(smoothDown1.getNormalized()), 10, 20);
	ofDrawBitmapString(ofToString(smoothDown2.getNormalized()), 640 + 10, 20);
}