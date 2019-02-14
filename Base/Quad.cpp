#include "Quad.h"

#include "glew\glew.h"

Quad::Quad()
{
}

Quad::~Quad()
{
}

void Quad::Load()
{
	float vertices[] = {
		-1.0f, 1.0f,	0.0f, 1.0f,
		-1.0f, -1.0f,	0.0f, 0.0f,
		1.0f, -1.0f,	1.0f, 0.0f,
		-1.0f, 1.0f,	0.0f, 1.0f,
		1.0f, -1.0f,	1.0f, 0.0f,
		1.0f, 1.0f,		1.0f, 1.0f };

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
	glBindVertexArray(0);
}

void Quad::Dispose()
{
	if (vao > 0)
		glDeleteVertexArrays(1, &vao);
	if (vbo > 0)
		glDeleteBuffers(1, &vbo);
}

void Quad::Render()
{
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}
