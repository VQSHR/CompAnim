#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

enum ShapeType {
	circle = 0,
	square = 1
};

class RigidBody
{
private: 
	glm::vec3 position;
	glm::vec3 linearVelocity;
	glm::vec3 rotation;
	glm::vec3 rotationVelocity;
public:
	GLfloat mass;
	GLfloat volume;
	GLfloat density;
	GLfloat restitution;
	GLfloat friction;

	RigidBody(	
		glm::vec3 position,
		glm::vec3 linearVelocity,
		glm::vec3 rotation,
		glm::vec3 rotationVelocity,
		GLfloat mass,
		GLfloat volume,
		GLfloat density,
		GLfloat restitution,
		GLfloat friction) 
	{

	}

};

