#pragma once
#include "Trackable.h"

class Vector2D : public Trackable
{

public:
	Vector2D();
	Vector2D(float x, float y);
	Vector2D(int x, int y);
	~Vector2D();

	Vector2D operator=(const Vector2D& other);

	Vector2D operator+(const Vector2D& other);
	Vector2D operator-(const Vector2D& other);
	Vector2D operator*(const float scalar);
	Vector2D operator/(const float scalar);

	Vector2D operator+=(const Vector2D& other);
	Vector2D operator-=(const Vector2D& other);
	Vector2D operator*=(const float scalar);
	Vector2D operator/=(const float scalar);

	float getX() { return mX; }
	float getY() { return mY; }

	static const Vector2D Zero() { return Vector2D(0, 0); }

private:
	float mX, mY;
	
};
