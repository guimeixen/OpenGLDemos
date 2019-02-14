#pragma once

class Sphere
{
public:
	void Load(float radius = 0.5f);
	void Render();
	void Dispose();

private:
	unsigned int vao;
	unsigned int vbo;
	unsigned int ibo;
	unsigned int indexCount;
};

