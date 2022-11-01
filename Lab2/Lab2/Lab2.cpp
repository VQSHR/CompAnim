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

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

GLfloat catmullRom(GLfloat p0, GLfloat p1, GLfloat p2, GLfloat p3, GLfloat t, bool tan);
GLfloat bSpline(GLfloat p0, GLfloat p1, GLfloat p2, GLfloat p3, GLfloat t, bool tan);
float vector2angle(float y, float x);

float lerp(float p0, float p1, float t);
void quaternionOperations(GLfloat(*splineFunc)(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, bool), int segment);
void legMotion();

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 20.0f, 30.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -40.0f);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// dt defaulted to 0.001
GLfloat dt = 0.001;

// startframe
GLint frameCount = -1;

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
#if 0
GLfloat eulerOriArray[24] = { // orientation in euler angles
	0, 90, 0,
	0, 90, 0,
	0, 90, 0,
	0, 90, 0,
	0, 90, 0,
	0, 90, 0,
	0, 90, 0,
	0, 90, 0
};
#endif
// lighting
glm::vec3 lightPos(0.0f, 10.0f, 10.0f);


//================================
// init
//================================
void init(void) {

	GLint splineMode = 1;
	std::cout << "Select interpolation mode: \n 1: Catmull-Rom \n 2: B-Spline" << "\n";
	std::cin >> splineMode;
	/*   std::cout << "Enter dt:" << "\n";
	   std::cin >> dt;*/

	// calculate animation frames
	legMotion();
	if (splineMode == 1) {
		for (size_t i = 0; i < 4; i++)
			quaternionOperations(catmullRom, i);
	}
	else if (splineMode == 2) {
		for (size_t i = 0; i < 4; i++)
			quaternionOperations(bSpline, i);
	}
	else {
		exit(1);
	}


}

int main()
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
	Model myModel("untitled.obj");


	float vertices[] = {
		 15.0f, 0,  15.0f,  // top right
		 15.0f, 0, -15.0f,  // bottom right
		-15.0f, 0,  15.0f,  // bottom left
		-15.0f, 0, -15.0f   // top left 
	};
	unsigned int indices[] = {  // note that we start from 0!
		0, 1, 3,   // first triangle
		0, 2, 3    // second triangle
	};

	// bind VBO VAO EBO for floor
	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
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
		float currentFrame = static_cast<float>(glfwGetTime());
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
		glm::vec3 diffuseColor = lightColor * glm::vec3(0.7f);
		glm::vec3 ambientColor = diffuseColor * glm::vec3(1.0f);
		modelShader.setVec3("light.ambient", ambientColor);
		modelShader.setVec3("light.diffuse", diffuseColor);
		modelShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

		// material properties
		modelShader.setVec3("material.ambient", 1.0f, 0.5f, 0.31f);
		modelShader.setVec3("material.diffuse", 1.0f, 0.5f, 0.31f);
		modelShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
		modelShader.setFloat("material.shininess", 32.0f);

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		modelShader.setMat4("projection", projection);
		modelShader.setMat4("view", view);


		// update the transformation matrix for each frame
		glm::mat4 torsoMat, legLMat, legRMat;
		if (frameCount >= 0 && frameCount < torsoAnim.size()) {
			torsoMat = torsoAnim[frameCount];
			legLMat = torsoMat * legAnim[frameCount % legAnim.size()];
			legRMat = torsoMat * legAnim[(frameCount + legAnimOffset) % legAnim.size()];
			frameCount++;
		}
		else {
			int lastframe = torsoAnim.size() - 1;
			torsoMat = torsoAnim[lastframe];
			legLMat = torsoMat * legAnim[lastframe % legAnim.size()];
			legRMat = torsoMat * legAnim[(lastframe + legAnimOffset) % legAnim.size()];
		}

		// draw the torso
		glm::mat4 torsoModel;
		torsoModel = glm::scale(torsoMat, glm::vec3(0.5f, 0.2f, 0.5f));
		torsoModel = glm::translate(torsoModel, glm::vec3(0, 10, 0));
		modelShader.setMat4("model", torsoModel);
		myModel.Draw(modelShader);

		// draw leg
		glm::mat4 legLModel = legLMat;
		legLModel = glm::translate(legLModel, glm::vec3(.5, 0, 0));
		legLModel = glm::scale(legLModel, glm::vec3(0.2f, 0.4f, 0.2f));
		modelShader.setMat4("model", legLModel);
		myModel.Draw(modelShader);

		glm::mat4 legRModel = legRMat;
		legRModel = glm::translate(legRModel, glm::vec3(-.5, 0, 0));
		legRModel = glm::scale(legRModel, glm::vec3(0.2f, 0.4f, 0.2f));
		modelShader.setMat4("model", legRModel);
		myModel.Draw(modelShader);

		// draw floor
		floorShader.use();
		floorShader.setMat4("projection", projection);
		floorShader.setMat4("view", view);
		glBindVertexArray(VAO);
		floorShader.setMat4("model", glm::mat4(1.0));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
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
		frameCount = 0;

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}


GLfloat catmullRom(GLfloat p0, GLfloat p1, GLfloat p2, GLfloat p3, GLfloat t, bool tan=false) {
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

GLfloat bSpline(GLfloat p0, GLfloat p1, GLfloat p2, GLfloat p3, GLfloat t, bool tan=false) {
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

float vector2angle(float z, float x)
{
	return glm::atan(z, x);
}

// calculate spline for 4 control points
void quaternionOperations(GLfloat (*splineFunc)(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, bool), int segment) {

	GLfloat* tempCtrlPos = positionArray + segment * 3;
	//GLfloat* tempCtrlOri = eulerOriArray + segment * 3;

	glm::mat4x3 controlPointsPos = glm::make_mat4x3(tempCtrlPos);
	//glm::mat4x3 controlPointsOri = glm::make_mat4x3(tempCtrlOri);

	// convert euler to quaternion
	/*std::vector<glm::quat> quaternions;
	for (int k = 0; k < 4; k++) {
		glm::vec3 eulerAngles = glm::radians(controlPointsOri[k]);
		glm::quat q = glm::quat(eulerAngles);
		quaternions.push_back(q);
	}*/

	// intermediate variables
	GLfloat xi, yi, zi;
	//GLfloat wq, xq, yq, zq;
	GLfloat angle;

	for (float i = 0; i < 1; i += dt) {

		// compute catmull-rom interpolation for position
		glm::vec3 posTransform;
		xi = splineFunc(controlPointsPos[0][0], controlPointsPos[1][0], controlPointsPos[2][0], controlPointsPos[3][0], i, false);
		yi = splineFunc(controlPointsPos[0][1], controlPointsPos[1][1], controlPointsPos[2][1], controlPointsPos[3][1], i, false);
		zi = splineFunc(controlPointsPos[0][2], controlPointsPos[1][2], controlPointsPos[2][2], controlPointsPos[3][2], i, false);
		posTransform.x = xi;
		posTransform.y = yi;
		posTransform.z = zi;

		// compute calmull-rom interpolation for orientation
		/*xq = splineFunc(quaternions[0][0], quaternions[1][0], quaternions[2][0], quaternions[3][0], i, false);
		yq = splineFunc(quaternions[0][1], quaternions[1][1], quaternions[2][1], quaternions[3][1], i, false);
		zq = splineFunc(quaternions[0][2], quaternions[1][2], quaternions[2][2], quaternions[3][2], i, false);
		wq = splineFunc(quaternions[0][3], quaternions[1][3], quaternions[2][3], quaternions[3][3], i, false);
		*/
		glm::quat quaternion = glm::normalize(quaternion);

		float tanx = splineFunc(controlPointsPos[0][0], controlPointsPos[1][0], controlPointsPos[2][0], controlPointsPos[3][0], i, true);
		float tanz = splineFunc(controlPointsPos[0][2], controlPointsPos[1][2], controlPointsPos[2][2], controlPointsPos[3][2], i, true);
		angle = vector2angle(tanx, tanz);

		// compute 4x4 transformation matrix 
		glm::mat4 transformMatrix(1.0f); // identity matrix 
		transformMatrix = glm::translate(transformMatrix, posTransform);

		quaternion = glm::normalize(glm::quat(glm::vec3(0, angle, 0)));
		glm::mat4 rotationMatrix = glm::toMat4(quaternion);
		transformMatrix = transformMatrix * rotationMatrix;

		// push result into vector for return
		torsoAnim.push_back(transformMatrix);
	}

}

// linear interpolation
float lerp(float p0, float p1, float t) {
	float MArray[4] = { -1, 1, 1, 0 };
	glm::vec2 T(t, 1);
	glm::mat2 M = glm::transpose(glm::make_mat2(MArray));
	glm::vec2 P(p0, p1);
	GLfloat result = glm::dot(T * M, P);
	return result;
}

void legMotion() {
	float legRotArray[9] = {
		 135, 0, 0,
		 225, 0, 0,
	};

	glm::vec3 posTransform = glm::vec3(0, 2.2, 0);
	glm::mat2x3 controlPointsOri = glm::make_mat3x3(legRotArray);

	float rolli, yawi, pitchi;
	// forward swing
	for (float i = 0; i < 1; i += (dt * 6)) {

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
	legAnimOffset = legAnim.size();
	// backward swing
	for (float i = 1; i > 0; i -= (dt * 6)) {

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