#include "testApp.h"
#include "ofAppGlutWindow.h"

int main() {
	ofAppGlutWindow window;
	ofSetupOpenGL(&window, 32*20, 32*20, OF_WINDOW);
	ofRunApp(new testApp());
}
