#include "Utils.h"

#include "gli\gli.hpp"
#include "gli\load_dds.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>

namespace utils
{
	GLuint LoadTexture(const std::string &path, bool srgb, GLint wrap)
	{
		GLuint id = 0;

		std::cout << path << std::endl;

		unsigned char* image = nullptr;
		int width, height, nChannels;

		image = stbi_load(path.c_str(), &width, &height, &nChannels, 0);

		if (!image)
		{
			std::cout << "Failed to load texture : " << path << std::endl;
			return 0;
		}

		GLint internalFormat;
		GLenum format;
		if (nChannels == 1)
		{
			internalFormat = GL_R8;
			format = GL_RED;
		}
		else if (nChannels == 3)
		{
			if (srgb)
				internalFormat = GL_SRGB8;
			else
				internalFormat = GL_RGB8;

			format = GL_RGB;
		}
		else if (nChannels == 4)
		{
			if (srgb)
				internalFormat = GL_SRGB8_ALPHA8;
			else
				internalFormat = GL_RGBA8;

			format = GL_RGBA;
		}

		//std::cout << nChannels << '\n';		

		glCreateTextures(GL_TEXTURE_2D, 1, &id);

		glTextureParameteri(id, GL_TEXTURE_WRAP_S, wrap);
		glTextureParameteri(id, GL_TEXTURE_WRAP_T, wrap);

		glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, image);

		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		stbi_image_free(image);

		return id;
	}

	GLuint LoadDDSKTX(const std::string &path, bool srgb)
	{
		GLuint id;
		int width, height;
		size_t mipLevels;

		std::cout << path << std::endl;

		gli::texture2d tex(gli::load(path));

		if (tex.empty())
		{
			std::cout << "Failed to load texture : " << path << std::endl;
			return -1;
		}

		gli::gl gl(gli::gl::PROFILE_GL33);
		gli::gl::format const format = gl.translate(tex.format(), tex.swizzles());
		GLenum target = gl.translate(tex.target());

		width = tex[0].extent().x;
		height = tex[0].extent().y;
		mipLevels = tex.levels();

		glCreateTextures(target, 1, &id);

		glTextureParameteri(id, GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteri(id, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(mipLevels - 1));
		glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (mipLevels > 1)
			glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		else
			glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		GLenum internalFormat;

		gli::texture::format_type type = tex.format();

		if (format.Internal == gli::gl::INTERNAL_RGB_DXT1)
		{
			//std::cout << "rgb: " << format.Internal << '\n';
			if (srgb)
				internalFormat = GL_COMPRESSED_SRGB_S3TC_DXT1_EXT;
			else
				internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
		}
		else if (format.Internal == gli::gl::INTERNAL_RGBA_DXT1)
		{
			//std::cout << "rgba: " << format.Internal << '\n';
			if (srgb)
				internalFormat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
			else
				internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		}
		else if (format.Internal == gli::gl::INTERNAL_RGBA_DXT5)
		{
			if (srgb)
				internalFormat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
			else
				internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		}

		if (tex.target() == gli::TARGET_2D)
			glTextureStorage2D(id, static_cast<GLuint>(mipLevels), internalFormat, width, height);

		for (std::size_t level = 0; level < mipLevels; ++level)
		{
			glm::vec2 extent = glm::vec2(tex.extent(level).x, tex.extent(level).y);
			glCompressedTextureSubImage2D(id, static_cast<GLint>(level), 0, 0, static_cast<GLsizei>(extent.x), static_cast<GLsizei>(extent.y), internalFormat, static_cast<GLsizei>(tex.size(level)), tex.data(0, 0, level));
		}

		return id;
	}

	GLuint LoadRAW2DTexture(const std::string &path, int width, int height, int channels, GLenum type)
	{
		FILE *file = nullptr;
		fopen_s(&file, path.c_str(), "rb");
		if (file == nullptr)
		{
			std::cout << "2d texture file is null\n";
			return 0;
		}

		float* data = new float[width * height * channels];

		if (type == GL_UNSIGNED_BYTE)
			fread(data, 1, width * height * channels * sizeof(unsigned char), file);
		else if (type == GL_FLOAT)
			fread(data, 1, width * height * channels * sizeof(float), file);

		fclose(file);

		GLenum format = GetInternalFormat(channels, type);
		GLuint id = 0;
		glCreateTextures(GL_TEXTURE_2D, 1, &id);
		glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(id, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureStorage2D(id, 1, format, width, height);
		glTextureSubImage2D(id, 0, 0, 0, width, height, GetFormat(format), type, data);

		delete[] data;

		return id;
	}

	GLuint Load3DTexture(const std::string &path, int width, int height, int depth, int channels, GLenum type)
	{
		FILE *file = nullptr;
		fopen_s(&file, path.c_str(), "rb");
		if (file == nullptr)
		{
			std::cout << "3d texture file is null\n";
			return 0;
		}

		float* data = new float[width * height * depth * channels];

		if (type == GL_UNSIGNED_BYTE)
			fread(data, 1, width * height * depth * channels * sizeof(unsigned char), file);
		else if (type == GL_FLOAT)
			fread(data, 1, width * height * depth * channels * sizeof(float), file);
		
		fclose(file);

		GLenum format = GetInternalFormat(channels, type);
		GLuint id = 0;
		glCreateTextures(GL_TEXTURE_3D, 1, &id);
		glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(id, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureStorage3D(id, 1, format, width, height, depth);
		glTextureSubImage3D(id, 0, 0, 0, 0, width, height, depth, GetFormat(format), type, data);

		delete[] data;

		return id;
	}

	GLuint Load3DTexture(int width, int height, int depth, int channels, GLenum type, GLint wrap, const void *data)
	{	
		GLenum format = GetInternalFormat(channels, type);
		GLuint id = 0;
		glCreateTextures(GL_TEXTURE_3D, 1, &id);
		glTextureParameteri(id, GL_TEXTURE_WRAP_S, wrap);
		glTextureParameteri(id, GL_TEXTURE_WRAP_T, wrap);
		glTextureParameteri(id, GL_TEXTURE_WRAP_R, wrap);
		glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureStorage3D(id, 1, format, width, height, depth);

		if (data)
			glTextureSubImage3D(id, 0, 0, 0, 0, width, height, depth, GetFormat(format), type, data);

		return id;
	}

	GLuint LoadCubemap(const char *faces[6])
	{
		GLuint id;
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &id);		
		glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		int width, height, nChannels;
		bool set = false;
		for (GLint i = 0; i < 6; i++)
		{
			unsigned char *image = stbi_load(faces[i], &width, &height, &nChannels, STBI_rgb);
			if (!image)
			{
				std::cout << "Failed to load face " << i << " : " << faces[i] << std::endl;
			}

			if (!set)
			{
				glTextureStorage2D(id, 1, GL_SRGB8, width, height);
				set = true;
			}

			glTextureSubImage3D(id, 0, 0, 0, i, width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, image);
			stbi_image_free(image);
		}

		glGenerateTextureMipmap(id);

		return id;
	}

	GLenum GetFormat(GLenum internalFormat)
	{
		switch (internalFormat)
		{
		case GL_R8:
			return GL_RED;
		case GL_RG8:
			return GL_RG;
		case GL_R16F:
			return GL_R16;
		case GL_RGBA16F:
			return GL_RGBA;
		case GL_RGB16F:
			return GL_RGB;
		case GL_RGB8:
			return GL_RGB;
		}

		return GL_RGBA;
	}

	GLenum GetInternalFormat(int channels, GLenum type)
	{
		GLenum format = 0;
		if (channels == 1)
		{
			if (type == GL_UNSIGNED_BYTE)
			{
				format = GL_R8;
			}
			else if (type == GL_FLOAT)
			{
				format = GL_R16F;
			}
		}
		else if (channels == 2)
		{
			if (type == GL_UNSIGNED_BYTE)
			{
				format = GL_RG8;
			}
			else if (type == GL_FLOAT)
			{
				format = GL_RG16F;
			}
		}
		else if (channels == 3)
		{
			if (type == GL_UNSIGNED_BYTE)
			{
				format = GL_RGB8;
			}
			else if (type == GL_FLOAT)
			{
				format = GL_RGB16F;
			}
		}
		else if (channels == 4)
		{
			if (type == GL_UNSIGNED_BYTE)
			{
				format = GL_RGBA8;
			}
			else if (type == GL_FLOAT)
			{
				format = GL_RGBA16F;
			}
		}
		return format;
	}
}