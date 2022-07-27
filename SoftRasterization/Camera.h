#pragma once
#include <cmath>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class Camera {
public:
	Camera(){};
	Camera(glm::vec3 cameraPos, glm::vec3 gazeDir, glm::vec3 up, float near, float far, float fov, float aspect);
	glm::mat4 getViewMatrix();
	glm::mat4 getPerspectiveMatrix();
	glm::mat4 getViewPerspectiveMatrix();
	glm::vec3 getPos();
private:
	glm::vec3 cameraPos;
	glm::vec3 gazeDir;
	glm::vec3 up;
	float fov;
	float aspect;
	float far;
	float near;
};
