#include "stdafx.h"
#include "N3sMath.hpp"

Vector2D::Vector2D() {}

Vector2D::Vector2D(int x, int y) : x(x), y(y) {}

void Vector2D::add(Vector2D v)
{
	x += v.x;
	y += v.y;
}

void Vector2D::sub(Vector2D v)
{
	x -= v.x;
	y -= v.y;
}

void Vector2D::snap()
{
	float xDiv = x / 8;
	float yDiv = y / 8;
	//xDiv += 0.5f;
	//yDiv += 0.5f;
	x = (int)roundDownPosOrNeg(xDiv) * 8;
	y = (int)roundDownPosOrNeg(yDiv) * 8;
}

void Vector2D::snapRelative(Vector2D v)
{
	x = floor((x - v.x + 4) / 8) * 8 + v.x;
	y = floor((y - v.y + 4) / 8) * 8 + v.y;
} 

Vector2D Vector2D::diff(Vector2D a, Vector2D b)
{
	// Return passed 
	return{ b.x - a.x, b.y - a.y };
}

Vector3D Vector3D::mirror(bool h, bool v)
{
	int newX, newY;
	if (h)
		newX = abs(x - 8);
	else
		newX = x;
	if (v)
	{
		newY = abs(y - 8);
	}
	else
		newY = y;
	return { newX, newY, z };
}
