#include "ImagesUtils.h"
#include "model.h"
#include"Lines.h"
#include "triangle.h"
#include "Camera.h"

/*-------------------- global variables ----------------*/
const ImgColor white = ImgColor(255, 255, 255, 255);
const ImgColor red = ImgColor(255, 0, 0, 255);
const ImgColor green = ImgColor(0, 255, 0, 255);
const int width = 1600;
const int height = 1600;
Image img(width, height, Image::RGB);
Image texture;

// =============================================
// Attention!! 在标准坐标系下, Z-Buffer越小（为负数）说明越深
float* zbuffer = new float[width * height];

int main(int argc, char** argv) {
	glm::vec3 cameraPos(1, 0.5, 1);
	glm::vec3 gaze = -cameraPos;
	glm::vec3 up(0,1,-0.5);
	Camera camera(cameraPos, gaze, up, -1.f, -10.f, 70.f, 1.f);
	glm::mat4 viewMatrix = camera.getViewMatrix();
	std::cout << (viewMatrix * glm::vec4(1.f)).x << std::endl;
	for (int idx = 0; idx<img.getWidth()*img.getHeight(); idx++) {
		zbuffer[idx] = -std::numeric_limits<float>::max();
		//zbuffer[idx] = 100;
	}
	texture.readImage("model/african_head_diffuse.tga");
	texture.flipVertically();
	Model model("model/african_head.obj");
	//DrawModelCameraViewTransformation(model, img, texture, zbuffer, camera);
	DrawModelCameraViewPerspectiveTransformation(model, img, texture,zbuffer,camera);
	img.flipVertically(); // i want to have the origin at the left bottom corner of the image
	img.writeImage("output_perspective2.tga");
}
