#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

const float stageSize = feetInchesToMillimeters(15, 8);

void ofApp::setup() {
	ofSetVerticalSync(true);
	kinect.init(false, false);
	kinect.setRegistration(false);
	kinect.open();
	
	osc.setup("sonoss-Mac-mini.local", 145145);
	
	gui.setup(280, 800);
	gui.addPanel("Grid");
	gui.addSlider("minArea", 2, 1, 40);
	gui.addSlider("maxArea", 40, 1, 40);
	gui.addSlider("contourThreshold", 5, 0, 64);
	gui.addToggle("showGrid", true);
	gui.addSlider("zoom", .16, 0, 1);
	gui.addSlider("gridDivisions", 64, 1, 128, true);
	gui.addSlider("presenceScale", 100, 1, 500);
	
	gui.addPanel("Camera");
	gui.addSlider("calibrationTime", 2, 1, 10);
	gui.addSlider("calibrationProgress", 0, 0, 1);
	gui.addToggle("calibrate", true);
	gui.addSlider("threshold", 16, 0, 64, true);
	gui.addSlider("upx", 0, -1, 1);
	gui.addSlider("upy", 1, -1, 1);
	gui.addSlider("upz", 0, -1, 1);
	gui.addSlider("gridOffsetX", 0, -4000, 4000);
	gui.addSlider("gridOffsetY", 1500, -4000, 4000);
	gui.addSlider("rotation", 0, -180, 180);
	
	calibrating = false;
	calibrationStart = 0;
	clearBackground = false;
	
	background.allocate(kinect.getWidth(), kinect.getHeight(), OF_IMAGE_GRAYSCALE);
	valid.allocate(kinect.getWidth(), kinect.getHeight(), OF_IMAGE_GRAYSCALE);
}

void ofApp::update() {
	float curTime = ofGetElapsedTimef();
	float calibrationProgress = curTime - calibrationStart;
	float calibrationTime = gui.getValueF("calibrationTime");
	if(calibrating) {
		if(calibrationProgress > calibrationTime) {
			calibrating = false;
			gui.setValueF("calibrate", false);
			gui.setValueF("calibrationProgress", 0);
		}
	} else if(gui.getValueF("calibrate")) {
		calibrationStart = curTime;
		calibrating = true;
		clearBackground = true;
	}
	
	kinect.update();
	if(kinect.isFrameNew()) {
		unsigned char* kinectPixels = kinect.getDepthPixels();
		unsigned char* validPixels = valid.getPixels();
		unsigned char* backgroundPixels = background.getPixels();
		float* presencePixels = presence.getPixels();
		int width = kinect.getWidth(), height = kinect.getHeight();
		int n = width * height;
		
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
			gui.setValueF("calibrationProgress", calibrationProgress / calibrationTime);
		}
		
		ofQuaternion orientationQuat;
		ofVec3f up(gui.getValueF("upx"), gui.getValueF("upy"), gui.getValueF("upz"));
		up.normalize();
		orientationQuat.makeRotate(ofVec3f(0, 0, 1), up);
		ofMatrix4x4 orientationMat;
		orientationQuat.get(orientationMat);
		
		int gridDivisions = gui.getValueF("gridDivisions");
		presence.allocate(gridDivisions, gridDivisions, OF_IMAGE_GRAYSCALE);
		int presenceArea = gridDivisions * gridDivisions;
		for(int i = 0; i < presenceArea; i++) {
			presence.getPixels()[i] = 0;
		}
		
		int threshold = gui.getValueI("threshold");
		const unsigned short* rawDepthPixels = kinect.getRawDepthPixels();
		for(int i = 0; i < n; i++) {
			int kinectPixel = kinectPixels[i];
			int backgroundPixel = backgroundPixels[i];
			bool far = abs(kinectPixel - backgroundPixel) > threshold;
			if(kinectPixel > 0 &&
				(backgroundPixel == 0 ||
				(backgroundPixel > 0 && far))) {
				validPixels[i] = 255;
			} else {
				validPixels[i] = 0;
			}
		}

		foregroundCloud.setMode(OF_PRIMITIVE_POINTS);
		foregroundCloud.clear();
		foregroundFlat.setMode(OF_PRIMITIVE_POINTS);
		foregroundFlat.clear();
		ofVec2f gridOffset(gui.getValueF("gridOffsetX"), gui.getValueF("gridOffsetY"));
		float rotation = gui.getValueF("rotation");
		float presenceScale = presenceArea / (float) (n * gui.getValueF("presenceScale"));
		int i = 0;
		for(int y = 0; y < height; y++) {
			for(int x = 0; x < width; x++) {
				if(x + 1 < width &&	validPixels[i] && validPixels[i + 1]) {
					ofVec3f cur = kinect.getWorldCoordinateAt(x, y, rawDepthPixels[i]);
					ofVec3f right = kinect.getWorldCoordinateAt(x + 1, y, rawDepthPixels[i + 1]);
					float curArea = cur.distance(right);
					curArea *= curArea;
					foregroundCloud.addVertex(cur);
					ofVec2f flat = orientationMat * cur;
					flat += gridOffset;
					flat.rotate(rotation);
					foregroundFlat.addVertex(flat);
					flat.x = (int) ofMap(flat.x, -stageSize / 2, +stageSize / 2, 0, gridDivisions, true);
					flat.y = (int) ofMap(flat.y, -stageSize / 2, +stageSize / 2, 0, gridDivisions, true);
					int i  = flat.y * gridDivisions + flat.x;
					if(presencePixels[i] < 1) {
						presencePixels[i] += curArea * presenceScale;
					}
				}
				i++;
			}
		}
		valid.update();
		presence.update();

		contourFinder.setMinArea(gui.getValueF("minArea"));
		contourFinder.setMaxArea(gui.getValueF("maxArea"));
		contourFinder.setThreshold(gui.getValueF("contourThreshold"));
		ofPixels presenceBytes;
		ofxCv::copy(presence, presenceBytes);
		contourFinder.findContours(presenceBytes);
		
		ofxOscMessage msg;
		msg.setAddress("/listeners");
		for(int i = 0; i < contourFinder.size(); i++) {
			ofVec2f cur = toOf(contourFinder.getCentroid(i));
			msg.addFloatArg(ofMap(cur.x, 0, gridDivisions, -stageSize / 2, +stageSize / 2));
			msg.addFloatArg(ofMap(cur.y, 0, gridDivisions, -stageSize / 2, +stageSize / 2));
		}
		osc.sendMessage(msg);
	}
}

void drawChunkyCloud(ofMesh& mesh, int innerRadius = 1, int outerRadius = 3) {
	ofPushStyle();
	ofSetColor(0);
	glPointSize(outerRadius);
	mesh.draw();
	ofSetColor(255);
	glPointSize(innerRadius);
	mesh.draw();
	ofPopStyle();
}

void ofApp::draw() {
	ofSetColor(255);
	if(kinect.isConnected()) {
		kinect.drawDepth(640, 0);
	}
	background.draw(0, 480);
	valid.draw(640, 480);
	
	if(gui.getValueB("showGrid")) {
		ofPushMatrix();
		ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
		float zoom = gui.getValueF("zoom");
		int gridDivisions = gui.getValueF("gridDivisions");
		ofScale(zoom, zoom);
		
		ofPushMatrix();
		float gridZoom = stageSize / gridDivisions;
		ofScale(gridZoom, gridZoom);
		ofTranslate(-gridDivisions / 2, -gridDivisions / 2);
		presence.bind();
		ofSetMinMagFilters(GL_NEAREST, GL_NEAREST);
		presence.unbind();
		presence.draw(0, 0);
		ofPushMatrix();
		ofPushStyle();
		ofTranslate(.5, .5);
		ofSetColor(magentaPrint);
		for(int i = 0; i < contourFinder.size(); i++) {
			contourFinder.getPolyline(i).draw();
		}
		ofPopStyle();
		ofPopMatrix();
		ofPopMatrix();
		
		ofSetColor(255);
		//drawChunkyCloud(foregroundFlat);
		ofPopMatrix();
	}
/*
	cam.begin();
	drawChunkyCloud(foregroundCloud);
	cam.end();*/
}

void ofApp::exit() {
	kinect.close();
}
