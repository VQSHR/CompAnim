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
const GLuint SCR_WIDTH = 800;
const GLuint SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 45.0f, 45.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -35.0f);

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
glm::vec3 lightPos(0.0f, 30.0f, 0.0f);

GLvoid drawBox(GLuint VAO, Shader modelShader);
GLvoid resolveCollision(Sphere& a, Sphere& b, glm::vec3 normal);

//================================
// init
//================================
GLvoid init(GLvoid) {
	sphereList.clear();
	const glm::vec3 ZERO_VEC = glm::vec3(0);
	// create 10 random spheres
	for (size_t i = 0; i < 10; i++) {
		// generate random parameters
		glm::vec3 random_position = glm::linearRand(glm::vec3(-10, 5, -10), glm::vec3(10, 25, 10));
		glm::vec3 random_linearVelocity = glm::linearRand(glm::vec3(-10, -10, -10), glm::vec3(10, 10, 10));
		GLfloat random_radius = glm::linearRand(2.0f, 3.0f);
		GLfloat random_mass = glm::linearRand(20, 30);
		// push back into list
		sphereList.push_back(Sphere(random_position, random_linearVelocity, ZERO_VEC, ZERO_VEC, ZERO_VEC, random_mass, 0.8, 0, random_radius));
		// random colors for each sphere
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
	Model sphere("models/sphere.obj");

	// vertices info for drawing the floor
	GLfloat vertices[] = {
	//	position		|	normals
		 15.0f, 0,  15.0f,	0, 1, 0, // top right
		 15.0f, 0, -15.0f,  0, 1, 0, // bottom right
		-15.0f, 0,  15.0f,  0, 1, 0, // bottom left
		-15.0f, 0, -15.0f,  0, 1, 0  // top left 
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

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 *sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

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
		modelShader.setVec3("material.diffuse", 0.7f, 0.3f, 0.0f);
		modelShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
		modelShader.setFloat("material.shininess", 2.0f);

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		modelShader.setMat4("projection", projection);
		modelShader.setMat4("view", view);

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
			modelShader.setVec3("material.diffuse", colorList[i]);
			sphere.Draw(modelShader);

			// normal and depth for boundary collisions
			glm::vec3 normal;
			glm::vec3 depth;

			// check for collision on boundaries
			if (s->intersectBound(normal, depth)) {
				// move out of overlap
				s->move(-normal * depth);
				// resolve collision
				s->linearVelocity += - (1 + s->restitution) * glm::dot(s->linearVelocity, normal) * (normal) / glm::dot(normal, normal);
			}
			// update new state for sphere: move according to velocity and deltaTime
			s->update(deltaTime);
		}

		// move spheres out of intersection
		for (int i = 0; i < sphereList.size() - 1; i++) {
			Sphere* a = &sphereList[i];
			for (int j = i + 1; j < sphereList.size(); j++) {
				Sphere* b = &sphereList[j];
				glm::vec3 normal;
				GLfloat depth;

				// detect collision between spheres
				if (Sphere::intersect(*a, *b, normal, depth)) {
					// move out of ovelap
					a->move(normal * depth * 0.5f);
					b->move(-normal * depth * 0.5f);
					// resolve collision
					resolveCollision(*a, *b, normal);
				}
				
			}
		}
		// draw physics simulation bounding box
		drawBox(VAO, modelShader);

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

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
GLvoid scroll_callback(GLFWwindow* window, GLdouble xoffset, GLdouble yoffset)
{
	camera.ProcessMouseScroll(static_cast<GLfloat>(yoffset));
}

GLvoid drawBox(GLuint VAO, Shader modelShader) {
	
	glBindVertexArray(VAO);
	modelShader.setVec3("material.ambient", 1.0f, 1.0f, 1.0f);
	modelShader.setVec3("material.diffuse", 1.0f, 1.0f, 1.0f);
	modelShader.setVec3("material.specular", 0.0f, 0.0f, 0.0f);
	modelShader.setFloat("material.shininess", 32);

	// draw floor
	glm::mat4 floorModel = glm::mat4(1.0);
	modelShader.setMat4("model", floorModel);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	// draw wall
	glm::mat4 wallModel;
	wallModel = MyUtil::translate(floorModel, glm::vec3(-15, 15, 0));
	wallModel = glm::rotate(wallModel, -glm::pi<GLfloat>() / 2.0f, glm::vec3(0, 0, 1));
	modelShader.setMat4("model", wallModel);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	wallModel = MyUtil::translate(floorModel, glm::vec3(15, 15, 0));
	wallModel = glm::rotate(wallModel, glm::pi<GLfloat>() / 2.0f, glm::vec3(0, 0, 1));
	modelShader.setMat4("model", wallModel);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	wallModel = MyUtil::translate(floorModel, glm::vec3(0, 15, -15));
	wallModel = glm::rotate(wallModel, glm::pi<GLfloat>() / 2.0f, glm::vec3(1, 0, 0));
	modelShader.setMat4("model", wallModel);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

GLvoid resolveCollision(Sphere& a, Sphere& b, glm::vec3 normal)
{
	// v1' = v1 + j * normal / mass1
	// v2' = v2 - j * normal / mass2
	// where:
	//		- (1 + restitution) * dot((v1 - v2), n)
	// j = ------------------------------------------
	//		 dot(n, n) * ((1 / mass1) + (1/ mass2))
	GLfloat eps = glm::min(a.restitution, b.restitution);
	glm::vec3 relativeVelocity = a.linearVelocity - b.linearVelocity;
	GLfloat j = - (1 + eps) * glm::dot(relativeVelocity, normal);
	j /= (1.0f / a.mass) + (1.0f / b.mass);
	a.linearVelocity += j * normal / a.mass;
	b.linearVelocity -= j * normal / b.mass;
	
}
