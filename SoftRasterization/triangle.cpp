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

	for (int x = bbmin[0]; x < bbmax[0]; x++) {
		for (int y = bbmin[1]; y < bbmax[1]; y++) {
			if (x >= img.getWidth() || y >= img.getHeight()||x<0||y<0) continue;
			if (Inside(glm::vec2(x, y), glm::vec2(P1.x, P1.y), glm::vec2(P2.x, P2.y), glm::vec2(P3.x, P3.y))) {
				glm::vec3 barycentric = BarycentricCoords(glm::vec2(x, y), glm::vec2(P1.x, P1.y), glm::vec2(P2.x, P2.y), glm::vec2(P3.x, P3.y));
				float depth = barycentric.x * P1.z + barycentric.y * P2.z + barycentric.z * P3.z;
				if (zbuffer[x + y * img.getWidth()] <= depth) {
					zbuffer[x + y * img.getWidth()] = depth;
					// 计算纹理坐标
					glm::vec2 texCoordsInterpolated (barycentric.x * T1.x + barycentric.y * T2.x + barycentric.z * T3.x,
											barycentric.x * T1.y + barycentric.y * T2.y + barycentric.z * T3.y );
					ImgColor color = texture.get(texCoordsInterpolated.x * texture.getWidth(), texCoordsInterpolated.y * texture.getHeight());
					img.set(x, y, color);
				}
			}
		}
	}
}


void DrawModelRandomColor(Model& model, Image& img, float* zbuffer) {
	for (int i = 0; i < model.nfaces(); i++) {
		std::vector<int> face = model.face(i);
		glm::vec3 screen_coords[3];
		for (int j = 0; j < 3; j++) {
			glm::vec3 world_coords = model.vert(face[j]);
			screen_coords[j] = glm::vec3((world_coords.x + 1.) * img.getWidth() / 2., (world_coords.y + 1.) * img.getHeight() / 2., world_coords.z);
		}
		DrawSingleTriangle(screen_coords[0], screen_coords[1], screen_coords[2], img, ImgColor(rand() % 255, rand() % 255, rand() % 255, 255), zbuffer);
	}
};
void DrawModelNormal(Model& model, Image& img, glm::vec3 lightDir, float* zbuffer) {
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
};
void DrawModelTexture(Model& model, Image& img, Image&texture, float* zbuffer) {
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
}
void DrawModelCameraProjection(Model& model, Image& img, Image& texture, float* zbuffer, float cameraZPos) {
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
}
void DrawModelCameraTransformation(Model& model, Image& img, Image& texture, float* zbuffer, float cameraZPos) {
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
