#pragma once

#pragma region INCLUDES

#ifndef SHADER
#define SHADER

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <GL/glew.h>;

#pragma endregion INCLUDES

class Shader
{
public:
	GLuint Program = -1;

	Shader(const GLchar* vertexPath = nullptr, const GLchar* fragmentPath = nullptr, const GLchar* geometryPath = nullptr)
	{
		if (vertexPath != nullptr || fragmentPath != nullptr || geometryPath != nullptr) {
			this->Program = glCreateProgram();

			GLuint vertexShader;
			GLuint fragmentShader;
			GLuint geometryShader;

			if (vertexPath != nullptr) {
				const GLchar* vertexShaderText = readShaderSource(vertexPath);
				vertexShader = compileShader(vertexShaderText, GL_VERTEX_SHADER);
				glAttachShader(this->Program, vertexShader);
			}

			if (fragmentPath != nullptr) {
				const GLchar* fragmentShaderText = readShaderSource(fragmentPath);
				fragmentShader = compileShader(fragmentShaderText, GL_FRAGMENT_SHADER);
				glAttachShader(this->Program, fragmentShader);
			}

			if (geometryPath != nullptr) {
				const GLchar* geometryShaderText = readShaderSource(geometryPath);
				geometryShader = compileShader(geometryShaderText, GL_GEOMETRY_SHADER);
				glAttachShader(this->Program, geometryShader);
			}

			glLinkProgram(this->Program);

			// Check for errors
			GLint success;
			GLchar ErrorLog[1024] = { 0 };
			glGetProgramiv(this->Program, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(this->Program, sizeof(ErrorLog), NULL, ErrorLog);
				fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
			}

			if (vertexPath != nullptr) {
				glDeleteShader(vertexShader);
			}

			if (fragmentPath != nullptr) {
				glDeleteShader(fragmentShader);
			}

			if (geometryPath != nullptr) {
				glDeleteShader(geometryShader);
			}
		}
	}

	void Use()
	{
		glUseProgram(this->Program);
	}

	GLint getUniformLocation(const char* name)
	{
		return glGetUniformLocation(this->Program, name);
	}

	GLint getAttributLocation(const char* name)
	{
		return glGetAttribLocation(this->Program, name);
	}

private:
	static GLchar* readShaderSource(const char* filename)
	{
		std::cout << "Reading in Shader : " << filename << std::endl;

		FILE* fp;
		fopen_s(&fp, filename, "rt");

		fseek(fp, 0, SEEK_END);
		long file_length = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		char* contents = new char[file_length + 1];

		// zero out memory
		for (int i = 0; i < file_length + 1; i++) {
			contents[i] = 0;
		}

		fread(contents, 1, file_length, fp);
		contents[file_length + 1] = '\0';
		fclose(fp);

		return contents;
	}

	static GLuint compileShader(const char* shaderText, GLenum shaderType)
	{
		// create a shader object
		GLuint shaderObj = glCreateShader(shaderType);

		if (shaderObj == 0) {
			fprintf(stderr, "Error creating shader type %d\n", shaderType);
		}

		// Bind the source code to the shader, this happens before compilation
		glShaderSource(shaderObj, 1, (const GLchar**)&shaderText, NULL);
		// compile the shader and check for errors
		glCompileShader(shaderObj);

		// check for shader related errors using glGetShaderiv
		GLint success;
		glGetShaderiv(shaderObj, GL_COMPILE_STATUS, &success);
		if (!success) {
			GLchar InfoLog[1024];
			glGetShaderInfoLog(shaderObj, 1024, NULL, InfoLog);
			fprintf(stderr, "Error compiling shader type %d: '%s'\n", shaderType, InfoLog);
		}

		return shaderObj;
	}
};

#endif