#pragma once

#pragma region INCLUDES

#ifndef SKYBOX
#define SKYBOX

#include "maths_funcs.h"
#include <GL/glew.h>
#include <vector> 
#include <iostream>
#include "Shader.h"
#include "BasicMesh.h"
#include "Camera.h"
#include "BasicShapeVertices.h"

#if !defined(STB_IMAGE_IMPLEMENTATION)
#include "stb_image.h"
#endif

#pragma endregion INCLUDES

#define SKYBOX_FRAG_SHADER_LOC "../Game/Shaders/skybox.frag"
#define SKYBOX_VERTEX_SHADER_LOC "../Game/Shaders/skybox.vertex"

class Skybox
{
public:
	Skybox(const char* imagePath = "")
	{
		this->path = imagePath;

		if (imagePath != "") {
			this->setup();
		}
	}

	void Draw(mat4 cameraView, mat4 projection) {
		glDepthFunc(GL_LEQUAL);

		skyboxShader.Use();
		mat4 view = removeTranslation(cameraView);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		skybox.Draw(skyboxShader, identity_mat4(), view, projection);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		glDepthFunc(GL_LESS);
	}

private:
	std::string path;
	const GLchar* vertexPath;
	const GLchar* fragmentPath;
	Shader skyboxShader;
	BasicMesh skybox;
	GLuint cubemapTexture;

	void setup()
	{
		skyboxShader = Shader(SKYBOX_VERTEX_SHADER_LOC, SKYBOX_FRAG_SHADER_LOC);
		skybox = BasicMesh(Shape_Skybox, sizeof(Shape_Skybox), 36);

		std::vector<std::string> faces;
		faces.push_back(path + "right.jpg");
		faces.push_back(path + "left.jpg");
		faces.push_back(path + "top.jpg");
		faces.push_back(path + "bottom.jpg");
		faces.push_back(path + "back.jpg");
		faces.push_back(path + "front.jpg");
		cubemapTexture = loadCubemap(faces);
	}

	GLuint loadCubemap(std::vector<std::string> faces)
	{
		GLuint textureID;
		glGenTextures(1, &textureID);

		int width, height;
		unsigned char* image;

		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
		for (GLuint i = 0; i < faces.size(); i++)
		{
			image = stbi_load(faces[i].c_str(), &width, &height, 0, STBI_rgb);

			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
				GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image
				);

			stbi_image_free(image);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		return textureID;
	}

	mat4 removeTranslation(mat4 cameraView)
	{
		mat4 returnMat = identity_mat4();
		returnMat.m[0] = cameraView.m[0];
		returnMat.m[1] = cameraView.m[1];
		returnMat.m[2] = cameraView.m[2];
		returnMat.m[4] = cameraView.m[4];
		returnMat.m[5] = cameraView.m[5];
		returnMat.m[6] = cameraView.m[6];
		returnMat.m[8] = cameraView.m[8];
		returnMat.m[9] = cameraView.m[9];
		returnMat.m[10] = cameraView.m[10];

		return returnMat;
	}
};

#endif