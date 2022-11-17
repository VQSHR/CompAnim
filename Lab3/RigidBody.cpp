#include "RigidBody.h"

void RigidBody::move()
{
}

bool RigidBody::createCube(glm::vec3 position, glm::vec3 linearVelocity, glm::vec3 rotation, glm::vec3 rotationVelocity, GLfloat scale, GLfloat mass, GLfloat volume, GLfloat density, GLfloat restitution, GLfloat friction)
{
	RigidBody(position, linearVelocity, rotation, rotationVelocity, scale, mass, volume, density, restitution, friction, shapeType);
	return false;
}

bool RigidBody::createSphere(glm::vec3 position, glm::vec3 linearVelocity, glm::vec3 rotation, glm::vec3 rotationVelocity, GLfloat scale, GLfloat mass, GLfloat volume, GLfloat density, GLfloat restitution, GLfloat friction)
{
	return false;
}
