#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"

#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

GLvoid framebuffer_size_callback(GLFWwindow* window, GLint width, GLint height);
GLvoid mouse_callback(GLFWwindow* window, GLdouble xpos, GLdouble ypos);
GLvoid scroll_callback(GLFWwindow* window, GLdouble xoffset, GLdouble yoffset);
GLvoid processInput(GLFWwindow* window);

GLfloat catmullRom(GLfloat p0, GLfloat p1, GLfloat p2, GLfloat p3, GLfloat t, GLboolean tan);
GLfloat bSpline(GLfloat p0, GLfloat p1, GLfloat p2, GLfloat p3, GLfloat t, GLboolean tan);
GLfloat vector2angle(GLfloat y, GLfloat x);
glm::quat euler2quat(glm::vec3 eularAngles);
glm::mat4 quat2mat4(glm::quat q);

GLfloat lerp(GLfloat p0, GLfloat p1, GLfloat t);
GLvoid quaternionOperations(GLfloat(*splineFunc)(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLboolean), GLint segment);
GLvoid legMotion();

// settings
const GLuint SCR_WIDTH = 800;
const GLuint SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 20.0f, 30.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -40.0f);

GLfloat lastX = SCR_WIDTH / 2.0f;
GLfloat lastY = SCR_HEIGHT / 2.0f;
GLboolean firstMouse = true;

// timing
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// dt default
GLfloat dt = 0.001;

// frame index
GLint frameCount = 0;
GLint animFrameCount = -1;

// vector of Transformation Matrices for each frame of interpolation
std::vector<glm::mat4> torsoAnim; // torso
std::vector<glm::mat4> legAnim; // leg
size_t legAnimOffset = 0;

// control points 
GLfloat positionArray[24] = { // positions
	-9.0,  0, -9,
	-9.0,  0,  9,
	 9.0,  0, -9,
	 9.0,  0,  9,
	-9.0,  0, -9,
	-9.0,  0,  9,
	 9.0,  0, -9,
	 9.0,  0,  9
};

// lighting
glm::vec3 lightPos(0.0f, 10.0f, 10.0f);


//================================
// init
//================================
GLvoid init(GLvoid) {

	GLint splineMode = 1;
	std::cout << "Select interpolation mode: \n 1: Catmull-Rom \n 2: B-Spline" << "\n";
	std::cin >> splineMode;
	/*   std::cout << "Enter dt:" << "\n";
	   std::cin >> dt;*/

	// calculate animation frames
	legMotion();
	if (splineMode == 1) {
		for (size_t i = 0; i < 5; i++)
			quaternionOperations(catmullRom, i);
	}
	else if (splineMode == 2) {
		for (size_t i = 0; i < 5; i++)
			quaternionOperations(bSpline, i);
	}
	else {
		exit(1);
	}


}

GLint main()
{
	init();
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Lab2", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
	stbi_set_flip_vertically_on_load(true);

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	Shader modelShader("model.vs", "model.fs");
	Shader floorShader("background.vs", "background.fs");

	// load models
	// -----------
	Model cylinder("models/cylinder.obj");
	Model cube("models/cube.obj");

	// vertices info for drawing the floor
	GLfloat vertices[] = {
		 15.0f, 0,  15.0f,  // top right
		 15.0f, 0, -15.0f,  // bottom right
		-15.0f, 0,  15.0f,  // bottom left
		-15.0f, 0, -15.0f   // top left 
	};
	GLuint indices[] = {  
		0, 1, 3,   // first triangle
		0, 2, 3    // second triangle
	};

	// bind VBO VAO EBO for floor
	GLuint VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// unbind buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// unbind VAO
	glBindVertexArray(0);

	// draw in wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		GLfloat currentFrame = static_cast<GLfloat>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		//std::cout << deltaTime;

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// enable shader before setting uniforms
		modelShader.use();
		modelShader.setVec3("light.position", lightPos);
		modelShader.setVec3("viewPos", camera.Position);

		// light properties
		glm::vec3 lightColor;
		lightColor.x = 1.0f;
		lightColor.y = 1.0f;
		lightColor.z = 1.0f;
		glm::vec3 diffuseColor = lightColor * glm::vec3(1.0f);
		glm::vec3 ambientColor = diffuseColor * glm::vec3(0.1f);
		modelShader.setVec3("light.ambient", ambientColor);
		modelShader.setVec3("light.diffuse", diffuseColor);
		modelShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

		// material properties
		modelShader.setVec3("material.ambient", 1.0f, 1.0f, 1.0f);
		modelShader.setVec3("material.diffuse", 0.3f, 0.3f, 0.7f);
		modelShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
		modelShader.setFloat("material.shininess", 2.0f);

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		modelShader.setMat4("projection", projection);
		modelShader.setMat4("view", view);


		// update the transformation matrix for each frame
		glm::mat4 torsoMat, legLMat, legRMat;
		if (animFrameCount >= 0 && animFrameCount < torsoAnim.size()) {
			torsoMat = torsoAnim[animFrameCount];
			legLMat = torsoMat * legAnim[animFrameCount % legAnim.size()];
			legRMat = torsoMat * legAnim[(animFrameCount + legAnimOffset) % legAnim.size()];
			animFrameCount++;
		}
		else {
			GLint lastframe = torsoAnim.size() - 1;
			torsoMat = torsoAnim[lastframe];
			legLMat = torsoMat * legAnim[lastframe % legAnim.size()];
			legRMat = torsoMat * legAnim[(lastframe + legAnimOffset) % legAnim.size()];
		}

		// draw the torso
		glm::mat4 torsoModel;
		torsoModel = glm::scale(torsoMat, glm::vec3(0.5f, 0.2f, 0.5f));
		torsoModel = glm::translate(torsoModel, glm::vec3(0, 10, 0));
		modelShader.setMat4("model", torsoModel);
		cylinder.Draw(modelShader);

		// draw left leg
		glm::mat4 legLModel = legLMat;
		legLModel = glm::translate(legLModel, glm::vec3(.5, 0, 0));
		legLModel = glm::scale(legLModel, glm::vec3(0.2f, 0.4f, 0.2f));
		modelShader.setMat4("model", legLModel);
		cylinder.Draw(modelShader);
		// draw right leg
		glm::mat4 legRModel = legRMat;
		legRModel = glm::translate(legRModel, glm::vec3(-.5, 0, 0));
		legRModel = glm::scale(legRModel, glm::vec3(0.2f, 0.4f, 0.2f));
		modelShader.setMat4("model", legRModel);
		cylinder.Draw(modelShader);

		// draw floor
		//floorShader.use();
		//floorShader.setMat4("projection", projection);
		//floorShader.setMat4("view", view);
		glBindVertexArray(VAO);

		glm::mat4 floorModel = glm::mat4(1.0);
		modelShader.setMat4("model", floorModel);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		floorModel = glm::translate(floorModel, glm::vec3(-15,15,0));
		floorModel = glm::rotate(floorModel, glm::pi<GLfloat>()/2.0f, glm::vec3(0, 0, 1));
		modelShader.setMat4("model", floorModel);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	// unbind buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// unbind VAO
	glBindVertexArray(0);
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
GLvoid processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	// press SPACE to start animation
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		animFrameCount = 0;

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
GLvoid framebuffer_size_callback(GLFWwindow* window, GLint width, GLint height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
GLvoid mouse_callback(GLFWwindow* window, GLdouble xposIn, GLdouble yposIn)
{
	GLfloat xpos = static_cast<GLfloat>(xposIn);
	GLfloat ypos = static_cast<GLfloat>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
GLvoid scroll_callback(GLFWwindow* window, GLdouble xoffset, GLdouble yoffset)
{
	camera.ProcessMouseScroll(static_cast<GLfloat>(yoffset));
}


GLfloat catmullRom(GLfloat p0, GLfloat p1, GLfloat p2, GLfloat p3, GLfloat t, GLboolean tan=false) {
	GLfloat MArray[16] = {
		-0.5,  1.5, -1.5,  0.5,
		 1.0, -2.5,  2.0, -0.5,
		-0.5,  0.0,  0.5,  0.0,
		 0.0,  1.0,  0.0,  0.0
	};
	glm::vec4 T;
	if (!tan) {
		GLdouble t2 = t * t;
		GLdouble t3 = t2 * t;
		T = glm::vec4(t3, t2, t, 1);
	}
	else {
		T = glm::vec4(3 * t * t, 2 * t, 1, 0);
	}
	
	glm::mat4 M = glm::transpose(glm::make_mat4(MArray));
	glm::vec4 P(p0, p1, p2, p3);
	GLfloat result = glm::dot(T * M, P);
	return result;
}

GLfloat bSpline(GLfloat p0, GLfloat p1, GLfloat p2, GLfloat p3, GLfloat t, GLboolean tan=false) {
	GLfloat MArray[16] = {
		-1 / 6.0,  3 / 6.0, -3 / 6.0, 1 / 6.0,
		 3 / 6.0, -6 / 6.0,  3 / 6.0,       0,
		-3 / 6.0,        0,  3 / 6.0,       0,
		 1 / 6.0,  4 / 6.0,  1 / 6.0,       0
	};
	glm::vec4 T;
	if (!tan) {
		GLdouble t2 = t * t;
		GLdouble t3 = t2 * t;
		T = glm::vec4(t3, t2, t, 1);
	}
	else {
		T = glm::vec4(3 * t * t, 2 * t, 1, 0);
	}
	glm::mat4 M = glm::transpose(glm::make_mat4(MArray));
	glm::vec4 P(p0, p1, p2, p3);

	GLfloat result = glm::dot(T * M, P);
	return result;
}

GLfloat vector2angle(GLfloat z, GLfloat x)
{
	return glm::atan(z, x);
}

glm::quat quatMul(glm::quat q1, glm::quat q2) {
	glm::quat q;
	GLfloat w1 = q1.w;
	GLfloat w2 = q2.w;
	glm::vec3 v1(q1.x, q1.y, q1.z);
	glm::vec3 v2(q2.x, q2.y, q2.z);

	q.w = w1 * w2 - glm::dot(v1, v2);
	glm::vec3 v = w1 * v2 + w2 * v1 + glm::cross(v1, v2);
	q.x = v.x;
	q.y = v.y;
	q.z = v.z;
	return q;
}

glm::quat euler2quat(glm::vec3 eulerAngles)
{
	GLfloat x = eulerAngles.x * 0.5;
	GLfloat y = eulerAngles.y * 0.5;
	GLfloat z = eulerAngles.z * 0.5;

	glm::quat qz, qy, qx;
	qz = glm::quat(glm::cos(z), 0, 0, glm::sin(z));
	qy = glm::quat(glm::cos(y), 0, glm::sin(y), 0);
	qx = glm::quat(glm::cos(x), glm::sin(x), 0, 0);

	glm::quat q = quatMul(quatMul(qz, qy), qx);
	return q;
}

glm::mat4 quat2mat4(glm::quat q) {
	GLfloat w = q.w;
	GLfloat x = q.x;
	GLfloat y = q.y;
	GLfloat z = q.z;

	GLfloat x2 = x * x;
	GLfloat y2 = y * y;
	GLfloat z2 = z * z;

	GLfloat mat4array[16] = {
		1 - 2 * y2 - 2 * z2,   2 * x * y - 2 * w * z,   2 * x * z + 2 * w * y, 0,
		  2 * x * y + 2 * w * z, 1 - 2 * x2 - 2 * z2,   2 * y * z - 2 * w * x, 0,
		  2 * x * z - 2 * w * y,   2 * y * z + 2 * w * x, 1 - 2 * x2 - 2 * y2, 0,
					  0,               0,               0, 1
	};

	return glm::transpose(glm::make_mat4(mat4array));
}

// calculate spline for 4 control points
GLvoid quaternionOperations(GLfloat (*splineFunc)(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLboolean), GLint segment) {

	GLfloat* tempCtrlPos = positionArray + segment * 3;

	glm::mat4x3 controlPointsPos = glm::make_mat4x3(tempCtrlPos);

	// intermediate variables
	GLfloat xi, yi, zi;
	GLfloat angle;

	for (GLfloat i = 0; i < 1; i += dt) {

		// compute catmull-rom interpolation for position
		glm::vec3 posTransform;
		xi = splineFunc(controlPointsPos[0][0], controlPointsPos[1][0], controlPointsPos[2][0], controlPointsPos[3][0], i, false);
		yi = splineFunc(controlPointsPos[0][1], controlPointsPos[1][1], controlPointsPos[2][1], controlPointsPos[3][1], i, false);
		zi = splineFunc(controlPointsPos[0][2], controlPointsPos[1][2], controlPointsPos[2][2], controlPointsPos[3][2], i, false);
		posTransform.x = xi;
		posTransform.y = yi;
		posTransform.z = zi;

		// calculate tangent along the spline to set facing direction
		glm::quat quaternion = glm::normalize(quaternion);
		GLfloat tanx = splineFunc(controlPointsPos[0][0], controlPointsPos[1][0], controlPointsPos[2][0], controlPointsPos[3][0], i, true);
		GLfloat tanz = splineFunc(controlPointsPos[0][2], controlPointsPos[1][2], controlPointsPos[2][2], controlPointsPos[3][2], i, true);
		angle = vector2angle(tanx, tanz);

		// compute 4x4 transformation matrix 
		glm::mat4 transformMatrix(1.0f);
		// translation 
		transformMatrix = glm::translate(transformMatrix, posTransform);
		// rotation
		quaternion = euler2quat(glm::vec3(0, angle, 0));
		glm::mat4 rotationMatrix = quat2mat4(quaternion);
		transformMatrix = transformMatrix * rotationMatrix;

		// push result into vector for return
		torsoAnim.push_back(transformMatrix);
	}

}

// linear interpolation
GLfloat lerp(GLfloat p0, GLfloat p1, GLfloat t) {
	GLfloat MArray[4] = { -1, 1, 1, 0 };
	glm::vec2 T(t, 1);
	glm::mat2 M = glm::transpose(glm::make_mat2(MArray));
	glm::vec2 P(p0, p1);
	GLfloat result = glm::dot(T * M, P);
	return result;
}

// define animation for legs wrt. torso
GLvoid legMotion() {

	// control points for leg rotation
	GLfloat legRotArray[9] = {
		 135, 0, 0,
		 225, 0, 0,
	};

	// fix the position wrt. torso
	glm::vec3 posTransform = glm::vec3(0, 2.2, 0);
	glm::mat2x3 controlPointsOri = glm::make_mat3x3(legRotArray);

	GLfloat rolli, yawi, pitchi;

	// forward swing
	for (GLfloat i = 0; i < 1; i += (dt * 6)) {

		// compute calmull-rom interpolation for orientation
		rolli = lerp(controlPointsOri[0][0], controlPointsOri[1][0], i);
		yawi = lerp(controlPointsOri[0][1], controlPointsOri[1][1], i);
		pitchi = lerp(controlPointsOri[0][2], controlPointsOri[1][2], i);

		// compute 4x4 transformation matrix 
		glm::mat4 transformMatrix(1.0f); // identity matrix 
		transformMatrix = glm::translate(transformMatrix, posTransform);
		transformMatrix = glm::rotate(transformMatrix, glm::radians(yawi), glm::vec3(0, 1, 0));
		transformMatrix = glm::rotate(transformMatrix, glm::radians(rolli), glm::vec3(1, 0, 0));
		transformMatrix = glm::rotate(transformMatrix, glm::radians(pitchi), glm::vec3(0, 0, 1));

		// push result into vector for return
		legAnim.push_back(transformMatrix);
	}

	// record the mid-point for leg animation to offset right leg animation on left leg animation
	legAnimOffset = legAnim.size();

	// backward swing
	for (GLfloat i = 1; i > 0; i -= (dt * 6)) {

		// compute calmull-rom interpolation for orientation
		rolli = lerp(controlPointsOri[0][0], controlPointsOri[1][0], i);
		yawi = lerp(controlPointsOri[0][1], controlPointsOri[1][1], i);
		pitchi = lerp(controlPointsOri[0][2], controlPointsOri[1][2], i);

		// compute 4x4 transformation matrix 
		glm::mat4 transformMatrix(1.0f); // identity matrix 
		transformMatrix = glm::translate(transformMatrix, posTransform);
		transformMatrix = glm::rotate(transformMatrix, glm::radians(yawi), glm::vec3(0, 1, 0));
		transformMatrix = glm::rotate(transformMatrix, glm::radians(rolli), glm::vec3(1, 0, 0));
		transformMatrix = glm::rotate(transformMatrix, glm::radians(pitchi), glm::vec3(0, 0, 1));

		// push result into vector for return
		legAnim.push_back(transformMatrix);
	}
}