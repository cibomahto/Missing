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
}

void ofApp::draw() {
	kinect1.drawDepth(0, 0);
	kinect1.draw(0, 480);
	kinect2.drawDepth(640, 0);
	kinect2.draw(640, 480);
}