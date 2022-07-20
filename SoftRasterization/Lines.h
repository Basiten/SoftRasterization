#pragma once
#include "model.h"
#include "ImagesUtils.h"
void line(glm::vec2 t0, glm::vec2 t1, Image& image, const ImgColor& color);
void line(int x0, int y0, int x1, int y1, Image& image, const ImgColor& color);
void wireFrame(Model& model, Image& img, int width, int height, const ImgColor& color);
