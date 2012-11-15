/*
 z axis is up/down, everything is on the x/y plane
 +z is up, -z is down
 everything on screen is done in millimeters, though data is diverse

 need to test with actual serial device
 */

#include "MissingControl.h"

#include "Conversion.h"

float stageSize = feetInchesToMillimeters(15, 8);
float stageHeight = inchesToMillimeters(118);
float eyeLevel = stageHeight / 2;
float rotationSpeed = 2;
float perspectiveScale = .15;

void MissingControl::setup() {
	ofSetVerticalSync(true);
	//ofSetLogLevel(OF_LOG_VERBOSE);
	
	driver.setup("tty.usbserial-A100S0L9", 57600);
	
	MiniFont::setup();
	
	midi.listPorts();
	midi.openPort("IAC Driver Bus 1");
	
	osc.setup(145145);
	
	presence.setDelay(.5);
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
	
	autorun = false;
	
	gui.setup(280, 400);
	gui.addPanel("Config");
	gui.addSlider("updateRate", 0, 0, 30, true);
	gui.addSlider("configMaxSpeed", 38, 1, 127, true);
	gui.addSlider("configMinSpeed", 100, 1, 127, true);
	gui.addSlider("configStop", 2, 1, 127, true);
	gui.addSlider("configStart", 10, 1, 127, true);
	
	gui.addPanel("Visual");
	gui.addSlider("rotationSpeed", 2, 0, 10);
	gui.addSlider("perspectiveScale", .15, 0, 1);
	
	gui.addPanel("Sound");
	gui.addSlider("volumeDelayLength", .5, 0, 5);
	gui.addSlider("volumeFadeLength", 3, 0, 5);
	
	gui.loadSettings("settings.xml");
	
	gui.hide();
}

void MissingControl::buildWires() {
	wires.setMode(OF_PRIMITIVE_LINES);
	for(int i = 0; i < wiresCloud.getNumVertices(); i++) {
		ofVec3f wireCenter = wiresCloud.getVertex(i);
		wires.addVertex(wireCenter);
		wires.addVertex(wireCenter + ofVec3f(0, 0, stageHeight));
	}
}

void MissingControl::buildSpeakers() {
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

void MissingControl::update() {
	driver.setUpdateRate(gui.getValueF("updateRate"));
	driver.setConfigMaxSpeed(gui.getValueF("configMaxSpeed"));
	driver.setConfigMinSpeed(gui.getValueF("configMinSpeed"));
	driver.setConfigStop(gui.getValueF("configStop"));
	driver.setConfigStart(gui.getValueF("configStart"));
	
	rotationSpeed = gui.getValueF("rotationSpeed");
	perspectiveScale = gui.getValueF("perspectiveScale");

	while(osc.hasWaitingMessages()) {
		ofxOscMessage msg;
		osc.getNextMessage(&msg);
		string address = msg.getAddress();
		if(address == "/listeners") {
			int n = msg.getNumArgs() / 2;
			oscListeners.clear();
			for(int i = 0; i < n; i++) {
				ofVec2f cur(msg.getArgAsFloat(2 * i), msg.getArgAsFloat(2 * i + 1));
				cur.rotate(+90);
				cur.y *= -1;
				oscListeners.push_back(cur);
			}
		}
	}

	// start by copying the current osc listeners
	listeners = oscListeners;
	// then add any autorun listeners
	if(autorun) {
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
		cur.x = ofMap(mouseX, 0, ofGetWidth(), -stageSize / 2, stageSize / 2);
		cur.y = ofMap(mouseY, 0, ofGetHeight(), -stageSize / 2, stageSize / 2);
		cur.rotate(+90);
		cur.y *= -1;
		listeners.push_back(cur);
	}
	
	rawPresence = !listeners.empty();
	presence.setDelay(gui.getValueF("volumeDelayLength"));
	presence.update(rawPresence);
	volume.setLength(gui.getValueF("volumeFadeLength"));
	volume.update(presence);
	midi.sendControlChange(1, 1, 127 * volume.get());
	
	for(int i = 0; i < speakers.size(); i++) {
		speakers[i].update(listeners);
	}
	
	driver.update(speakers);
}

void MissingControl::drawScene(bool showLabels) {
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

void MissingControl::drawPlan(float x, float y, float side) {
	ofPushView();
	ofViewport(x, y, side, side);
	ofSetupScreenOrtho(side, side, OF_ORIENTATION_DEFAULT, false, -stageHeight, stageHeight);
	ofTranslate(side / 2, side / 2);
	float scale = side / stageSize;
	ofScale(scale, scale, scale);
	drawScene(false);
	ofPopView();
}

void MissingControl::drawSection(float x, float y, float side) {
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

void MissingControl::drawPerspective() {
	cam.begin();
	ofScale(perspectiveScale, perspectiveScale, perspectiveScale);
	ofRotateX(-90);
	ofRotateZ(rotationSpeed * ofGetElapsedTimef());
	drawScene(true);
	cam.end();
}

void MissingControl::draw() {
	ofPushStyle();
	ofEnableAlphaBlending();
	
	ofBackground(0);
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
	ofLine(0, 2, presence.get(), 2);
	ofSetColor(ofColor::fromHex(0xec008c));
	ofLine(0, 4, volume.get(), 4);
	ofPopStyle();
	ofPopMatrix();
	
	ofPopStyle();
}

void MissingControl::keyPressed(int key) {
	if(key == 'a') {
		autorun = !autorun;
	}
}