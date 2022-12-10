#include "RigidBody.h"


RigidBody::RigidBody(
	glm::vec3 position,
	glm::vec3 linearVelocity,
	glm::vec3 rotation,
	glm::vec3 rotationVelocity,
	GLfloat mass,
	GLfloat restitution,
	GLfloat friction)
{
	this->position = position;
	this->linearVelocity = glm::vec3(0.0);
	this->rotation = rotation;
	this->rotationVelocity = glm::vec3(0.0);
	this->mass = mass;
	this->restitution = restitution;
	this->friction = friction;
}

void RigidBody::move()
{
}

Sphere::Sphere(glm::vec3 position,
	glm::vec3 linearVelocity,
	glm::vec3 rotation,
	glm::vec3 rotationVelocity,
	GLfloat mass,
	GLfloat restitution,
	GLfloat friction,
	GLfloat radius)
	: RigidBody(position,
		linearVelocity,
		rotation,
		rotationVelocity,
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
