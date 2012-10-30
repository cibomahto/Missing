#include "ofApp.h"

void ofApp::setup() {
	ofSetVerticalSync(true);
	kinect.init(false, false);
	kinect.setRegistration(false);
	kinect.open();
	
	panel.setup(280, 600);
	panel.addPanel("Settings");
	panel.addSlider("calibrationTime", 2, 1, 10);
	panel.addSlider("calibrationProgress", 0, 0, 1);
	panel.addToggle("calibrate", true);
	panel.addSlider("threshold", 8, 0, 64, true);
	panel.addSlider("upx", 0.02, -1, 1);
	panel.addSlider("upy", 0.63, -1, 1);
	panel.addSlider("upz", -.41, -1, 1);
	panel.addSlider("zoom", .16, 0, 1);
	panel.addSlider("gridDivisions", 40, 1, 60, true);
	panel.addSlider("gridScale", 4000, 0, 4000);
	
	calibrating = false;
	calibrationStart = 0;
	clearBackground = false;
	
	result.allocate(kinect.getWidth(), kinect.getHeight(), OF_IMAGE_GRAYSCALE);
	background.allocate(kinect.getWidth(), kinect.getHeight(), OF_IMAGE_GRAYSCALE);
	presence.setAnchorPercent(.5, .5);
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
		
		ofQuaternion quat;
		ofVec3f up(panel.getValueF("upx"), panel.getValueF("upy"), panel.getValueF("upz"));
		up.normalize();
		quat.makeRotate(ofVec3f(0, 0, 1), up);
		ofMatrix4x4 mat;
		quat.get(mat);
		
		int gridDivisions = panel.getValueF("gridDivisions");
		float gridScale = panel.getValueF("gridScale");
		presence.allocate(gridDivisions, gridDivisions, OF_IMAGE_GRAYSCALE);
		n = gridDivisions * gridDivisions;
		for(int i = 0; i < n; i++) {
			presence.getPixels()[i] = 0;
		}
		
		int threshold = panel.getValueI("threshold");
		const unsigned short* rawDepthPixels = kinect.getRawDepthPixels();
		int width = kinect.getWidth(), height = kinect.getHeight();
		foregroundCloud.setMode(OF_PRIMITIVE_POINTS);
		foregroundCloud.clear();
		foregroundFlat.setMode(OF_PRIMITIVE_POINTS);
		foregroundFlat.clear();
		int i = 0;
		for(int y = 0; y < height; y++) {
			for(int x = 0; x < width; x++) {
				int kinectPixel = kinectPixels[i];
				int backgroundPixel = backgroundPixels[i];
				bool far = abs(kinectPixel - backgroundPixel) > threshold;
				if(far && kinectPixel > 0 && backgroundPixel > 0) {
					resultPixels[i] = 255;
					ofVec3f cur = kinect.getWorldCoordinateAt(x, y, rawDepthPixels[i]);
					foregroundCloud.addVertex(cur);
					
					ofVec2f flat = mat * cur;
					foregroundFlat.addVertex(flat);
					flat.x = (int) ofMap(flat.x, -gridScale, +gridScale, 0, gridDivisions, true);
					flat.y = (int) ofMap(flat.y, -gridScale, +gridScale, 0, gridDivisions, true);
					int i  = flat.y * gridDivisions + flat.x;
					if(presence.getPixels()[i] < 255) {
						presence.getPixels()[i]++;
					}
				} else {
					resultPixels[i] = 0;
				}
				i++;
			}
		}
		
		result.update();
		presence.update();
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
	kinect.drawDepth(640, 0);
	background.draw(0, 480);
	result.draw(640, 480);
	
	ofPushMatrix();
	ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
	float zoom = panel.getValueF("zoom");
	float gridScale = panel.getValueF("gridScale");
	int gridDivisions = panel.getValueF("gridDivisions");
	ofScale(zoom, zoom);
	
	ofPushMatrix();
	float gridZoom = 2 * gridScale / gridDivisions;
	ofScale(gridZoom, gridZoom);
	presence.bind();
	ofSetMinMagFilters(GL_NEAREST, GL_NEAREST);
	presence.unbind();
	presence.draw(0, 0);
	ofPopMatrix();
	
	ofDrawGrid(gridScale, gridDivisions, false, false, false, true);
	drawChunkyCloud(foregroundFlat);
	ofPopMatrix();
/*
	cam.begin();
	drawChunkyCloud(foregroundCloud);
	cam.end();*/
}

void ofApp::exit() {
	kinect.close();
}
