#include "ofApp.h"

void ofApp::setup() {
	ofSetVerticalSync(true);
	kinect.init(false, false);
	kinect.setRegistration(false);
	kinect.open();
	
	gui.setup(280, 800);
	gui.addPanel("Settings");
	gui.addSlider("calibrationTime", 2, 1, 10);
	gui.addSlider("calibrationProgress", 0, 0, 1);
	gui.addToggle("calibrate", true);
	gui.addSlider("threshold", 16, 0, 64, true);
	gui.addSlider("upx", 0, -1, 1);
	gui.addSlider("upy", 0.49, -1, 1);
	gui.addSlider("upz", -.13, -1, 1);
	gui.addSlider("zoom", .16, 0, 1);
	gui.addToggle("showGrid", true);
	gui.addSlider("gridDivisions", 64, 1, 128, true);
	gui.addSlider("gridScale", 2500, 0, 4000);
	gui.addSlider("gridOffsetX", 0, -4000, 4000);
	gui.addSlider("gridOffsetY", 1500, -4000, 4000);
	gui.addSlider("presenceScale", 100, 1, 500);
	
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
			gui.setValueF("calibrationProgress", calibrationProgress / calibrationTime);
		}
		
		ofQuaternion quat;
		ofVec3f up(gui.getValueF("upx"), gui.getValueF("upy"), gui.getValueF("upz"));
		up.normalize();
		quat.makeRotate(ofVec3f(0, 0, 1), up);
		ofMatrix4x4 mat;
		quat.get(mat);
		
		int gridDivisions = gui.getValueF("gridDivisions");
		float gridScale = gui.getValueF("gridScale");
		presence.allocate(gridDivisions, gridDivisions, OF_IMAGE_GRAYSCALE);
		int presenceArea = gridDivisions * gridDivisions;
		for(int i = 0; i < presenceArea; i++) {
			presence.getPixels()[i] = 0;
		}
		
		int threshold = gui.getValueI("threshold");
		const unsigned short* rawDepthPixels = kinect.getRawDepthPixels();
		int width = kinect.getWidth(), height = kinect.getHeight();
		foregroundCloud.setMode(OF_PRIMITIVE_POINTS);
		foregroundCloud.clear();
		foregroundFlat.setMode(OF_PRIMITIVE_POINTS);
		foregroundFlat.clear();
		ofVec2f gridOffset(gui.getValueF("gridOffsetX"), gui.getValueF("gridOffsetY"));
		int kinectArea = kinect.getWidth() * kinect.getHeight();
		float presenceScale = presenceArea / (kinectArea * gui.getValueF("presenceScale"));
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
					flat += gridOffset;
					foregroundFlat.addVertex(flat);
					flat.x = (int) ofMap(flat.x, -gridScale, +gridScale, 0, gridDivisions, true);
					flat.y = (int) ofMap(flat.y, -gridScale, +gridScale, 0, gridDivisions, true);
					int i  = flat.y * gridDivisions + flat.x;
					if(presence.getPixels()[i] < 1) {
						presence.getPixels()[i] += presenceScale;
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
	
	if(gui.getValueB("showGrid")) {
		ofPushMatrix();
		ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
		float zoom = gui.getValueF("zoom");
		float gridScale = gui.getValueF("gridScale");
		int gridDivisions = gui.getValueF("gridDivisions");
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
		
		ofPushMatrix();
		ofSetColor(32);
		ofRotateY(90);
		ofDrawGridPlane(gridScale, gridDivisions, false);
		ofPopMatrix();
		
		ofSetColor(255);
		drawChunkyCloud(foregroundFlat);
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
