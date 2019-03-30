#pragma once

#include "glew/glew.h"
#include "glm/glm.hpp"

#include <string>

namespace utils
{
	GLuint LoadTexture(const std::string &path, bool srgb, GLint wrap = GL_REPEAT);
	GLuint LoadDDSKTX(const std::string &path, bool srgb);
	GLuint LoadRAW2DTexture(const std::string &path, int width, int height, int channels, GLenum type);
	GLuint Load3DTexture(const std::string &path, int width, int height, int depth, int channels, GLenum type);
	GLuint Load3DTexture(int width, int height, int depth, int channels, GLenum type, GLint wrap, const void *data);
	GLuint LoadCubemap(const char *faces[6]);
	GLenum GetFormat(GLenum internalFormat);
	GLenum GetInternalFormat(int channels, GLenum type);
	glm::vec3 HSVtoRGB(const glm::vec3 &hsv);
}