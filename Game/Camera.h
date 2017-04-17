#pragma once

#ifndef CAMERA
#define CAMERA

#include <vector>
#include <GL/glew.h>
#include "maths_funcs.h"

enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// Default camera values
const GLfloat YAW = -90.0f;
const GLfloat PITCH = 0.0f;
const GLfloat SPEED = 6.0f;
const GLfloat SENSITIVTY = 0.25f;
const GLfloat ZOOM = 45.0f;
const float PITH_LIMIT = 80.0f;

class Camera
{
public:
	// Camera Attributes
	vec3 Position;
	vec3 Front;
	vec3 Up;
	vec3 Right;
	vec3 WorldUp;

	// Eular Angles
	GLfloat Yaw;
	GLfloat Pitch;

	// Camera options
	GLfloat MovementSpeed;
	GLfloat MouseSensitivity;
	GLfloat Zoom;

	// Constraints
	float pitchLimit;

	// Constructor with vectors
	Camera(vec3 position = vec3(0.0f, 0.0f, 0.0f), vec3 up = vec3(0.0f, 1.0f, 0.0f), GLfloat yaw = YAW, GLfloat pitch = PITCH) 
		: Front(vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM), pitchLimit(PITH_LIMIT)
	{
		this->Position = position;
		this->WorldUp = up;
		this->Yaw = yaw;
		this->Pitch = pitch;
		this->updateCameraVectors();
	}

	// Returns the view matrix calculated using Eular Angles and the LookAt Matrix
	mat4 GetViewMatrix()
	{
		return look_at(this->Position, this->Position + this->Front, this->Up);
	}

	void setNewPosition(vec3 newPos) {
		this->Position = newPos;
	}

	vec3 ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime)
	{
		GLfloat velocity = this->MovementSpeed * deltaTime;
		vec3 newPos;

		switch (direction)
		{
		case FORWARD:
			newPos = this->Position + (this->Front * velocity);
			break;
		case BACKWARD:
			newPos = this->Position - (this->Front * velocity);
			break;
		case LEFT:
			newPos = this->Position - (this->Right * velocity);
			break;
		case RIGHT:
			newPos = this->Position + (this->Right * velocity);
			break;
		}

		return newPos;
	}

	void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= this->MouseSensitivity;
		yoffset *= this->MouseSensitivity;

		this->Yaw += xoffset;
		this->Pitch += yoffset;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (this->Pitch > pitchLimit)
				this->Pitch = pitchLimit;
			if (this->Pitch < -pitchLimit)
				this->Pitch = -pitchLimit;
		}

		// Update Front, Right and Up Vectors using the updated Eular angles
		this->updateCameraVectors();
	}

	// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessMouseScroll(GLfloat yoffset)
	{
		if (this->Zoom >= 1.0f && this->Zoom <= 45.0f)
			this->Zoom -= yoffset * SPEED;
		if (this->Zoom <= 1.0f)
			this->Zoom = 1.0f;
		if (this->Zoom >= 45.0f)
			this->Zoom = 45.0f;
	}

private:
	// Calculates the front vector from the Camera's (updated) Eular Angles
	void updateCameraVectors()
	{
		// Calculate the new Front vector
		vec3 front;
		front.v[0] = cos(degree_to_rad(this->Yaw)) * cos(degree_to_rad(this->Pitch));
		front.v[1] = sin(degree_to_rad(this->Pitch));
		front.v[2] = sin(degree_to_rad(this->Yaw)) * cos(degree_to_rad(this->Pitch));
		this->Front = normalise(front);

		// Also re-calculate the Right and Up vector
		this->Right = normalise(cross(this->Front, this->WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		this->Up = normalise(cross(this->Right, this->Front));
	}
};

#endif