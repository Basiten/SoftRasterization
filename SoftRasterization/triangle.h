#pragma once
#include "model.h"
#include "ImagesUtils.h"
#include "Lines.h"
#include <cmath>
void DrawSingleTriangleWireFrame(glm::vec2 t0,glm::vec2 t1, glm::vec2 t2, Image& img, const ImgColor& color);
bool Inside(glm::vec2 Q, glm::vec2 P1, glm::vec2 P2, glm::vec2 P3);
void DrawSingleTriangle(glm::vec2 P1, glm::vec2 P2, glm::vec2 P3, Image& img, const ImgColor& color);
void DrawSingleTriangle(glm::vec3 P1, glm::vec3 P2, glm::vec3 P3, Image& img, const ImgColor& color, float* zbuffer = nullptr);

void DrawModelRandomColor(Model& model, Image& img, float * zbuffer = nullptr);
void DrawModelNormal(Model& model, Image& img, glm::vec3 lightDir, float * zbuffer = nullptr);
void DrawModelTexture(Model& model, Image& img, Image& texture, float* zbuffer = nullptr);
void DrawModelCameraProjection(Model& model, Image& img, Image& texture, float* zbuffer, float cameraZPos);
glm::vec3 BarycentricCoords(glm::vec2 P, glm::vec2 P1, glm::vec2 P2, glm::vec2 P3);
glm::vec2 normalize(glm::vec2 v);
glm::vec3 normalize(glm::vec3 v);
