#pragma once

class KinectFilter {
protected:	
	class Hysteresis {
	protected:
		unsigned int lastTime;
		bool curValue;
	public:
		Hysteresis()
		:lastTime(0)
		,curValue(false)
		{
		}
		const bool& update(const bool& value, const unsigned int& curTime, const unsigned int& delay) {
			if(value != curValue) {
				lastTime = curTime;
			}
			curValue = value;
			return curTime - lastTime > delay;
		}
	};
	
	unsigned int kinectFrameCount;
	vector<Hysteresis> hysteresis;
	ofImage masked;
	unsigned int delay;
	
public:
	KinectFilter()
	:kinectFrameCount(0)
	,delay(8)
	{
	}
	void setDelay(unsigned int delay) {
		this->delay = delay;
	}
	void update(ofPixels& pixels) {
		ofxCv::imitate(masked, pixels);
		unsigned char* kinectPixels = pixels.getPixels();
		unsigned char* maskedPixels = masked.getPixels();
		int n = pixels.getWidth() * pixels.getHeight();
		if(hysteresis.size() != n) {
			hysteresis.resize(n);
		}
		for(int i = 0; i < n; i++) {
			if(hysteresis[i].update(kinectPixels[i] > 0, kinectFrameCount, delay)) {
				maskedPixels[i] = pixels[i];
			} else {
				maskedPixels[i] = 0;
			}
		}
		masked.update();
		kinectFrameCount++;
	}
	ofImage& getMasked() {
		return masked;
	}
};