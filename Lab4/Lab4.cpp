
//Some Windows Headers (For Time, IO, etc.)
#include <windows.h>
#include <mmsystem.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include "maths_funcs.h"
#include "teapot.h" // teapot mesh

// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using namespace std;
GLuint shaderProgramID;

unsigned int teapot_vao = 0;
int width = 800;
int height = 600;

GLuint loc1;
GLuint loc2;
GLfloat rotatez = 0.0f;


// Shader Functions- click on + to expand
#pragma region SHADER_FUNCTIONS

// Create a NULL-terminated string by reading the provided file
char* readFile(const char* filename) {
	// Open the file
	FILE* fp;
	fopen_s(&fp, filename, "rt");
	// Move the file pointer to the end of the file and determing the length
	fseek(fp, 0, SEEK_END);
	long file_length = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* contents = new char[file_length + 1];
	// zero out memory
	for (int i = 0; i < file_length + 1; i++) {
		contents[i] = 0;
	}
	// Here's the actual read
	fread(contents, 1, file_length, fp);
	// This is how you denote the end of a string in C
	contents[file_length + 1] = '\0';
	fclose(fp);
	return contents;
}


static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
		exit(0);
	}
	const char* pShaderSource = readFile(pShaderText);

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		exit(1);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders()
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(shaderProgramID, "../Lab4/Shaders/simpleVertexShader.txt", GL_VERTEX_SHADER);
	AddShader(shaderProgramID, "../Lab4/Shaders/simpleFragmentShader.txt", GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };
	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	glUseProgram(shaderProgramID);
	return shaderProgramID;
}
#pragma endregion SHADER_FUNCTIONS

// VBO Functions - click on + to expand
#pragma region VBO_FUNCTIONS

void generateObjectBufferTeapot() {
	GLuint vp_vbo = 0;

	loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc2 = glGetAttribLocation(shaderProgramID, "vertex_normals");

	glGenBuffers(1, &vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glBufferData(GL_ARRAY_BUFFER, 3 * teapot_vertex_count * sizeof(float), teapot_vertex_points, GL_STATIC_DRAW);
	GLuint vn_vbo = 0;
	glGenBuffers(1, &vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glBufferData(GL_ARRAY_BUFFER, 3 * teapot_vertex_count * sizeof(float), teapot_normals, GL_STATIC_DRAW);

	glGenVertexArrays(1, &teapot_vao);
	glBindVertexArray(teapot_vao);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
}


#pragma endregion VBO_FUNCTIONS


mat4 rootObject = translate(identity_mat4(), vec3(0.0, 0.0, -40.0));

void display() {

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramID);

	//Declare your uniform variables that will be used in your shader
	int matrix_location = glGetUniformLocation(shaderProgramID, "model");
	int view_mat_location = glGetUniformLocation(shaderProgramID, "view");
	int proj_mat_location = glGetUniformLocation(shaderProgramID, "proj");

	// Hierarchy of Teapots

	// Root of the Hierarchy
	mat4 view = translate(identity_mat4(), vec3(0.0f, 0.0f, -20.0f));
	mat4 persp_proj = perspective(45.0, (float)width / (float)height, 0.1, 300.0);

	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, rootObject.m);
	glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);

	// child of hierarchy
	mat4 local2 = identity_mat4();
	local2 = scale(local2, vec3(.4f, .4f, .4f));
	local2 = rotate_y_deg(local2, rotatez);
	local2 = translate(local2, vec3(-0.8, 11.0, 0.0));
	mat4 centerTeapot = rootObject*local2;

	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, centerTeapot.m);
	glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);

	// Forwards
	mat4 global = centerTeapot;
	for (int i = 0; i < 3; i++) {
		mat4 local = identity_mat4();
		local = translate(local, vec3(23.0, 0.0, 0.0));
		global = global * local;

		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global.m);
		glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);
	}

	// Backwards
	global = centerTeapot;
	for (int i = 0; i < 3; i++) {
		mat4 local = identity_mat4();
		local = translate(local, vec3(-23.0, 0.0, 0.0));
		global = global * local;

		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global.m);
		glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);
	}

	//Middle 90 degrees turned
	global = centerTeapot;

	mat4 local3 = identity_mat4();
	local3 = rotate_y_deg(local3, 90);
	mat4 centerTeapot2 = global*local3;

	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, centerTeapot2.m);
	glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);


	//Side 1
	global = centerTeapot2;
	for (int i = 0; i < 3; i++) {
		mat4 local = identity_mat4();
		local = translate(local, vec3(-23.0, 0.0, 0.0));
		global = global * local;

		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global.m);
		glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);
	}

	//Side 2
	global = centerTeapot2;
	for (int i = 0; i < 3; i++) {
		mat4 local = identity_mat4();
		local = translate(local, vec3(23.0, 0.0, 0.0));
		global = global * local;

		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global.m);
		glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);
	}

	glutSwapBuffers();
}



void idleFunction_UpdateScene() {

	// Placeholder code, if you want to work with framerate
	// Wait until at least 16ms passed since start of last frame (Effectively caps framerate at ~60fps)
	static DWORD  last_time = 0;
	DWORD  curr_time = timeGetTime();
	float  delta = (curr_time - last_time) * 0.001f;
	if (delta > 0.03f)
		delta = 0.03f;
	last_time = curr_time;

	rotatez += 0.5f;
	// Draw the next frame
	glutPostRedisplay();
}


void init()
{
	// Set up the shaders
	GLuint shaderProgramID = CompileShaders();
	// load teapot mesh into a vertex buffer array
	generateObjectBufferTeapot();

}

void onKeypress(unsigned char key, int x, int y) {
	mat4 pointlight_Diffuse_R = identity_mat4();

	switch (key) {
	case 'r':
		rootObject = translate(identity_mat4(), vec3(0.0, 0.0, -40.0));
	case 'w':
		rootObject = translate(rootObject, vec3(0.0, 1.0, 0.0));
		break;
	case 's':
		rootObject = translate(rootObject, vec3(0.0, -1.0, 0.0));
		break;
	case 'a':
		rootObject = translate(rootObject, vec3(-1.0, 0.0, 0.0));
		break;
	case 'd':
		rootObject = translate(rootObject, vec3(1.0, 0.0, 0.0));
		break;
	case 'q':
		rootObject = translate(rootObject, vec3(0.0, 0.0, 1.0));
		break;
	case 'e':
		rootObject = translate(rootObject, vec3(0.0, 0.0, -1.0));
		break;
	case 'z':
		pointlight_Diffuse_R = rotate_z_deg(pointlight_Diffuse_R, 1);
		rootObject = rootObject * pointlight_Diffuse_R;
		break;
	case 'Z':
		pointlight_Diffuse_R = rotate_z_deg(pointlight_Diffuse_R, -1);
		rootObject = rotate_z_deg(rootObject, -1);
		break;
	case 'x':
		pointlight_Diffuse_R = rotate_x_deg(pointlight_Diffuse_R, 1);
		rootObject = rootObject * pointlight_Diffuse_R;
		break;
	case 'X':
		pointlight_Diffuse_R = rotate_x_deg(pointlight_Diffuse_R, -1);
		rootObject = rootObject * pointlight_Diffuse_R;
		break;
	case 'y':
		pointlight_Diffuse_R = rotate_y_deg(pointlight_Diffuse_R, 1);
		rootObject = rootObject * pointlight_Diffuse_R;
		break;
	case 'Y':
		pointlight_Diffuse_R = rotate_y_deg(pointlight_Diffuse_R, -1);
		rootObject = rootObject * pointlight_Diffuse_R;
		break;
	}
}

int main(int argc, char** argv) {

	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("Hello Triangle");

	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutIdleFunc(idleFunction_UpdateScene);
	glutKeyboardFunc(onKeypress);

	// A call to glewInit() must be done after glut is initialized!
	GLenum res = glewInit();
	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}
	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();
	return 0;
}