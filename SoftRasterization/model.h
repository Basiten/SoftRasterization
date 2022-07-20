#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<std::vector<int> > faces_;
	std::vector<std::vector<int>> facesToTexCoords_;
	std::vector<Vec2f> texcoords_;
public:
	Model(const char* filename);
	~Model();
	int nverts() const;
	int nfaces() const;
	int nTexCoords() const;
	Vec3f vert(int i);
	Vec2f texCoords(int i);
	std::vector<int> face(int idx);
	std::vector<int> faceTexCoords(int idx);
};

#endif //__MODEL_H__