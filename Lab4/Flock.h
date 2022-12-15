#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <vector>
#include <glm/gtc/random.hpp>
#include "MyUtil.h"

class FlockAgent {
public:
	glm::vec3 position;
	glm::vec3 linearVelocity;
	glm::quat rotation;
	glm::vec3 rotationVelocity;
	glm::vec3 force;
	std::vector<FlockAgent*> neighbors;
	std::vector<FlockAgent*> closeNeighbors;
	
	FlockAgent(
		glm::vec3 position,
		glm::vec3 linearVelocity
	);

	void update(GLfloat dt);
	void move(glm::vec3 amount);
	void applyForce(glm::vec3 force);

	void addNeighbor(FlockAgent* f);
	void addCloseNeighbor(FlockAgent* f);

	void doCohesion();
	void doAlignment();
	void doAvoidance();
};

class Flock {
public:
	std::vector<FlockAgent> list;
	GLfloat avoidanceRadius;
	GLfloat cohesionRadius;
	Flock();
	Flock(GLfloat avoidanceRadius, GLfloat cohesionRadius, GLuint size);
	void findNeighbors();

	size_t size() {
		return list.size();
	}
};
