// Also shamelessly stolen from http://www.rastertek.com/dx11tut04.html

#include "stdafx.h"
#include "VoxelCamera.h"

VoxelCamera::VoxelCamera()
{
	m_positionX = 0.0f;
	m_positionY = 0.0f;
	m_positionZ = 0.0f;

	m_rotationX = 0.0f;
	m_rotationY = 0.0f;
	m_rotationZ = 0.0f;
}


VoxelCamera::VoxelCamera(const VoxelCamera& other)
{
}


VoxelCamera::~VoxelCamera()
{
}

void VoxelCamera::SetPosition(float x, float y, float z)
{
	m_positionX = x;
	m_positionY = y;
	m_positionZ = z;
	return;
}


void VoxelCamera::SetRotation(float x, float y, float z)
{
	m_rotationX = x;
	m_rotationY = y;
	m_rotationZ = z;
	return;
}

XMFLOAT3 VoxelCamera::GetPosition()
{
	return XMFLOAT3(m_positionX, m_positionY, m_positionZ);
}


XMFLOAT3 VoxelCamera::GetRotation()
{
	return XMFLOAT3(m_rotationX, m_rotationY, m_rotationZ);
}

void VoxelCamera::Render()
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
	m_viewMatrix = XMMatrixLookAtLH(position, lookAt, up);

	return;
}

void VoxelCamera::GetViewMatrix(XMMATRIX& viewMatrix)
{
	viewMatrix = m_viewMatrix;
	return;
}