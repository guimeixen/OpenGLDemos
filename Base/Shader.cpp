#include "shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "glm\gtc\type_ptr.hpp"

Shader::Shader()
{
	program = 0;
}

Shader::~Shader()
{
}

void Shader::Load(const std::string &vertexPath, const std::string &fragmentPath)
{
	std::ifstream vertexFile, fragmentFile;

	vertexFile.open(vertexPath);
	fragmentFile.open(fragmentPath);

	if (!vertexFile.is_open() || !fragmentFile.is_open())
	{
		std::cout << "Failed to load shader: \n\t" << vertexPath << "\n\t" << fragmentPath << std::endl;
		return;
	}

	this->vertexPath = vertexPath;
	this->fragmentPath = fragmentPath;

	std::string dir = vertexPath.substr(0, vertexPath.find_last_of("/\\"));
	std::string vertexCode = ReadFile(vertexFile, dir);
	std::string fragmentCode = ReadFile(fragmentFile, dir);

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

	int status;
	char log[1024];

	const char *vsCode = vertexCode.c_str();
	const char *fsCode = fragmentCode.c_str();

	glShaderSource(vs, 1, &vsCode, NULL);
	glShaderSource(fs, 1, &fsCode, NULL);

	glCompileShader(vs);
	glCompileShader(fs);

	glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
	if (!status)
	{
		glGetShaderInfoLog(vs, 1024, NULL, log);
		std::cout << "Vertex shader compilation failed:" << vertexPath << std::endl << std::string(log) << std::endl;
	}

	glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
	if (!status)
	{
		glGetShaderInfoLog(fs, 1024, NULL, log);
		std::cout << "Fragment shader compilation failed:" << fragmentPath << std::endl << std::string(log) << std::endl;
	}

	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (!status)
	{
		glGetProgramInfoLog(program, 1024, NULL, log);
		std::cout << "Shader program linking failed:\n" << std::string(log) << std::endl;
	}

	glDeleteShader(vs);
	glDeleteShader(fs);

	SetUniformLocations();
}

void Shader::Load(const std::string &vertexPath, const std::string &geometryPath, const std::string &fragmentPath)
{
	std::ifstream vertexFile, geometryFile, fragmentFile;

	vertexFile.open(vertexPath);
	geometryFile.open(geometryPath);
	fragmentFile.open(fragmentPath);

	if (!vertexFile.is_open() || !geometryFile.is_open() || !fragmentFile.is_open())
	{
		std::cout << "Failed to load shader: \n\t" << vertexPath << "\n\t" << geometryPath << "\n\t" << fragmentPath << std::endl;
		return;
	}

	this->vertexPath = vertexPath;
	this->geometryPath = geometryPath;
	this->fragmentPath = fragmentPath;

	std::string dir = vertexPath.substr(0, vertexPath.find_last_of("/\\"));
	std::string vertexCode = ReadFile(vertexFile, dir);
	std::string geometryCode = ReadFile(geometryFile, dir);
	std::string fragmentCode = ReadFile(fragmentFile, dir);

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	GLuint gs = glCreateShader(GL_GEOMETRY_SHADER);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

	int status;
	char log[1024];

	const char *vsCode = vertexCode.c_str();
	const char *gsCode = geometryCode.c_str();
	const char *fsCode = fragmentCode.c_str();

	glShaderSource(vs, 1, &vsCode, NULL);
	glShaderSource(gs, 1, &gsCode, NULL);
	glShaderSource(fs, 1, &fsCode, NULL);

	glCompileShader(vs);
	glCompileShader(gs);
	glCompileShader(fs);

	glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
	if (!status)
	{
		glGetShaderInfoLog(vs, 1024, NULL, log);
		std::cout << "Vertex shader compilation failed:" << vertexPath << '\n' << std::string(log) << '\n';
	}

	glGetShaderiv(gs, GL_COMPILE_STATUS, &status);
	if (!status)
	{
		glGetShaderInfoLog(gs, 1024, NULL, log);
		std::cout << "Geometry shader compilation failed:" << geometryPath << '\n' << std::string(log) << '\n';
	}

	glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
	if (!status)
	{
		glGetShaderInfoLog(fs, 1024, NULL, log);
		std::cout << "Fragment shader compilation failed:" << fragmentPath << '\n' << std::string(log) << '\n';
	}

	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, gs);
	glAttachShader(program, fs);

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (!status)
	{
		glGetProgramInfoLog(program, 1024, NULL, log);
		std::cout << "Shader program linking failed:\n" << std::string(log) << std::endl;
	}

	glDeleteShader(vs);
	glDeleteShader(gs);
	glDeleteShader(fs);

	SetUniformLocations();
}

void Shader::Load(const std::string &computePath)
{
	std::ifstream computeFile;

	computeFile.open(computePath);

	if (!computeFile.is_open())
	{
		std::cout << "Failed to load shader: \n\t" << computePath << std::endl;
		return;
	}

	this->computePath = computePath;

	std::string dir = computePath.substr(0, computePath.find_last_of("/\\"));
	std::string computeCode = ReadFile(computeFile, dir);

	GLuint cs = glCreateShader(GL_COMPUTE_SHADER);

	int status;
	char log[1024];

	const char *csCode = computeCode.c_str();

	glShaderSource(cs, 1, &csCode, NULL);

	glCompileShader(cs);

	glGetShaderiv(cs, GL_COMPILE_STATUS, &status);
	if (!status)
	{
		glGetShaderInfoLog(cs, 1024, NULL, log);
		std::cout << "Compute shader compilation failed:" << computePath << std::endl << std::string(log) << std::endl;
	}

	program = glCreateProgram();
	glAttachShader(program, cs);

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (!status)
	{
		glGetProgramInfoLog(program, 1024, NULL, log);
		std::cout << "Shader program linking failed:\n" << std::string(log) << std::endl;
	}

	glDeleteShader(cs);

	SetUniformLocations();
}

void Shader::Use()
{
	glUseProgram(program);
}

void Shader::Unuse()
{
	glUseProgram(0);
}

void Shader::Dispose()
{
	if (program > 0)
		glDeleteProgram(program);
}

void Shader::Reload()
{
	glUseProgram(0);
	Dispose();

	if (computePath.length() != 0)
		Load(computePath);
	else if (geometryPath.length() != 0)
		Load(vertexPath, geometryPath, fragmentPath);
	else
		Load(vertexPath, fragmentPath);
}

GLuint Shader::GetProgram() const
{
	return program;
}

void Shader::SetMat4(const std::string &name, const glm::mat4& matrix)
{
	glUniformMatrix4fv(uniforms[name], 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shader::SetMat4Array(const std::string &name, const glm::mat4 *matrices, unsigned int count)
{
	glUniformMatrix4fv(uniforms[name], count, GL_FALSE, (const GLfloat*)matrices);
}

void Shader::SetMat3(const std::string &name, const glm::mat3 &matrix)
{
	glUniformMatrix3fv(uniforms[name], 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shader::SetVec4(const std::string &name, const glm::vec4 &v)
{
	glUniform4f(uniforms[name], v.x, v.y, v.z, v.w);
}

void Shader::SetVec3(const std::string &name, const glm::vec3 &v)
{
	glUniform3f(uniforms[name], v.x, v.y, v.z);
}

void Shader::SetUVec2(const std::string &name, const glm::uvec2 &v)
{
	glUniform2ui(uniforms[name], v.x, v.y);
}

void Shader::SetVec2(const std::string &name, const glm::vec2 &v)
{
	glUniform2f(uniforms[name], v.x, v.y);
}

void Shader::SetFloat(const std::string &name, const float x)
{
	glUniform1f(uniforms[name], x);
}

void Shader::SetBool(const std::string &name, bool value)
{
	glUniform1i(uniforms[name], value);
}

void Shader::SetUint(const std::string &name, unsigned int value)
{
	glUniform1ui(uniforms[name], value);
}

void Shader::SetInt(const std::string &name, int value)
{
	std::map<std::string, int>::const_iterator pos = uniforms.find(name);			// Check if we have the uniform in the map otherwise it would add a new one with the default value of 0, which could
	if (pos == uniforms.end())														// overwrite an already set uniform and for it to not happen we use -1
	{
		glUniform1i(-1, value);
	}
	else
	{
		glUniform1i(uniforms[name], value);
	}
}

void Shader::SetUniformLocations()
{
	uniforms.clear();

	int nrUniforms;
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &nrUniforms);

	char buffer[128];
	int size;
	GLenum glType;
	std::string uniformName;

	for (int i = 0; i < nrUniforms; i++)
	{
		glGetActiveUniform(program, i, sizeof(buffer), 0, &size, &glType, buffer);

		uniformName = buffer;

		size_t pos = uniformName.find_last_of("[");

		if (pos != std::string::npos)
		{
			std::string startingName = uniformName.substr(0, pos);
			uniformName = startingName;

			for (int i = 0; i < size; i++)
			{
				GLint loc = glGetUniformLocation(program, buffer);
				if (loc >= 0)
				{
					uniformName += "[" + std::to_string(i) + "]";
					uniforms.insert({ uniformName, loc });
					uniformName = startingName;
				}
			}
		}
		else
		{
			GLint loc = glGetUniformLocation(program, buffer);
			if (loc >= 0)
			{
				uniforms.insert({ uniformName, loc });
			}
		}
	}
}

std::string Shader::ReadFile(std::ifstream &file, const std::string &dir)
{
	std::string src, line;

	while (std::getline(file, line))
	{
		if (line.substr(0, 8) == "#include")
		{
			std::string includePath = dir + "/" + line.substr(9);
			std::ifstream includeFile(includePath);

			if (includeFile.is_open())
			{
				src += ReadFile(includeFile, dir);
				includeFile.close();
			}
			else
			{
				std::cout << "Failed to include shader:\n\t" + includePath << std::endl;
			}
		}
		else
			src += line + "\n";
	}

	return src;
}
