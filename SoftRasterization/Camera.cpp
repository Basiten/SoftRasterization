#include "Camera.h"
#include <iostream>
#define PI 3.1415926;

Camera::Camera(glm::vec3 cameraPos, glm::vec3 gazeDir, glm::vec3 up, float near, float far, float fov, float aspect):
	near(near),far(far),fov(fov),aspect(aspect) {
	this->cameraPos = cameraPos;
	this->gazeDir = glm::normalize(gazeDir);
	this->up = glm::normalize(up);
}

glm::mat4 Camera::getViewMatrix()
{
	// std::cout << "get view matrix.\n" << std::endl;
	// T-View (translate the origin)
	// !!! GLM 中的矩阵初始化和直觉相反！ 按照正常矩阵元素初始化后需要进行转置
	//glm::mat4 translate(1, 0, 0, -cameraPos.x,
	//	0, 1, 0, -cameraPos.y,
	//	0, 0, 1, -cameraPos.z,
	//	0, 0, 0, 1);
	glm::mat4 translate = glm::translate(glm::mat4(1.f), -cameraPos);
	glm::vec3 tmp = glm::cross(gazeDir, up);
	//std::cout << tmp.x << " "<<tmp.y << " "<<tmp.z << std::endl;
	
	// 根据GAMES101, rotate的转置即为逆rotate，加上glm的矩阵初始化问题，此处不需要再做任何求逆、转置处理
	glm::mat4 rotate(tmp.x, up.x, -gazeDir.x, 0.,
		tmp.y, up.y, -gazeDir.y, 0,
		tmp.z, up.z, -gazeDir.z, 0,
		0, 0, 0, 1);
	glm::mat4 res = rotate * translate;
	return res;
}

glm::mat4 Camera::getPerspectiveMatrix()
{
	float t, b, l, r;
	float fov_radians = this->fov / 180 * PI;
	t = abs(this->near) * std::tan(fov_radians / 2);
	// std::cout << "T: " << t<< std::endl;
	b = -t;
	r = this->aspect * t;
	l = -r;
	glm::mat4 ortho_sacle(2. / (r - l), 0, 0, 0,
		0, 2. / (t - b), 0, 0,
		0, 0, 2. / (this->near - this->far), 0,
		0, 0, 0, 1);
	glm::mat4 ortho_translate(1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		-(r + l) / 2., -(t + b) / 2., -(this->near + this->far) / 2., 1);
	glm::mat4 ortho = ortho_sacle * ortho_translate;

	glm::mat4 persp2ortho(this->near, 0, 0, 0,
		0, this->near, 0, 0,
		0, 0, this->near + this->far, 1,
		0, 0, -this->near * this->far, 0);

	// Get Orthogonal matrix
	return ortho * persp2ortho;
}

glm::mat4 Camera::getViewPerspectiveMatrix() {
	return this->getPerspectiveMatrix() * this->getViewMatrix();
}

glm::vec3 Camera::getPos()
{
	return this->cameraPos;
}
