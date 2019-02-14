#pragma once

#include <map>
#include <string>

struct TextureInf
{
	std::string path;
	unsigned int id;
};

class ResourcesLoader
{
public:
	static unsigned int LoadTexture2D(const std::string &path, bool srgb);

	static void Dispose();

private:
	static std::map<unsigned int, TextureInf> textures;
};

