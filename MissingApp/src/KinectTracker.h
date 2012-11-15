#pragma once

#include "ofxKinect.h"
#include "KinectFilter.h"

class KinectTracker {
protected:
	ofxKinect kinect;
	ofImage valid, background;
	KinectFilter filter;
	bool newFrame, clearBackground, calibrating;
	int backgroundThreshold;
	ofVec3f accelerationSum;
	int accelerationCount;
	ofVec3f upVector;
	ofMatrix4x4 orientationMat;
	ofVec2f offset;
	float rotation;
	ofMesh mesh;
	vector<float> meshArea;
	float zClipMin, zClipMax;
	float maxStretch;
	
public:	
	KinectTracker()
	:newFrame(false)
	,clearBackground(false)
	,calibrating(false)
	,backgroundThreshold(0)
	,rotation(0)
	,zClipMin(-5000)
	,zClipMax(+5000)
	,maxStretch(100)
	{
	}
	
	void setup(string serial) {
		kinect.init(false, false);
		kinect.setRegistration(false);
		kinect.open(serial);
		kinect.setDepthClipping(1000, 4000);
		ofxCv::imitate(background, kinect, CV_8UC1);
		ofxCv::imitate(valid, kinect, CV_8UC1);
	}
	void setZClip(float zClipMin, float zClipMax) {
		this->zClipMin = zClipMin;
		this->zClipMax = zClipMax;
	}
	void setMaxStretch(float maxStretch) {
		this->maxStretch = maxStretch;
	}
	void setClearBackground() {
		clearBackground = true;
	}
	void setCalibrating(bool calibrating) {
		this->calibrating = calibrating;
	}
	void setBackgroundThreshold(int backgroundThreshold) {
		this->backgroundThreshold = backgroundThreshold;
	}
	void setOffset(ofVec2f offset) {
		this->offset = offset;
	}
	void setRotation(float rotation) {
		this->rotation = rotation;
	}
	void update() {
		kinect.update();
		if(kinect.isFrameNew()) {
			newFrame = true;
			filter.update(kinect.getDepthPixelsRef());
			unsigned char* kinectPixels = filter.getMasked().getPixels();
			unsigned char* validPixels = valid.getPixels();
			unsigned char* backgroundPixels = background.getPixels();
			int width = kinect.getWidth(), height = kinect.getHeight();
			int n = width * height;
			if(clearBackground) {
				for(int i = 0; i < n; i++) {
					backgroundPixels[i] = 0;
				}
				background.update();
				clearBackground = false;
				accelerationSum = ofVec3f();
				accelerationCount = 0;
			}
			if(calibrating) {
				for(int i = 0; i < n; i++) {
					if(kinectPixels[i] > 0) {
						if(backgroundPixels[i] == 0) {
							backgroundPixels[i] = kinectPixels[i];
						}	else {
							backgroundPixels[i] = (backgroundPixels[i] + kinectPixels[i]) / 2;
						}
					}
				}
				background.update();
				accelerationSum += kinect.getRawAccel();
				accelerationCount++;
				upVector = -accelerationSum / accelerationCount;
				upVector.y *= -1;
				ofQuaternion orientationQuat;
				upVector.normalize();
				orientationQuat.makeRotate(ofVec3f(0, 0, 1), upVector);
				orientationQuat.get(orientationMat);
			}
			for(int i = 0; i < n; i++) {
				int kinectPixel = kinectPixels[i];
				int backgroundPixel = backgroundPixels[i];
				bool far = abs(kinectPixel - backgroundPixel) > backgroundThreshold;
				if(kinectPixel < 255 && kinectPixel > 0 && (backgroundPixel == 0 || (backgroundPixel > 0 && far))) {
					validPixels[i] = 255;
				} else {
					validPixels[i] = 0;
				}
			}
			valid.update();			
			mesh.setMode(OF_PRIMITIVE_POINTS);
			mesh.clear();
			meshArea.clear();
			const unsigned short* rawDepthPixels = kinect.getRawDepthPixels();
			int i = 0;
			for(int y = 0; y < height; y++) {
				for(int x = 0; x < width; x++) {
					if(x + 1 < width &&	validPixels[i] && validPixels[i + 1]) {
						// only need to do one of these, can cache it for the next loop
						ofVec3f cur = kinect.getWorldCoordinateAt(x, y, rawDepthPixels[i]);
						ofVec3f right = kinect.getWorldCoordinateAt(x + 1, y, rawDepthPixels[i + 1]);
						float curArea = cur.distance(right);
						if(curArea < maxStretch) {
							curArea *= curArea;
							cur = orientationMat * cur;
							if(cur.z > zClipMin && cur.z < zClipMax) {
								cur += offset;
								cur.rotate(rotation, ofVec3f(0, 0, 1));
								mesh.addVertex(cur);
								meshArea.push_back(curArea);
							}
						}
					}
					i++;
				}
			}
		}
	}
	ofMesh& getMesh() {
		return mesh;
	}
	vector<float>& getMeshArea() {
		return meshArea;
	}
	bool isFrameNew() {
		bool cur = newFrame;
		newFrame = false;
		return cur;
	}
	ofxKinect& getKinect() {
		return kinect;
	}
	ofImage& getBackground() {
		return background;
	}
	ofImage& getValid() {
		return valid;
	}
};