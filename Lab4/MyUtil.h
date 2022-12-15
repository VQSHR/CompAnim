#pragma once
#include <glad/glad.h>
#include <gl/GL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

class MyUtil {
public:

	static GLfloat distance(const glm::vec3& p1, const glm::vec3& p2) {
		return
			glm::sqrt(
				glm::pow((p1.x - p2.x), 2) +
				glm::pow((p1.y - p2.y), 2) +
				glm::pow((p1.z - p2.z), 2));
	}

	static glm::mat4 translate(const glm::mat4& m, const glm::vec3& v) {
		glm::mat4 result = m;
		
		result[3] = m[0] * v[0] + m[1] * v[1] + m[2] * v[2] + m[3];
		return result;
	}

	static glm::mat4 scale(const glm::mat4& m, const glm::vec3& v) {
		glm::mat4 result = m;
		result[0] = m[0] * v[0];
		result[1] = m[1] * v[1];
		result[2] = m[2] * v[2];
		result[3] = m[3];
		return result;
	}

	static glm::mat4 rotate(const glm::mat4& m, const glm::vec3& v) {
		glm::quat q = euler2quat(v);
		glm::mat4 R = quat2mat4(q);
		return m * R;
	}

	static GLfloat catmullRom(GLfloat p0, GLfloat p1, GLfloat p2, GLfloat p3, GLfloat t, GLboolean tan = false) {
		GLfloat MArray[16] = {
			-0.5,  1.5, -1.5,  0.5,
			 1.0, -2.5,  2.0, -0.5,
			-0.5,  0.0,  0.5,  0.0,
			 0.0,  1.0,  0.0,  0.0
		};
		glm::vec4 T;
		if (!tan) {
			GLdouble t2 = t * t;
			GLdouble t3 = t2 * t;
			T = glm::vec4(t3, t2, t, 1);
		}
		else {
			T = glm::vec4(3 * t * t, 2 * t, 1, 0);
		}

		glm::mat4 M = glm::transpose(glm::make_mat4(MArray));
		glm::vec4 P(p0, p1, p2, p3);
		GLfloat result = glm::dot(T * M, P);
		return result;
	}

	static GLfloat bSpline(GLfloat p0, GLfloat p1, GLfloat p2, GLfloat p3, GLfloat t, GLboolean tan = false) {
		GLfloat MArray[16] = {
			-1 / 6.0,  3 / 6.0, -3 / 6.0, 1 / 6.0,
			 3 / 6.0, -6 / 6.0,  3 / 6.0,       0,
			-3 / 6.0,        0,  3 / 6.0,       0,
			 1 / 6.0,  4 / 6.0,  1 / 6.0,       0
		};
		glm::vec4 T;
		if (!tan) {
			GLdouble t2 = t * t;
			GLdouble t3 = t2 * t;
			T = glm::vec4(t3, t2, t, 1);
		}
		else {
			T = glm::vec4(3 * t * t, 2 * t, 1, 0);
		}
		glm::mat4 M = glm::transpose(glm::make_mat4(MArray));
		glm::vec4 P(p0, p1, p2, p3);

		GLfloat result = glm::dot(T * M, P);
		return result;
	}

	static GLfloat vector2angle(GLfloat z, GLfloat x)
	{
		return glm::atan(z, x);
	}

	static glm::quat quatMul(glm::quat q1, glm::quat q2) {
		glm::quat q;
		GLfloat w1 = q1.w;
		GLfloat w2 = q2.w;
		glm::vec3 v1(q1.x, q1.y, q1.z);
		glm::vec3 v2(q2.x, q2.y, q2.z);

		q.w = w1 * w2 - glm::dot(v1, v2);
		glm::vec3 v = w1 * v2 + w2 * v1 + glm::cross(v1, v2);
		q.x = v.x;
		q.y = v.y;
		q.z = v.z;
		return q;
	}

	static glm::quat euler2quat(glm::vec3 eulerAngles)
	{
		GLfloat x = eulerAngles.x * 0.5;
		GLfloat y = eulerAngles.y * 0.5;
		GLfloat z = eulerAngles.z * 0.5;

		glm::quat qz, qy, qx;
		qz = glm::quat(glm::cos(z), 0, 0, glm::sin(z));
		qy = glm::quat(glm::cos(y), 0, glm::sin(y), 0);
		qx = glm::quat(glm::cos(x), glm::sin(x), 0, 0);

		glm::quat q = quatMul(quatMul(qz, qy), qx);
		return q;
	}

	static glm::mat4 quat2mat4(glm::quat q) {
		GLfloat w = q.w;
		GLfloat x = q.x;
		GLfloat y = q.y;
		GLfloat z = q.z;

		GLfloat x2 = x * x;
		GLfloat y2 = y * y;
		GLfloat z2 = z * z;

		GLfloat mat4array[16] = {
			1 - 2 * y2 - 2 * z2,   2 * x * y - 2 * w * z,   2 * x * z + 2 * w * y, 0,
			  2 * x * y + 2 * w * z, 1 - 2 * x2 - 2 * z2,   2 * y * z - 2 * w * x, 0,
			  2 * x * z - 2 * w * y,   2 * y * z + 2 * w * x, 1 - 2 * x2 - 2 * y2, 0,
						  0,               0,               0, 1
		};

		return glm::transpose(glm::make_mat4(mat4array));
	}

	static GLfloat lerp(GLfloat p0, GLfloat p1, GLfloat t) {
		GLfloat MArray[4] = { -1, 1, 1, 0 };
		glm::vec2 T(t, 1);
		glm::mat2 M = glm::transpose(glm::make_mat2(MArray));
		glm::vec2 P(p0, p1);
		GLfloat result = glm::dot(T * M, P);
		return result;
	}

	static glm::quat findRotation(const glm::vec3& v1, const glm::vec3& v2) {
		glm::quat q;
		glm::vec3 a = glm::cross(v1, v2);
		q.x = a.x;
		q.y = a.y;
		q.z = a.z;
		q.w = glm::sqrt(glm::dot(v1, v1) * glm::dot(v2, v2)) + glm::dot(v1, v2);
		q = glm::normalize(q);
		return q;
	}

};