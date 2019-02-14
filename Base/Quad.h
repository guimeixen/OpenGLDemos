#pragma once

class Quad
{
public:
	Quad();
	~Quad();

	void Load();
	void Dispose();
	void Render();

private:
	unsigned int vao;
	unsigned int vbo;
};

