#include "vec.h"
#include <cmath>
#include <cstdlib>
#include <cstdio>

using namespace AJParallelRendering;
using namespace std;

void Vec3i::setValue(int *xyz)
{
	_x = xyz[0];
	_y = xyz[1];
	_z = xyz[2];
}

Vec3i Vec3i::operator + (const Vec3i & operand)
{
	Vec3i newVec = *this;
	newVec._x += operand._x;
	newVec._y += operand._y;
	newVec._z += operand._z;
	return newVec;
}

Vec3i Vec3i::operator - (const Vec3i & operand)
{
	Vec3i newVec = *this;
	newVec._x -= operand._x;
	newVec._y -= operand._y;
	newVec._z -= operand._z;
	return newVec;
}

Vec3i& Vec3i::operator += (const Vec3i & operand)
{
	this->_x += operand._x;
	this->_y += operand._y;
	this->_z += operand._z;
	return *this;
}

Vec3i& Vec3i::operator -= (const Vec3i & operand)
{
	this->_x -= operand._x;
	this->_y -= operand._y;
	this->_z -= operand._z;
	return *this;
}

int& Vec3i::operator [] (int index)
{
	switch (index)
	{
	case 0:
		return _x;
	case 1:
		return _y;
	case 2:
		return _z;
	}
}

Vec3f Vec3f::operator + (const Vec3f & operand)
{
	Vec3f newVec = *this;
	newVec._x += operand._x;
	newVec._y += operand._y;
	newVec._z += operand._z;
	return newVec;
}

Vec3f Vec3f::operator - (const Vec3f & operand)
{
	Vec3f newVec = *this;
	newVec._x -= operand._x;
	newVec._y -= operand._y;
	newVec._z -= operand._z;
	return newVec;
}

Vec3f& Vec3f::operator += (const Vec3f & operand)
{
	this->_x += operand._x;
	this->_y += operand._y;
	this->_z += operand._z;
	return *this;
}

Vec3f& Vec3f::operator -= (const Vec3f & operand)
{
	this->_x -= operand._x;
	this->_y -= operand._y;
	this->_z -= operand._z;
	return *this;
}

Vec3f& Vec3f::operator *= (float scale )
{
	this->_x -= this->_x * scale;
	this->_y -= this->_y * scale;
	this->_z -= this->_z * scale;
	return *this;
}

Vec3f Vec3f::operator * (float scale )
{
	Vec3f ret;
	ret._x = this->_x * scale;
	ret._y = this->_y * scale;
	ret._z = this->_z * scale;
	return ret;
}

Vec3f Vec3f::operator / (float scale )
{
	Vec3f ret;
	ret._x = this->_x / scale;
	ret._y = this->_y / scale;
	ret._z = this->_z / scale;
	return ret;
}


Vec3f Vec3f::operator * (const Vec3f & operand)
{
	Vec3f ret;
	ret._x = this->_x * operand._x;
	ret._y = this->_y * operand._y;
	ret._z = this->_z * operand._z;
	return ret;
}

Vec3f Vec3f::operator / (const Vec3f & operand)
{
	Vec3f ret;
	ret._x = this->_x / operand._x;
	ret._y = this->_y / operand._y;
	ret._z = this->_z / operand._z;
	return ret;
}

float& Vec3f::operator [] (int index)
{
	switch (index)
	{
	case 0:
		return _x;
	case 1:
		return _y;
	case 2:
		return _z;
	}
}

float Vec3f::length()
{
	return sqrt( _x*_x + _y*_y + _z*_z );
}

void Vec3f::normalize()
{
	float q = length();
	if ( q != 0 ) {
		_x /= q;
		_y /= q;
		_z /= q;
	} else {
		printf ( "normalize() a zero vector. return (1, 0, 0)\n" );
		_x = 1;
		_y = 0;
		_z = 0;
	}
}

float Vec3f::dot ( const Vec3f& operand)
{
	float ret;
	ret = this->_x * operand._x + this->_y * operand._y + this->_z * operand._z;
	return ret;
}

Vec3f Vec3f::cross ( const Vec3f& operand)
{
	Vec3f ret;
	ret._x = this->_y * operand._z - this->_z * operand._y;
	ret._y = this->_z * operand._x - this->_x * operand._z;
	ret._z = this->_x * operand._y - this->_y * operand._x;
	return ret;
}

float Vec4f::operator [] (int index)
{
	switch (index)
	{
	case 0:
		return _x;
	case 1:
		return _y;
	case 2:
		return _z;
	case 3:
		return _a;
	}
}

void Vec4f::getValue(float *rgba)
{
	rgba[0] = _x;
	rgba[1] = _y;
	rgba[2] = _z;
	rgba[3] = _a;
}

void Vec4f::setValue(float *rgba)
{
	_x = rgba[0];
	_y = rgba[1];
	_z = rgba[2];
	_a = rgba[3];
}
