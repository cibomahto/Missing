#include "MissingTracker.h"

using namespace ofxCv;
using namespace cv;

const float stageSize = feetInchesToMillimeters(15, 8);
const string kinectSerialSw = "A00363A04112112A";
const string kinectSerialNe = "A00362913019109A";
const int gridSize = 1024;

void MissingTracker::setup() {
	ofSetVerticalSync(true);
	ofSetLogLevel(OF_LOG_VERBOSE);
	
	osc.setup("thexxexhibit.local", 145145);
	
	gui.setup(280, 800);
	gui.addPanel("Analysis");
	gui.addToggle("enableKinect", false);
	gui.addSlider("calibrationTime", 2, 1, 10);
	gui.addSlider("calibrationProgress", 0, 0, 1);
	gui.addToggle("calibrate", true);
	gui.addToggle("showCloud");
	gui.addSlider("maxStretch", 100, 0, 500);
	gui.addSlider("zClipMin", 200, 0, 3000);
	gui.addSlider("zClipMax", 2600, 0, 3000);
	gui.addSlider("minAreaRadius", 0, 0, 10);
	gui.addSlider("maxAreaRadius", 40, 0, 40);
	gui.addSlider("contourThreshold", 5, 0, 64);
	gui.addSlider("gridDivisions", 64, 1, 256, true);
	gui.addSlider("presenceBlur", 0, 0, 16, true);
	gui.addSlider("presenceFade", 0, 0, 10);
	gui.addSlider("presenceScale", 32, 1, 64);
	
	gui.addPanel("KinectSw");
	gui.addSlider("thresholdSw", 16, 0, 128, true);
	gui.addSlider("gridOffsetXSw", 0, -4000, 4000);
	gui.addSlider("gridOffsetYSw", 0, -4000, 4000);
	gui.addSlider("rotationSw", 0, -180, 180);
	
	gui.addPanel("KinectNe");
	gui.addSlider("thresholdNe", 16, 0, 128, true);
	gui.addSlider("gridOffsetXNe", 0, -4000, 4000);
	gui.addSlider("gridOffsetYNe", 0, -4000, 4000);
	gui.addSlider("rotationNe", 0, -180, 180);
	
	gui.loadSettings("settings.xml");
	
	connected = false;
	calibrating = false;
	calibrationStart = 0;
	clearBackground = false;
}

void MissingTracker::update() {
	if(gui.getValueB("enableKinect") && !connected) {
		kinectSw.setup(kinectSerialSw);
		kinectNe.setup(kinectSerialNe);
		connected = true;
		ofResetElapsedTimeCounter();
	}

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
	
	vector<KinectTracker*> kinects;
	kinects.push_back(&kinectSw);
	kinects.push_back(&kinectNe);
	vector<string> suffixes;
	suffixes.push_back("Sw");
	suffixes.push_back("Ne");
	bool newFrame = false;
	for(int i = 0; i < kinects.size(); i++) {
		KinectTracker* kinect = kinects[i];
		string& suffix = suffixes[i];
		if(clearBackground) {
			kinect->setClearBackground();
		}
		kinect->setCalibrating(calibrating);
		kinect->setZClip(gui.getValueF("zClipMin"), gui.getValueF("zClipMax"));
		kinect->setMaxStretch(gui.getValueF("maxStretch"));
		kinect->setBackgroundThreshold(gui.getValueI("threshold" + suffix));
		kinect->setOffset(ofVec2f(gui.getValueF("gridOffsetX" + suffix), gui.getValueF("gridOffsetY" + suffix)));
		kinect->setRotation(gui.getValueF("rotation" + suffix));
		
		kinect->update();
		if(kinect->isFrameNew()) {
			newFrame = true;
		}
	}
	clearBackground = false;
	
	if(newFrame) {
		int gridDivisions = gui.getValueF("gridDivisions");
		ofFloatImage curPresence;
		curPresence.allocate(gridDivisions, gridDivisions, OF_IMAGE_GRAYSCALE);
		imitate(presence, curPresence);
		float* curPresencePixels = curPresence.getPixels();
		int presenceArea = gridDivisions * gridDivisions;
		for(int i = 0; i < presenceArea; i++) {
			curPresencePixels[i] = 0;
		}
		int n = 640 * 480;
		float presenceScale = presenceArea / (float) (n * gui.getValueF("presenceScale"));
		
		for(int i = 0; i < kinects.size(); i++) {
			KinectTracker* kinect = kinects[i];
			vector<ofVec3f>& meshVertices = kinect->getMesh().getVertices();
			vector<float>& meshArea = kinect->getMeshArea();
			for(int i = 0; i < meshArea.size(); i++) {
				ofVec3f& cur = meshVertices[i];
				int x = (cur.x + stageSize / 2) * gridDivisions / stageSize;
				int y = (cur.y + stageSize / 2) * gridDivisions / stageSize;
				int j  = y * gridDivisions + x;
				if(x >= 0 && x < gridDivisions && y >= 0 && y < gridDivisions && curPresencePixels[j] < 1) {
					curPresencePixels[j] += meshArea[i] * presenceScale;
				}
			}
		}
		int presenceBlur = gui.getValueI("presenceBlur");
		if(presenceBlur > 0) {
			blur(curPresence, presenceBlur);
		}
		
		float contourThreshold = gui.getValueF("contourThreshold");
		Mat curPresenceMat = toCv(curPresence);
		Mat presenceMat = toCv(presence);
		float learningTime = gui.getValueF("presenceFade");
		if(learningTime > 0) {
			float learningRate = 1. - powf(1. - (contourThreshold / 255.), 1. / learningTime);
			accumulateWeighted(curPresenceMat, presenceMat, learningRate);
		} else {
			presence = curPresence;
		}
		presence.update();
	
		// wrapping up analysis
		contourFinder.setMinAreaRadius(gui.getValueF("minAreaRadius"));
		contourFinder.setMaxAreaRadius(gui.getValueF("maxAreaRadius"));
		contourFinder.setThreshold(contourThreshold);
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

void drawChunkyCloud(ofMesh& mesh, ofColor color, int innerRadius = 1, int outerRadius = 3) {
	ofPushStyle();
	ofSetColor(0);
	glPointSize(outerRadius);
	mesh.draw();
	ofSetColor(color);
	glPointSize(innerRadius);
	mesh.draw();
	ofPopStyle();
}

void MissingTracker::draw() {
	ofBackground(255);
	ofSetColor(255);
	
	if(presence.isAllocated()) {
		ofPushMatrix();
		int gridDivisions = gui.getValueF("gridDivisions");
		float gridScale = (float) gridSize / gridDivisions;
		ofScale(gridScale, gridScale, gridScale);
		
		presence.bind();
		ofSetMinMagFilters(GL_NEAREST, GL_NEAREST);
		presence.unbind();
		presence.draw(0, 0);
		
		ofPushStyle();
		ofSetColor(cyanPrint);
		ofLine(gridDivisions / 2, 0, gridDivisions / 2, gridDivisions);
		ofLine(0, gridDivisions / 2, gridDivisions, gridDivisions / 2);
		ofPopStyle();
		
		ofPushMatrix();
		ofPushStyle();
		ofTranslate(.5, .5);
		ofSetColor(cyanPrint);
		for(int i = 0; i < contourFinder.size(); i++) {
			contourFinder.getPolyline(i).draw();
			ofVec2f position = toOf(contourFinder.getCentroid(i));
			ofDrawBitmapString(ofToString(contourFinder.getLabel(i)), position);
		}
		ofPopStyle();
		ofPopMatrix();
		ofPopMatrix();
		
		if(gui.getValueB("showCloud")) {
			ofPushView();
			ofViewport(0, 0, gridSize, gridSize);
			ofSetupScreenOrtho(gridSize, gridSize, OF_ORIENTATION_DEFAULT, true, -10000, 10000);
			ofTranslate(gridSize / 2, gridSize / 2);
			float cloudScale = (float) gridSize / stageSize;
			// could do this scale during the point remapping process, then presence calc is simpler
			ofScale(cloudScale, cloudScale, cloudScale);
			drawChunkyCloud(kinectSw.getMesh(), magentaPrint);
			drawChunkyCloud(kinectNe.getMesh(), yellowPrint);
			ofPopView();
		}
	}
	
	if(ofGetKeyPressed(' ')) {
		vector<KinectTracker*> kinects;
		kinects.push_back(&kinectSw);
		kinects.push_back(&kinectNe);
		ofPushMatrix();
		for(int i = 0; i < kinects.size(); i++) {
			KinectTracker* kinect = kinects[i];		
			if(kinect->getKinect().isConnected()) {
				kinect->getKinect().drawDepth(0, 0, 320, 240);
				kinect->getBackground().draw(0, 240, 320, 240);
				kinect->getValid().draw(0, 480, 320, 240);
			}
			ofTranslate(320, 0);
		}
		ofPopMatrix();
	}
}

