#include "triangle.h"
void DrawSingleTriangleWireFrame(Vec2i t0, Vec2i t1, Vec2i t2, Image& img, const ImgColor& color) {
	line(t0, t1, img, color);
	line(t1, t2, img, color);
	line(t2, t0, img, color);
}
template<class T>
bool Inside(Vec2i Q, Vec2<T> P1, Vec2<T> P2, Vec2<T> P3) {
	// 如何判断点在三角形内？
	// 向量叉积同侧！即 三次运算x1y2 - x2y1 同号!
	// P1P2 P1Q
	T tmp1 = (float)(P2.x - P1.x) * (Q.y - P1.y) - (Q.x - P1.x) * (P2.y - P1.y);
	T tmp2 = (float)(P3.x - P2.x) * (Q.y - P2.y) - (Q.x - P2.x) * (P3.y - P2.y);
	T tmp3 = (float)(P1.x - P3.x) * (Q.y - P3.y) - (Q.x - P3.x) * (P1.y - P3.y);
	// 大于等于零而非大于零：避免黑线产生
	return ((tmp1 >= 0 && tmp2 >= 0 && tmp3 >= 0) || (tmp1 <= 0 && tmp2 <= 0 && tmp3 <= 0)) ? true : false;
}

// 仅屏幕空间坐标 (无Zbuffer)
template<class T>
void DrawSingleTriangle(Vec2<T> P1, Vec2<T> P2, Vec2<T> P3, Image& img, const ImgColor& color) {
	//1. 找到bounding box
	//2. 对bbox内的每一个点做判断
	// 都转化为int，这个过程会向下取整
	Vec2<int> bbmin = { (int)(std::min(std::min(P1.x, P2.x), P3.x)),(int)(std::min(std::min(P1.y, P2.y), P3.y)) + 1 };
	Vec2<int> bbmax = { (int)(std::max(std::max(P1.x, P2.x), P3.x)),(int)(std::max(std::max(P1.y, P2.y), P3.y)) + 1 };
	for (int x = bbmin.x; x < bbmax.x; x++) {
		for (int y = bbmin.y; y < bbmax.y; y++) {
			if (Inside(Vec2i(x, y), P1, P2, P3)) {
				img.set(x, y, color);
			}
		}
	}
}
// 屏幕空间坐标 + zbuffer 单一color
template<class T>
void DrawSingleTriangle(Vec3<T> P1, Vec3<T> P2, Vec3<T> P3, Image& img, const ImgColor& color, float* zbuffer) {
	// 不启用深度测试
	if (zbuffer == nullptr) {
		DrawSingleTriangle(Vec2<T>{P1.x, P1.y}, Vec2<T>{P2.x, P2.y}, Vec2<T>{P3.x, P3.y}, img, color);
		return;
	}
	// 使用深度测试

	Vec2<int> bbmin = { (int)(std::min(std::min(P1.x, P2.x), P3.x)),(int)(std::min(std::min(P1.y, P2.y), P3.y)) + 1 };
	Vec2<int> bbmax = { (int)(std::max(std::max(P1.x, P2.x), P3.x)),(int)(std::max(std::max(P1.y, P2.y), P3.y)) + 1 };

	for (int x = bbmin.x; x < bbmax.x; x++) {
		for (int y = bbmin.y; y < bbmax.y; y++) {
			if (Inside(Vec2i(x, y), Vec2<T>(P1.x,P1.y), Vec2<T>(P2.x, P2.y), Vec2<T>(P3.x, P3.y))) {
				Vec3f barycentric = BarycentricCoords(Vec2i(x, y), Vec2<T>(P1.x, P1.y), Vec2<T>(P2.x, P2.y), Vec2<T>(P3.x, P3.y));
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
template<class T>
void DrawSingleTriangle(Vec3<T> P1, Vec3<T> P2, Vec3<T> P3, Vec2f T1, Vec2f T2, Vec2f T3, Image& img, const Image& texture, float* zbuffer) {
	// 不启用深度测试
	if (zbuffer == nullptr) {
		throw std::runtime_error("Must use depth test!\n");
		return;
	}
	// 使用深度测试
	Vec2<int> bbmin = { (int)(std::min(std::min(P1.x, P2.x), P3.x)),(int)(std::min(std::min(P1.y, P2.y), P3.y)) + 1 };
	Vec2<int> bbmax = { (int)(std::max(std::max(P1.x, P2.x), P3.x)),(int)(std::max(std::max(P1.y, P2.y), P3.y)) + 1 };

	for (int x = bbmin.x; x < bbmax.x; x++) {
		for (int y = bbmin.y; y < bbmax.y; y++) {
			if (x >= img.getWidth() || y >= img.getHeight()||x<0||y<0) continue;
			if (Inside(Vec2i(x, y), Vec2<T>(P1.x, P1.y), Vec2<T>(P2.x, P2.y), Vec2<T>(P3.x, P3.y))) {
				Vec3f barycentric = BarycentricCoords(Vec2i(x, y), Vec2<T>(P1.x, P1.y), Vec2<T>(P2.x, P2.y), Vec2<T>(P3.x, P3.y));
				float depth = barycentric.x * P1.z + barycentric.y * P2.z + barycentric.z * P3.z;
				if (zbuffer[x + y * img.getWidth()] <= depth) {
					zbuffer[x + y * img.getWidth()] = depth;
					// 计算纹理坐标
					Vec2f texCoordsInterpolated = { barycentric.x * T1.x + barycentric.y * T2.x + barycentric.z * T3.x,
											barycentric.x * T1.y + barycentric.y * T2.y + barycentric.z * T3.y };
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
		Vec3f screen_coords[3];
		for (int j = 0; j < 3; j++) {
			Vec3f world_coords = model.vert(face[j]);
			screen_coords[j] = Vec3f((world_coords.x + 1.) * img.getWidth() / 2., (world_coords.y + 1.) * img.getHeight() / 2., world_coords.z);
		}
		DrawSingleTriangle(screen_coords[0], screen_coords[1], screen_coords[2], img, ImgColor(rand() % 255, rand() % 255, rand() % 255, 255), zbuffer);
	}
};
void DrawModelNormal(Model& model, Image& img, Vec3f lightDir, float* zbuffer) {
	for (int i = 0; i < model.nfaces(); i++) {
		std::vector<int> face = model.face(i);
		Vec3f screen_coords[3];
		Vec3f world_coords[3];
		for (int j = 0; j < 3; j++) {
			world_coords[j] = model.vert(face[j]);
			screen_coords[j] = Vec3f((world_coords[j].x + 1.) * img.getWidth() / 2., (world_coords[j].y + 1.) * img.getHeight() / 2., world_coords[j].z);
		}
		// order of vec1 / vec2 matters
		Vec3f vec1 = world_coords[2] - world_coords[0];
		Vec3f vec2 = world_coords[1] - world_coords[0];
		Vec3f normal((vec1.y * vec2.z - vec2.y * vec1.z),
			(vec2.x * vec1.z - vec1.x * vec2.z),
			(vec1.x * vec2.y - vec2.x * vec1.y));
		normal = normal.normalize();
		lightDir = lightDir.normalize();
		float intensity = normal * lightDir;
		if (intensity > 0) {
			DrawSingleTriangle(screen_coords[0], screen_coords[1], screen_coords[2], img, ImgColor(intensity * 255, intensity * 255, intensity * 255, 255), zbuffer);
		}
	}
};
void DrawModelTexture(Model& model, Image& img, Image&texture, float* zbuffer) {
	for (int i = 0; i < model.nfaces(); i++) {
		std::vector<int> face = model.face(i);
		std::vector<int> faceTexcoords = model.faceTexCoords(i);
		Vec3f screen_coords[3];
		Vec3f world_coords[3];
		Vec2f TexCoords[3];
		for (int j = 0; j < 3; j++) {
			world_coords[j] = model.vert(face[j]);
			TexCoords[j] = model.texCoords(faceTexcoords[j]);
			screen_coords[j] = Vec3f((world_coords[j].x + 1.) * img.getWidth() / 2., (world_coords[j].y + 1.) * img.getHeight() / 2., world_coords[j].z);
		}
		DrawSingleTriangle(screen_coords[0], screen_coords[1], screen_coords[2], TexCoords[0], TexCoords[1], TexCoords[2], img, texture, zbuffer);
	}
}
void DrawModelCameraProjection(Model& model, Image& img, Image& texture, float* zbuffer, float cameraZPos) {
	auto cameraProjection = [cameraZPos](Vec3f world_coords) ->Vec3f {
		// scale x and y while keep z.
		return Vec3f(world_coords.x / (1 - world_coords.z / cameraZPos), world_coords.y / (1 - world_coords.z / cameraZPos), world_coords.z);
	};
	for (int i = 0; i < model.nfaces(); i++) {
		std::vector<int> face = model.face(i);
		std::vector<int> faceTexcoords = model.faceTexCoords(i);
		Vec3f screen_coords[3];
		Vec3f world_coords[3];
		Vec2f TexCoords[3];
		for (int j = 0; j < 3; j++) {
			world_coords[j] = cameraProjection(model.vert(face[j]));
			TexCoords[j] = model.texCoords(faceTexcoords[j]);
			screen_coords[j] = Vec3f((world_coords[j].x + 1.) * img.getWidth() / 2., (world_coords[j].y + 1.) * img.getHeight() / 2., world_coords[j].z);
		}
		DrawSingleTriangle(screen_coords[0], screen_coords[1], screen_coords[2], TexCoords[0], TexCoords[1], TexCoords[2], img, texture, zbuffer);
	}
}
template<class T1, class T2>
Vec3f BarycentricCoords(Vec2<T1> P, Vec2<T2>P1, Vec2<T2>P2, Vec2<T2>P3) {
	Vec3f barycentric;
	// P = alpha P1 + beta P2 + gama P3, alpha + beta + gamma = 1
	barycentric.z = ((P1.y - P2.y) * P.x + (P2.x - P1.x) * P.y + P1.x * P2.y - P2.x * P1.y) /
		(float)(((P1.y - P2.y) * P3.x + (P2.x - P1.x) * P3.y + P1.x * P2.y - P2.x * P1.y));
	barycentric.y = ((P1.y - P3.y) * P.x + (P3.x - P1.x) * P.y + P1.x * P3.y - P3.x * P1.y) /
		(float)(((P1.y - P3.y) * P2.x + (P3.x - P1.x) * P2.y + P1.x * P3.y - P3.x * P1.y));
	barycentric.x = 1.f - barycentric.y - barycentric.z;
	return barycentric;
}		
