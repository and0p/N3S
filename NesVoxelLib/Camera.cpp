// Also shamelessly stolen from http://www.rastertek.com/dx11tut04.html

#include "stdafx.h"
#include "N3s3d.hpp"
#include "Camera.hpp"

FreeCamera::FreeCamera()
{
	m_viewMatrix = (XMMATRIX*)_aligned_malloc(sizeof(XMMATRIX), alignof(XMMATRIX));
	*m_viewMatrix = XMMatrixIdentity();
	m_positionX = 0.0f;
	m_positionY = 0.0f;
	m_positionZ = 0.0f;

	m_rotationX = 0.0f;
	m_rotationY = 0.0f;
	m_rotationZ = 0.0f;
}

void FreeCamera::SetPosition(float x, float y, float z)
{
	m_positionX = x;
	m_positionY = y;
	m_positionZ = z;
	return;
}

void FreeCamera::SetRotation(float x, float y, float z)
{
	m_rotationX = x;
	m_rotationY = y;
	m_rotationZ = z;
	return;
}

void FreeCamera::AdjustPosition(float x, float y, float z)
{
	m_positionX += x;
	m_positionY += y;
	m_positionZ += z;
}

void FreeCamera::AdjustRotation(float x, float y, float z)
{
	m_rotationX += x;
	m_rotationY += y;
	m_rotationZ += z;
	if (m_rotationX > 360)
		m_rotationX -= 360.0f;
	if (m_rotationX < 0)
		m_rotationX += 360.0f;
}

XMFLOAT3 FreeCamera::GetPosition()
{
	return XMFLOAT3(m_positionX, m_positionY, m_positionZ);
}


XMFLOAT3 FreeCamera::GetRotation()
{
	return XMFLOAT3(m_rotationX, m_rotationY, m_rotationZ);
}

void FreeCamera::Render()
{
	XMVECTOR up, position, lookAt;
	float yaw, pitch, roll;
	XMMATRIX rotationMatrix;


	// Setup the vector that points upwards.
	up = { 0.0f, 1.0f, 0.0f };

	// Setup the position of the camera in the world.
	position = { m_positionX, m_positionY, m_positionZ };

	// Setup where the camera is looking by default.
	lookAt = { 0.0f, 0.0f, 1.0f };

	// Set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotations in radians.
	pitch = m_rotationX * 0.0174532925f;
	yaw = m_rotationY * 0.0174532925f;
	roll = m_rotationZ * 0.0174532925f;


	// Create the rotation matrix from the yaw, pitch, and roll values.
	XMMATRIX mtxRotation = XMMatrixRotationRollPitchYaw(roll, pitch, yaw);;

	// Transform the lookAt and up vector by the rotation matrix so the view is correctly rotated at the origin.
	lookAt = XMVector3TransformCoord(lookAt, mtxRotation);
	up = XMVector3TransformCoord(up, mtxRotation);

	// Translate the rotated camera position to the location of the viewer.
	lookAt = position + lookAt;

	// Finally create the view matrix from the three updated vectors.
	//*m_viewMatrix = XMMatrixIdentity();
	*m_viewMatrix = XMMatrixLookAtLH(position, lookAt, up);
}

XMMATRIX FreeCamera::GetViewMatrix()
{
	return *m_viewMatrix;
}

OrbitCamera::OrbitCamera()
{
	m_viewMatrix = (XMMATRIX*)_aligned_malloc(sizeof(XMMATRIX), alignof(XMMATRIX));
	*m_viewMatrix = XMMatrixIdentity();
	targetX = 0.0f;
	targetY = 0.0f;
	targetZ = pixelSizeW * 16;
	rotationX = 0.0f;
	rotationY = 0.0f;
}

void OrbitCamera::SetPosition(float x, float y, float z)
{
	targetX = x;
	targetY = y;
	targetZ = z;
	return;
}

void OrbitCamera::SetRotation(float x, float y, float z)
{
	rotationX = x;
	rotationY = y;
	return;
}

void OrbitCamera::AdjustPosition(float x, float y, float z)
{
	targetX += x;
	targetY += y;
	targetZ += z;
}

void OrbitCamera::AdjustRotation(float x, float y, float z)
{
	rotationX += x;
	rotationY += y;
	if (rotationX > 360)
		rotationX -= 360.0f;
	if (rotationX < 0)
		rotationX += 360.0f;
}

XMFLOAT3 OrbitCamera::GetPosition()
{
	return XMFLOAT3(targetX, targetY, targetZ);
}


XMFLOAT3 OrbitCamera::GetRotation()
{
	return XMFLOAT3(rotationX, rotationY, 0.0f);
}

void OrbitCamera::Render()
{
	XMVECTOR cameraPos, targetPos, up;
	XMMATRIX yaw = XMMatrixRotationX(XMConvertToRadians(rotationY));
	XMMATRIX pitch = XMMatrixRotationY(XMConvertToRadians(rotationX));
	XMVECTOR v = { 0, 0, -zoom };
	v = XMVector3Transform(v, yaw);
	v = XMVector3Transform(v, pitch);

	up = { 0.0f, 1.0f, 0.0f };
	targetPos = { targetX, targetY, targetZ };
	*m_viewMatrix = XMMatrixLookAtLH(targetPos + v, targetPos, up);
}

XMMATRIX OrbitCamera::GetViewMatrix()
{
	return *m_viewMatrix;
}

void OrbitCamera::setOverhead(bool o) {
	overhead = o;
	// TODO change pitch/yaw if needed?
}

void OrbitCamera::adjustZoom(float amount)
{
	zoom -= amount;
	if (zoom < 0.2f)
		zoom = 0.2f;
	else if (zoom > 20.0f)
		zoom = 20.0f;
}
