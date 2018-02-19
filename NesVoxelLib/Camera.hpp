// Shamelessly stolen from http://www.rastertek.com/dx11tut04.html

#pragma once
#include <malloc.h>
#include "N3sMath.hpp"


enum ViewSide { v_front, v_back, v_left, v_right, v_top, v_facing, v_bottom };
struct ViewingAngle { 
	ViewSide x; 
	ViewSide y; 
};

using namespace DirectX;

class Camera
{
public:
	Camera() {}

	virtual void SetPosition(float, float, float) = 0;
	virtual void SetRotation(float, float, float) = 0;

	virtual void AdjustPosition(float, float, float) = 0;
	virtual void AdjustRotation(float, float, float) = 0;

	virtual XMFLOAT3 GetPosition() = 0;
	virtual XMFLOAT3 GetRotation() = 0;

	virtual void Render() = 0;
	virtual XMMATRIX GetViewMatrix() = 0;
};

class FreeCamera : public Camera
{
public:
	FreeCamera();

	void SetPosition(float, float, float);
	void SetRotation(float, float, float);

	void AdjustPosition(float, float, float);
	void AdjustRotation(float, float, float);

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetRotation();

	void Render();
	XMMATRIX GetViewMatrix();

private:
	float m_positionX, m_positionY, m_positionZ;
	float m_rotationX, m_rotationY, m_rotationZ;
	XMMATRIX* m_viewMatrix;
};

class OrbitCamera : public Camera
{
public:
	OrbitCamera();

	void SetPosition(float, float, float);
	void SetRotation(float, float, float);

	void AdjustPosition(float, float, float);
	void AdjustRotation(float, float, float);

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetRotation();

	void Render();
	XMMATRIX GetViewMatrix();

	void setOverhead(bool o);
	void adjustZoom(float amount);
	void setZoom(float z);
	ViewingAngle getViewingAngle();
	MirrorDirection getMirrorDirection();
	float zoom;
private:
	float targetX, targetY, targetZ;
	float rotationX, rotationY;
	XMMATRIX* m_viewMatrix;
	bool overhead = false;
	XMFLOAT3 cameraPosition;
};