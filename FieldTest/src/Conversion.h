#pragma once

inline float centimetersToMillimeters(float centimeters) {
	return 10 * centimeters;
}

inline float inchesToMillimeters(float inches) {
	return 25.4 * inches;
}

inline float feetToInches(float inches) {
	return 12 * inches;
}

inline float feetToMillimeters(float feet) {
	return inchesToMillimeters(feetToInches(feet));
}

inline float feetInchesToMillimeters(float feet, float inches) {
	return inchesToMillimeters(feetToInches(feet) + inches);
}

inline void scale(ofMesh& mesh, float amount) {
	for(int i = 0; i < mesh.getNumVertices(); i++) {
		mesh.getVertices()[i] *= amount;
	}
}