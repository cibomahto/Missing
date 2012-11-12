#include "ofApp.h"

void ofApp::setup() {
	kinect1.init(false, false);
	kinect2.init(false, false);
	
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
	kinect2.drawDepth(640, 0);
}