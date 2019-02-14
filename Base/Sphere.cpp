#include "Sphere.h"

#include "glew\glew.h"
#include "glm\glm.hpp"
#include <vector>

void Sphere::Load(float radius)
{
	// Create the debug sphere vertices and indices
	int latBands = 16;
	int longBands = 24;
	std::vector<glm::vec3> vertices;
	std::vector<unsigned short> indices;

	for (int y = 0; y <= latBands; y++)
	{
		float theta = y * 3.14159f / latBands;
		float sinTheta = glm::sin(theta);
		float cosTheta = glm::cos(theta);

		for (int x = 0; x <= longBands; x++)
		{
			float phi = x * 2.0f * 3.14159f / longBands;
			float sinPhi = glm::sin(phi);
			float cosPhi = glm::cos(phi);

			glm::vec3 v;
			v.x = radius * cosPhi * sinTheta;
			v.z = radius * sinPhi * sinTheta;
			v.y = radius * cosTheta;
			vertices.push_back(v);
		}
	}

	for (short y = 0; y < latBands; y++)
	{
		for (short x = 0; x < longBands; x++)
		{
			short first = (y * (longBands + 1)) + x;
			short second = first + longBands + 1;

			indices.push_back(first + 1);
			indices.push_back(second);
			indices.push_back(first);

			indices.push_back(first + 1);
			indices.push_back(second + 1);
			indices.push_back(second);
		}
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), indices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

	glBindVertexArray(0);

	indexCount = static_cast<unsigned int>(indices.size());
}

void Sphere::Render()
{
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, nullptr);
}

void Sphere::Dispose()
{
	if (vao > 0)
		glDeleteVertexArrays(1, &vao);
	if (vbo > 0)
		glDeleteBuffers(1, &vbo);
	if (ibo > 0)
		glDeleteBuffers(1, &ibo);
}
