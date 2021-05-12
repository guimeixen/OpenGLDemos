#pragma once

#include "glew\glew.h"
#include "glm\glm.hpp"

#include <vector>

struct Vertex
{
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 bitangent;
	glm::vec3 color;
};

class Mesh
{
public:
	Mesh();
	Mesh(GLuint vao, GLuint ibo, GLuint vbo, const std::vector<unsigned int> &textures, GLuint indexCount);
	~Mesh();

	void Dispose();
	void Render() const;

private:
	GLuint vao;
	GLuint vbo;
	GLuint ibo;
	GLuint indexCount;
	std::vector<unsigned int> textures;
};

