#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
class Model {
private:
	std::vector<glm::vec3> verts_;
	std::vector<std::vector<int> > faces_;
	std::vector<std::vector<int>> facesToTexCoords_;
	std::vector<glm::vec2> texcoords_;
public:
	Model(const char* filename);
	~Model();
	int nverts() const;
	int nfaces() const;
	int nTexCoords() const;
	glm::vec3 vert(int i);
	glm::vec2 texCoords(int i);
	std::vector<int> face(int idx);
	std::vector<int> faceTexCoords(int idx);
};

#endif //__MODEL_H__