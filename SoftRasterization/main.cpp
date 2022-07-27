#include "ImagesUtils.h"
#include "model.h"
#include"Lines.h"
#include "triangle.h"
#include "Camera.h"
#include "Light.h"


/*-------------------- global variables ----------------*/
const ImgColor white = ImgColor(255, 255, 255, 255);
const ImgColor red = ImgColor(255, 0, 0, 255);
const ImgColor green = ImgColor(0, 255, 0, 255);
const int width = 1600;
const int height = 1600;
Image img(width, height, Image::RGB);
Image shadowMap(width, height, Image::RGB);
Image texture;
Image normalMap;
// =============================================
// Attention!! 在标准坐标系下, Z-Buffer越小（为负数）说明越深
// float* zbuffer = new float[width * height];
std::vector<pointLight> lights;
glm::vec3 cameraPos(0, 1, 2);
glm::vec3 gaze = -cameraPos;
glm::vec3 up(0, 0.6, -0.3);
//glm::vec3 cameraPos(0, 0, 1);
//glm::vec3 gaze = -cameraPos;
//glm::vec3 up(0,1,0);
Camera camera(cameraPos, gaze, up, -1.f, -10.f, 70.f, 1.f);

int main(int argc, char** argv) {
	lights.emplace_back(glm::vec3(2, 0, 1), glm::vec3(1.f), 0.2, 1, 0.1, 0.1);
	texture.readImage("model/diablo3_pose_diffuse.tga");
	normalMap.readImage("model/diablo3_pose_nm.tga");
	texture.flipVertically();
	normalMap.flipVertically();
	Model model("model/diablo3_pose.obj");
	//DrawModelCameraViewTransformation(model, img, texture, zbuffer, camera);
	//DrawModelCameraViewPerspectiveTransformationUsingBlinnPhongShader(model, img, texture, normalMap, zbuffer, camera, lights);
	DrawModelShadowMap(model, shadowMap, lights[0]);
	img.flipVertically(); // i want to have the origin at the left bottom corner of the image
	//img.writeImage("diablo_BlinnPhongShader.tga");

	shadowMap.flipVertically(); // i want to have the origin at the left bottom corner of the image
	shadowMap.writeImage("diablo_shadowMap.tga");

}
