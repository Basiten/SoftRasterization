#pragma once
#include "model.h"
#include "ImagesUtils.h"
#include "Lines.h"
void DrawSingleTriangleWireFrame(Vec2i t0, Vec2i t1, Vec2i t2, Image& img, const ImgColor& color);
template<class T>
bool Inside(Vec2i Q, Vec2<T> P1, Vec2<T> P2, Vec2<T> P3);
template<class T>
void DrawSingleTriangle(Vec2<T> P1, Vec2<T> P2, Vec2<T> P3, Image& img, const ImgColor& color);
template<class T>
void DrawSingleTriangle(Vec3<T> P1, Vec3<T> P2, Vec3<T> P3, Image& img, const ImgColor& color, float* zbuffer = nullptr);
void DrawModelRandomColor(Model& model, Image& img, float * zbuffer = nullptr);
void DrawModelNormal(Model& model, Image& img, Vec3f lightDir, float * zbuffer = nullptr);
void DrawModelTexture(Model& model, Image& img, Image& texture, float* zbuffer = nullptr);
void DrawModelCameraProjection(Model& model, Image& img, Image& texture, float* zbuffer, float cameraZPos);
template<class T1, class T2>
Vec3f BarycentricCoords(Vec2<T1> P, Vec2<T2>P1, Vec2<T2>P2, Vec2<T2>P3);
