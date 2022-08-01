#pragma once
#include "model.h"
#include "ImagesUtils.h"
#include "Lines.h"
#include "Camera.h"
#include "Shaders.h"
#include <cmath>
void DrawSingleTriangleWireFrame(glm::vec2 t0,glm::vec2 t1, glm::vec2 t2, Image& img, const ImgColor& color);
bool Inside(glm::vec2 Q, glm::vec2 P1, glm::vec2 P2, glm::vec2 P3);
void DrawSingleTriangle(glm::vec2 P1, glm::vec2 P2, glm::vec2 P3, Image& img, const ImgColor& color);
void DrawSingleTriangle(glm::vec3 P1, glm::vec3 P2, glm::vec3 P3, Image& img, const ImgColor& color, float* zbuffer = nullptr);
void DrawSingleTriangle(glm::vec3 P1, glm::vec3 P2, glm::vec3 P3, glm::vec2 T1, glm::vec2 T2, glm::vec2 T3, Image& img, const Image& texture, float* zbuffer);
void DrawSingleTriangle(glm::vec3 P[], glm::vec2 T[], Image& img, const Image& texture, float* zbuffer, BaseShader& shader);
void DrawSingleTriangle(glm::vec3 P[], glm::vec2 T[], Image& img, const Image& texture, float* zbuffer, BlinnPhongShader& shader);
void DrawSingleTriangleShadowMap(glm::vec3 P[], Image& shadowMap, DepthShader& shader);

void DrawModelRandomColor(Model& model, Image& img);
void DrawModelNormal(Model& model, Image& img, glm::vec3 lightDir);
void DrawModelTexture(Model& model, Image& img, Image& texture);
void DrawModelCameraProjection(Model& model, Image& img, Image& texture, float cameraZPos);
void DrawModelCameraViewTransformation(Model& model, Image& img, Image& texture, Camera& camera);
void DrawModelCameraViewPerspectiveTransformation(Model& model, Image& img, Image& texture, Camera& camera);
void DrawModelCameraViewPerspectiveTransformationUsingShader(Model& model, Image& img, Image& texture, Camera& camera);
void DrawModelCameraViewPerspectiveTransformationUsingBlinnPhongShader(Model& model, Image& img, Image& texture, Image& normalMap, Camera& camera, std::vector<pointLight>& lights);
void DrawModelShadowMap(Model& model, Image& shadowMap, pointLight& light);
void DrawModelCameraViewPerspectiveTransformationUsingBlinnPhongShaderWithShadowMap(Model& model, Image& img, Image& texture, Image& normalMap, Camera& camera, std::vector<pointLight>& lights);
glm::vec3 BarycentricCoords(glm::vec2 P, glm::vec2 P1, glm::vec2 P2, glm::vec2 P3);
glm::vec2 normalize(glm::vec2 v);
glm::vec3 normalize(glm::vec3 v);
void initializeShadowMap(Image& shadowMap);