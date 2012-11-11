#include "MiniFont.h"

namespace MiniFont {
	ofTrueTypeFont font;
	
	GLdouble modelviewMatrix[16], projectionMatrix[16];
	GLint viewport[4];
	void updateProjectionState() {
		glGetDoublev(GL_MODELVIEW_MATRIX, modelviewMatrix);
		glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
		glGetIntegerv(GL_VIEWPORT, viewport);
	}

	ofVec3f ofWorldToScreen(ofVec3f world) {
		GLdouble x, y, z;
		gluProject(world.x, world.y, world.z, modelviewMatrix, projectionMatrix, viewport, &x, &y, &z);
		ofVec3f screen(x, y, z);
		screen.y = ofGetHeight() - screen.y;
		return screen;
	}

	void setup() {
		font.loadFont("uni05_53.ttf", 6, false);		
	}
	
	void draw(string str, float x, float y) {
		font.drawString(str, x, y);
	}
}