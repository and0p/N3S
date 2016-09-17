// Shamelessly stolen from http://www.rastertek.com/dx11tut04.html

#pragma once

using namespace DirectX;

////////////////////////////////////////////////////////////////////////////////
// Filename: cameraclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _CAMERACLASS_H_
#define _CAMERACLASS_H_

////////////////////////////////////////////////////////////////////////////////
// Class name: VxlCamera
////////////////////////////////////////////////////////////////////////////////
class VxlCamera
{
public:
	VxlCamera();
	VxlCamera(const VxlCamera&);
	~VxlCamera();

	void SetPosition(float, float, float);
	void SetRotation(float, float, float);

	void AdjustPosition(float, float, float);
	void AdjustRotation(float, float, float);

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetRotation();

	void Render();
	void GetViewMatrix(XMMATRIX&);

private:
	float m_positionX, m_positionY, m_positionZ;
	float m_rotationX, m_rotationY, m_rotationZ;
	XMMATRIX m_viewMatrix = XMMatrixIdentity();
};

#endif