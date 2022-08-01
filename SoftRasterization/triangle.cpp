#include "triangle.h"
void DrawSingleTriangleWireFrame(glm::vec2 t0, glm::vec2 t1, glm::vec2 t2, Image& img, const ImgColor& color) {
	line(t0, t1, img, color);
	line(t1, t2, img, color);
	line(t2, t0, img, color);
}
bool Inside(glm::vec2 Q, glm::vec2 P1, glm::vec2 P2, glm::vec2 P3) {
	// 如何判断点在三角形内？
	// 向量叉积同侧！即 三次运算x1y2 - x2y1 同号!
	// P1P2 P1Q
	 float tmp1 = (P2.x - P1.x) * (Q.y - P1.y) - (Q.x - P1.x) * (P2.y - P1.y);
	 float tmp2 = (P3.x - P2.x) * (Q.y - P2.y) - (Q.x - P2.x) * (P3.y - P2.y);
	 float tmp3 = (P1.x - P3.x) * (Q.y - P3.y) - (Q.x - P3.x) * (P1.y - P3.y);
	// 大于等于零而非大于零：避免黑线产生
	return ((tmp1 >= 0 && tmp2 >= 0 && tmp3 >= 0) || (tmp1 <= 0 && tmp2 <= 0 && tmp3 <= 0)) ? true : false;
}

// 仅屏幕空间坐标 (无Zbuffer)
void DrawSingleTriangle(glm::vec2 P1, glm::vec2 P2, glm::vec2 P3, Image& img, const ImgColor& color) {
	//1. 找到bounding box
	//2. 对bbox内的每一个点做判断
	// 都转化为int，这个过程会向下取整
	int bbmin[2] = {(int)(std::min(std::min(P1.x, P2.x), P3.x)),(int)(std::min(std::min(P1.y, P2.y), P3.y)) + 1};
	int bbmax[2] = {(int)(std::max(std::max(P1.x, P2.x), P3.x)),(int)(std::max(std::max(P1.y, P2.y), P3.y)) + 1};
	for (int x = bbmin[0]; x < bbmax[0]; x++) {
		for (int y = bbmin[1]; y < bbmax[1]; y++) {
			if (Inside(glm::vec2(x, y), P1, P2, P3)) {
				img.set(x, y, color);
			}
		}
	}
}
// 屏幕空间坐标 + zbuffer 单一color
void DrawSingleTriangle(glm::vec3 P1, glm::vec3 P2, glm::vec3 P3, Image& img, const ImgColor& color, float* zbuffer) {
	// 不启用深度测试
	if (zbuffer == nullptr) {
		DrawSingleTriangle(glm::vec2(P1.x, P1.y), glm::vec2(P2.x, P2.y), glm::vec2(P3.x, P3.y), img, color);
		return;
	}
	// 使用深度测试

	int bbmin[2] = { (int)(std::min(std::min(P1.x, P2.x), P3.x)),(int)(std::min(std::min(P1.y, P2.y), P3.y)) + 1 };
	int bbmax[2] = { (int)(std::max(std::max(P1.x, P2.x), P3.x)),(int)(std::max(std::max(P1.y, P2.y), P3.y)) + 1 };

	for (int x = bbmin[0]; x < bbmax[0]; x++) {
		for (int y = bbmin[1]; y < bbmax[1]; y++) {
			if (Inside(glm::vec2(x, y), glm::vec2(P1.x, P1.y), glm::vec2(P2.x, P2.y), glm::vec2(P3.x, P3.y))) {
				glm::vec3 barycentric = BarycentricCoords(glm::vec2(x, y), glm::vec2(P1.x, P1.y), glm::vec2(P2.x, P2.y), glm::vec2(P3.x, P3.y));
				float depth = barycentric.x * P1.z + barycentric.y * P2.z + barycentric.z * P3.z;
				if (zbuffer[x + y * img.getWidth()] <= depth) {
					zbuffer[x + y * img.getWidth()] = depth;
					img.set(x, y, color);
				}
			}
		}
	}
}
// 屏幕空间坐标 + zbuffer + 纹理采样
void DrawSingleTriangle(glm::vec3 P1, glm::vec3 P2, glm::vec3 P3, glm::vec2 T1, glm::vec2 T2, glm::vec2 T3, Image& img, const Image& texture, float* zbuffer) {
	// 不启用深度测试
	if (zbuffer == nullptr) {
		throw std::runtime_error("Must use depth test!\n");
		return;
	}
	// 使用深度测试
	int bbmin[2] = {(int)(std::min(std::min(P1.x, P2.x), P3.x)),(int)(std::min(std::min(P1.y, P2.y), P3.y)) + 1};
	int bbmax[2] = {(int)(std::max(std::max(P1.x, P2.x), P3.x)),(int)(std::max(std::max(P1.y, P2.y), P3.y)) + 1};
	//std::cout << bbmin[0] << " " << bbmin[1] << " " << bbmax[0] << " " << bbmax[1] << std::endl;
	for (int x = bbmin[0]; x < bbmax[0]; x++) {
		for (int y = bbmin[1]; y < bbmax[1]; y++) {
			if (x >= img.getWidth() || y >= img.getHeight()||x<0||y<0) continue;
			if (Inside(glm::vec2(x, y), glm::vec2(P1.x, P1.y), glm::vec2(P2.x, P2.y), glm::vec2(P3.x, P3.y))) {
				//std::cout << "Pass inside test at: " << x << " " << y << std::endl;
				glm::vec3 barycentric = BarycentricCoords(glm::vec2(x, y), glm::vec2(P1.x, P1.y), glm::vec2(P2.x, P2.y), glm::vec2(P3.x, P3.y));
				float depth = barycentric.x * P1.z + barycentric.y * P2.z + barycentric.z * P3.z;
				if (zbuffer[x + y * img.getWidth()] <= depth) {
					zbuffer[x + y * img.getWidth()] = depth;
					// 计算纹理坐标
					glm::vec2 texCoordsInterpolated (barycentric.x * T1.x + barycentric.y * T2.x + barycentric.z * T3.x,
											barycentric.x * T1.y + barycentric.y * T2.y + barycentric.z * T3.y );
					ImgColor color = texture.get(texCoordsInterpolated.x * texture.getWidth(), texCoordsInterpolated.y * texture.getHeight());
					img.set(x, y, color);
					//std::cout << "set color at: " << x<<" " << y << std::endl;
				}
			}
		}
	}
}
// 屏幕空间坐标 + zbuffer + 纹理采样 + shader
void DrawSingleTriangle(glm::vec3 P[], glm::vec2 T[], Image& img, const Image& texture, float* zbuffer, BaseShader& shader) {
	// 不启用深度测试
	if (zbuffer == nullptr) {
		throw std::runtime_error("Must use depth test!\n");
		return;
	}
	// 使用深度测试
	int bbmin[2] = { (int)(std::min(std::min(P[0].x, P[1].x), P[2].x)),(int)(std::min(std::min(P[0].y, P[1].y), P[2].y)) + 1};
	int bbmax[2] = { (int)(std::max(std::max(P[0].x, P[1].x), P[2].x)),(int)(std::max(std::max(P[0].y, P[1].y), P[2].y)) + 1};
	//std::cout << bbmin[0] << " " << bbmin[1] << " " << bbmax[0] << " " << bbmax[1] << std::endl;
	for (int x = bbmin[0]; x < bbmax[0]; x++) {
		for (int y = bbmin[1]; y < bbmax[1]; y++) {
			if (x >= img.getWidth() || y >= img.getHeight() || x < 0 || y < 0) continue;
			if (Inside(glm::vec2(x, y), glm::vec2(P[0].x, P[0].y), glm::vec2(P[1].x, P[1].y), glm::vec2(P[2].x, P[2].y))) {
				//std::cout << "Pass inside test at: " << x << " " << y << std::endl;
				glm::vec3 barycentric = BarycentricCoords(glm::vec2(x, y), glm::vec2(P[0].x, P[0].y), glm::vec2(P[1].x, P[1].y), glm::vec2(P[2].x, P[2].y));
				float depth = barycentric.x * P[0].z + barycentric.y * P[1].z + barycentric.z * P[2].z;
				if (zbuffer[x + y * img.getWidth()] <= depth) {
					zbuffer[x + y * img.getWidth()] = depth;
					// 计算纹理坐标
					ImgColor color;
					shader.fragment(barycentric, color);
					img.set(x, y, color);
					//std::cout << "set color at: " << x<<" " << y << std::endl;
				}
			}
		}
	}
}
// 屏幕空间坐标 + zbuffer + 纹理采样 + blinnPhong shader
void DrawSingleTriangle(glm::vec3 P[], glm::vec2 T[], Image& img, const Image& texture, float* zbuffer, BlinnPhongShader& shader)
{
	// 不启用深度测试
	if (zbuffer == nullptr) {
		throw std::runtime_error("Must use depth test!\n");
		return;
	}
	// 使用深度测试
	int bbmin[2] = { (int)(std::min(std::min(P[0].x, P[1].x), P[2].x)),(int)(std::min(std::min(P[0].y, P[1].y), P[2].y)) + 1 };
	int bbmax[2] = { (int)(std::max(std::max(P[0].x, P[1].x), P[2].x)),(int)(std::max(std::max(P[0].y, P[1].y), P[2].y)) + 1 };
	//std::cout << bbmin[0] << " " << bbmin[1] << " " << bbmax[0] << " " << bbmax[1] << std::endl;
	for (int x = bbmin[0]; x < bbmax[0]; x++) {
		for (int y = bbmin[1]; y < bbmax[1]; y++) {
			if (x >= img.getWidth() || y >= img.getHeight() || x < 0 || y < 0) continue;
			if (Inside(glm::vec2(x, y), glm::vec2(P[0].x, P[0].y), glm::vec2(P[1].x, P[1].y), glm::vec2(P[2].x, P[2].y))) {
				// std::cout << "Pass inside test at: " << x << " " << y << std::endl;
				glm::vec3 barycentric = BarycentricCoords(glm::vec2(x, y), glm::vec2(P[0].x, P[0].y), glm::vec2(P[1].x, P[1].y), glm::vec2(P[2].x, P[2].y));
				float depth = barycentric.x * P[0].z + barycentric.y * P[1].z + barycentric.z * P[2].z;
				if (zbuffer[x + y * img.getWidth()] <= depth) {
					//printf("Passed depth test. \n");
					zbuffer[x + y * img.getWidth()] = depth;
					ImgColor color;
					shader.fragment(barycentric, color);
					//printf("%d\n",color.r);
					img.set(x, y, color);
					//std::cout << "set color at: " << x<<" " << y << std::endl;
				}
			}
		}
	}
}
// DepthsShader 
void DrawSingleTriangleShadowMap(glm::vec3 P[], Image& shadowMap, DepthShader& shader) {
	int bbmin[2] = { (int)(std::min(std::min(P[0].x, P[1].x), P[2].x)),(int)(std::min(std::min(P[0].y, P[1].y), P[2].y)) + 1 };
	int bbmax[2] = { (int)(std::max(std::max(P[0].x, P[1].x), P[2].x)),(int)(std::max(std::max(P[0].y, P[1].y), P[2].y)) + 1 };
	// printf("bbmin[0] : %d, bbmin[1] : %d, ,bbmax[0] : %d, ,bbmax[1] : %d \n", bbmin[0], bbmin[1], bbmax[0], bbmax[1]);
	for (int x = bbmin[0]; x < bbmax[0]; x++) {
		for (int y = bbmin[1]; y < bbmax[1]; y++) {
			//printf("%d %d\n", x, y);
			if (x >= shadowMap.getWidth() || y >= shadowMap.getHeight() || x < 0 || y < 0) continue;
			if (Inside(glm::vec2(x, y), glm::vec2(P[0].x, P[0].y), glm::vec2(P[1].x, P[1].y), glm::vec2(P[2].x, P[2].y))) {
				//std::cout << "Pass inside test at: " << x << " " << y << std::endl;
				glm::vec3 barycentric = BarycentricCoords(glm::vec2(x, y), glm::vec2(P[0].x, P[0].y), glm::vec2(P[1].x, P[1].y), glm::vec2(P[2].x, P[2].y));
				float depth = barycentric.x * P[0].z + barycentric.y * P[1].z + barycentric.z * P[2].z;
				// printf("depth %f.\n", depth);
				if (shader.depthBuffer[x + y * shadowMap.getWidth()] <= depth) {
					shader.depthBuffer[x + y * shadowMap.getWidth()] = depth;
					//printf("Depth %f.\n", shader.depthBuffer[x + y * shadowMap.getWidth()]);
					ImgColor color;
					shader.fragment(barycentric, color);
					shadowMap.set(x, y, color);
					//std::cout << "set color at: " << x<<" " << y << std::endl;
				}
				//else { printf("Not Pass Depth Test.\n"); }
			}
		}
	}
};

void DrawModelRandomColor(Model& model, Image& img) {
	float* zbuffer = new float[img.getHeight() * img.getWidth()];
	for (int idx = 0; idx < img.getWidth() * img.getHeight(); idx++) {
		zbuffer[idx] = -std::numeric_limits<float>::max();
	}
	for (int i = 0; i < model.nfaces(); i++) {
		std::vector<int> face = model.face(i);
		glm::vec3 screen_coords[3];
		for (int j = 0; j < 3; j++) {
			glm::vec3 world_coords = model.vert(face[j]);
			screen_coords[j] = glm::vec3((world_coords.x + 1.) * img.getWidth() / 2., (world_coords.y + 1.) * img.getHeight() / 2., world_coords.z);
		}
		DrawSingleTriangle(screen_coords[0], screen_coords[1], screen_coords[2], img, ImgColor(rand() % 255, rand() % 255, rand() % 255, 255), zbuffer);
	}

	delete[]zbuffer;
};
void DrawModelNormal(Model& model, Image& img, glm::vec3 lightDir) {
	float* zbuffer = new float[img.getHeight() * img.getWidth()];
	for (int idx = 0; idx < img.getWidth() * img.getHeight(); idx++) {
		zbuffer[idx] = -std::numeric_limits<float>::max();
	}
	for (int i = 0; i < model.nfaces(); i++) {
		std::vector<int> face = model.face(i);
		glm::vec3 screen_coords[3];
		glm::vec3  world_coords[3];
		for (int j = 0; j < 3; j++) {
			world_coords[j] = model.vert(face[j]);
			screen_coords[j] = glm::vec3((world_coords[j].x + 1.) * img.getWidth() / 2., (world_coords[j].y + 1.) * img.getHeight() / 2., world_coords[j].z);
		}
		// order of vec1 / vec2 matters
		glm::vec3 vec1 = world_coords[2] - world_coords[0];
		glm::vec3 vec2 = world_coords[1] - world_coords[0];
		glm::vec3 normal((vec1.y * vec2.z - vec2.y * vec1.z),
			(vec2.x * vec1.z - vec1.x * vec2.z),
			(vec1.x * vec2.y - vec2.x * vec1.y));
		normal = normalize(normal);
		lightDir = normalize(lightDir);
		float intensity = normal.x * lightDir.x + normal.y * lightDir.y;
		if (intensity > 0) {
			DrawSingleTriangle(screen_coords[0], screen_coords[1], screen_coords[2], img, ImgColor(intensity * 255, intensity * 255, intensity * 255, 255), zbuffer);
		}
	}
	delete[]zbuffer;
};
void DrawModelTexture(Model& model, Image& img, Image&texture) {
	float* zbuffer = new float[img.getHeight() * img.getWidth()];
	for (int idx = 0; idx < img.getWidth() * img.getHeight(); idx++) {
		zbuffer[idx] = -std::numeric_limits<float>::max();
	}
	for (int i = 0; i < model.nfaces(); i++) {
		std::vector<int> face = model.face(i);
		std::vector<int> faceTexcoords = model.faceTexCoords(i);
		glm::vec3 screen_coords[3];
		glm::vec3 world_coords[3];
		glm::vec2 TexCoords[3];
		for (int j = 0; j < 3; j++) {
			world_coords[j] = model.vert(face[j]);
			TexCoords[j] = model.texCoords(faceTexcoords[j]);
			screen_coords[j] = glm::vec3((world_coords[j].x + 1.) * img.getWidth() / 2., (world_coords[j].y + 1.) * img.getHeight() / 2., world_coords[j].z);
		}
		DrawSingleTriangle(screen_coords[0], screen_coords[1], screen_coords[2], TexCoords[0], TexCoords[1], TexCoords[2], img, texture, zbuffer);
	}
	delete[]zbuffer;
}
void DrawModelCameraProjection(Model& model, Image& img, Image& texture, float cameraZPos) {
	float* zbuffer = new float[img.getHeight() * img.getWidth()];
	for (int idx = 0; idx < img.getWidth() * img.getHeight(); idx++) {
		zbuffer[idx] = -std::numeric_limits<float>::max();
	}
	auto cameraProjection = [cameraZPos](glm::vec3 world_coords) ->glm::vec3 {
		// scale x and y while keep z.
		return glm::vec3(world_coords.x / (1 - world_coords.z / cameraZPos), world_coords.y / (1 - world_coords.z / cameraZPos), world_coords.z);
	};
	for (int i = 0; i < model.nfaces(); i++) {
		std::vector<int> face = model.face(i);
		std::vector<int> faceTexcoords = model.faceTexCoords(i);
		glm::vec3 screen_coords[3];
		glm::vec3 world_coords[3];
		glm::vec2 TexCoords[3];
		for (int j = 0; j < 3; j++) {
			world_coords[j] = cameraProjection(model.vert(face[j]));
			TexCoords[j] = model.texCoords(faceTexcoords[j]);
			screen_coords[j] = glm::vec3((world_coords[j].x + 1.) * img.getWidth() / 2., (world_coords[j].y + 1.) * img.getHeight() / 2., world_coords[j].z);
		}
		DrawSingleTriangle(screen_coords[0], screen_coords[1], screen_coords[2], TexCoords[0], TexCoords[1], TexCoords[2], img, texture, zbuffer);
	}
	delete[]zbuffer;
}
void DrawModelCameraViewTransformation(Model& model, Image& img, Image& texture, Camera &camera) {
	float* zbuffer = new float[img.getHeight() * img.getWidth()];
	for (int idx = 0; idx < img.getWidth() * img.getHeight(); idx++) {
		zbuffer[idx] = -std::numeric_limits<float>::max();
	}
	glm::mat4 viewMatrix = camera.getViewMatrix();
	auto cameraProjection = [&](glm::vec3 world_coords) ->glm::vec3 {
		// scale x and y while keep z.
		glm::vec4 world_coords_4d = viewMatrix * glm::vec4(world_coords,1.f);
		return glm::vec3(world_coords_4d.x, world_coords_4d.y, world_coords_4d.z) / world_coords_4d.w;
	};
	for (int i = 0; i < model.nfaces(); i++) {
		std::vector<int> face = model.face(i);
		std::vector<int> faceTexcoords = model.faceTexCoords(i);
		glm::vec3 screen_coords[3];
		glm::vec3 world_coords[3];
		glm::vec2 TexCoords[3];
		for (int j = 0; j < 3; j++) {
			world_coords[j] = cameraProjection(model.vert(face[j]));
			//std::cout << world_coords[j].x << " " << world_coords[j].y << " " << world_coords[j].z << std::endl;
			TexCoords[j] = model.texCoords(faceTexcoords[j]);
			screen_coords[j] = glm::vec3((world_coords[j].x + 1.) * img.getWidth() / 2., (world_coords[j].y + 1.) * img.getHeight() / 2., world_coords[j].z);
		}
		DrawSingleTriangle(screen_coords[0], screen_coords[1], screen_coords[2], TexCoords[0], TexCoords[1], TexCoords[2], img, texture, zbuffer);
	}
	delete[]zbuffer;
}
void DrawModelCameraViewPerspectiveTransformation(Model& model, Image& img, Image& texture, Camera& camera) {
	float* zbuffer = new float[img.getHeight() * img.getWidth()];
	for (int idx = 0; idx < img.getWidth() * img.getHeight(); idx++) {
		zbuffer[idx] = -std::numeric_limits<float>::max();
	}
	glm::mat4 viewPerspectiveMatrix = camera.getViewPerspectiveMatrix();
	auto cameraProjection = [&](glm::vec3 world_coords) ->glm::vec3 {
		// scale x and y while keep z.
		glm::vec4 world_coords_4d = viewPerspectiveMatrix * glm::vec4(world_coords, 1.f);
		return glm::vec3(world_coords_4d.x, world_coords_4d.y, world_coords_4d.z) / world_coords_4d.w;
	};
	for (int i = 0; i < model.nfaces(); i++) {
		std::vector<int> face = model.face(i);
		std::vector<int> faceTexcoords = model.faceTexCoords(i);
		glm::vec3 screen_coords[3];
		glm::vec3 world_coords[3];
		glm::vec2 TexCoords[3];
		for (int j = 0; j < 3; j++) {
			world_coords[j] = cameraProjection(model.vert(face[j]));
			//std::cout << world_coords[j].x << " " << world_coords[j].y << " " << world_coords[j].z << std::endl;
			TexCoords[j] = model.texCoords(faceTexcoords[j]);
			screen_coords[j] = glm::vec3((world_coords[j].x + 1.) * img.getWidth() / 2., (world_coords[j].y + 1.) * img.getHeight() / 2., world_coords[j].z);
		}
		DrawSingleTriangle(screen_coords[0], screen_coords[1], screen_coords[2], TexCoords[0], TexCoords[1], TexCoords[2], img, texture, zbuffer);
	}
	delete[]zbuffer;
}
void DrawModelCameraViewPerspectiveTransformationUsingShader(Model& model, Image& img, Image& texture, Camera& camera) {
	float* zbuffer = new float[img.getHeight() * img.getWidth()];
	for (int idx = 0; idx < img.getWidth() * img.getHeight(); idx++) {
		zbuffer[idx] = -std::numeric_limits<float>::max();
	}
	BlinnPhongShader shader;
	shader.setMatrix(glm::mat4(1.f), camera.getViewMatrix(), camera.getPerspectiveMatrix());
	shader.setViewPort(&img);
	shader.setTexture(&texture);

	for (int i = 0; i < model.nfaces(); i++) {
		std::vector<int> face = model.face(i);
		std::vector<int> faceTexcoords = model.faceTexCoords(i);
		glm::vec3 screen_coords[3];
		glm::vec2 TexCoords[3];
		for (int j = 0; j < 3; j++) {
			TexCoords[j] = model.texCoords(faceTexcoords[j]);
			screen_coords[j] = shader.vertex(model.vert(face[j]), TexCoords[j], j);
			//std::cout << world_coords[j].x << " " << world_coords[j].y << " " << world_coords[j].z << std::endl;
		}
		DrawSingleTriangle(screen_coords, TexCoords,img, texture, zbuffer, shader);
	}
	delete[]zbuffer;
}
void DrawModelCameraViewPerspectiveTransformationUsingBlinnPhongShader(Model& model, Image& img, Image& texture, Image& normalMap, Camera& camera, std::vector<pointLight> &lights)
{
	float* zbuffer = new float[img.getHeight() * img.getWidth()];
	for (int idx = 0; idx < img.getWidth() * img.getHeight(); idx++) {
		zbuffer[idx] = -std::numeric_limits<float>::max();
	}
	BlinnPhongShader shader;
	shader.setMatrix(glm::mat4(1.f), camera.getViewMatrix(), camera.getPerspectiveMatrix());
	shader.setViewPort(&img);
	shader.setTexture(&texture);
	shader.setNormalMap(&normalMap);
	// set light to camera pos
	for (pointLight & light : lights) shader.addPointLight(light);

	for (int i = 0; i < model.nfaces(); i++) {
		std::vector<int> face = model.face(i);
		std::vector<int> faceTexcoords = model.faceTexCoords(i);
		glm::vec3 screen_coords[3];
		glm::vec2 TexCoords[3];
		for (int j = 0; j < 3; j++) {
			glm::vec3 modelVert = model.vert(face[j]);
			TexCoords[j] = model.texCoords(faceTexcoords[j]);
			screen_coords[j] = shader.vertex(model.vert(face[j]), TexCoords[j], j);
		}
		DrawSingleTriangle(screen_coords, TexCoords, img, texture, zbuffer, shader);
	}
	delete[]zbuffer;
}
void DrawModelShadowMap(Model& model, Image& shadowMap, pointLight& light)
{
	DepthShader shader(light, shadowMap);

	for (int i = 0; i < model.nfaces(); i++) {
		std::vector<int> face = model.face(i);
		std::vector<int> faceTexcoords = model.faceTexCoords(i);
		glm::vec3 screen_coords[3];
		for (int j = 0; j < 3; j++) {
			glm::vec3 modelVert = model.vert(face[j]);
			screen_coords[j] = shader.vertex(model.vert(face[j]), j);
		}
		DrawSingleTriangleShadowMap(screen_coords, shadowMap, shader);
	}
}
void DrawModelCameraViewPerspectiveTransformationUsingBlinnPhongShaderWithShadowMap(Model& model, Image& img, Image& texture, Image& normalMap, Camera& camera, std::vector<pointLight>& lights)
{
	float* zbuffer = new float[img.getHeight() * img.getWidth()];
	for (int idx = 0; idx < img.getWidth() * img.getHeight(); idx++) {
		zbuffer[idx] = -std::numeric_limits<float>::max();
	}
	// Shadow Path
	// 记录所有光源的阴影贴图
	std::vector<Image> shadowMaps;
	// 绘制所有阴影贴图
	int idx = 0;
	for (pointLight &light : lights) {
		shadowMaps.emplace_back(1600, 1600, Image::RGBA);
		initializeShadowMap(shadowMaps[idx]);
		DrawModelShadowMap(model, shadowMaps[idx], lights[idx]);
		shadowMaps[idx].flipVertically();
		shadowMaps[idx].writeImage("ShadowMaps.tga");
		shadowMaps[idx].flipVertically();
		idx++;
	}

	// Shading Path
	BlinnPhongShader shader;
	shader.setMatrix(glm::mat4(1.f), camera.getViewMatrix(), camera.getPerspectiveMatrix());
	shader.setViewPort(&img);
	shader.setTexture(&texture);
	shader.setNormalMap(&normalMap);
	// set light to camera pos
	for (pointLight& light : lights) shader.addPointLight(light);
	// set corresponding shadow map
	shader.attachShadowMaps(shadowMaps);

	for (int i = 0; i < model.nfaces(); i++) {
		std::vector<int> face = model.face(i);
		std::vector<int> faceTexcoords = model.faceTexCoords(i);
		glm::vec3 screen_coords[3];
		glm::vec2 TexCoords[3];
		for (int j = 0; j < 3; j++) {
			glm::vec3 modelVert = model.vert(face[j]);
			TexCoords[j] = model.texCoords(faceTexcoords[j]);
			screen_coords[j] = shader.vertex(model.vert(face[j]), TexCoords[j], j);
		}
		DrawSingleTriangle(screen_coords, TexCoords, img, texture, zbuffer, shader);
	}
	delete []zbuffer;
}
glm::vec3 BarycentricCoords(glm::vec2 P, glm::vec2 P1, glm::vec2 P2, glm::vec2 P3) {
	glm::vec3 barycentric;
	// P = alpha P1 + beta P2 + gama P3, alpha + beta + gamma = 1
	barycentric.z = ((P1.y - P2.y) * P.x + (P2.x - P1.x) * P.y + P1.x * P2.y - P2.x * P1.y) /
		(float)(((P1.y - P2.y) * P3.x + (P2.x - P1.x) * P3.y + P1.x * P2.y - P2.x * P1.y));
	barycentric.y = ((P1.y - P3.y) * P.x + (P3.x - P1.x) * P.y + P1.x * P3.y - P3.x * P1.y) /
		(float)(((P1.y - P3.y) * P2.x + (P3.x - P1.x) * P2.y + P1.x * P3.y - P3.x * P1.y));
	barycentric.x = 1.f - barycentric.y - barycentric.z;
	return barycentric;
}		
glm::vec2 normalize(glm::vec2 v) {
	return v / std::sqrt(v.x * v.x + v.y * v.y);
}
glm::vec3 normalize(glm::vec3 v){
	return v / std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
};
void initializeShadowMap(Image& shadowMap){
	for (int x = 0; x < shadowMap.getWidth(); x++) {
		for (int y = 0; y < shadowMap.getHeight(); y++) {
			ImgColor color(-std::numeric_limits<float>::max());
			shadowMap.set(x, y, color);
		}
	}
}
