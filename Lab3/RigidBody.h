#include "Model.h"

#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>


enum ShapeType {
	cube = 0,
	sphere = 1
};

glm::vec3 ZERO_VEC = glm::vec3(0.0);

class RigidBody
{
private: 
	glm::vec3 position;
	glm::vec3 linearVelocity;
	glm::vec3 rotation;
	glm::vec3 rotationVelocity;
public:
	GLfloat scale;
	GLfloat mass;
	GLfloat volume;
	GLfloat density;
	GLfloat restitution;
	GLfloat friction;
	ShapeType shapeType;
	

	RigidBody(	
		glm::vec3 position,
		glm::vec3 linearVelocity,
		glm::vec3 rotation,
		glm::vec3 rotationVelocity,
		GLfloat scale,
		GLfloat mass,
		GLfloat volume,
		GLfloat density,
		GLfloat restitution,
		GLfloat friction,
		ShapeType shapeType) 
	{
		this->position = position;
		this->linearVelocity = ZERO_VEC;
		this->rotation = rotation;
		this->rotationVelocity = ZERO_VEC;
		this->scale = scale;
		this->mass = mass;
		this->volume = volume;
		this->density = density;
		this->restitution = restitution;
		this->friction = friction;
		this->shapeType = shapeType;
	} 

	void move();
	bool createCube(
		glm::vec3 position,
		glm::vec3 linearVelocity,
		glm::vec3 rotation,
		glm::vec3 rotationVelocity,
		GLfloat scale,
		GLfloat mass,
		GLfloat volume,
		GLfloat density,
		GLfloat restitution,
		GLfloat friction);
	bool createSphere(
		glm::vec3 position,
		glm::vec3 linearVelocity,
		glm::vec3 rotation,
		glm::vec3 rotationVelocity,
		GLfloat scale,
		GLfloat mass,
		GLfloat volume,
		GLfloat density,
		GLfloat restitution,
		GLfloat friction);

	

};

