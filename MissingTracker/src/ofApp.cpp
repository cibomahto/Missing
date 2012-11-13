#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

const float stageSize = feetInchesToMillimeters(15, 8);

void ofApp::setup() {
	ofSetVerticalSync(true);
	ofSetLogLevel(OF_LOG_VERBOSE);
	
	kinect.setup();
	
	osc.setup("sonoss-Mac-mini.local", 145145);
	
	gui.setup(280, 800);
	gui.addPanel("Analysis");
	gui.addSlider("calibrationTime", 2, 1, 10);
	gui.addSlider("calibrationProgress", 0, 0, 1);
	gui.addToggle("calibrate", true);
	gui.addToggle("showCloud");
	gui.addToggle("showFlatCloud");
	gui.addToggle("showGrid", true);
	gui.addSlider("zoom", .16, 0, 1);
	gui.addSlider("minArea", 0, 0, 10);
	gui.addSlider("maxArea", 40, 0, 40);
	gui.addSlider("contourThreshold", 5, 0, 64);
	gui.addSlider("gridDivisions", 64, 1, 128, true);
	gui.addSlider("presenceScale", 100, 1, 500);
	
	gui.addPanel("Kinect");
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
	
	ofResetElapsedTimeCounter();
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
	
	gui.setValueF("calibrationProgress", calibrationProgress / calibrationTime);
	
	if(clearBackground) {
		kinect.setClearBackground();
		clearBackground = false;
	}
	kinect.setCalibrating(calibrating);
	kinect.setUpVector(ofVec3f(gui.getValueF("upx"), gui.getValueF("upy"), gui.getValueF("upz")));
	kinect.setBackgroundThreshold(gui.getValueI("threshold"));
	kinect.setOffset(ofVec2f(gui.getValueF("gridOffsetX"), gui.getValueF("gridOffsetY")));
	kinect.setRotation(gui.getValueF("rotation"));
	
	kinect.update();
	if(kinect.isFrameNew()) {
		// for both kinects
		foregroundCloud.setMode(OF_PRIMITIVE_POINTS);
		foregroundCloud.clear();
		int gridDivisions = gui.getValueF("gridDivisions");
		presence.allocate(gridDivisions, gridDivisions, OF_IMAGE_GRAYSCALE);
		float* presencePixels = presence.getPixels();
		int presenceArea = gridDivisions * gridDivisions;
		for(int i = 0; i < presenceArea; i++) {
			presencePixels[i] = 0;
		}
		int n = 640 * 480;
		float presenceScale = presenceArea / (float) (n * gui.getValueF("presenceScale"));
		
		// with each kinect
		vector<ofVec3f>& meshVertices = kinect.getMesh().getVertices();
		vector<float>& meshArea = kinect.getMeshArea();
		foregroundCloud.addVertices(meshVertices);
		for(int i = 0; i < meshArea.size(); i++) {
			ofVec3f& cur = meshVertices[i];
			int x = (cur.x + stageSize / 2) * gridDivisions / stageSize;
			int y = (cur.y + stageSize / 2) * gridDivisions / stageSize;
			int j  = y * gridDivisions + x;
			if(presencePixels[j] < 1) {
				presencePixels[j] += meshArea[i] * presenceScale;
			}
		}
		
		foregroundFlat.setMode(OF_PRIMITIVE_POINTS);
		foregroundFlat.clear();
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
	if(kinect.getKinect().isConnected()) {
		kinect.getKinect().drawDepth(640, 0);
		kinect.getBackground().draw(0, 480);
		kinect.getValid().draw(640, 480);
	}
	
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
		
		if(gui.getValueB("showFlatCloud")) {
			ofSetColor(255);
			drawChunkyCloud(foregroundFlat);
		}
		ofPopMatrix();
	}
	
	if(gui.getValueB("showCloud")) {
		cam.begin();
		drawChunkyCloud(foregroundCloud);
		cam.end();
	}
}

