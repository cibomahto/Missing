#include "ofApp.h"

void ofApp::setup() {
	ofSetVerticalSync(true);
	kinect.init(false, false);
	kinect.open();
	
	panel.setup(280, 400);
	panel.addPanel("Settings");
	panel.addSlider("calibrationTime", 4, 1, 10);
	panel.addSlider("calibrationProgress", 0, 0, 1);
	panel.addToggle("calibrate");
	panel.addSlider("threshold", 4, 0, 64, true);
	
	calibrating = false;
	calibrationStart = 0;
	clearBackground = false;
	
	result.allocate(kinect.getWidth(), kinect.getHeight(), OF_IMAGE_GRAYSCALE);
	background.allocate(kinect.getWidth(), kinect.getHeight(), OF_IMAGE_GRAYSCALE);
}

void ofApp::update() {
	float curTime = ofGetElapsedTimef();
	float calibrationProgress = curTime - calibrationStart;
	float calibrationTime = panel.getValueF("calibrationTime");
	if(calibrating) {
		if(calibrationProgress > calibrationTime) {
			calibrating = false;
			panel.setValueF("calibrate", false);
			panel.setValueF("calibrationProgress", 0);
		}
	} else if(panel.getValueF("calibrate")) {
		calibrationStart = curTime;
		calibrating = true;
		clearBackground = true;
	}

	kinect.update();
	if(kinect.isFrameNew()) {
		unsigned char* kinectPixels = kinect.getDepthPixels();
		unsigned char* resultPixels = result.getPixels();
		unsigned char* backgroundPixels = background.getPixels();
		int n = kinect.getWidth() * kinect.getHeight();
		
		if(clearBackground) {
			for(int i = 0; i < n; i++) {
				backgroundPixels[i] = 0;
			}
			background.update();
			clearBackground = false;
		}
		
		if(calibrating) {
			for(int i = 0; i < n; i++) {
				if(kinectPixels[i] != 0) {
					if(backgroundPixels[i] == 0) {
						backgroundPixels[i] = kinectPixels[i];
					}	else {
						backgroundPixels[i] = (backgroundPixels[i] + kinectPixels[i]) / 2;
					}
				}
			}
			background.update();
			panel.setValueF("calibrationProgress", calibrationProgress / calibrationTime);
		}
		
		int threshold = panel.getValueI("threshold");
		for(int i = 0; i < n; i++) {
			int kinectPixel = kinectPixels[i];
			int backgroundPixel = backgroundPixels[i];
			bool far = abs(kinectPixel - backgroundPixel) > threshold;
			if(far && kinectPixel > 0 && backgroundPixel > 0) {
				resultPixels[i] = 255;
			} else {
				resultPixels[i] = 0;
			}
		}
		
		result.update();
	}
}

void ofApp::draw() {
	ofSetColor(255);
	kinect.drawDepth(640, 0);
	background.draw(0, 480);
	result.draw(640, 480);
}

void ofApp::exit() {
	kinect.close();
}
