#pragma once

class Cube
{
public:
	Cube();
	~Cube();

	void Load();
	void Render();
	void Dispose();

private:
	unsigned int vao;
	unsigned int vbo;
};

