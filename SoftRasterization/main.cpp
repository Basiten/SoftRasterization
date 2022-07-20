#include "ImagesUtils.h"
#include "model.h"
#include"Lines.h"
#include "triangle.h"


/*-------------------- global variables ----------------*/
const ImgColor white = ImgColor(255, 255, 255, 255);
const ImgColor red = ImgColor(255, 0, 0, 255);
const ImgColor green = ImgColor(0, 255, 0, 255);
const int width = 3200;
const int height = 3200;
Image img(width, height, Image::RGB);
Image texture;

// =============================================
// Attention!! 在标准坐标系下, Z-Buffer越小（为负数）说明越深
float* zbuffer = new float[width * height];

int main(int argc, char** argv) {
	for (int idx = 0; idx<img.getWidth()*img.getHeight(); idx++) {
		zbuffer[idx] = -std::numeric_limits<float>::max();
		//zbuffer[idx] = 100;
	}
	texture.readImage("model/african_head_diffuse.tga");
	texture.flipVertically();
	Model model("model/african_head.obj");
	DrawModelCameraProjection(model, img, texture,zbuffer,5);
	img.flipVertically(); // i want to have the origin at the left bottom corner of the image
	img.writeImage("output.tga");
}
