/*!
 *
 *	CS 595 Assignment 10
 *	Jie Jiang
 *	
 */

#include "volume.h"
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cmath>

using namespace AJParallelRendering;
using namespace std;

Volume::Volume (int *size) : _x(size[0]), _y(size[1]), _z(size[2])
{
	_valArray = (float *) calloc ( _x * _y * _z, sizeof(float));
}


Volume::Volume (int x, int y, int z) : _x(x), _y(y), _z(z)
{
	_valArray = (float *) calloc ( x * y * z, sizeof(float));
}

Volume::Volume (int x, int y, int z, float *val) : _x(x), _y(y), _z(z)
{
	_valArray = (float *) malloc ( x * y * z *sizeof(float));
	memcpy( _valArray, val, x*y*z*sizeof(float) );
}

Volume::~Volume()
{
	free(_valArray);
}

void Volume::setSize(int x, int y, int z)
{
	_x = x;
	_y = y;
	_z = z;
	//if ( x*y*z > sizeof(_valArray))
	//{
	//	free(_valArray);
	//	_valArray = (float*) malloc ( x*y*z*sizeof(float));
	//}
	if ( sizeof(_valArray) > 0 ){
		free(_valArray);
		_valArray = (float*) malloc ( x*y*z*sizeof(float));
	}
}

void Volume::getSize(int &x, int &y, int &z)
{
	x = _x;
	y = _y;
	z = _z;
}

void Volume::getSize(int *size)
{
	size[0] = _x;
	size[1] = _y;
	size[2] = _z;
}

bool Volume::setVolume(int x, int y, int z, float val)
{
	if ( isWithinRange(x,y,z) )
	{
		int index = x + _x * ( y + _y * z);
		_valArray[index] = val;
		return true;
	} else {
		cout << "setVolume input: <" << x << ", " << y << ", " << z << "> out of range: <" << _x << ", " << _y << ", " << _z << ">" << endl; 
		return false;
	}
}

void Volume::setAllVolume(int x, int y, int z, float *val)
{
	setSize(x, y, z);
	memcpy( _valArray, val, x*y*z*sizeof(float) );
}

bool Volume::addVolume(int x, int y, int z, float val)
{
	if ( isWithinRange(x,y,z) )
	{
		int index = x + _x * ( y + _y * z);
		_valArray[index] += val;
		return true;
	} else {
		cout << "setVolume input: <" << x << ", " << y << ", " << z << "> out of range: <" << _x << ", " << _y << ", " << _z << ">" << endl;
		return false;
	}
}

bool Volume::addVolume(float x, float y, float z, float val)
{
	if ( isWithinRange(x,y,z) )
	{
		int xFloor = floor(x);
		float xWeight = x - xFloor;
		int yFloor = floor(y);
		float yWeight = y - yFloor;
		int zFloor = floor(z);
		float zWeight = z - zFloor;
		int index = xFloor + _x * ( yFloor + _y * zFloor);
		_valArray[index] += val * (1-xWeight) * (1-yWeight) * (1-zWeight);
		index += 1;
		_valArray[index] += val * (xWeight) * (1-yWeight) * (1-zWeight);
		index += _x;
		_valArray[index] += val * (xWeight) * (yWeight) * (1-zWeight);
		index += _y;
		_valArray[index] += val * (xWeight) * (yWeight) * (zWeight);
		index -= _x;
		_valArray[index] += val * (xWeight) * (1-yWeight) * (zWeight);
		index -= 1;
		_valArray[index] += val * (1-xWeight) * (1-yWeight) * (zWeight);
		index += x;
		_valArray[index] += val * (1-xWeight) * (yWeight) * (1-zWeight);
		index -= _y;
		_valArray[index] += val * (1-xWeight) * (yWeight) * (zWeight);
		return true;
	} else {
		cout << "addVolume input: <" << x << ", " << y << ", " << z << "> out of range: <" << _x << ", " << _y << ", " << _z << ">" << endl;
		return false;
	}
}

bool Volume::addVolume(float *xyz, float val)
{
	return addVolume(xyz[0], xyz[1], xyz[2], val);
}

float Volume::getVolume(int x, int y, int z)
{
	if ( isWithinRange(x,y,z) )
	{
		int index = x + _x * ( y + _y * z);
		return _valArray[index];
	} else {
		cout << "getVolume input: <" << x << ", " << y << ", " << z << "> out of range: <" << _x << ", " << _y << ", " << _z << ">" << endl; 
		return 0;
	}
}

float Volume::getVolume(int *xyz)
{
	return getVolume(xyz[0], xyz[1], xyz[2]);
}

float Volume::getVolume(const Vec3i &vec)
{
	return getVolume(vec._x, vec._y, vec._z);
}

void Volume::addWholeVolume( float *_data)
{
	int size = _x * _y * _z;
	for ( int i = 0; i < size; i++) {
		_valArray[i] += _data[i];
	}
}

bool Volume::addSubVolume(	int x, int y, int z,
							int sizeX, int sizeY, int sizeZ,
							float *data)
{
	if ( !isWithinRange(x, y, z) || !isWithinRange(x+sizeX-1, y+sizeY-1, z+sizeZ-1) )
	{
		cout << "createSubVolume size out of range" << endl;
		return false;
	}

	for ( int u = 0; u < sizeX; u++)
	{
		for ( int v = 0; v < sizeY; v++)
		{
			for ( int w = 0; w < sizeZ; w++)
			{
				int subIndex = u + sizeX * ( v + sizeY * w );
				int index = (x+u) + _x * ( (y+v) + _y * (z+w) );
				_valArray[index] += data[subIndex];
			}
		}
	}
	return true;
}


bool Volume::createSubVolume(	int x, int y, int z,
								int sizeX, int sizeY, int sizeZ,
								Volume *volume)
{
	if ( !isWithinRange(x, y, z) || !isWithinRange(x+sizeX-1, y+sizeY-1, z+sizeZ-1) )
	{
		cout << "createSubVolume size out of range" << endl;
		return false;
	}
	
	volume->setSize(sizeX, sizeY, sizeZ);
	for ( int u = 0; u < sizeX; u++)
	{
		for ( int v = 0; v < sizeY; v++)
		{
			for ( int w = 0; w < sizeZ; w++)
			{
				int subIndex = u + sizeX * ( v + sizeY * w );
				int index = (x+u) + _x * ( (y+v) + _y * (z+w) );
				volume->_valArray[subIndex] = _valArray[index];
			}	
		}	
	}
	return true;
}

bool Volume::createSubVolume(	int x, int y, int z,
						int sizeX, int sizeY, int sizeZ,
						float *data)
{
	if ( !isWithinRange(x, y, z) || !isWithinRange(x+sizeX-1, y+sizeY-1, z+sizeZ-1) )
	{
		cout << "createSubVolume size out of range" << endl;
		return false;
	}

	for ( int u = 0; u < sizeX; u++)
	{
		for ( int v = 0; v < sizeY; v++)
		{
			for ( int w = 0; w < sizeZ; w++)
			{
				int subIndex = u + sizeX * ( v + sizeY * w );
				int index = (x+u) + _x * ( (y+v) + _y * (z+w) );
				data[subIndex] = _valArray[index];
			}
		}
	}
	return true;
}

bool Volume::subVolume(	int x, int y, int z,
						int sizeX, int sizeY, int sizeZ )
{
	if ( !isWithinRange(x, y, z) || !isWithinRange(x+sizeX-1, y+sizeY-1, z+sizeZ-1) )
	{
		cout << "subVolume size out of range " << x << " " << y << " " << z << " " << endl;
		return false;
	}

	float *data = (float*) malloc ( sizeX*sizeY*sizeZ * sizeof(float));
	for ( int w = 0; w < sizeZ; w++) {
		for ( int v = 0; v < sizeY; v++) {
			for ( int u = 0; u < sizeX; u++) {
				int subIndex = u + sizeX * ( v + sizeY * w );
				int index = (x+u) + _x * ( (y+v) + _y * (z+w) );
				data[subIndex] = _valArray[index];
			}
		}
	}
	setSize(sizeX, sizeY, sizeZ);
	cout << "***************copy volume***************" << endl;
	memcpy( _valArray, data, sizeX*sizeY*sizeZ*sizeof(float) );
	_volumeOrigin.setValue(x, y, z);
	free(data);
	return true;
}



void Volume::updateMetaInfo()
{
	_max = _valArray[0];
	_min = _valArray[0];
	for ( int index = 0; index < _x * _y * _z; index++)
	{
		if (_valArray[index] > _max)
		{
			_max = _valArray[index];
			continue;
		}
		if (_valArray[index] < _min)
		{
			_min = _valArray[index];
			continue;
		}
	}
}

bool Volume::isWithinRange(int x, int y, int z)
{
	if ( x >= 0 && x < _x && y >= 0 && y < _y && z >= 0 && z < _z)
	{
		return true;
	} else {
		return false;
	}
}

bool Volume::isWithinRange(float x, float y, float z)
{
	if ( x >= 0 && x < _x && y >= 0 && y < _y && z >= 0 && z < _z)
	{
		return true;
	} else {
		return false;
	}
}

void Volume::normalize(float min, float max)
{
	int size = _x*_y*_z;
	for ( int i = 0; i < size; i++ )
	{
		_valArray[i] = interpolate(_valArray[i], min, max);
	}
}

float Volume::interpolate(float val, float min, float max)
{
	return (val - min) / (max - min);
}

bool Volume::intercept(Ray ray, float nearPlane, float farPlane, float &hit0, float &hit1)
{
	float t0 = -1e30f;
	float t1 = 1e30f;
	Vec3f offset;
	offset.setValue(_volumeOrigin._x, _volumeOrigin._y, _volumeOrigin._z);
	Vec3f pMin = offset * _volumeSpacing;
	pMin += _volumeOffset;

	Vec3f pMax;
	pMax.setValue(_x-1, _y-1, _z-1);
	pMax += offset;
	pMax = pMax * _volumeSpacing;
	pMax += _volumeOffset;

	Vec3f tNear = (pMin - ray.origin) / ray.dir;
    Vec3f tFar  = (pMax - ray.origin) / ray.dir;
    //printf("x: %f, y: %f, z: %f, farX: %f, farY: %f, farZ: %f\n", pMin._x, pMin._y, pMin._z, pMax._x, pMax._y, pMax._z );
	//printf("x: %f, y: %f, z: %f, farX: %f, farY: %f, farZ: %f\n", tNear._x, tNear._y, tNear._z, tFar._x, tFar._y, tFar._z );

    if (tNear._x > tFar._x) {
        float tmp = tNear._x;
        tNear._x = tFar._x;
        tFar._x = tmp;
    }
    t0 = std::max(tNear._x, t0);
    t1 = std::min(tFar._x, t1);

    if (tNear._y > tFar._y) {
        float tmp = tNear._y;
        tNear._y = tFar._y;
        tFar._y = tmp;
    }
    t0 = std::max(tNear._y, t0);
    t1 = std::min(tFar._y, t1);

    if (tNear._z > tFar._z) {
        float tmp = tNear._z;
        tNear._z = tFar._z;
        tFar._z = tmp;
    }
    t0 = std::max(tNear._z, t0);
    t1 = std::min(tFar._z, t1);

    t0 = std::max(t0, nearPlane);
	t1 = std::min(t1, farPlane);

    if (t0 <= t1 ) {
        hit0 = t0;
        hit1 = t1;
        return true;
    }
    else
        return false;
}

void Volume::test()
{
	Vec3f offset;
	offset.setValue(_volumeOrigin._x, _volumeOrigin._y, _volumeOrigin._z);
	Vec3f pMin = offset * _volumeSpacing;
	pMin += _volumeOffset;

	Vec3f pMax;
	pMax.setValue(_x-1, _y-1, _z-1);
	pMax += offset;
	pMax = pMax * _volumeSpacing;
	pMax += _volumeOffset;

	printf("x: %f, y: %f, z: %f, farX: %f, farY: %f, farZ: %f\n", pMin._x, pMin._y, pMin._z, pMax._x, pMax._y, pMax._z );
}
