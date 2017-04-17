#pragma once

#pragma region INCLUDES

#ifndef LIGHT_FILE
#define LIGHT_FILE

#include <GL/glew.h>

#include "maths_funcs.h"
#include "Shader.h"
#include "BasicMesh.h"
#include "BasicShapeVertices.h"

#pragma endregion INCLUDES

#define LIGHT_FRAG_SHADER_LOC "../Game/Shaders/light.frag"
#define LIGHT_VERTEX_SHADER_LOC "../Game/Shaders/light.vertex"

class Light {
public:
	Shader lightShader;

	Light() {
	}

	void Draw(vec3 color, vec3 position, mat4 view, mat4 projection) {
		checkShader();

		mat4 lightPos = scale(identity_mat4(), vec3(0.2, 0.2, 0.2));
		lightPos = translate(lightPos, position);

		lightShader.Use();
		glUniform3fv(lightShader.getUniformLocation("FragColor"), 1, color.v);
		mesh.Draw(lightShader, lightPos, view, projection);
	}

private:
	BasicMesh mesh;

	void checkShader() {
		if (lightShader.Program == -1) {

			lightShader = Shader(LIGHT_VERTEX_SHADER_LOC, LIGHT_FRAG_SHADER_LOC);

			mesh = BasicMesh(Shape_Cube, sizeof(Shape_Cube), 36);
		}
	}
};

#endif