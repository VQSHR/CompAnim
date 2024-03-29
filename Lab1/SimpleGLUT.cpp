﻿#include "stdafx.h"

// standard
#include <assert.h>
#include <math.h>
#include <iostream>

// glut
#include <GL/glut.h>

//glm
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp> 
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/ext.hpp>

//================================
// global variables
//================================
// screen size
int g_screenWidth  = 0;
int g_screenHeight = 0;

// frame index
int g_frameIndex = 0;

// dt defaulted to 0.01
GLfloat dt = 0.01;

// vector of Transformation Matrices for each frame of interpolation
std::vector<glm::mat4> transformMatrices;
// intermediate matrix
glm::mat4 transformMat;


GLfloat catmullRom(GLfloat p0, GLfloat p1, GLfloat p2, GLfloat p3, GLfloat t) {
	GLdouble t2 = t * t;
	GLdouble t3 = t2 * t;
	return ((2 * p1) +
		(-p0 + p2) * t +
		(2 * p0 - 5 * p1 + 4 * p2 - p3) * t2 +
		(-p0 + 3 * p1 - 3 * p2 + p3) * t3) * 0.5;
}

GLfloat bSpline(GLfloat p0, GLfloat p1, GLfloat p2, GLfloat p3, GLfloat t) {
	GLdouble t2 = t * t;
	GLdouble t3 = t2 * t;
	GLfloat MArray[16] = { 
		-1/6.0, 3/6.0, -3/6.0, 1/6.0,
		3/6.0, -6/6.0, 3/6.0, 0,
		-3/6.0, 0, 3/6.0, 0,
		1/6.0, 4/6.0, 1/6.0, 0
	};
	glm::vec4 T(t3, t2, t, 1);
	glm::mat4 M = glm::transpose(glm::make_mat4(MArray));
	glm::vec4 P(p0, p1, p2, p3);

	GLfloat result = glm::dot(T * M, P);
	return result;
}

void eulerOperations(GLint interpolationMode) {

	GLfloat positionArray[12] = {-10,0,-20,-5,-5,-20,5,5,-20,10,0,-20}; 
	glm::mat4x3 controlPointsPos = glm::make_mat4x3(positionArray);

	GLfloat eulerOriArray[12] = {-180,0,0,-90,-90,0,90,90,0,180,0,0};
	glm::mat4x3 controlPointsOri = glm::make_mat4x3(eulerOriArray);

	GLfloat xi, yi, zi;
	GLfloat rolli, yawi, pitchi;

	if (interpolationMode == 1) {
		for (float i = 0; i < 1; i += dt) {

			// compute catmull-rom interpolation for position
			xi = catmullRom(controlPointsPos[0][0], controlPointsPos[1][0], controlPointsPos[2][0], controlPointsPos[3][0], i);
			yi = catmullRom(controlPointsPos[0][1], controlPointsPos[1][1], controlPointsPos[2][1], controlPointsPos[3][1], i);
			zi = catmullRom(controlPointsPos[0][2], controlPointsPos[1][2], controlPointsPos[2][2], controlPointsPos[3][2], i);
			glm::vec3 posTransform(xi, yi, zi);

			// compute calmull-rom interpolation for orientation
			rolli = catmullRom(controlPointsOri[0][0], controlPointsOri[1][0], controlPointsOri[2][0], controlPointsOri[3][0], i);
			yawi = catmullRom(controlPointsOri[0][1], controlPointsOri[1][1], controlPointsOri[2][1], controlPointsOri[3][1], i);
			pitchi = catmullRom(controlPointsOri[0][2], controlPointsOri[1][2], controlPointsOri[2][2], controlPointsOri[3][2], i);

			// compute 4x4 transformation matrix 
			glm::mat4 transformMatrix(1.0f); // identity matrix 
			transformMatrix = glm::translate(transformMatrix, posTransform);
			transformMatrix = glm::rotate(transformMatrix, glm::radians(yawi), glm::vec3(0, 1, 0));
			transformMatrix = glm::rotate(transformMatrix, glm::radians(pitchi), glm::vec3(0, 0, 1));
			transformMatrix = glm::rotate(transformMatrix, glm::radians(rolli), glm::vec3(1, 0, 0));

			// push result into vector for return
			transformMatrices.push_back(transformMatrix);
		}
	}
	else if (interpolationMode == 2) {
		for (float i = 0; i < 1; i += dt) {

			// compute bSpline interpolation for position
			xi = bSpline(controlPointsPos[0][0], controlPointsPos[1][0], controlPointsPos[2][0], controlPointsPos[3][0], i);
			yi = bSpline(controlPointsPos[0][1], controlPointsPos[1][1], controlPointsPos[2][1], controlPointsPos[3][1], i);
			zi = bSpline(controlPointsPos[0][2], controlPointsPos[1][2], controlPointsPos[2][2], controlPointsPos[3][2], i);
			glm::vec3 posTransform(xi, yi, zi);

			// compute bSpline interpolation for orientation
			rolli = bSpline(controlPointsOri[0][0], controlPointsOri[1][0], controlPointsOri[2][0], controlPointsOri[3][0], i);
			yawi = bSpline(controlPointsOri[0][1], controlPointsOri[1][1], controlPointsOri[2][1], controlPointsOri[3][1], i);
			pitchi = bSpline(controlPointsOri[0][2], controlPointsOri[1][2], controlPointsOri[2][2], controlPointsOri[3][2], i);

			// compute 4x4 transformation matrix 
			glm::mat4 transformMatrix(1.0f); // identity matrix 
			transformMatrix = glm::translate(transformMatrix, posTransform);
			transformMatrix = glm::rotate(transformMatrix, glm::radians(yawi), glm::vec3(0, 1, 0));
			transformMatrix = glm::rotate(transformMatrix, glm::radians(pitchi), glm::vec3(0, 0, 1));
			transformMatrix = glm::rotate(transformMatrix, glm::radians(rolli), glm::vec3(1, 0, 0));

			// push result into vector for return
			transformMatrices.push_back(transformMatrix);
		}
	}
	else {
		exit(1);
	}
	
}
void quaternionOperations(GLint interpolationMode) {

	GLfloat positionArray[12] = { -10,0,-20,-5,-5,-20,5,5,-20,10,0,-20 };
	glm::mat4x3 controlPointsPos = glm::make_mat4x3(positionArray);

	GLfloat eulerOriArray[12] = { -180,0,0,-90,-90,0,90,90,0,180,0,0 };
	glm::mat4x3 controlPointsOri = glm::make_mat4x3(eulerOriArray);

	// convert euler to quaternion
	std::vector<glm::quat> quaternions;
	for (int k = 0; k < 4; k++) {
		glm::vec3 eulerAngles = glm::radians(controlPointsOri[k]);
		glm::quat q = glm::quat(eulerAngles);
		quaternions.push_back(q);
	}

	// intermediate variables
	GLfloat xi, yi, zi;
	GLfloat wq, xq, yq, zq;
	

	if (interpolationMode == 1) {
		for (float i = 0; i < 1; i += dt) {

			// compute catmull-rom interpolation for position
			xi = catmullRom(controlPointsPos[0][0], controlPointsPos[1][0], controlPointsPos[2][0], controlPointsPos[3][0], i);
			yi = catmullRom(controlPointsPos[0][1], controlPointsPos[1][1], controlPointsPos[2][1], controlPointsPos[3][1], i);
			zi = catmullRom(controlPointsPos[0][2], controlPointsPos[1][2], controlPointsPos[2][2], controlPointsPos[3][2], i);
			glm::vec3 posTransform(xi, yi, zi);

			// compute calmull-rom interpolation for orientation
			xq = catmullRom(quaternions[0][0], quaternions[1][0], quaternions[2][0], quaternions[3][0], i);
			yq = catmullRom(quaternions[0][1], quaternions[1][1], quaternions[2][1], quaternions[3][1], i);
			zq = catmullRom(quaternions[0][2], quaternions[1][2], quaternions[2][2], quaternions[3][2], i);
			wq = catmullRom(quaternions[0][3], quaternions[1][3], quaternions[2][3], quaternions[3][3], i);
			glm::quat quaternion(wq, xq, yq, zq);
			quaternion = glm::normalize(quaternion);

			// compute 4x4 transformation matrix 
			glm::mat4 transformMatrix(1.0f); // identity matrix 
			transformMatrix = glm::translate(transformMatrix, posTransform);
			glm::mat4 rotationMatrix = glm::toMat4(quaternion);
			transformMatrix = transformMatrix * rotationMatrix;

			// push result into vector for return
			transformMatrices.push_back(transformMatrix);
		}
	}
	else if (interpolationMode == 2) {
		for (float i = 0; i < 1; i += dt) {

			// compute bSpline interpolation for position
			xi = bSpline(controlPointsPos[0][0], controlPointsPos[1][0], controlPointsPos[2][0], controlPointsPos[3][0], i);
			yi = bSpline(controlPointsPos[0][1], controlPointsPos[1][1], controlPointsPos[2][1], controlPointsPos[3][1], i);
			zi = bSpline(controlPointsPos[0][2], controlPointsPos[1][2], controlPointsPos[2][2], controlPointsPos[3][2], i);
			glm::vec3 posTransform(xi, yi, zi);

			// compute bSpline interpolation for orientation
			xq = bSpline(quaternions[0][0], quaternions[1][0], quaternions[2][0], quaternions[3][0], i);
			yq = bSpline(quaternions[0][1], quaternions[1][1], quaternions[2][1], quaternions[3][1], i);
			zq = bSpline(quaternions[0][2], quaternions[1][2], quaternions[2][2], quaternions[3][2], i);
			wq = bSpline(quaternions[0][3], quaternions[1][3], quaternions[2][3], quaternions[3][3], i);
			glm::quat quaternion(wq, xq, yq, zq);
			quaternion = glm::normalize(quaternion);

			// compute 4x4 transformation matrix 
			glm::mat4 transformMatrix(1.0f); // identity matrix 
			transformMatrix = glm::translate(transformMatrix, posTransform);
			glm::mat4 rotationMatrix = glm::toMat4(quaternion);
			transformMatrix = transformMatrix * rotationMatrix;

			// push result into vector for return
			transformMatrices.push_back(transformMatrix);
		}
	}
	else {
		exit(1);
	}

}

//================================
// init
//================================
void init( void ) {
	// init something before main loop...
	GLint orientationMode = 1;
	std::cout << "Select rotation mode: 1 for Euler angle, 2 for Quaternion.." << "\n";
	std::cin >> orientationMode;
	GLint interpolationMode = 1;
	std::cout << "Select interpolation mode: 1 for Catmull-Rom, 2 for B-Spline.." << "\n";
	std::cin >> interpolationMode;
	std::cout << "Enter dt:" << "\n";
	std::cin >> dt;

	if (orientationMode == 1) {
		eulerOperations(interpolationMode);
	}
	else if (orientationMode == 2) {
		quaternionOperations(interpolationMode);
	}
	else {
		exit(1);
	}

}

//================================
// update
//================================
void update( void ) {
	// update the transformation matrix for each frame
	if (g_frameIndex < transformMatrices.size()) {
		transformMat = transformMatrices[g_frameIndex];
	}
	else {
		transformMat = transformMatrices[transformMatrices.size() - 1];
	}
	

}

//================================
// render
//================================
void render( void ) {
	// clear buffer
	glClearColor (0.0, 0.0, 0.0, 0.0);
	glClearDepth (1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	
	// render state
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);

	// enable lighting
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// light source attributes
	GLfloat LightAmbient[]	= { 0.4f, 0.4f, 0.4f, 1.0f };
	GLfloat LightDiffuse[]	= { 0.3f, 0.3f, 0.3f, 1.0f };
	GLfloat LightSpecular[]	= { 0.4f, 0.4f, 0.4f, 1.0f };
	GLfloat LightPosition[] = { 5.0f, 5.0f, 5.0f, 1.0f };

	glLightfv(GL_LIGHT0, GL_AMBIENT , LightAmbient );
	glLightfv(GL_LIGHT0, GL_DIFFUSE , LightDiffuse );
	glLightfv(GL_LIGHT0, GL_SPECULAR, LightSpecular);
	glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);

	// surface material attributes
	GLfloat material_Ka[]	= { 0.11f, 0.06f, 0.11f, 1.0f };
	GLfloat material_Kd[]	= { 0.43f, 0.47f, 0.54f, 1.0f };
	GLfloat material_Ks[]	= { 0.33f, 0.33f, 0.52f, 1.0f };
	GLfloat material_Ke[]	= { 0.1f , 0.0f , 0.1f , 1.0f };
	GLfloat material_Se		= 10;

	glMaterialfv(GL_FRONT, GL_AMBIENT	, material_Ka);
	glMaterialfv(GL_FRONT, GL_DIFFUSE	, material_Kd);
	glMaterialfv(GL_FRONT, GL_SPECULAR	, material_Ks);
	glMaterialfv(GL_FRONT, GL_EMISSION	, material_Ke);
	glMaterialf (GL_FRONT, GL_SHININESS	, material_Se);

	// modelview matrix
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glLoadMatrixf(glm::value_ptr(transformMat));
	
	// render objects
	glutSolidTeapot(1.0);

	// disable lighting
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);

	// swap back and front buffers
	glutSwapBuffers();
}

//================================
// keyboard input
//================================
void keyboard( unsigned char key, int x, int y ) {
}

//================================
// reshape : update viewport and projection matrix when the window is resized
//================================
void reshape( int w, int h ) {
	// screen size
	g_screenWidth  = w;
	g_screenHeight = h;	
	
	// viewport
	glViewport( 0, 0, (GLsizei)w, (GLsizei)h );

	// projection matrix
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective(45.0, (GLfloat)w/(GLfloat)h, 1.0, 2000.0);
}


//================================
// timer : triggered every 16ms ( about 60 frames per second )
//================================
void timer( int value ) {	
	// increase frame index
	g_frameIndex++;

	update();
	
	// render
	glutPostRedisplay();

	// reset timer
	// 16 ms per frame ( about 60 frames per second )
	glutTimerFunc( 16, timer, 0 );
}

//================================
// main
//================================
int main( int argc, char** argv ) {
	// create opengL window
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
	glutInitWindowSize( 600, 600 ); 
	glutInitWindowPosition( 100, 100 );
	glutCreateWindow( argv[0] );

	// init
	init();
	
	// set callback functions
	glutDisplayFunc( render );
	glutReshapeFunc( reshape );
	glutKeyboardFunc( keyboard );
	glutTimerFunc( 16, timer, 0 );
	
	// main loop
	glutMainLoop();

	return 0;
}