#include "stdafx.h"

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

//================================
// global variables
//================================
// screen size
int g_screenWidth  = 0;
int g_screenHeight = 0;

// frame index
int g_frameIndex = 0;

// position variables
GLfloat x = 0.0;
GLfloat y = 0.0;
GLfloat z = -10.0;
// orientation variables




GLfloat catmullRom(glm::vec4 controlPoints, GLfloat t) {
	GLfloat p0 = controlPoints[0];
	GLfloat p1 = controlPoints[1]; 
	GLfloat p2 = controlPoints[2]; 
	GLfloat p3 = controlPoints[3];
	GLdouble t2 = t * t;
	GLdouble t3 = t2 * t;
	return ((2 * p1) +
		(-p0 + p2) * t +
		(2 * p0 - 5 * p1 + 4 * p2 - p3) * t2 +
		(-p0 + 3 * p1 - 3 * p2 + p3) * t3) * 0.5;
}

std::vector<glm::mat4> eulerOperations(GLint interpolationMode) {
	GLfloat positionArray[12] = {-10,0,0,-5,-5,0,5,5,0,10,0,0}; // column-wise
	glm::mat3x4 controlPointsPos = glm::make_mat3x4(positionArray);
	GLfloat eularOriArray[12] = {-90,0,0,-45,-45,0,45,45,0,90,0,0};
	glm::mat3x4 controlPointsOri = glm::make_mat3x4(eularOriArray);

	GLfloat xi, yi, zi;
	GLfloat rolli, yawi, pitchi;

	std::vector<glm::mat4> transformMatrices;

	for (int i = 0; i < 1; i += 0.1) {

		// compute catmull-rom interpolation for position
		xi = catmullRom(controlPointsPos[0], i);
		yi = catmullRom(controlPointsPos[1], i);
		zi = catmullRom(controlPointsPos[2], i);
		glm::vec3 posTransform(xi, yi, zi);

		// compute calmull-rom interpolation for orientation
		rolli = catmullRom(controlPointsOri[0], i);
		yawi = catmullRom(controlPointsOri[1], i);
		pitchi = catmullRom(controlPointsOri[2], i);

		// compute 4x4 transformation matrix 
		glm::mat4 transformMatrix(1.0f); // identity matrix 
		transformMatrix = glm::translate(transformMatrix, posTransform);
		transformMatrix = glm::rotate(transformMatrix, glm::radians(pitchi), glm::vec3(0, 0, 1));
		transformMatrix = glm::rotate(transformMatrix, glm::radians(yawi), glm::vec3(0, 1, 0));
		transformMatrix = glm::rotate(transformMatrix, glm::radians(rolli), glm::vec3(1, 0, 0));
		
		// push result into vector for return
		transformMatrices.push_back(transformMatrix);
		
	}
	return transformMatrices;
}
void quaternionOperations(GLint interpolationMode) {
	GLfloat positionArray[12] = { -10,0,0,-5,-5,0,5,5,0,10,0,0 }; // column-wise
	glm::mat3x4 controlPointsPos = glm::make_mat3x4(positionArray);
	GLfloat quatOriArray[16] = { -90,0,0,-45,-45,0,45,45,0,90,0,0 };
	glm::mat3x4 controlPointsOri = glm::make_mat4x4(quatOriArray);

	// intermediate variables
	GLfloat xi, yi, zi;
	GLfloat wq, xq, yq, zq;

	std::vector<glm::mat4> transformMatrices;

	for (int i = 0; i < 1; i += 0.1) {

		// compute catmull-rom interpolation for position
		xi = catmullRom(controlPointsPos[0], i);
		yi = catmullRom(controlPointsPos[1], i);
		zi = catmullRom(controlPointsPos[2], i);
		glm::vec3 posTransform(xi, yi, zi);

		// compute calmull-rom interpolation for orientation
		wq = catmullRom(controlPointsOri[0], i);
		xq = catmullRom(controlPointsOri[1], i);
		yq = catmullRom(controlPointsOri[2], i);
		zq = catmullRom(controlPointsOri[3], i);

		// compute 4x4 transformation matrix 
		glm::mat4 transformMatrix(1.0f); // identity matrix 
		transformMatrix = glm::translate(transformMatrix, posTransform);
		transformMatrix = glm::rotate(transformMatrix, glm::radians(zq), glm::vec3(0, 0, 1));
		transformMatrix = glm::rotate(transformMatrix, glm::radians(yawi), glm::vec3(0, 1, 0));
		transformMatrix = glm::rotate(transformMatrix, glm::radians(rolli), glm::vec3(1, 0, 0));

		// push result into vector for return
		transformMatrices.push_back(transformMatrix);

	}
	return transformMatrices;

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

	if (orientationMode == 1) {
		eulerOperations(interpolationMode);
	}
	else if (orientationMode == 1) {
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
	// do something before rendering...

	// set position
	
	// set orientation
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
	glTranslatef (0.0, 0.0, -10.0);
	glRotated(0, 0, 0, 0);
	

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