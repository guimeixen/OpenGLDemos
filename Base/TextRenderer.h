#pragma once

#include "Shader.h"

#include "glm\glm.hpp"

#include <string>
#include <vector>
#include <map>

#define MAX_QUADS 1532 * 6	// * 6 because a quad has 6 vertices

struct VertexPOS2D_UV_COLOR
{
	glm::vec4 posuv;
	glm::vec4 color;
};

struct Character
{
	int id;
	glm::vec2 uv;
	glm::vec2 size;
	glm::vec2 offset;
	float advance;
};

struct Text
{
	std::string text;
	glm::vec2 pos;
	glm::vec2 scale;
	glm::vec4 color;
};

class TextRenderer
{
public:
	TextRenderer();
	~TextRenderer();

	void Init(const std::string &fontPath, const std::string &fontAtlasPath);
	void Render();
	void Dispose();
	void Enable(bool enable) { enabled = enable; }
	void Resize(unsigned int width, unsigned int height);
	void AddText(const std::string &text, const glm::vec2 &pos, const glm::vec2 &scale = glm::vec2(1.0f), const glm::vec4 &color = glm::vec4(1.0f));
	void AddText(const char *text, const glm::vec2 &pos, const glm::vec2 &scale = glm::vec2(1.0f), const glm::vec4 &color = glm::vec4(1.0f));
	glm::vec2 CalculateTextSize(const std::string &text, const glm::vec2 &scale);
	glm::vec2 CalculateCharSize(char c, const glm::vec2 &scale);

private:
	void PrepareText();

private:
	unsigned int width;
	unsigned int height;

	GLuint vao;
	GLuint vbo;
	Shader textShader;
	GLuint textAtlas;
	glm::mat4 projectionMatrix;

	bool enabled;

	std::vector<Text> textBuffer;
	std::map<int, Character> characters;
	VertexPOS2D_UV_COLOR quadsBuffer[MAX_QUADS];
	unsigned int maxCharsBuffer;					// max number of characters that are stored. Once it reaches the max we render them and start storing them again
	unsigned int curQuadCount;
	int paddingWidth;
	int padding[4];
	unsigned int lineHeight;
};

