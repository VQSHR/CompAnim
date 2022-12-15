#include "Flock.h"


FlockAgent::FlockAgent(glm::vec3 position, glm::vec3 linearVelocity)
{
	this->position = position;
	this->linearVelocity = linearVelocity;
	this->rotation = glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
	this->rotationVelocity = glm::vec3(0.0f);
	this->force = glm::vec3(0.0f);

}

// updates linearVelocity based on force
// updates position based on linearVelocity
// updates rotation
// resets force
// clear neighbors lists
void FlockAgent::update(GLfloat dt)
{
	//updates linearVelocity based on force
	linearVelocity += force * dt;
	// truncate velocity larger than 10 in magnitude
	if (glm::dot(linearVelocity, linearVelocity) > 100.0f)
		linearVelocity = 10.0f * glm::normalize(linearVelocity);
	// updates position based on linearVelocity
	position += linearVelocity * dt;
	// set rotation to the same orientation as velocity
	rotation = MyUtil::findRotation(glm::vec3(0, 1, 0), glm::normalize(linearVelocity));
	// reset force to 0
	force = glm::vec3(0, 0, 0);
	// clear neighbors lists
	neighbors.clear();
	closeNeighbors.clear();
}

void FlockAgent::move(glm::vec3 amount)
{
	this->position += position;
}

void FlockAgent::applyForce(glm::vec3 force)
{
	this->force += force;
}

void FlockAgent::addNeighbor(FlockAgent* f)
{
	neighbors.push_back(f);
}

void FlockAgent::addCloseNeighbor(FlockAgent* f)
{
	closeNeighbors.push_back(f);
}

// add a force pointing towards the center of all neighbors
void FlockAgent::doCohesion()
{
	GLfloat coeff = 10.0f; // weight of cohesion
	if (neighbors.empty()) return;
	glm::vec3 destination{};
	// calculate average position of neighbors
	for (size_t i = 0; i < neighbors.size(); i++) {
		destination += neighbors[i]->position;
	}
	destination /= neighbors.size();
	glm::vec3 force = glm::normalize(destination);
	applyForce(force * coeff);
}

// add a force pointing towards the average velocity of all neighbors
void FlockAgent::doAlignment()
{
	GLfloat coeff = 10.0f; // weight of alignment
	if (neighbors.empty()) return;
	glm::vec3 destination{};
	// calculate average velocity of neighbors
	for (size_t i = 0; i < neighbors.size(); i++) {
		destination += neighbors[i]->linearVelocity;
	}
	destination /= neighbors.size();
	glm::vec3 force = glm::normalize(destination);
	applyForce(force * coeff);
}

// add a force pointing away from the average position of nearby agents
// cancel all force and add a force to move away from walls when near walls
void FlockAgent::doAvoidance()
{
	GLfloat coeff = 10.0f; // weight of avoidance
	if (!closeNeighbors.empty()) {
		glm::vec3 destination{};
		for (size_t i = 0; i < closeNeighbors.size(); i++) {
			// add all vectors pointing from neighbor to this agent
			destination += position - closeNeighbors[i]->position;
		}
		destination /= closeNeighbors.size();
		glm::vec3 force = glm::normalize(destination);
		applyForce(force * coeff);
	}
	// cancel all force and add a force to move away from walls when near walls
	if (position.x < -20) {
		force = glm::vec3(0, 0, 0);
		applyForce(glm::vec3(coeff, 0, 0));
	}
	if (position.x > 20) {
		force = glm::vec3(0, 0, 0);
		applyForce(glm::vec3(-coeff, 0, 0));
	}
	if (position.z < -20) {
		force = glm::vec3(0, 0, 0);
		applyForce(glm::vec3(0, 0, coeff));
	}
	if (position.z > 20) {
		force = glm::vec3(0, 0, 0);
		applyForce(glm::vec3(0, 0, -coeff));
	}
	if (position.y < 10) {
		force = glm::vec3(0, 0, 0);
		applyForce(glm::vec3(0, coeff, 0));
	}
	if (position.y > 50) {
		force = glm::vec3(0, 0, 0);
		applyForce(glm::vec3(0, -coeff, 0));
	}
}

Flock::Flock()
{
	avoidanceRadius = 0;
	cohesionRadius = 0;
}

Flock::Flock(GLfloat avoidanceRadius, GLfloat cohesionRadius, GLuint size) : avoidanceRadius(avoidanceRadius), cohesionRadius(cohesionRadius)
{
	const glm::vec3 ZERO_VEC = glm::vec3(0);
	for (size_t i = 0; i < size; i++) {
		glm::vec3 random_position = glm::linearRand(glm::vec3(-10, 5, -10), glm::vec3(10, 25, 10));
		glm::vec3 random_linearVelocity = glm::linearRand(glm::vec3(-10, -10, -10), glm::vec3(10, 10, 10));
		list.push_back(FlockAgent(random_position, random_linearVelocity));
	}
	findNeighbors();
}

// find neighbors for all flockAgents in the list
// push those in cohesion range to agent->neighbors
// push those in avoidance range to agent->closeNeighbors
void Flock::findNeighbors()
{
	for (size_t i = 0; i < list.size() - 1; i++) {
		FlockAgent* agentA = &list[i];
		for (size_t j = i + 1; j < list.size(); j++) {
			FlockAgent* agentB = &list[j];
			GLfloat dist = MyUtil::distance(agentA->position, agentB->position);
			if (dist < cohesionRadius) {
				// push those in cohesion range to agent->neighbors
				agentA->addNeighbor(agentB);
				agentB->addNeighbor(agentA);

				if (dist < avoidanceRadius) {
					// push those in avoidance range to agent->closeNeighbors
					agentA->addCloseNeighbor(agentB);
					agentB->addCloseNeighbor(agentA);
				}
			}

		}
	}
}
