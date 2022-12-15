#include "RigidBody.h"


RigidBody::RigidBody(
	glm::vec3 position,
	glm::vec3 linearVelocity,
	glm::vec3 rotation,
	glm::vec3 rotationVelocity,
	glm::vec3 force,
	GLfloat mass,
	GLfloat restitution,
	GLfloat friction)
{
	this->position = position;
	this->linearVelocity = linearVelocity;
	this->rotation = rotation;
	this->rotationVelocity = rotationVelocity;
	this->force = force;
	this->mass = mass;
	this->restitution = restitution;
	this->friction = friction;
}

void RigidBody::update(GLfloat dt)
{
	linearVelocity += force / mass * dt;
	position += linearVelocity * dt;
	rotation += rotationVelocity * dt;
	force = glm::vec3(0, 0, 0) * mass;
}

void RigidBody::move(glm::vec3 amount)
{
	position += amount;
}

void RigidBody::setPosition(glm::vec3 position)
{
	this->position = position;
}

void RigidBody::applyForce(glm::vec3 force)
{
	this->force += force;
}

Sphere::Sphere(glm::vec3 position,
	glm::vec3 linearVelocity,
	glm::vec3 rotation,
	glm::vec3 rotationVelocity,
	glm::vec3 force,
	GLfloat mass,
	GLfloat restitution,
	GLfloat friction,
	GLfloat radius)
	: RigidBody(position,
		linearVelocity,
		rotation,
		rotationVelocity,
		force,
		mass,
		restitution,
		friction), radius(radius) {}

bool Sphere::intersect(Sphere a, Sphere b, glm::vec3& normal, GLfloat& depth)
{
	normal = glm::vec3(0.0);
	depth = 0.0f;

	GLfloat distance = glm::distance(a.position, b.position);
	GLfloat radii = a.radius + b.radius;

	if (distance >= radii)
	{
		return false;
	}

	normal = glm::normalize(a.position - b.position);
	depth = radii - distance;

	return true;
}

bool Sphere::intersectBound(glm::vec3& normal, glm::vec3& depth)
{
	normal = glm::vec3(0.0);
	depth = glm::vec3(0.0);

	bool isHit = false;

	GLfloat dist;
	/*dist = position.x - (-15) - radius;
	if (dist < 0) {
		isHit = true;
		normal.x += 1;
		depth.x += dist; 
	}
	dist = 15 - position.x - radius;
	if (dist < 0) {
		isHit = true;
		normal.x -= 1;
		depth.x += dist; 
	}
	dist = position.y - 0 - radius;
	if (dist < 0) {
		isHit = true;
		normal.y += 1;
		depth.y += dist; 
	}
	dist = 30 - position.y - radius;
	if (dist < 0) {
		isHit = true;
		normal.y -= 1;
		depth.y += dist;
	}
	dist = position.z - (-15) - radius;
	if (dist < 0) {
		isHit = true;
		normal.z += 1;
		depth.z += dist; 
	}
	dist = 15 - position.z - radius;
	if (dist < 0) {
		isHit = true;
		normal.z -= 1;
		depth.z += dist;
	}*/
	return isHit;
}
