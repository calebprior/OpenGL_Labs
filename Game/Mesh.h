#pragma once

#pragma region INCLUDES 

#ifndef MESH
#define MESH

#include "maths_funcs.h"
#include <GL/glew.h>

#include <vector>
#include "Shader.h"
#include <assimp/cimport.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#pragma endregion INCLUDES

struct Vertex {
	vec3 Position;
	vec3 Normal;
	vec2 TexCoords;
	vec3 Tangent;
};

struct Texture {
	GLuint id;
	std::string type;
	aiString path;
};

class Mesh {
public:
	// Mesh Data
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;

	Mesh(std::vector<Vertex> vertices = {}, std::vector<GLuint> indices = {}, std::vector<Texture> textures = {})
	{
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;

		this->setupMesh();
	}

	void Draw(Shader meshShader, bool multiTexture)
	{
		for (GLuint i = 0; i < this->textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);

			if (this->textures[i].type == "texture_diffuse") {
				glUniform1i(glGetUniformLocation(meshShader.Program, "material.hasSpecMap"), false);
				glUniform1i(glGetUniformLocation(meshShader.Program, "material.hasNormMap"), false);
				glUniform1i(glGetUniformLocation(meshShader.Program, "material.texture"), i);
			}
			if (multiTexture) {
				if (this->textures[i].type == "texture_specular") {
					glUniform1i(glGetUniformLocation(meshShader.Program, "material.hasSpecMap"), true);
					glUniform1i(glGetUniformLocation(meshShader.Program, "material.specMap"), i);
				}
				else if (this->textures[i].type == "texture_normal") {
					glUniform1i(glGetUniformLocation(meshShader.Program, "material.hasNormMap"), true);
					glUniform1i(glGetUniformLocation(meshShader.Program, "material.normMap"), i);
				}
			}

			glBindTexture(GL_TEXTURE_2D, this->textures[i].id);
		}

		// Draw mesh
		glBindVertexArray(this->VAO);
		glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		for (GLuint i = 0; i < this->textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

private:
	GLuint VAO, VBO, EBO;

	void setupMesh()
	{
		// Create buffers/arrays
		glGenVertexArrays(1, &this->VAO);
		glGenBuffers(1, &this->VBO);
		glGenBuffers(1, &this->EBO);

		glBindVertexArray(this->VAO);

		// Load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);

		// Memory is stored sequentially in a struct 
		glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW);

		// Set the vertex attribute pointers
		// Vertex Positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);

		// Vertex Normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Normal));

		// Vertex Texture Coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, TexCoords));

		glBindVertexArray(0);
	}
};

#endif