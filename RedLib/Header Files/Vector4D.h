#pragma once
#include "Trackable.h"
#include <ostream>
#include "Vector3D.h"

class Matrix4D;

class Vector4D : public Trackable
{

public:
	Vector4D();
	Vector4D(float x, float y, float z, float w);
	Vector4D(int x, int y, int z, int w);
	Vector4D(double x, double y, double z, double w);
	Vector4D(unsigned int x, unsigned int y, unsigned int z, unsigned int w);
	Vector4D(Vector3D subset, float w);
	~Vector4D();

	Vector4D operator=(const Vector4D& other);

	Vector4D operator+(const Vector4D& other) const;
	Vector4D operator-(const Vector4D& other) const;
	Vector4D operator*(const float scalar) const;
	Vector4D operator/(const float scalar) const;
	Vector4D operator*(const int scalar) const;
	Vector4D operator/(const int scalar) const;
	Vector4D operator*(const double scalar) const;
	Vector4D operator/(const double scalar) const;

	float operator*(const Vector4D& other) const;
	Vector4D operator*(const Matrix4D& other) const;

	Vector4D operator+=(const Vector4D& other);
	Vector4D operator-=(const Vector4D& other);
	Vector4D operator*=(const float scalar);
	Vector4D operator/=(const float scalar);

	bool operator==(const Vector4D& other) const;
	bool operator!=(const Vector4D& other) const;

	float operator[](int) const;

	std::ostream& write(std::ostream& out) const;
	std::string toString() const;

	float length() const;
	void normalize();

	Vector4D normalized() const;

	float getX() const { return mX; }
	float getY() const { return mY; }
	float getZ() const { return mZ; }
	float getW() const { return mW; }

	void setX(float x) { mX = x; }
	void setX(int x) { mX = (float)x; }
	void setX(double x) { mX = (float)x; }

	void setY(float y) { mY = y; }
	void setY(int y) { mY = (float)y; }
	void setY(double y) { mY = (float)y; }

	void setZ(float z) { mZ = z; }
	void setZ(int z) { mZ = (float)z; }
	void setZ(double z) { mZ = (float)z; }

	void setW(float w) { mW = w; }
	void setW(int w) { mW = (float)w; }
	void setW(double w) { mW = (float)w; }

	static const Vector4D Zero() { return Vector4D(0, 0, 0, 0); }
	static const Vector4D One() { return Vector4D(1, 1, 1, 1); }
	static const Vector4D Left() { return Vector4D(-1, 0, 0, 1); }
	static const Vector4D Right() { return Vector4D(1, 0, 0, 1); }
	static const Vector4D Up() { return Vector4D(0, -1, 0, 1); }
	static const Vector4D Down() { return Vector4D(0, 1, 0, 1); }
	static const Vector4D Forward() { return Vector4D(0, 0, 1, 1); }
	static const Vector4D Backward() { return Vector4D(0, 0, -1, 1); }

	static float Dot(const Vector4D a, const Vector4D b);

private:
	float mX, mY, mZ, mW;
	
};

//global operator overloads (to get better implementation with ostream)
std::ostream& operator<<(std::ostream& out, Vector4D const &vec);
