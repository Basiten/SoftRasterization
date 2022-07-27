#include "Shaders.h"

BaseShader::BaseShader() {
	texCoords.resize(3);
}

glm::vec3 BaseShader::vertex(glm::vec3 vert, glm::vec2 texCoords, int index)
{
	this->texCoords[index] = texCoords;
	glm::vec4 world_coords_4d = projection * view * model * glm::vec4(vert,1.f);
	glm::vec3 world_coords = glm::vec3(world_coords_4d.x, world_coords_4d.y, world_coords_4d.z) / world_coords_4d.w;
	glm::vec3 screen_coords = glm::vec3((world_coords.x + 1.) * viewPort->getWidth() / 2., (world_coords.y + 1.) * viewPort->getHeight() / 2., world_coords.z);
	return screen_coords;
}

bool BaseShader::fragment(glm::vec3 barycentric, ImgColor& color)
{
	glm::vec2 texCoordsInterpolated(barycentric.x * texCoords[0].x + barycentric.y * texCoords[1].x + barycentric.z * texCoords[2].x,
		barycentric.x * texCoords[0].y + barycentric.y * texCoords[1].y + barycentric.z * texCoords[2].y);
	color = texture->get(texCoordsInterpolated.x * texture->getWidth(), texCoordsInterpolated.y * texture->getHeight());
	return true;
}

void BaseShader::setMatrix(glm::mat4 model, glm::mat4 view, glm::mat4 projection) {
	this->model = model;
	this->view = view;
	this->projection = projection;
}

void BaseShader::setViewPort(Image* viewPort){
	this->viewPort = viewPort;
}

void BaseShader::setTexture(Image* texture)
{
	this->texture = texture;
}

// Blinn-Phong着色模型
//---------------------------------------------------
BlinnPhongShader::BlinnPhongShader() {
	coords.resize(3);
	texCoords.resize(3);
}

glm::vec3 BlinnPhongShader::vertex(glm::vec3 vert, glm::vec2 texCoords, int index)
{
	this->texCoords[index] = texCoords;
	glm::vec4 word_coords_4d = model * glm::vec4(vert, 1.f);
	this->coords[index] = glm::vec3(word_coords_4d.x, word_coords_4d.y, word_coords_4d.z) / word_coords_4d.w;
	glm::vec4 coords_4d = projection * view * model * glm::vec4(vert, 1.f);
	glm::vec3 coords = glm::vec3(coords_4d.x, coords_4d.y, coords_4d.z) / coords_4d.w;
	glm::vec3 screen_coords = glm::vec3((coords.x + 1.) * viewPort->getWidth() / 2., (coords.y + 1.) * viewPort->getHeight() / 2., coords.z);
	return screen_coords;
}

bool BlinnPhongShader::fragment(glm::vec3 barycentric, ImgColor& color)
{
	glm::vec3 colorSum(0.f);
	int idx = 0;
	for (auto light : this->pointLights) {
		colorSum = colorSum + calcSingleLight(barycentric, light, shadowMaps[idx]);
		idx++;
	}
	color.r = (int)colorSum.x;
	color.g = (int)colorSum.y;
	color.b = (int)colorSum.z;
	color.a = (uint8_t) 255;
	return true;
}

glm::vec3 BlinnPhongShader::calcSingleLight(glm::vec3 barycentric, pointLight light, Image& shadowMap) {
	// 世界坐标系
	glm::vec3 worldPos(
		barycentric.x * coords[0].x + barycentric.y * coords[1].x + barycentric.z * coords[2].x,
		barycentric.x * coords[0].y + barycentric.y * coords[1].y + barycentric.z * coords[2].y,
		barycentric.x * coords[0].z + barycentric.y * coords[1].z + barycentric.z * coords[2].z);

	// 是否在阴影中？
	Camera lightCam(light.lightPos, -light.lightPos, glm::vec3(0.f, 1.f, 0.f), -1.f, -10.f, 45, 1);
	glm::vec4 camSpaceCoords = lightCam.getViewPerspectiveMatrix() * glm::vec4(worldPos,1.f);
	camSpaceCoords = camSpaceCoords / camSpaceCoords.w;
	// 在camScreen中的深度
	ImgColor depth = shadowMap.get((camSpaceCoords.x + 1.) * shadowMap.getWidth() / 2., (camSpaceCoords.y + 1.) * shadowMap.getHeight() / 2.);
	bool inShadow = false;
	camSpaceCoords.z < depth.r ? true : false;

	// texture 和 normal map、 shininess信息
	glm::vec2 texCoordsInterpolated(barycentric.x * texCoords[0].x + barycentric.y * texCoords[1].x + barycentric.z * texCoords[2].x,
		barycentric.x * texCoords[0].y + barycentric.y * texCoords[1].y + barycentric.z * texCoords[2].y);
	ImgColor color = texture->get(texCoordsInterpolated.x * texture->getWidth(), texCoordsInterpolated.y * texture->getHeight());
	glm::vec3 texColor(color.r, color.g, color.b);
	ImgColor normal = normalMap->get(texCoordsInterpolated.x * normalMap->getWidth(), texCoordsInterpolated.y * normalMap->getHeight());
	glm::vec3 normalDir(normal.r, normal.g, normal.b);
	// RGB 范围为0-255（r,g,b分量）， NormalDir的范围为-1-1（x,y,z分量）
	normalDir = glm::normalize(normalDir-128.f);
	int shininess = 4;
	if (this->shininessMap) {
		ImgColor shininessColor = shininessMap->get(texCoordsInterpolated.x * shininessMap->getWidth(), texCoordsInterpolated.y * shininessMap->getHeight());
		shininess = shininessColor.r;
	}
	// std::cout << "World Pos: " << worldPos.x << " " << worldPos.y << " " << worldPos.z << std::endl;
	glm::vec3 lightDir = light.lightPos - worldPos;
	float lightDistance = glm::length(lightDir);
	lightDir = glm::normalize(lightDir);

	// ambient
	glm::vec3 ambient = light.ambient * texColor * light.color;

	// diffuse
	float diffuseCoeff = std::max(glm::dot(lightDir, normalDir),0.f);
	glm::vec3 diffuse = (1.f/(light.attenuationCoeff[0]+light.attenuationCoeff[1] * (lightDistance) + light.attenuationCoeff[2] * (lightDistance * lightDistance)))
		+ diffuseCoeff * light.color * texColor;

	// specular

	glm::vec3 viewDir = glm::normalize((cameraPos - worldPos));
	glm::vec3 halfVec = glm::normalize(viewDir + lightDir);
	float specularCoeff = std::pow(std::max(glm::dot(halfVec, normalDir),0.f),shininess);
	//std::cout << "SpecularCoeff: " << specularCoeff << std::endl;
	glm::vec3 specular = (1.f / (light.attenuationCoeff[0] + light.attenuationCoeff[1] * (lightDistance)+light.attenuationCoeff[2] * (lightDistance * lightDistance)))
		+ specularCoeff * light.color * texColor;

	// return glm::vec3(normalDir * 255.f);
	// return specular;
	// return glm::vec3(lightDistance * 100.f);
	// return lightDir * 255.f;
	// return glm::vec3(diffuseCoeff * 255.f);
	// return glm::uvec3(specularCoeff * 255.f);
	// return ambient;
	// return diffuse;
	if (inShadow) {
		return ambient;
	}
	return ambient + diffuse + specular;


}

void BlinnPhongShader::setMatrix(glm::mat4 model, glm::mat4 view, glm::mat4 projection) {
	this->model = model;
	this->view = view;
	this->projection = projection;
}

void BlinnPhongShader::setViewPort(Image* viewPort) {
	this->viewPort = viewPort;
}

void BlinnPhongShader::setTexture(Image* texture)
{
	this->texture = texture;
}

void BlinnPhongShader::setNormalMap(Image* normalMap) {
	this->normalMap = normalMap;
}

void BlinnPhongShader::setShininessMap(Image* shininessMap)
{
	this->shininessMap = shininessMap;
}

void BlinnPhongShader::setCameraPos(Camera& camera)
{
	cameraPos = camera.getPos();
}

void BlinnPhongShader::addPointLight(pointLight &light)
{
	pointLights.push_back(light);
}

void BlinnPhongShader::addPointLight(glm::vec3 pos, glm::vec3 color, float ambient, float attenuationConstant, float attenuationFirstOrder, float attenuationSecondOrder)
{
	this->pointLights.emplace_back(pos, color, ambient, attenuationConstant, attenuationFirstOrder, attenuationSecondOrder);
}

void BlinnPhongShader::attachShadowMaps(std::vector<Image> shadowMaps)
{
	this->shadowMaps = shadowMaps;
}

// Depth-Shader
DepthShader::DepthShader(pointLight& light, Image& shadowMap)
{
	coords.resize(3);
	// light 位置的camera不需要特意设定up
	this->lightViewCamera = Camera(light.lightPos, -light.lightPos, glm::vec3(0.f,1.f,0.f), -1.f, -10.f, 45, 1);
	this->setMatrix();
	this->shadowMap = &shadowMap;
	this->depthBuffer = new float[shadowMap.getWidth() * shadowMap.getHeight()];
	for (int idx = 0; idx < shadowMap.getWidth() * shadowMap.getHeight(); idx++) {
		depthBuffer[idx] = -std::numeric_limits<float>::max();
		//zbuffer[idx] = 100;
	}
	printf("Depth Shader Size: %d \n", shadowMap.getWidth() * shadowMap.getHeight());
}

void DepthShader::setMatrix()
{
	this->model = glm::mat4(1.f);
	this->view = lightViewCamera.getViewMatrix();
	this->projection = lightViewCamera.getPerspectiveMatrix();
}

glm::vec3 DepthShader::vertex(glm::vec3 vert, int index)
{
	glm::vec4 word_coords_4d = model * glm::vec4(vert, 1.f);
	this->coords[index] = glm::vec3(word_coords_4d.x, word_coords_4d.y, word_coords_4d.z) / word_coords_4d.w;
	glm::vec4 coords_4d = projection * view * model * glm::vec4(vert, 1.f);
	glm::vec3 coords = glm::vec3(coords_4d.x, coords_4d.y, coords_4d.z) / coords_4d.w;
	glm::vec3 screen_coords = glm::vec3((coords.x + 1.) * shadowMap->getWidth() / 2., (coords.y + 1.) * shadowMap->getHeight() / 2., coords.z);
	return screen_coords;
}

bool DepthShader::fragment(glm::vec3 barycentric, ImgColor& color)
{
	float depth = barycentric.x * coords[0].z + barycentric.y * coords[1].z + barycentric.z * coords[2].z + 3;
	//printf("depth: %f.\n", depth);
	color.r = depth * 255 / 3;
	color.g = depth * 255 / 3;
	color.b = depth * 255 / 3;
	return true;
}
