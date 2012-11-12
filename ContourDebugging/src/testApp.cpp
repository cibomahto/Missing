#include "testApp.h"

using namespace ofxCv;
using namespace cv;

void testApp::setup() {
	ofSetVerticalSync(true);
	img.loadImage("in.png");
}

void testApp::update() {
	Mat imgMat = toCv(img);
	cv::findContours(imgMat, allContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
}

void testApp::draw() {
	ofScale(20, 20);
	ofSetColor(128);
	img.bind();
	ofSetMinMagFilters(GL_NEAREST, GL_NEAREST);
	img.draw(0, 0);
	img.unbind();
	ofSetColor(255, 0, 0);
	ofNoFill();
	for(int i = 0; i < allContours.size(); i++) {
		ofBeginShape();
		for(int j = 0; j < allContours[i].size(); j++) {
			ofVertex(allContours[i][j].x, allContours[i][j].y);
		}
		ofEndShape();
	}
}
