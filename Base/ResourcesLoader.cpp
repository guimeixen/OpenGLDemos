#include "ResourcesLoader.h"

#include "Utils.h"
#include "StringID.h"

std::map<unsigned int, TextureInf> ResourcesLoader::textures;

unsigned int ResourcesLoader::LoadTexture2D(const std::string &path, bool srgb)
{
	unsigned int id = SID(path);

	// If the texture already exists return that one instead
	if (textures.find(id) != textures.end())
	{
		return textures[id].id;
	}

	TextureInf info = {};
	info.path = path;

	if (std::strstr(path.c_str(), ".dds") > 0 || std::strstr(path.c_str(), ".ktx"))
		info.id = utils::LoadDDSKTX(path, srgb);
	else
		info.id = utils::LoadTexture(path, srgb);

	textures[id] = info;
	return info.id;
}

void ResourcesLoader::Dispose()
{
	for (auto it = textures.begin(); it != textures.end(); it++)
	{
		glDeleteTextures(1, &(*it).second.id);
	}
}
