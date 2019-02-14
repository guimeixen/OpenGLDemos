#include "TextRenderer.h"

#include "Utils.h"

#include "glm\gtc\matrix_transform.hpp"

#include <fstream>
#include <iostream>
#include <algorithm>

TextRenderer::TextRenderer()
{
	maxCharsBuffer = MAX_QUADS / 6;
	curQuadCount = 0;
	enabled = true;
	lineHeight = 0;
}

TextRenderer::~TextRenderer()
{
}

void TextRenderer::Init(const std::string &fontPath, const std::string &fontAtlasPath)
{
	textAtlas = utils::LoadTexture(fontAtlasPath, false);
	//TextureParams params = { TextureWrap::REPEAT, TextureFilter::LINEAR, TextureFormat::RGBA, TextureInternalFormat::RGBA8, TextureDataType::UNSIGNED_BYTE, true };

	textShader.Load("Data/text.vert", "Data/text.frag");

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, maxCharsBuffer * 6 * sizeof(VertexPOS2D_UV_COLOR), nullptr, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), nullptr);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(4 * sizeof(float)));

	glBindVertexArray(0);

	std::ifstream file(fontPath);

	if (!file.is_open())
	{
		std::cout << "Error! Failed to load font file\n";
		std::cout << fontPath << "\n";
	}

	std::string line;
	std::string temp;

	while (std::getline(file, line))
	{
		if (line.substr(0, 10) == "info face=")
		{
			size_t pos = line.find("padding=");

			if (pos != std::string::npos && line.substr(pos, 8) == "padding=")
			{
				padding[0] = std::stoi(line.substr(pos + 8));

				size_t next = line.find(",", pos);
				padding[1] = std::stoi(line.substr(next + 1));

				next = line.find(",", next + 1);			// Add plus one so we go forward, otherwise we would keep getting the position of the first comma
				padding[2] = std::stoi(line.substr(next + 1));

				next = line.find(",", next + 1);
				padding[3] = std::stoi(line.substr(next + 1));

				paddingWidth = padding[1] + padding[3];
			}
		}

		if (line.substr(0, 18) == "common lineHeight=")
			lineHeight = (unsigned int)std::stoi(line.substr(18, 3));	// Line height is stored as lineHeight=xx   We get the next 3 chars instead of 2 because there might be line heights bigger than 100

																		// Only add a character if this line describes the character 
		if (line.substr(0, 8) == "char id=")
		{
			Character c = {};
			c.id = std::stoi(line.substr(8));

			size_t pos = line.find("x=");

			if (pos != std::string::npos && line.substr(pos, 2) == "x=")
				c.uv.x = std::stof(line.substr(pos + 2));

			pos = line.find("y=");

			if (pos != std::string::npos && line.substr(pos, 2) == "y=")
				c.uv.y = std::stof(line.substr(pos + 2));

			pos = line.find("width=");

			if (pos != std::string::npos && line.substr(pos, 6) == "width=")
				c.size.x = std::stof(line.substr(pos + 6));

			pos = line.find("height=");

			if (pos != std::string::npos && line.substr(pos, 7) == "height=")
				c.size.y = std::stof(line.substr(pos + 7));

			pos = line.find("xoffset=");

			if (pos != std::string::npos && line.substr(pos, 8) == "xoffset=")
				c.offset.x = std::stof(line.substr(pos + 8));

			pos = line.find("yoffset=");

			if (pos != std::string::npos && line.substr(pos, 8) == "yoffset=")
				c.offset.y = std::stof(line.substr(pos + 8));

			pos = line.find("xadvance=");

			if (pos != std::string::npos && line.substr(pos, 9) == "xadvance=")
				c.advance = std::stof(line.substr(pos + 9));

			characters[c.id] = c;
		}
	}

	file.close();
}

void TextRenderer::PrepareText()
{
	for (size_t i = 0; i < textBuffer.size(); i++)
	{
		const Text &t = textBuffer[i];

		float x = t.pos.x;
		float y = t.pos.y;

		// We have a limit on how much text we can render
		if (curQuadCount > maxCharsBuffer)
			break;

		for (const char &cc : t.text)
		{
			// This second if breaks this inner loop the other if breaks the first one
			if (curQuadCount > maxCharsBuffer)
				break;

			if (cc == '\n')
			{
				x = t.pos.x;
				y -= lineHeight * t.scale.y;		// Line height seems a bit too high, check angel code fnt file
				continue;
			}

			const Character &c = characters[cc];

			float xpos = x + c.offset.x * t.scale.x;
			float ypos = y - (c.size.y + c.offset.y) * t.scale.y;
			float w = c.size.x * t.scale.x;
			float h = c.size.y * t.scale.y;

			// Replace with textAtlas->getwidth

			float val1 = c.uv.x / 512.0f;
			float val2 = (c.uv.x + c.size.x) / 512.0f;
			float val4 = c.uv.y / 512.0f;
			float val3 = (c.uv.y + c.size.y) / 512.0f;

			glm::vec4 topLeft = glm::vec4(xpos, ypos + h, 0.0f, 1.0f);
			glm::vec4 bottomLeft = glm::vec4(xpos, ypos, 0.0f, 1.0f);
			glm::vec4 bottomRight = glm::vec4(xpos + w, ypos, 0.0f, 1.0f);
			glm::vec4 topRight = glm::vec4(xpos + w, ypos + h, 0.0f, 1.0f);

			quadsBuffer[curQuadCount * 6 + 0] = { glm::vec4(topLeft.x,		topLeft.y,		val1, val4), t.color };
			quadsBuffer[curQuadCount * 6 + 1] = { glm::vec4(bottomLeft.x,	bottomLeft.y,	val1, val3), t.color };
			quadsBuffer[curQuadCount * 6 + 2] = { glm::vec4(bottomRight.x,	bottomRight.y,	val2, val3), t.color };
			quadsBuffer[curQuadCount * 6 + 3] = { glm::vec4(topLeft.x,		topLeft.y,		val1, val4), t.color };
			quadsBuffer[curQuadCount * 6 + 4] = { glm::vec4(bottomRight.x,	bottomRight.y,	val2, val3), t.color };
			quadsBuffer[curQuadCount * 6 + 5] = { glm::vec4(topRight.x,		topRight.y,		val2, val4), t.color };

			x += (c.advance - paddingWidth) * t.scale.x;

			curQuadCount++;
		}
	}

	if (curQuadCount > 0)
	{
		glNamedBufferSubData(vbo, 0, curQuadCount * 6 * sizeof(VertexPOS2D_UV_COLOR), quadsBuffer);
		//glBindBuffer(GL_ARRAY_BUFFER, vbo);
		//glBufferSubData(GL_ARRAY_BUFFER, 0, curQuadCount * 6 * sizeof(VertexPOS2D_UV_COLOR), quadsBuffer);
	}
}

void TextRenderer::Render()
{
	PrepareText();

	if (curQuadCount > 0)
	{
		textShader.Use();
		textShader.SetMat4("projectionMatrix", projectionMatrix);
		glBindTextureUnit(0, textAtlas);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, curQuadCount * 6);

		textBuffer.clear();
		curQuadCount = 0;
	}
}

void TextRenderer::Dispose()
{
	if (vao > 0)
		glDeleteVertexArrays(1, &vao);
	if (vbo > 0)
		glDeleteBuffers(1, &vbo);
}

void TextRenderer::Resize(unsigned int width, unsigned int height)
{
	this->width = width;
	this->height = height;

	projectionMatrix = glm::ortho(0.0f, (float)width, 0.0f, (float)height, 0.0f, 10.0f);
}

void TextRenderer::AddText(const std::string &text, const glm::vec2 &pos, const glm::vec2 &scale, const glm::vec4 &color)
{
	if (enabled)
		textBuffer.push_back({ text, pos, scale, color });
}

void TextRenderer::AddText(const char *text, const glm::vec2 &pos, const glm::vec2 &scale, const glm::vec4 &color)
{
	if (enabled)
		textBuffer.push_back({ text, pos, scale, color });
}

glm::vec2 TextRenderer::CalculateTextSize(const std::string &text, const glm::vec2 &scale)
{
	glm::vec2 size = glm::vec2();

	for (const char &cc : text)
	{
		const Character &c = characters[cc];
		size.x += (c.advance - paddingWidth) * scale.x;
		float h = c.size.y * scale.y;
		size.y = std::max(size.y, h);
	}

	return size;
}

glm::vec2 TextRenderer::CalculateCharSize(char c, const glm::vec2 &scale)
{
	glm::vec2 size = glm::vec2();

	const Character &cc = characters[c];
	size.x += (cc.advance - paddingWidth) * scale.x;
	size.y = cc.size.y * scale.y;

	return size;
}
