#include "Cube.h"

#include "glew\glew.h"

Cube::Cube()
{
}

Cube::~Cube()
{
}

void Cube::Load()
{
	float size = 0.5f;
	float vertices[] = {
		size,  size, -size, 0.0, 0.0,  0.0,  0.0, -1.0,
		size, -size, -size, 0.0, 1.0,  0.0,  0.0, -1.0,
		-size, -size, -size, 1.0, 1.0,  0.0,  0.0, -1.0,
		-size, -size, -size, 1.0, 1.0,  0.0,  0.0, -1.0,
		-size,  size, -size, 1.0, 0.0,  0.0,  0.0, -1.0,
		size,  size, -size, 0.0, 0.0,  0.0,  0.0, -1.0,

		-size, -size,  size, 0.0, 0.0,  0.0,  0.0,  1.0,
		size, -size,  size, 1.0, 0.0,  0.0,  0.0,  1.0,
		size,  size,  size, 1.0, 1.0,  0.0,  0.0,  1.0,
		size,  size,  size, 1.0, 1.0,  0.0,  0.0,  1.0,
		-size,  size,  size, 0.0, 1.0,  0.0,  0.0,  1.0,
		-size, -size,  size, 0.0, 0.0,  0.0,  0.0,  1.0,

		-size,  size,  size, 1.0, 1.0, -1.0,  0.0,  0.0,
		-size,  size, -size, 1.0, 0.0, -1.0,  0.0,  0.0,
		-size, -size, -size, 0.0, 0.0, -1.0,  0.0,  0.0,
		-size, -size, -size, 0.0, 0.0, -1.0,  0.0,  0.0,
		-size, -size,  size, 0.0, 1.0, -1.0,  0.0,  0.0,
		-size,  size,  size, 1.0, 1.0, -1.0,  0.0,  0.0,

		size, -size,  -size, 0.0, 0.0,  1.0,  0.0,  0.0,
		size, size, -size, 1.0, 0.0,  1.0,  0.0,  0.0,
		size,  size,  size, 1.0, 1.0,  1.0,  0.0,  0.0,
		size,  size,  size, 1.0, 1.0,  1.0,  0.0,  0.0,
		size, -size,  size, 0.0, 1.0,  1.0,  0.0,  0.0,
		size, -size, -size, 0.0, 0.0,  1.0,  0.0,  0.0,

		-size, -size, -size, 0.0, 0.0,  0.0, -1.0,  0.0,
		size, -size, -size, 1.0, 0.0,  0.0, -1.0,  0.0,
		size, -size,  size, 1.0, 1.0,  0.0, -1.0,  0.0,
		size, -size,  size, 1.0, 1.0,  0.0, -1.0,  0.0,
		-size, -size,  size, 0.0, 1.0,  0.0, -1.0,  0.0,
		-size, -size, -size, 0.0, 0.0,  0.0, -1.0,  0.0,

		size,  size,  size, 1.0, 1.0,  0.0,  1.0,  0.0,
		size,  size, -size, 1.0, 0.0,  0.0,  1.0,  0.0,
		-size,  size, -size, 0.0, 0.0,  0.0,  1.0,  0.0,
		size,  size,  size, 1.0, 1.0,  0.0,  1.0,  0.0,
		-size,  size, -size, 0.0, 0.0,  0.0,  1.0,  0.0,
		-size,  size,  size, 0.0, 1.0,  0.0,  1.0,  0.0,
	};

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), nullptr);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(5 * sizeof(float)));

	glBindVertexArray(0);
}

void Cube::Render()
{
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

void Cube::Dispose()
{
	if (vao > 0)
		glDeleteVertexArrays(1, &vao);
	if (vbo > 0)
		glDeleteBuffers(1, &vbo);
}
