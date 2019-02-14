#pragma once

#include "Mesh.h"
#include "Utils.h"

#include "assimp\Importer.hpp"
#include "assimp\scene.h"
#include "assimp\postprocess.h"

#include <string>

class Model
{
public:
	Model();
	~Model();

	void Load(std::string path);
	void Dispose();
	void Render();

	const std::vector<Mesh> &GetMeshes() const { return meshes; }

private:
	void LoadAssimpModel();

	void ProcessNode(aiNode* node, const aiScene* scene);
	Mesh ProcessMesh(aiMesh *mesh, const aiScene *scene);
	void LoadMaterialTextures(std::vector<unsigned int> &textures, aiMaterial* mat, aiTextureType type, bool srgb);

private:
	std::vector<Mesh> meshes;
	std::string directory;
	std::string path;
};

