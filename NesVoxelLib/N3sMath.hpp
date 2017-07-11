#pragma once

enum MirrorStyle { no_mirroring, mirror_single, mirror_offset };
enum MirrorDirection { mirror_x, mirror_y, mirror_z };

class Vector2D {
public:
	Vector2D();
	Vector2D(int x, int y);
	void add(Vector2D v);
	void sub(Vector2D v);
	void snap();
	void snapRelative(Vector2D v);
	static Vector2D diff(Vector2D a, Vector2D b);
	int x, y;
};

struct Vector3D {
	int x, y, z;
	bool equals(Vector3D v);
	Vector3D mirror(bool h, bool v);
	Vector3D mirrorMesh(Vector3D mirrorPoint,
		MirrorDirection mirrorDirection,
		MirrorStyle mirrorType);
};

struct Vector2F {
	float x;
	float y;
};

class Color4F {
public:
	float r;
	float g;
	float b;
	float a;
};

class Vector3F {
public:
	float x, y, z;
};



inline int getArrayIndexFromXY(int x, int y, int arrayWidth)
{
	return (y * arrayWidth) + x;
}

inline Vector2D unwrapArrayIndex(int i, int arrayWidth)
{
	int y = floor(i / arrayWidth);
	return{ (i % arrayWidth), y };
}

inline int roundDownPosOrNeg(float n)
{
	if (n < 0)
		return ceil(n);
	else
		return floor(n);
}

// Snap to nearest nametable point
inline Vector2D nametableSnap(int x, int y)
{
	float xDiv = x / 8;
	float yDiv = y / 8;
	xDiv += 0.5f;
	yDiv += 0.5f;
	return{ (int)floor(xDiv) * 8, (int)floor(yDiv) * 8 };
}