#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

struct pointLight {
	glm::vec3 lightPos;
	float attenuationCoeff[3];
	glm::vec3 color;
	float ambient;
	pointLight(glm::vec3 lightPos, glm::vec3 color, float ambient, float attenuationConstant, float attenuationFirstOrder, float attenuationSecondOrder) :lightPos(lightPos), color(color), ambient(ambient) {
		attenuationCoeff[0] = attenuationConstant;
		attenuationCoeff[1] = attenuationFirstOrder;
		attenuationCoeff[2] = attenuationSecondOrder;
	};
};