#include "ofApp.h"
#include "ofAppGlutWindow.h"

int main() {
	ofAppGlutWindow window;
	ofSetupOpenGL(&window, 1280, 480 * 2, OF_WINDOW);
	ofRunApp(new ofApp());
}
