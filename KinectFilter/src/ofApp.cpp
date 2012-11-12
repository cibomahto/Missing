#include "ofApp.h"

using namespace cv;
using namespace ofxCv;

const unsigned int SimpleHysteresis::delay = 8;

void ofApp::setup() {
	kinect.init(false, false);
	kinect.setDepthClipping(500, 2000);
	kinect.open();
	
	kinectFrameCount = 0;
	hysteresis.resize(kinect.getWidth() * kinect.getHeight());
	mask.allocate(kinect.getWidth(), kinect.getHeight(), OF_IMAGE_GRAYSCALE);
}

void ofApp::update() {
	kinect.update();
	if(kinect.isFrameNew()) {
		unsigned char* kinectPixels = kinect.getDepthPixels();
		unsigned char* maskPixels = mask.getPixels();
		int n = kinect.getWidth() * kinect.getHeight();
		for(int i = 0; i < n; i++) {
			maskPixels[i] = hysteresis[i].update(kinectPixels[i] > 0, kinectFrameCount) ? 255 : 0;
		}
		mask.update();
		
		bitwise_and(kinect.getDepthPixelsRef(), mask, masked);
		masked.update();
		
		kinectFrameCount++;
	}
}

void ofApp::draw() {
	kinect.drawDepth(0, 0);
	mask.draw(640, 0);
	masked.draw(0, 480);
}