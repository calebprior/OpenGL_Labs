
#pragma region INCLUDES

#include <windows.h>
#include <mmsystem.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <conio.h>
#include <stdio.h>
#include <math.h>
#include <vector>

#include "maths_funcs.h"
#include "text.h"
#include "Shader.h"
#include "Mesh.h"
#include "Model.h"
#include "Camera.h"
#include "BasicMesh.h"
#include "BasicShapeVertices.h"
#include "Light.h"
#include "Skybox.h"
#include "Map.h"
#include "ShadowRenderer.h"
#include "PointLight.h"

#pragma endregion INCLUDES

#pragma region FILE LOCATIONS

#define VERTEX_LOC "../Game/Shaders/Mesh.vertex"
#define FRAGMENT_LOC "../Game/Shaders/Mesh.frag" 

#define PATH_TO_TEXTURES "../Game/Meshs/"
#define PATH_TO_SKYBOX "../Game/Skybox/"

#define MESH_WELL_TEX "../Game/Meshs/Well/"
#define MESH_WELL "../Game/Meshs/Well/well.obj"
#define MESH_PEDESTAL_TEX "../Game/Meshs/Pedestal/"
#define MESH_PEDESTAL "../Game/Meshs/Pedestal/pedestal2.obj"

#define GROUND_LOC "../Game/Meshs/groundSingle.obj"
#define GROUND2_LOC "../Game/Meshs/groundSingle2.obj"
#define BUSH_LOC "../Game/Meshs/hedgeSingle.obj"
#define DOOR_LOC "../Game/Meshs/door.obj"
#define TOMBSTONE_RIP_LOC "../Game/Meshs/tombstone.obj"
#define TOMBSTONE2_LOC "../Game/Meshs/tombstone2.obj"
#define FLAG_LOC "../Game/Meshs/flag.obj"
#define POLE_LOC "../Game/Meshs/pole.obj"

#pragma endregion FILE LOCATIONS

#pragma region FUNCTION DECLS

void windowReshapeFunc(GLint newWidth, GLint newHeight);
void onKeypress(unsigned char key, int x, int y);
void onKeyDepress(unsigned char key, int x, int y);
void mouse(int x, int y);
void mouseWheel(int button, int dir, int x, int y);
void processInput();
void idleFunction_UpdateScene();
void render(Shader meshShader);
void display();
void init();
void addText();
void setupGame();
void draw(Shader meshShader);

#pragma endregion FUNCTION DECLS

#pragma region GLOBALS

// Window Constants
int windowWidth = 800;
int windowHeight = 800;

// Shader
Shader meshShader;

// Models
Model pedestal;
Model well;
Model ground;
Model bush;
Model door;
Model tombstoneRip;
Model tombstone2;
Model flag;
Model pole;
Model ground2;

//Skybox
Skybox skybox;

// Camera Variables and Controls
Camera camera(vec3(1.0f, 1.5f, 0.0f));
bool keys[1024];
GLfloat lastX = 400;
GLfloat lastY = 300;
GLfloat deltaTime = 0.0f;
bool firstMouse = true;
bool trapMouse = true;

// FlashLight Properties
bool flashLight = false;
vec3 spotlight_Ambient = vec3(0.1f, 0.1f, 0.1f);
vec3 spotlight_Diffuse = vec3(0.7f, 0.7f, 0.7f);
vec3 spotlight_Specular = vec3(0.1f, 0.1f, 0.1f);
float spotlight_Constant = 1.0f;
float spotlight_Linear = 0.09f;
float spotlight_Quadratic = 0.032f;

// Program Variables
vec4 clearColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
bool enableFog = true;
bool enableShadows = true;
bool multiTexture = true;
bool showLightBoxes = false;

// Game Variables
Map map;
int timeTextId;
int maxTimeRemaining = 50;
bool canMove = true;
bool gameOver = false;
bool gameWon = false;
float doorRotation = 0.0;
float flagRotation = 0.0;
bool flagDirSwitch = true;
bool hasFlag = false;
std::vector<PointLight> lights;

// Text
const char* atlas_image = "../Game/Fonts/freemono.png";
const char* atlas_meta = "../Game/Fonts/freemono.meta";

char* mapPath = "../Game/Maps/map1 (2).png";

#pragma endregion GLOBALS

#pragma region MAIN FUNCTIONS

int main(int argc, char** argv) {
	// Setup GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_MULTISAMPLE); // Depth and Multisampling new
	glutInitWindowSize(windowWidth, windowHeight);
	glutSetOption(GLUT_MULTISAMPLE, 8);
	glutCreateWindow("Testing");

	// Setup Callbacks
	glutDisplayFunc(display);
	glutIdleFunc(idleFunction_UpdateScene);
	glutKeyboardFunc(onKeypress);
	glutKeyboardUpFunc(onKeyDepress);
	glutMouseWheelFunc(mouseWheel);
	glutPassiveMotionFunc(mouse);
	glutSetCursor(GLUT_CURSOR_NONE);
	glutReshapeFunc(windowReshapeFunc);

	// Check for any errors
	GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}

	map.printMap();

	// initialise font, load from files
	if (!init_text_rendering(atlas_image, atlas_meta, windowWidth, windowHeight)) {
		fprintf(stderr, "ERROR init text rendering\n");
	}

	// Setup
	init();

	// Begin infinite event loop
	glutMainLoop();

	return 0;
}

int count = 0;
bool started = false;
float startTime = 0.0f;

void idleFunction_UpdateScene() {
	static double last_time = 0;
	double curr_time = timeGetTime();
	float delta = (curr_time - last_time) * 0.001f;
	if (delta > 0.03f)
		delta = 0.03f;
	last_time = curr_time;

	if (gameWon) {
		update_text(timeTextId, "");
	}
	else if (!gameOver) {
		double t = (double)glutGet(GLUT_ELAPSED_TIME);

		if (!started) {
			startTime = t / 1000;
			started = true;
		}

		t = t / 1000;
		float deltaTime = t - startTime;

		int currTimeLeft = maxTimeRemaining - deltaTime;

		update_text(timeTextId, ("Time Remaining: " + std::to_string(currTimeLeft) + " seconds").c_str());

		if (currTimeLeft <= 0) {
			gameOver = true;
			gameWon = false;
		}
	}

	// Update deltaTime and process input
	deltaTime = delta;
	processInput();

	// Make light flicker
	vec3 diffuse = lights[0].diffuse;

	if ((int)curr_time % 6 == 0) {
		float pointlight_Diffuse_R = (float)rand() / (float)RAND_MAX;

		if (pointlight_Diffuse_R < 0.4f) {
			pointlight_Diffuse_R = 0.4f;
		}

		if (pointlight_Diffuse_R > 0.8f) {
			pointlight_Diffuse_R = 0.8f;
		}

		diffuse.v[0] = pointlight_Diffuse_R;
	}
	else {
		diffuse.v[0] = 0.4;
	}

	for (int i = 0; i < lights.size(); i++) {
		lights[i].setDiffuse(diffuse);
	}

	glutPostRedisplay();
}

void init()
{
	// Setup Shaders and Meshes
	meshShader = Shader(VERTEX_LOC, FRAGMENT_LOC);

	skybox = Skybox(PATH_TO_SKYBOX);

	well = Model(MESH_WELL, MESH_WELL_TEX);
	pedestal = Model(MESH_PEDESTAL, MESH_PEDESTAL_TEX);
	ground = Model(GROUND_LOC, PATH_TO_TEXTURES);
	ground2 = Model(GROUND2_LOC, PATH_TO_TEXTURES);
	bush = Model(BUSH_LOC, PATH_TO_TEXTURES);
	door = Model(DOOR_LOC, PATH_TO_TEXTURES);
	tombstoneRip = Model(TOMBSTONE_RIP_LOC, PATH_TO_TEXTURES);
	tombstone2 = Model(TOMBSTONE2_LOC, PATH_TO_TEXTURES);
	flag = Model(FLAG_LOC, PATH_TO_TEXTURES);
	pole = Model(POLE_LOC, PATH_TO_TEXTURES);

	map = Map(mapPath);

	//Setup Lights
	lights.resize(map.lightModelPositions.size());
	for (int i = 0; i < map.lightModelPositions.size(); i++)
	{
		lights[i] = (PointLight(map.lightModelPositions[i], render));
	}

	setupGame();
}

void setupGame() {
	meshShader.Use();

	camera.setNewPosition(map.playerStartPos);

	addText();
}

void addText() {
	timeTextId = add_text(("Time Remaining: " + std::to_string(maxTimeRemaining) + " seconds").c_str(),
		-0.25f, 0.85f, 35.0f, 1.0f, 1.0f, 1.0f, 1.0f);
}

void display() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LESS);
	glClearColor(clearColor.v[0], clearColor.v[1], clearColor.v[2], clearColor.v[3]);

	glViewport(0, 0, windowWidth, windowHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (!gameOver) {
		meshShader.Use();
		mat4 projection = perspective(camera.Zoom, (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);
		mat4 view = camera.GetViewMatrix();
		glUniformMatrix4fv(glGetUniformLocation(meshShader.Program, "projection"), 1, GL_FALSE, projection.m);
		glUniformMatrix4fv(glGetUniformLocation(meshShader.Program, "view"), 1, GL_FALSE, view.m);

		// Set light uniforms
		glUniform3fv(glGetUniformLocation(meshShader.Program, "viewPos"), 1, camera.Position.v);
		glUniform1i(glGetUniformLocation(meshShader.Program, "shadows"), enableShadows);

		for (int i = 0; i < lights.size(); i++)
		{
			glUniform1i(glGetUniformLocation(meshShader.Program, ("depthMaps[" + std::to_string(i) + "]").c_str()), i + 3);
		}

		render(meshShader);


		glDisable(GL_CULL_FACE);

		if (showLightBoxes) {
			for (int i = 0; i < lights.size(); i++)
			{
				lights[i].Draw(meshShader, view, projection);
			}
		}

		meshShader.Use();
		skybox.Draw(camera.GetViewMatrix(), projection);
	}
	else {
		if (!gameWon) {
			change_text_colour(timeTextId, 1.0, 0.0, 0.0, 1.0);
			update_text(timeTextId, "GAME OVER \nClick SPACE to retry");
		}
		else {
			change_text_colour(timeTextId, 1.0, 1.0, 1.0, 1.0);
			update_text(timeTextId, "YOU MADE IT! \nClick SPACE start again");
		}
	}

	draw_texts();

	glutSwapBuffers();
}

void render(Shader meshShader) {
	meshShader.Use();

	glUniform1i(meshShader.getUniformLocation("enableFog"), enableFog);

	// Spotlight uniforms
	glUniform1i(meshShader.getUniformLocation("spotLightOn"), flashLight);
	if (flashLight) {
		glUniform3fv(meshShader.getUniformLocation("spotlight.position"), 1, camera.Position.v);
		glUniform3fv(meshShader.getUniformLocation("spotlight.direction"), 1, camera.Front.v);
		glUniform3fv(meshShader.getUniformLocation("spotlight.ambient"), 1, spotlight_Ambient.v);
		glUniform3fv(meshShader.getUniformLocation("spotlight.diffuse"), 1, spotlight_Diffuse.v);
		glUniform3fv(meshShader.getUniformLocation("spotlight.specular"), 1, spotlight_Specular.v);
		glUniform1f(meshShader.getUniformLocation("spotlight.cutOff"), cos(degree_to_rad(12.5f)));
		glUniform1f(meshShader.getUniformLocation("spotlight.outerCutOff"), cos(degree_to_rad(17.5f)));
		glUniform1f(meshShader.getUniformLocation("spotlight.constant"), spotlight_Constant);
		glUniform1f(meshShader.getUniformLocation("spotlight.linear"), spotlight_Linear);
		glUniform1f(meshShader.getUniformLocation("spotlight.quadratic"), spotlight_Quadratic);
	}

	// Mesh Properties
	glUniform3fv(meshShader.getUniformLocation("viewPos"), 1, camera.Position.v);
	glUniform1f(meshShader.getUniformLocation("material.shininess"), 0.5f);

	draw(meshShader);
}

void draw(Shader meshShader) {
	glUniform1i(meshShader.getUniformLocation("numLights"), lights.size());
	for (int k = 0; k < lights.size(); k++) {
		lights[k].BindShadow(meshShader, k);
		lights[k].UpdateUniforms(meshShader, k);
	}

	for (int i = map.height - 1; i >= 0; i--) {
		for (int j = 0; j < map.width; j++)
		{
			if (map.map[i][j] == Types::Bush) {
				mat4 pos = identity_mat4();
				pos = translate(pos, vec3(j, 0, -(((map.height - 1) - i))));
				glUniformMatrix4fv(meshShader.getUniformLocation("model"), 1, GL_FALSE, pos.m);
				bush.Draw(meshShader, multiTexture);
			}

			if (map.map[i][j] == Types::Ground || map.map[i][j] == Types::Light ||
				map.map[i][j] == Types::Well || map.map[i][j] == Types::End ||
				map.map[i][j] == Types::Flag) {

				mat4 pos = identity_mat4();
				pos = translate(pos, vec3(j, 0, -(((map.width - 1) - i))));
				glUniformMatrix4fv(meshShader.getUniformLocation("model"), 1, GL_FALSE, pos.m);
				ground.Draw(meshShader, multiTexture);
			}

			if (map.map[i][j] == Types::Pedestal || map.map[i][j] == Types::TombStone1 ||
				map.map[i][j] == Types::TombStone2) {
				mat4 pos = identity_mat4();
				pos = translate(pos, vec3(j, 0, -(((map.width - 1) - i))));
				glUniformMatrix4fv(meshShader.getUniformLocation("model"), 1, GL_FALSE, pos.m);
				ground.Draw(meshShader, multiTexture);
			}

			if (map.map[i][j] == Types::End) {
				if (!map.atEnd) {
					mat4 pos = identity_mat4();
					pos = translate(pos, vec3(j, 0, -(((map.height - 1) - i + 1))));
					glUniformMatrix4fv(meshShader.getUniformLocation("model"), 1, GL_FALSE, pos.m);
					door.Draw(meshShader, multiTexture);
				}
				else {
					canMove = false;
					gameWon = true;
					mat4 pos = identity_mat4();
					pos = rotate_y_deg(pos, doorRotation);
					pos = translate(pos, vec3(j, 0, -(((map.height - 1) - i + 1))));
					glUniformMatrix4fv(meshShader.getUniformLocation("model"), 1, GL_FALSE, pos.m);
					door.Draw(meshShader, multiTexture);

					doorRotation += deltaTime * 20;
					if (doorRotation > 70) {
						doorRotation = 70;
						gameOver = true;
					}
				}
			}

			if (map.map[i][j] == Types::Well) {
				mat4 pos = identity_mat4();
				pos = translate(pos, vec3(j, 0, -(((map.height - 1) - i))));
				glUniformMatrix4fv(meshShader.getUniformLocation("model"), 1, GL_FALSE, pos.m);
				well.Draw(meshShader, multiTexture);
			}

			if (map.map[i][j] == Types::TombStone1) {
				mat4 pos = identity_mat4();
				pos = translate(pos, vec3(j, 0, -(((map.height - 1) - i))));
				glUniformMatrix4fv(meshShader.getUniformLocation("model"), 1, GL_FALSE, pos.m);
				tombstoneRip.Draw(meshShader, multiTexture);
			}

			if (map.map[i][j] == Types::TombStone2) {
				mat4 pos = identity_mat4();
				pos = translate(pos, vec3(j, 0, -(((map.height - 1) - i))));
				glUniformMatrix4fv(meshShader.getUniformLocation("model"), 1, GL_FALSE, pos.m);
				tombstone2.Draw(meshShader, multiTexture);
			}

			if (map.map[i][j] == Types::Pedestal) {
				mat4 pos = identity_mat4();
				pos = translate(pos, vec3(j + 0.5, 0, -(((map.height - 1) - i) + 0.5)));
				glUniformMatrix4fv(meshShader.getUniformLocation("model"), 1, GL_FALSE, pos.m);
				pedestal.Draw(meshShader, multiTexture);
			}

			if (map.map[i][j] == Types::Flag) {
				hasFlag = true;

				mat4 pos = identity_mat4();
				pos = translate(pos, vec3(j + 0.5, 0, -(((map.height - 1) - i) + 0.5)));
				glUniformMatrix4fv(meshShader.getUniformLocation("model"), 1, GL_FALSE, pos.m);
				pole.Draw(meshShader, multiTexture);

				mat4 pos2 = identity_mat4();
				pos2 = rotate_y_deg(pos2, flagRotation);
				pos2 = pos * pos2;

				glDisable(GL_CULL_FACE);
				glUniformMatrix4fv(meshShader.getUniformLocation("model"), 1, GL_FALSE, pos2.m);
				flag.Draw(meshShader, multiTexture);
				glEnable(GL_CULL_FACE);
			}
		}
	}

	if (hasFlag) {
		if (flagDirSwitch) {
			flagRotation += deltaTime * 6;
		}
		else {
			flagRotation -= deltaTime * 6;
		}

		if (flagRotation > 15 && flagDirSwitch) {
			flagDirSwitch = false;
		}
		else if (flagRotation < -15 && !flagDirSwitch) {
			flagDirSwitch = true;
		}
	}

	for (int k = 0; k < lights.size(); k++) {
		glActiveTexture(GL_TEXTURE0 + k);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
}

#pragma endregion MAIN FUNCTIONS

#pragma region Input Functions

void move(vec3 newPos) {
	newPos.v[1] = 1.5f;
	if (map.canMove(vec2(newPos.v[0], newPos.v[2])))
		camera.setNewPosition(newPos);
}

void processInput()
{
	vec3 newPos;

	// Camera controls
	if (keys['w'] && canMove) {
		newPos = camera.ProcessKeyboard(FORWARD, deltaTime);
		move(newPos);
	}
	if (keys['s'] && canMove) {
		newPos = camera.ProcessKeyboard(BACKWARD, deltaTime);
		move(newPos);
	}
	if (keys['a'] && canMove) {
		newPos = camera.ProcessKeyboard(LEFT, deltaTime);
		move(newPos);
	}
	if (keys['d'] && canMove) {
		newPos = camera.ProcessKeyboard(RIGHT, deltaTime);
		move(newPos);
	}

	// Let Mouse Escape
	if (keys['t'])
		trapMouse = false;
	if (keys['T'])
		trapMouse = true;

	// FlashLight controls
	if (keys['f'])
		flashLight = true;
	if (keys['g'])
		flashLight = false;

	if (keys['i']) {
		lights[0].position.v[2] += 0.2;
		lights[0].updatePosition(lights[0].position);
	}
	if (keys['k']) {
		lights[0].position.v[2] -= 0.2;
		lights[0].updatePosition(lights[0].position);
	}
	if (keys['j']) {
		lights[0].position.v[0] -= 0.2;
		lights[0].updatePosition(lights[0].position);
	}
	if (keys['l']) {
		lights[0].position.v[0] += 0.2;
		lights[0].updatePosition(lights[0].position);
	}

	if (keys['u']) {
		lights[0].position.v[1] -= 0.2;
		lights[0].updatePosition(lights[0].position);
	}
	if (keys['o']) {
		lights[0].position.v[1] += 0.2;
		lights[0].updatePosition(lights[0].position);
	}


	// Testing Multisampling 
	if (keys[',']) {
		glEnable(GL_MULTISAMPLE);
		std::cout << "GL Multisample: ON" << std::endl;
	}
	if (keys['<']) {
		glDisable(GL_MULTISAMPLE);
		std::cout << "GL Multisample: OFF" << std::endl;
	}

	if (keys['m']) {
		multiTexture = true;
		std::cout << "MultiTextures: ON" << std::endl;
	}
	if (keys['M']) {
		multiTexture = false;
		std::cout << "MultiTextures: OFF" << std::endl;
	}

	if (keys['n']) {
		enableFog = true;
		std::cout << "Fog: ON" << std::endl;
	}
	if (keys['N']) {
		enableFog = false;
		std::cout << "Fog: OFF" << std::endl;
	}
	if (keys['b']) {
		enableShadows = true;
		std::cout << "Shadow Mapping: ON" << std::endl;
	}
	if (keys['B']) {
		enableShadows = false;
		std::cout << "Shadow Mapping: OFF" << std::endl;
	}
	if (keys['v']) {
		showLightBoxes = true;
		std::cout << "showLightBoxes: ON" << std::endl;
	}
	if (keys['V']) {
		showLightBoxes = false;
		std::cout << "showLightBoxes: OFF" << std::endl;
	}




	if (keys[' ']) {
		std::cout << "SPACE" << std::endl;
		if (gameOver) {
			gameOver = false;
			gameWon = false;
			canMove = true;
			started = false;
			map.atEnd = false;
			doorRotation = 0;
			move_text(timeTextId, -(windowHeight + 10), -(windowWidth + 10));
			setupGame();
		}
	}
}

void mouse(int x, int y)
{
	if (trapMouse &&
		(x <= 50 || x >= glutGet(GLUT_WINDOW_WIDTH) - 50 ||
			y <= 50 || y >= glutGet(GLUT_WINDOW_HEIGHT) - 50))
	{
		lastX = windowWidth / 2;
		lastY = windowHeight / 2;

		glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
		return;
	}

	if (firstMouse)
	{
		lastX = x;
		lastY = y;
		firstMouse = false;
	}

	GLfloat xoffset = x - lastX;
	GLfloat yoffset = lastY - y;  // Reversed since y-coordinates go from bottom to left

	lastX = x;
	lastY = y;

	if (canMove) {
		camera.ProcessMouseMovement(xoffset, yoffset);
	}
}

void mouseWheel(int button, int dir, int x, int y) {
	camera.ProcessMouseScroll(dir);
}

void onKeyDepress(unsigned char key, int x, int y) {
	if (key >= 0 && key < 1024)
	{
		keys[key] = false;
	}
}

void onKeypress(unsigned char key, int x, int y) {
	if (key >= 0 && key < 1024)
	{
		keys[key] = true;
	}
}

void windowReshapeFunc(GLint newWidth, GLint newHeight) {
	windowWidth = newWidth;
	windowHeight = newHeight;

	if (!init_text_rendering(atlas_image, atlas_meta, windowWidth, windowHeight)) {
		fprintf(stderr, "ERROR init text rendering\n");
	}

	move_text(timeTextId, -(windowHeight + 10), -(windowWidth + 10));
	addText();

	glutPostRedisplay();
}

#pragma endregion Input Functions