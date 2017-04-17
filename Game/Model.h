#pragma once

#pragma region INCLUDES

#ifndef MODEL
#define MODEL

#include "maths_funcs.h"
#include <GL/glew.h>
#include <vector> 
#include <iostream>

#include "Shader.h"
#include "Mesh.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#pragma endregion INCLUDES

GLint TextureFromFile(const char* fileName, std::string path);

class Model
{
public:
	Model(const char* fileName = "", std::string path = "")
	{
		this->path = path;
		if (fileName != "") {
			this->loadModel(fileName);
		}
	}

	void Draw(Shader meshShader, bool multiTexture)
	{
		for (GLuint i = 0; i < this->meshes.size(); i++)
			this->meshes[i].Draw(meshShader, multiTexture);
	}

private:
	std::vector<Mesh> meshes;
	std::vector<Texture> texturesLoaded;
	std::string path;

	void loadModel(const char* fileName)
	{
		std::cout << "Loading model : " << fileName << std::endl;

		Assimp::Importer modelImporter;
		const aiScene* scene = modelImporter.ReadFile(fileName, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

		if (!scene) {
			std::cout << modelImporter.GetErrorString() << std::endl;
			fprintf(stderr, "ERROR: reading mesh %s\n", fileName);
			return;
		}

		printf("  %i animations\n", scene->mNumAnimations);
		printf("  %i cameras\n", scene->mNumCameras);
		printf("  %i lights\n", scene->mNumLights);
		printf("  %i materials\n", scene->mNumMaterials);
		printf("  %i meshes\n", scene->mNumMeshes);
		printf("  %i textures\n", scene->mNumTextures);

		this->processNode(scene->mRootNode, scene);
	}

	void processNode(aiNode* node, const aiScene* scene)
	{
		// Process all the node's meshes
		// Each node contains array of meshes which are indexes into the scenes mMeshes array
		for (GLuint i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			this->meshes.push_back(this->processMesh(mesh, scene));
		}

		// Process all child nodes
		for (GLuint i = 0; i < node->mNumChildren; i++)
		{
			this->processNode(node->mChildren[i], scene);
		}
	}

	Mesh processMesh(aiMesh* mesh, const aiScene* scene)
	{
		std::vector<Vertex> vertices;
		std::vector<GLuint> indices;
		std::vector<Texture> textures;

		for (GLuint i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			vec3 vector;
			
			if (mesh->HasPositions()) {
				const aiVector3D* v_pos = &(mesh->mVertices[i]);

				vector.v[0] = v_pos->x;
				vector.v[1] = v_pos->y;
				vector.v[2] = v_pos->z;

				vertex.Position = vector;
			}

			if (mesh->HasNormals()) {
				const aiVector3D* v_norm = &(mesh->mNormals[i]);

				vector.v[0] = v_norm->x;
				vector.v[1] = v_norm->y;
				vector.v[2] = v_norm->z;

				vertex.Normal = vector;
			}
		
			if (mesh->HasTextureCoords(0))
			{
				const aiVector3D* v_tex = &(mesh->mTextureCoords[0][i]);
				vec2 vec;

				vec.v[0] = v_tex->x;
				vec.v[1] = v_tex->y;

				vertex.TexCoords = vec;
			}
			else {
				vertex.TexCoords = vec2(0.0f, 0.0f);
			}

			if (mesh->HasTangentsAndBitangents()) {
				vec3 tangent;
				tangent.v[0] = mesh->mTangents[i].x;
				tangent.v[1] = mesh->mTangents[i].y;
				tangent.v[2] = mesh->mTangents[i].z;

				vertex.Tangent = tangent;
			}

			vertices.push_back(vertex);
		}

		// Add the indices for all faces to the indices vector
		for (GLuint i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];

			for (GLuint j = 0; j < face.mNumIndices; j++) {
				indices.push_back(face.mIndices[j]);
			}
		}

		// Process materials
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			// 1. Diffuse maps
			std::vector<Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

			// 2. Specular maps
			std::vector<Texture> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

			std::vector<Texture> normalMaps = this->loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
			textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
		}

		return Mesh(vertices, indices, textures);
	}

	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
	{
		std::vector<Texture> textures;

		for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString pathToTexture;

			mat->GetTexture(type, i, &pathToTexture);

			// If its already loaded just use that instead of reloading it again
			GLboolean skip = false;
			for (GLuint j = 0; j < texturesLoaded.size(); j++)
			{
				if (texturesLoaded[j].path == pathToTexture)
				{
					std::cout << "Already Loaded : " << pathToTexture.C_Str() << " -- Reusuing old one" << std::endl;
					textures.push_back(texturesLoaded[j]);
					skip = true;
					break;
				}
			}

			if (!skip)
			{
				std::cout << "Loaded : " << pathToTexture.C_Str() << std::endl;
				Texture texture;
				texture.id = TextureFromFile(pathToTexture.C_Str(), this->path);
				texture.type = typeName;
				texture.path = pathToTexture;

				textures.push_back(texture);
				this->texturesLoaded.push_back(texture);
			}
		}

		return textures;
	}
};


GLint TextureFromFile(const char* fileName, std::string path)
{
	GLuint textureID;
	glGenTextures(1, &textureID);

	std::string file = std::string(fileName);
	std::string fullPath = path + fileName;

	int width, height, n;
	unsigned char* image = stbi_load(fullPath.c_str(), &width, &height, &n, STBI_rgb_alpha);

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(image);

	return textureID;
}

#endif