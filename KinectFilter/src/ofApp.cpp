#include "ofApp.h"

using namespace cv;
using namespace ofxCv;

void ofApp::setup() {
	kinect.init(false, false);
	kinect.setDepthClipping(500, 2000);
	kinect.open();
	
	filter.setDelay(8);
}

void ofApp::update() {
	kinect.update();
	if(kinect.isFrameNew()) {
		filter.update(kinect.getDepthPixelsRef());
	}
}

void ofApp::draw() {
	kinect.drawDepth(0, 0);
	filter.getMasked().draw(640, 0);
}