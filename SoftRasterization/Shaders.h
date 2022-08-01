#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "ImagesUtils.h"
#include "Camera.h"
#include <vector>
#include "Light.h"

class BaseShader{
public:
	glm::mat4 model = glm::mat4(1.f);
	glm::mat4 view = glm::mat4(1.f);
	glm::mat4 projection = glm::mat4(1.f);
	std::vector<glm::vec2> texCoords;
	Image* viewPort;
	Image* texture;
	BaseShader();

	glm::vec3 vertex(glm::vec3 vert, glm::vec2 texCoords, int index);
	bool fragment(glm::vec3 barycentric, ImgColor& color);
	void setMatrix(glm::mat4 model, glm::mat4 view, glm::mat4 projection);
	void setViewPort(Image* viewPort);
	void setTexture(Image* texture);

};

// Blinn-Phong着色模型
class BlinnPhongShader{
public:
	glm::mat4 model = glm::mat4(1.f);
	glm::mat4 view = glm::mat4(1.f);
	glm::mat4 projection = glm::mat4(1.f);
	std::vector<glm::vec3> coords;
	std::vector<glm::vec2> texCoords;
	Image* viewPort;
	Image* texture;
	Image* normalMap;
	Image* shininessMap;
	std::vector<pointLight> pointLights;
	std::vector<Image> shadowMaps;
	glm::vec3 cameraPos;
	BlinnPhongShader();

	glm::vec3 vertex(glm::vec3 vert, glm::vec2 texCoords, int index);
	bool fragment(glm::vec3 barycentric, ImgColor& color);
	glm::vec3 calcSingleLight(glm::vec3 barycentric, pointLight light, Image& shadowMap);
	void setMatrix(glm::mat4 model, glm::mat4 view, glm::mat4 projection);
	void setViewPort(Image* viewPort);
	void setTexture(Image* texture);
	void setNormalMap(Image* normalMap);
	void setShininessMap(Image* shininessMap);
	void setCameraPos(Camera& camera);
	void addPointLight(pointLight &light);
	void addPointLight(glm::vec3 pos, glm::vec3 color, float ambient, float attenuationConstant, float attenuationFirstOrder, float attenuationSecondOrder);
	void attachShadowMaps(std::vector<Image> shadowMaps);
};


// Depth Shader
class DepthShader {
public:
	Camera lightViewCamera;
	glm::mat4 model = glm::mat4(1.f);
	glm::mat4 view = glm::mat4(1.f);
	glm::mat4 projection = glm::mat4(1.f);
	// 记录的是相机坐标系的坐标
	std::vector<glm::vec3> coords;
	Image* shadowMap;
	float* depthBuffer;

	DepthShader(pointLight& light, Image& shadowMap);
	~DepthShader() { delete[] depthBuffer; };
	void setMatrix();
	glm::vec3 vertex(glm::vec3 vert, int index);
	bool fragment(glm::vec3 barycentric, ImgColor& color);
};
