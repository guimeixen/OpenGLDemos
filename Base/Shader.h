#pragma once

#include "glew/glew.h"
#include "glm/glm.hpp"

#include <map>
#include <string>

class Shader
{
public:
	Shader();
	~Shader();

	void Load(const std::string &vertexPath, const std::string &fragmentPath);
	void Load(const std::string &vertexPath, const std::string &geometryPath, const std::string &fragmentPath);
	void Load(const std::string &computePath);

	void Use();
	void Unuse();
	void Dispose();
	void Reload();

	GLuint GetProgram() const;
	void SetMat4(const std::string &name, const glm::mat4 &matrix);
	void SetMat4Array(const std::string &name, const glm::mat4 *matrices, unsigned int count);
	void SetMat3(const std::string &name, const glm::mat3 &matrix);
	void SetVec4(const std::string &name, const glm::vec4 &v);
	void SetVec3(const std::string &name, const glm::vec3 &v);
	void SetUVec2(const std::string &name, const glm::uvec2 &v);
	void SetVec2(const std::string &name, const glm::vec2 &v);
	void SetFloat(const std::string &name, const float x);
	void SetBool(const std::string &name, bool value);
	void SetUint(const std::string &name, unsigned int value);
	void SetInt(const std::string &name, int value);

private:
	void SetUniformLocations();
	std::string ReadFile(std::ifstream &file, const std::string &dir);

private:
	GLuint program;
	std::map<std::string, int> uniforms;
	std::string vertexPath;
	std::string fragmentPath;
	std::string geometryPath;
	std::string computePath;
};
