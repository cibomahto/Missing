#include "MissingApp.h"
#include "ofAppGlutWindow.h"

int main() {
	ofAppGlutWindow window;
	//window.setGlutDisplayString("rgba double samples>=4 depth");
	ofSetupOpenGL(&window, 1280, 720, OF_FULLSCREEN);
	ofRunApp(new MissingApp());
}
