#pragma once

#pragma region INCLUDES


#ifndef POINTLIGHT
#define POINTLIGHT

#include "maths_funcs.h"
#include "Light.h"

#pragma endregion INCLUDES

class PointLight {
public:
	vec3 ambient = vec3(0.0f, 0.0f, 0.0f);
	vec3 diffuse = vec3(0.4f, 0.2f, 0.1f);
	vec3 specular = vec3(0.2f, 0.2f, 0.2f);

	float constant = 1.0f;
	float linear = 0.09f;
	float quadratic = 0.032f;

	bool drawCube = true;
	vec3 position;

	ShadowRenderer shadowRenderer;

	PointLight(vec3 position = vec3(0.0, 0.0, 0.0), void(*renderFunc)(Shader) = nullptr) {
		if (renderFunc != nullptr) {
			lightCube = Light();
			this->position = position;
			shadowRenderer = ShadowRenderer(position, renderFunc);
			shadowRenderer.Setup();
		}
	}

	void updatePosition(vec3 position) {
		this->position = position;
		shadowRenderer.Update(this->position);
	}

	void setDrawCube(bool drawCube) {
		this->drawCube = drawCube;
	}

	void setDiffuse(vec3 color) {
		this->diffuse = color;
	}

	void UpdateUniforms(Shader shader, int id) {
		glUniform3fv(shader.getUniformLocation(("pointlights[" + std::to_string(id) + "].ambient").c_str()), 1, ambient.v);
		glUniform3fv(shader.getUniformLocation(("pointlights[" + std::to_string(id) + "].diffuse").c_str()), 1, diffuse.v);
		glUniform3fv(shader.getUniformLocation(("pointlights[" + std::to_string(id) + "].specular").c_str()), 1, specular.v);

		glUniform1f(shader.getUniformLocation(("pointlights[" + std::to_string(id) + "].constant").c_str()), constant);
		glUniform1f(shader.getUniformLocation(("pointlights[" + std::to_string(id) + "].linear").c_str()), linear);
		glUniform1f(shader.getUniformLocation(("pointlights[" + std::to_string(id) + "].quadratic").c_str()), quadratic);

		glUniform3fv(shader.getUniformLocation(("pointlights[" + std::to_string(id) + "].position").c_str()), 1, position.v);

		shadowRenderer.UpdateUniforms(shader);
	}

	void BindShadow(Shader shader, int k) {
		shadowRenderer.BindShadowToTexture(shader, k);
	}

	void Draw(Shader shader, mat4 view, mat4 projection) {
		if (drawCube) {
			lightCube.Draw(diffuse, position, view, projection);
		}
	}

private:
	Light lightCube;
};

#endif