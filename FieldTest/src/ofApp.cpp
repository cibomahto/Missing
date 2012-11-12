/*
 z axis is up/down, everything is on the x/y plane
 +z is up, -z is down
 everything on screen is done in millimeters, though data is diverse

 need to test with actual serial device
 */

#include "ofApp.h"

#include "Conversion.h"

float stageSize = feetInchesToMillimeters(15, 8);
float stageHeight = inchesToMillimeters(118);
float eyeLevel = stageHeight / 2;
float rotationSpeed = 2;
float perspectiveScale = .15;

void ofApp::setup() {
	ofSetVerticalSync(true);
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofEnableAlphaBlending();
	
	driver.setup("tty", 9600);
	driver.setDeadZone(10);
	
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
}

void ofApp::buildWires() {
	wires.setMode(OF_PRIMITIVE_LINES);
	for(int i = 0; i < wiresCloud.getNumVertices(); i++) {
		ofVec3f wireCenter = wiresCloud.getVertex(i);
		wires.addVertex(wireCenter);
		wires.addVertex(wireCenter + ofVec3f(0, 0, stageHeight));
	}
}

void ofApp::buildSpeakers() {
	ofSeedRandom(0);
	for(int i = 0; i < centersCloud.getNumVertices(); i++) {
		Speaker speaker;
		ofVec3f position = centersCloud.getVertex(i);
		position.z = feetToMillimeters(ofRandom(2, 6));
		speaker.setup(position, wiresCloud);
		speakers.push_back(speaker);
	}
}

void ofApp::update() {
	while(osc.hasWaitingMessages()) {
		ofxOscMessage msg;
		osc.getNextMessage(&msg);
		string address = msg.getAddress();
		if(address == "/listeners") {
			int n = msg.getNumArgs() / 2;
			oscListeners.clear();
			for(int i = 0; i < n; i++) {
				ofVec2f cur(msg.getArgAsFloat(2 * i), msg.getArgAsFloat(2 * i + 1));
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
		listeners.push_back(ofVec2f(
			ofMap(mouseY, 0, ofGetHeight(), -stageSize / 2, stageSize / 2),
			ofMap(mouseX, 0, ofGetWidth(), stageSize / 2, -stageSize / 2)));
	}
	
	rawPresence = !listeners.empty();
	presence.update(rawPresence);
	volume.update(presence);
	midi.sendControlChange(1, 1, 127 * volume.get());
	
	for(int i = 0; i < speakers.size(); i++) {
		speakers[i].update(listeners);
	}
	
	vector<float> angles;
	for(int i = 0; i < speakers.size(); i++) {
		angles.push_back(speakers[i].getAngle());
	}
	driver.update(angles);
}

void ofApp::drawScene(bool showLabels) {
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

void ofApp::drawPlan(float x, float y, float side) {
	ofPushView();
	ofViewport(x, y, side, side);
	ofSetupScreenOrtho(side, side, OF_ORIENTATION_DEFAULT, false, -stageHeight, stageHeight);
	ofTranslate(side / 2, side / 2);
	float scale = side / stageSize;
	ofScale(scale, scale, scale);
	drawScene(false);
	ofPopView();
}

void ofApp::drawSection(float x, float y, float side) {
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

void ofApp::drawPerspective() {
	cam.begin();
	ofScale(perspectiveScale, perspectiveScale, perspectiveScale);
	ofRotateX(-90);
	ofRotateZ(rotationSpeed * ofGetElapsedTimef());
	drawScene(true);
	cam.end();
}

void ofApp::draw() {
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
}

void ofApp::keyPressed(int key) {
	if(key == 'a') {
		autorun = !autorun;
	}
}