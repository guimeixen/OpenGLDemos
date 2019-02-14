#include "Mesh.h"

#include <iostream>

Mesh::Mesh()
{
}

Mesh::Mesh(GLuint vao, GLuint ibo, GLuint vbo, const std::vector<unsigned int> &textures, GLuint indexCount)
{
	this->vao = vao;
	this->vbo = vbo;
	this->ibo = ibo;
	this->textures = textures;
	this->indexCount = indexCount;
}

Mesh::~Mesh()
{
}

void Mesh::Dispose()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ibo);

	for (size_t i = 0; i < textures.size(); i++)
	{
		glDeleteTextures(1, &textures[i]);
	}
}

void Mesh::Render() const
{
	for (size_t i = 0; i < textures.size(); i++)
	{
		glBindTextureUnit(i, textures[i]);
	}

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}
