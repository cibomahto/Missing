#include "MissingApp.h"

#include "Conversion.h"

using namespace ofxCv;
using namespace cv;

const string kinectSerialSw = "A00363A04112112A";
const string kinectSerialNe = "A00362913019109A";
const int gridSize = 1024;

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

void MissingApp::setupControlPanel() {
	ofSetLogLevel(OF_LOG_VERBOSE);
	
	gui.setup(280, 800);
	gui.addPanel("Config");
	gui.addSlider("updateRate", 0, 0, 30, true);
	gui.addSlider("configMaxSpeed", 38, 1, 127, true);
	gui.addSlider("configMinSpeed", 100, 1, 127, true);
	gui.addSlider("configStop", 2, 1, 127, true);
	gui.addSlider("configStart", 10, 1, 127, true);
	
	gui.addPanel("Visual");
	gui.addToggle("autorun", false);
	gui.addSlider("rotationSpeed", 2, 0, 10);
	gui.addSlider("perspectiveScale", .15, 0, 1);
	
	gui.addPanel("Sound");
	gui.addSlider("volumeDelayLength", .5, 0, 5);
	gui.addSlider("volumeFadeLength", 3, 0, 5);
	
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
	
	gui.hide();
}

void MissingApp::setup() {
	ofSetVerticalSync(true);
	setupControlPanel();
	setupTracker();
	setupControl();
}
void MissingApp::update() {
	updateTracker();
	updateControl();
}
void MissingApp::draw() {
	ofBackground(0);
	drawTracker();
	drawControl();
}

float stageSize = feetInchesToMillimeters(15, 8);
float stageHeight = inchesToMillimeters(118);
float eyeLevel = stageHeight / 2;
float rotationSpeed = 2;
float perspectiveScale = .15;

void MissingApp::setupControl() {	
	driver.setup("tty.usbserial-A100S0L9", 57600);
	
	MiniFont::setup();
	
	midi.listPorts();
	midi.openPort("IAC Driver Bus 1");
	
	presenceHysteresis.setDelay(.5);
	volume.setLength(3);
	
	Speaker::setupMesh();
	
	wiresCloud.setMode(OF_PRIMITIVE_POINTS);
	wiresCloud.load("wires.ply");
	scale(wiresCloud, inchesToMillimeters(1));
	
	centersCloud.setMode(OF_PRIMITIVE_POINTS);
	centersCloud.load("centers.ply");
	scale(centersCloud, inchesToMillimeters(1));
	
	buildWires();
	buildSpeakers();
}

void MissingApp::buildWires() {
	wires.setMode(OF_PRIMITIVE_LINES);
	for(int i = 0; i < wiresCloud.getNumVertices(); i++) {
		ofVec3f wireCenter = wiresCloud.getVertex(i);
		wires.addVertex(wireCenter);
		wires.addVertex(wireCenter + ofVec3f(0, 0, stageHeight));
	}
}

void MissingApp::buildSpeakers() {
	ofxXmlSettings calibration;
	calibration.loadFile("calibration.xml");
	ofSeedRandom(0);
	for(int i = 0; i < centersCloud.getNumVertices(); i++) {
		Speaker speaker;
		ofVec3f position = centersCloud.getVertex(i);
		position.z = feetToMillimeters(ofRandom(2, 6));
		calibration.pushTag("speaker", i);
		int posMin = calibration.getValue("posMin", 0);
		int posCenter = calibration.getValue("posCenter", 64);
		int posMax = calibration.getValue("posMax", 127);
		calibration.popTag();
		speaker.setup(position, wiresCloud, posMin, posCenter, posMax);
		speakers.push_back(speaker);
	}
}

void MissingApp::updateControl() {
	driver.setUpdateRate(gui.getValueF("updateRate"));
	driver.setConfigMaxSpeed(gui.getValueF("configMaxSpeed"));
	driver.setConfigMinSpeed(gui.getValueF("configMinSpeed"));
	driver.setConfigStop(gui.getValueF("configStop"));
	driver.setConfigStart(gui.getValueF("configStart"));
	
	rotationSpeed = gui.getValueF("rotationSpeed");
	perspectiveScale = gui.getValueF("perspectiveScale");

	// start by copying the current osc listeners
	listeners = realListeners;
	// then add any autorun listeners
	if(gui.getValueB("autorun")) {
		ofVec2f planet;
		float t = .05 * ofGetElapsedTimef();
		planet.x = ofMap(ofNoise(t, 0), 0, 1, -stageSize, stageSize) / 2; 
		planet.y = ofMap(ofNoise(0, t), 0, 1, -stageSize, stageSize) / 2;
		listeners.push_back(planet);
	}
	// a center listener if 'm' is pressed
	if(ofGetKeyPressed('m')) {
		listeners.push_back(ofVec2f(0, 0));
	}
	// a mouse-following listener if ' ' is pressed
	if(ofGetKeyPressed(' ')) {
		ofVec2f cur;
		cur.x = ofMap(ofGetMouseX(), 0, ofGetWidth(), -stageSize / 2, stageSize / 2);
		cur.y = ofMap(ofGetMouseY(), 0, ofGetHeight(), -stageSize / 2, stageSize / 2);
		cur.rotate(+90);
		cur.y *= -1;
		listeners.push_back(cur);
	}
	
	rawPresence = !listeners.empty();
	presenceHysteresis.setDelay(gui.getValueF("volumeDelayLength"));
	presenceHysteresis.update(rawPresence);
	volume.setLength(gui.getValueF("volumeFadeLength"));
	volume.update(presenceHysteresis);
	midi.sendControlChange(1, 1, 127 * volume.get());
	
	for(int i = 0; i < speakers.size(); i++) {
		speakers[i].update(listeners);
	}

	driver.update(speakers);
}

void MissingApp::drawScene(bool showLabels) {
	ofRotateZ(90); // make north point up
	ofTranslate(0, 0, -eyeLevel);
	ofPushMatrix();
	ofNoFill();
	ofRect(-stageSize / 2, -stageSize / 2, stageSize, stageSize);		
	ofTranslate(0, 0, stageHeight);
	ofRect(-stageSize / 2, -stageSize / 2, stageSize, stageSize);
	ofPopMatrix();
	
	for(int i = 0; i < speakers.size(); i++) {
		speakers[i].draw(showLabels);
	}
	
	for(int i = 0; i < listeners.size(); i++) {
		ofCircle(listeners[i], feetToMillimeters(1));
	}
	
	wiresCloud.draw();
	centersCloud.draw();
	wires.draw();
}

void MissingApp::drawPlan(float x, float y, float side) {
	ofPushView();
	ofViewport(x, y, side, side);
	ofSetupScreenOrtho(side, side, OF_ORIENTATION_DEFAULT, false, -stageHeight, stageHeight);
	ofTranslate(side / 2, side / 2);
	float scale = side / stageSize;
	ofScale(scale, scale, scale);
	drawScene(false);
	ofPopView();
}

void MissingApp::drawSection(float x, float y, float side) {
	ofPushView();
	ofViewport(x, y, side, side);
	ofSetupScreenOrtho(side, side, OF_ORIENTATION_DEFAULT, false, -stageHeight, stageHeight);
	ofTranslate(side / 2, side / 2);
	ofRotateX(-90);
	float scale = side / stageSize;
	ofScale(scale, scale, scale);
	drawScene(false);
	ofPopView();
}

void MissingApp::drawPerspective() {
	cam.begin();
	ofScale(perspectiveScale, perspectiveScale, perspectiveScale);
	ofRotateX(-90);
	ofRotateZ(rotationSpeed * ofGetElapsedTimef());
	drawScene(true);
	cam.end();
}

void MissingApp::drawControl() {
	ofPushStyle();
	ofEnableAlphaBlending();
	
	ofSetColor(255, 128);
	
	drawPerspective();
	drawPlan(ofGetWidth() - 256, 0, 256);
	drawSection(ofGetWidth() - 256, 256, 256);
	
	vector<unsigned char>& packet = driver.getPacket();
	string msg;
	for(int i = 0; i < packet.size(); i++) {
		msg += ofToString(i, 2, '0') + " 0x" + ofToHex(packet[i]) + "\n";
	}
	MiniFont::draw(msg, 10, 10);
	
	ofPushMatrix();
	ofPushStyle();
	ofTranslate(10, ofGetHeight() - 10);
	ofScale(100, 1);
	ofSetColor(ofColor::fromHex(0xffee00));
	ofLine(0, 0, (rawPresence ? 1 : 0), 0);
	ofSetColor(ofColor::fromHex(0x00abec));
	ofLine(0, 2, presenceHysteresis.get(), 2);
	ofSetColor(ofColor::fromHex(0xec008c));
	ofLine(0, 4, volume.get(), 4);
	ofPopStyle();
	ofPopMatrix();
	
	ofPopStyle();
}

void MissingApp::setupTracker() {
	connected = false;
	calibrating = false;
	calibrationStart = 0;
	clearBackground = false;
}

void MissingApp::updateTracker() {
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
		
		realListeners.clear();
		for(int i = 0; i < contourFinder.size(); i++) {
			ofVec2f cur = toOf(contourFinder.getCentroid(i));
			cur.x = ofMap(cur.x, 0, gridDivisions, -stageSize / 2, +stageSize / 2);
			cur.y = ofMap(cur.y, 0, gridDivisions, -stageSize / 2, +stageSize / 2);
			cur.rotate(+90);
			cur.y *= -1;
			realListeners.push_back(cur);
		}
	}
}

void MissingApp::drawTracker() {
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
