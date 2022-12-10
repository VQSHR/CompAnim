

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

class RigidBody
{
public: 
	glm::vec3 position;
	glm::vec3 linearVelocity;
	glm::vec3 rotation;
	glm::vec3 rotationVelocity;

	GLfloat mass;
	GLfloat restitution;
	GLfloat friction;
	

	RigidBody(
		glm::vec3 position,
		glm::vec3 linearVelocity,
		glm::vec3 rotation,
		glm::vec3 rotationVelocity,
		GLfloat mass,
		GLfloat restitution,
		GLfloat friction);

	void move();

};

class Sphere : public RigidBody {
public:
	GLfloat radius;
	Sphere(glm::vec3 position,
		glm::vec3 linearVelocity,
		glm::vec3 rotation,
		glm::vec3 rotationVelocity,
		GLfloat mass,
		GLfloat restitution,
		GLfloat friction,
		GLfloat radius);
	bool static intersect(Sphere a, Sphere b, glm::vec3& normal, GLfloat& depth);
};