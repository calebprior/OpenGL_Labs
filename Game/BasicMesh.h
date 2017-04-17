#pragma once

#ifndef BASIC_MESH
#define BASIC_MESH

#include "maths_funcs.h"
#include <GL/glew.h>

#include <vector>
#include "Shader.h"
#include <assimp/cimport.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


class BasicMesh {
public:
	GLuint VAO;

	BasicMesh() {
		// Empty Default Constructor
	}

	BasicMesh(GLfloat vertices[], GLint size, GLint numVertices) {
		this->vertices = new GLfloat[size];

		for (int i = 0; i < size; i++) {
			this->vertices[i] = vertices[i];
		}
		this->size = size;
		this->numVertices = numVertices;

		setupMesh();
	}

	void Draw(Shader meshShader, mat4 model, mat4 view, mat4 proj) {
		GLint modelLoc = glGetUniformLocation(meshShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(meshShader.Program, "view");
		GLint projLoc = glGetUniformLocation(meshShader.Program, "projection");

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model.m);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view.m);
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, proj.m);

		glBindVertexArray(this->VAO);
		glDrawArrays(GL_TRIANGLES, 0, this->numVertices);

		glBindVertexArray(0);
	}

private:
	GLfloat* vertices;
	GLint size;
	GLint numVertices;
	GLuint VBO;

	void setupMesh() {
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		glGenBuffers(1, &VBO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

		glBindVertexArray(0);
	}
};

#endif