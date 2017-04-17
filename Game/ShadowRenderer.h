#pragma once

#pragma region INCLUDES

#ifndef SHADOW_RENDERER
#define SHADOW_RENDERER

#include <GL/glew.h>

#include "maths_funcs.h"
#include "Shader.h"
#include <vector>

#pragma endregion INCLUDES

#define POINT_SHADOW_FRAG "../Game/Shaders/point_shadows_depth.frag"
#define POINT_SHADOW_VERTEX "../Game/Shaders/point_shadows_depth.vertex"
#define POINT_SHADOW_GEOM "../Game/Shaders/point_shadows_depth.geom"

const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

class ShadowRenderer {
public:
	GLfloat near_plane = 1.0f;
	GLfloat far_plane = 25.0f;

	ShadowRenderer(vec3 intialLightPos = vec3(0, 0, 0), void(*renderFunc)(Shader) = nullptr) {
			this->lightPos = intialLightPos;
			this->renderFunc = renderFunc;
	}

	void Setup() {
		checkShader();

		glGenFramebuffers(1, &depthMapFBO);

		glGenTextures(1, &depthCubemap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
		for (GLuint i = 0; i < 6; ++i)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		// Attach cubemap as depth map FBO's color buffer
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;

		glBindFramebuffer(GL_FRAMEBUFFER, 1);

		shadowRender();
	}

	void BindShadowToTexture(Shader shader, int k) {
		glActiveTexture(GL_TEXTURE3 + k);
		glBindTexture(GL_TEXTURE_CUBE_MAP, this->depthCubemap);
	}

	void UpdateUniforms(Shader shader) {
		glUniform1f(glGetUniformLocation(shader.Program, "far_plane"), far_plane);
	}

	void setRenderFunc(void(*renderFunc)(Shader)) {
		this->renderFunc = renderFunc;
	}

	void Update(vec3 newLightPosition) {
		this->lightPos = newLightPosition;

		shadowRender();
	}

private:
	GLuint depthMapFBO;
	GLuint depthCubemap;
	Shader simpleDepthShader;
	vec3 lightPos;
	void(*renderFunc)(Shader);

	void checkShader() {
		if (simpleDepthShader.Program == -1) {
			this->simpleDepthShader = Shader(POINT_SHADOW_VERTEX, POINT_SHADOW_FRAG, POINT_SHADOW_GEOM);
		}
	}

	void shadowRender() {
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glDepthFunc(GL_LESS);

		GLfloat aspect = (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT;
		mat4 shadowProj = perspective(90.0f, aspect, near_plane, far_plane);

		std::vector<mat4> shadowTransforms;
		shadowTransforms.push_back(shadowProj * look_at(lightPos, lightPos + vec3(1.0, 0.0, 0.0), vec3(0.0, -1.0, 0.0)));
		shadowTransforms.push_back(shadowProj * look_at(lightPos, lightPos + vec3(-1.0, 0.0, 0.0), vec3(0.0, -1.0, 0.0)));
		shadowTransforms.push_back(shadowProj * look_at(lightPos, lightPos + vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0)));
		shadowTransforms.push_back(shadowProj * look_at(lightPos, lightPos + vec3(0.0, -1.0, 0.0), vec3(0.0, 0.0, -1.0)));
		shadowTransforms.push_back(shadowProj * look_at(lightPos, lightPos + vec3(0.0, 0.0, 1.0), vec3(0.0, -1.0, 0.0)));
		shadowTransforms.push_back(shadowProj * look_at(lightPos, lightPos + vec3(0.0, 0.0, -1.0), vec3(0.0, -1.0, 0.0)));

		// Render scene to depth cubemap
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		simpleDepthShader.Use();

		for (GLuint i = 0; i < 6; ++i)
			glUniformMatrix4fv(glGetUniformLocation(simpleDepthShader.Program, "shadowTransforms") + i, 1, GL_FALSE, shadowTransforms[i].m);
		glUniform1f(glGetUniformLocation(simpleDepthShader.Program, "far_plane"), far_plane);
		glUniform3fv(glGetUniformLocation(simpleDepthShader.Program, "lightPos"), 1, lightPos.v);

		this->renderFunc(simpleDepthShader);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
	}
};

#endif