#pragma once

#include "Frustum.h"

enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float MOVE_SPEED = 8.0f;
const float SENSITIVTY = 0.5f;

class Camera
{
public:

	bool firstMouse;

	Camera();
	~Camera();

	void Look();
	glm::mat4 GetViewMatrix();
	void SetProjectionMatrix(float fov, float aspectRatio, float nearPlane, float farPlane);
	const glm::mat4 &GetProjectionMatrix();
	void Move(Camera_Movement direction, float deltaTime);

	void SetPos(const glm::vec3 &pos);
	void SetFOV(float fov);
	void SetNearPlane(float near);
	void SetFarPlane(float far);
	void SetPitch(float pitch);
	void SetYaw(float yaw);
	void SetMoveSpeed(float speed) { moveSpeed = speed; }

	const glm::vec3 &GetForward() const { return front; }
	const glm::vec3 &GetRight() const { return right; }
	const glm::vec3 &GetPos() { return position; }
	float GetMoveSpeed() const { return moveSpeed; }
	float GetPitch() const { return pitch; }
	float GetYaw() const { return yaw; }
	float GetFOV()const { return fov; }

	const Frustum &GetFrustum() const { return frustum; }

private:
	Frustum frustum;
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 right;
	glm::vec3 up;
	glm::vec3 worldUp;

	glm::vec2 lastMousePos;
	float yaw;
	float pitch;
	float moveSpeed;
	float sensitivity;

	float nearPlane;
	float farPlane;
	float aspectRatio;
	float fov;
	glm::mat4 projectionMatrix;

	void UpdateCamVectors();
};


