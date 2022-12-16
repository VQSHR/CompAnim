#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "RigidBody.h"
#include "MyMath.h"

#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>


GLvoid framebuffer_size_callback(GLFWwindow* window, GLint width, GLint height);
GLvoid mouse_callback(GLFWwindow* window, GLdouble xpos, GLdouble ypos);
GLvoid scroll_callback(GLFWwindow* window, GLdouble xoffset, GLdouble yoffset);
GLvoid processInput(GLFWwindow* window);

// settings
const GLuint SCR_WIDTH = 1600;
const GLuint SCR_HEIGHT = 900;

// camera
Camera camera(glm::vec3(0.0f, 100.0f, 100.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -45.0f);

GLfloat lastX = SCR_WIDTH / 2.0f;
GLfloat lastY = SCR_HEIGHT / 2.0f;
GLboolean firstMouse = true;

// timing
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// frame index
GLint frameCount = 0;

// object list
std::vector<Sphere> sphereList;
std::vector<glm::vec3> colorList;

// lighting
glm::vec3 lightPos[] = {
	glm::vec3(0.0f, 0.0f, 0.0f), 
	glm::vec3(0.0f, 0.0f, 0.0f)
};

// Universal gravity constant
GLfloat G = 6.67E-11;

//GLvoid drawBox(GLuint VAO, Shader modelShader);
//GLvoid resolveCollision(Sphere& a, Sphere& b, glm::vec3 normal);
GLvoid resolveGravitationalForce(Sphere* a, Sphere* b);

//================================
// init
//================================
GLvoid init(GLvoid) {
	sphereList.clear();
	const glm::vec3 ZERO_VEC = glm::vec3(0);
	
	GLfloat M = 1E15; // mass of star
	GLfloat d = 10; // distance from center
	GLfloat coeff = 0.9f; // coefficient to the velocity
	GLfloat radius = 0;
	// add star: center of the system at 0,0,0
	sphereList.push_back(Sphere(ZERO_VEC, ZERO_VEC, ZERO_VEC, ZERO_VEC, ZERO_VEC, M, 0, 0, 10));
	colorList.push_back(glm::vec3(0.9, 0.9, 0.0));
	// add planets
	for (int i = 0; i < 4; i++) {
		coeff += 0.1f;
		d += 10;
		radius += 1;
		sphereList.push_back(Sphere(glm::vec3(0, 0, d), glm::vec3(glm::sqrt(coeff * G * M / d), 0, 0), ZERO_VEC, ZERO_VEC, ZERO_VEC, 10, 0, 0, radius));
		// random colors
		glm::vec3 random_color = glm::linearRand(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		colorList.push_back(random_color);
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
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Lab5", NULL, NULL);
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
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

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
	Model sphere("models/sphere.obj");

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

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// enable shader before setting uniforms
		modelShader.use();
		modelShader.setVec3("lights[0].position", lightPos[0]);
		modelShader.setVec3("lights[1].position", lightPos[1]);
		modelShader.setVec3("viewPos", camera.Position);

		// light properties

		glm::vec3 lightColor;
		lightColor.x = 1.0f;
		lightColor.y = 1.0f;
		lightColor.z = 1.0f;
		glm::vec3 diffuseColor = lightColor * glm::vec3(1.0f);
		glm::vec3 ambientColor = diffuseColor * glm::vec3(0.3f);

		modelShader.setVec3("lights[0].ambient", ambientColor);
		modelShader.setVec3("lights[0].diffuse", diffuseColor);
		modelShader.setVec3("lights[0].specular", 1.0f, 1.0f, 1.0f);

		modelShader.setVec3("lights[1].ambient", ambientColor);
		modelShader.setVec3("lights[1].diffuse", diffuseColor);
		modelShader.setVec3("lights[1].specular", 1.0f, 1.0f, 1.0f);

		// material properties
		modelShader.setVec3("material.ambient", 1.0f, 1.0f, 1.0f);
		modelShader.setVec3("material.diffuse", 1.0f, 1.0f, 1.0f);
		modelShader.setVec3("material.specular", 1.0f, 1.0f, 1.0f);
		modelShader.setFloat("material.shininess", 32.0f);

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.1f, 1000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		modelShader.setMat4("projection", projection);
		modelShader.setMat4("view", view);

		// move spheres out of intersection
		for (int i = 0; i < sphereList.size() - 1; i++) {
			Sphere* a = &sphereList[i];
			for (int j = i + 1; j < sphereList.size(); j++) {
				Sphere* b = &sphereList[j];
				resolveGravitationalForce(a, b);
			}
		}
		// draw spheres
		for (int i = 0; i < sphereList.size(); i++) {
			Sphere* s = &sphereList[i];

			// draw according to Sphere properties
			glm::vec3 posVec = s->position;
			GLfloat radius = s->radius;
			glm::mat4 model;
			model = MyUtil::translate(glm::mat4(1.0), posVec);
			model = MyUtil::scale(model, glm::vec3(radius));
			modelShader.setMat4("model", model);
			// render the star differently: inverse the normal(making it emissive)
			if (i == 0) modelShader.setInt("inverseNormal", 1);
			else modelShader.setInt("inverseNormal", 0);
			// assign diffuse colors
			modelShader.setVec3("material.diffuse", colorList[i]);
			sphere.Draw(modelShader);

			// update new state for sphere: move according to velocity and deltaTime
			s->update(deltaTime);
		}

		
		// draw physics simulation bounding box
		//drawBox(VAO, modelShader);

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

	// press SPACE to start simulation
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) 
		init();

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

	//camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
GLvoid scroll_callback(GLFWwindow* window, GLdouble xoffset, GLdouble yoffset)
{
	camera.ProcessMouseScroll(static_cast<GLfloat>(yoffset));
}

GLvoid resolveGravitationalForce(Sphere* a, Sphere* b) {
	
	glm::vec3 diff = a->position - b->position;
	GLfloat d_sqr = glm::dot(diff, diff);
	GLfloat force = G * a->mass * b->mass / d_sqr;
	a->applyForce(-force * glm::normalize(diff));
	b->applyForce(force * glm::normalize(diff));
}
