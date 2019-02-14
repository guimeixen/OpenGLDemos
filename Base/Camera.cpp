#include "Camera.h"

#include "Input.h"

#include "glm\gtc\matrix_transform.hpp"

Camera::Camera()
{
	firstMouse = true;
	position = glm::vec3(0.0f, 0.0f, 0.0f);
	worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	front = glm::vec3(0.0f, 0.0f, -1.0f);
	yaw = YAW;
	pitch = PITCH;
	moveSpeed = MOVE_SPEED;
	sensitivity = SENSITIVTY;
	UpdateCamVectors();
}

Camera::~Camera()
{
}

void Camera::Move(Camera_Movement direction, float deltaTime)
{
	float velocit = moveSpeed * deltaTime;
	if (direction == FORWARD)
	{
		position += front * velocit;
	}
	if (direction == BACKWARD)
	{
		position -= front * velocit;
	}
	if (direction == LEFT)
	{
		position -= right * velocit;
	}
	if (direction == RIGHT)
	{
		position += right * velocit;
	}
	frustum.SetCamDef(position, position + front, up);
}

void Camera::SetPos(glm::vec3 pos)
{
	position = pos;
	UpdateCamVectors();
}

void Camera::SetFOV(float fov)
{
	this->fov = fov;
	projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
	frustum.SetCamInternals(fov, aspectRatio, nearPlane, farPlane);
}

void Camera::SetNearPlane(float near)
{
	nearPlane = near;
	projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
	frustum.SetCamInternals(fov, aspectRatio, nearPlane, farPlane);
}

void Camera::SetFarPlane(float far)
{
	farPlane = far;
	projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
	frustum.SetCamInternals(fov, aspectRatio, nearPlane, farPlane);
}

void Camera::SetPitch(float pitch)
{
	this->pitch = pitch;
	UpdateCamVectors();
}

void Camera::SetYaw(float yaw)
{
	this->yaw = yaw;
	UpdateCamVectors();
}

void Camera::Look()
{
	const glm::vec2 &mousePos = Input::GetMousePosition();

	if (firstMouse == true)
	{
		lastMousePos = mousePos;
		firstMouse = false;
	}

	float offsetX = mousePos.x - lastMousePos.x;
	float offsetY = lastMousePos.y - mousePos.y;
	lastMousePos = mousePos;

	offsetX *= sensitivity;
	offsetY *= sensitivity;

	yaw += offsetX;
	pitch += offsetY;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	UpdateCamVectors();
}

// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
glm::mat4 Camera::GetViewMatrix()
{
	return glm::lookAt(position, position + front, up);
}

void Camera::SetProjectionMatrix(float fov, float aspectRatio, float nearPlane, float farPlane)
{
	this->fov = fov;
	this->aspectRatio = aspectRatio;
	this->nearPlane = nearPlane;
	this->farPlane = farPlane;
	projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
	frustum.SetCamInternals(fov, aspectRatio, nearPlane, farPlane);
}

glm::mat4 Camera::GetProjectionMatrix()
{
	return projectionMatrix;
}

void Camera::UpdateCamVectors()
{
	// Calculate the new Front vector
	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	this->front = glm::normalize(front);
	// Also re-calculate the Right and Up vector
	right = glm::normalize(glm::cross(this->front, worldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	up = glm::normalize(glm::cross(right, this->front));

	frustum.SetCamDef(position, position + front, up);
}
