/*!	darkSkySim volume data LODvolume.cxx
 * 
 * 	This LOF volume is a subclass of Volume with added feature: 
 *		1. wavelet-transform based level of detail ( LOD ).
 *		2. data brick awareness. ( this will be useful for parallel rendering )
 * 
 *	Author: Jie Jiang
 *	Date:	2015 Spring
 *	Class:	CS595 High Performance Simulation
 *	Project:DarkSkySimViz ( Partly from Scivis 2015 Contest )	
 */

#include "LODvolume.h"
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <cmath>
#include <algorithm>

using namespace AJParallelRendering;
using namespace std;

LODVolume::LODVolume(int levels, int *size) :
		Volume(size),	_maxLevel(levels)
{
	//_dataRAW = (float *) malloc ( _size[0] * _size[1] * _size[2] *sizeof(float));
	_dim.push_back(_x);
	_dim.push_back(_y);
	_dim.push_back(_z);
	initiateHaarFilter();
	_curLevel = 0;
	_dirty = true;
	vector<double> data(_x*_y*_z);
	_wa = new WaveletAnalysis(data, _dim, 0, _h0, _h1, _h2, _h3);
}

LODVolume::LODVolume(int levels, int x, int y, int z) :
		Volume(x, y, z),	_maxLevel(levels)
{
	//_dataRAW = (float *) malloc ( _size[0] * _size[1] * _size[2] *sizeof(float));
	_dim.push_back(_x);
	_dim.push_back(_y);
	_dim.push_back(_z);
	initiateHaarFilter();
	_curLevel = 0;
	_dirty = true;
	vector<double> data(_x*_y*_z);
	_wa = new WaveletAnalysis(data, _dim, 0, _h0, _h1, _h2, _h3);
}

LODVolume::LODVolume(int levels, int x, int y, int z, float *val) :
		Volume(x, y, z, val),	_maxLevel(levels)
{
	//_dataRAW = (float *) malloc ( _size[0] * _size[1] * _size[2] *sizeof(float));
	_dim.push_back(_x);
	_dim.push_back(_y);
	_dim.push_back(_z);
	initiateHaarFilter();
	_curLevel = 0;
	_dirty = true;
	vector<double> data(_x*_y*_z);
	_wa = new WaveletAnalysis(data, _dim, 0, _h0, _h1, _h2, _h3);
}

LODVolume::~LODVolume()
{
	delete (_wa);
}

void LODVolume::initiateHaarFilter()
{
	double s = 0.5*sqrtf(2.0);
	//Define Haar filters
	_h0.push_back(s);
	_h0.push_back(s);
	_h1.push_back(-s);
	_h1.push_back(s);
	_h2.push_back(s);
	_h2.push_back(s);
	_h3.push_back(-s);
	_h3.push_back(s);
}

void LODVolume::setSize(int x, int y, int z)
{
	Volume::setSize(x, y, z);
	_dim[0] = x;
	_dim[1] = y;
	_dim[2] = z;
	_dirty = true;
}

bool LODVolume::setVolume(int x, int y, int z, float val)
{
	_dirty = true;
	return Volume::setVolume(x, y, z, val);
}

void LODVolume::normalize(float min, float max)
{
	Volume::normalize(min, max);
	_dirty = true;
}

void LODVolume::updateData()
{
	printf("updating data of size: %d, %d, %d\n", _x, _y, _z);
	printf("updating data of dim: %d, %d, %d\n", _dim[0], _dim[1], _dim[2]);
	int size = _x * _y * _z;
	_dim.clear();
	_dim.push_back(_x);
	_dim.push_back(_y);
	_dim.push_back(_z);
	vector<double> dataRAW;
	for ( int i = 0; i <= size; i++) {
		dataRAW.push_back(_valArray[i]);
	}
	_wa->resetinput( dataRAW, _dim);
	_dirty = false;
	_curLevel = 0;
}

float LODVolume::getVolumeOfDetail(int newLevel, int x, int y, int z)
{
	if ( _dirty ) {
		updateData();
	}
	float ret;
	if (newLevel == 0)	{

		ret = Volume::getVolume(x, y, z);
	} else if ( newLevel >= _maxLevel || newLevel < 0) {
		printf ("requiring approximation level out of range");
		ret = -1;
	} else {
		if ( _curLevel != newLevel ) {
			if ( _curLevel != 0 )
			{
				printf("reverseTransform");
				_wa->itransform();
			}
			_curLevel = newLevel;
			printf("Transform");
			_wa->resetlevel(newLevel);
			_wa->transform();
		}
		vector<unsigned int> coord;
		int scale = pow( 2, newLevel);
		coord.push_back(x);
		coord.push_back(y);
		coord.push_back(z);
//		printf ("pos: %d, %d, %d\n", x, y, z);
		ret = _wa->getOutputValue(coord);
		ret /= scale;
	}
	return ret;
}

float LODVolume::getVolumeOfDetail(int newLevel, float x, float y, float z)
{
	Vec3f pos;
	pos.setValue(x, y, z);
	return getVolumeOfDetail(newLevel, pos);
}

float LODVolume::getVolumeOfDetail(int newLevel, Vec3i pos)
{
	return getVolumeOfDetail(newLevel, pos[0], pos[1], pos[2]);
}

float LODVolume::getVolumeOfDetail(int newLevel, Vec3f pos)
{
	//printf ("pos: %.5g, %.5g, %.5g\n", pos[0], pos[1], pos[2]);
	pos -= _volumeOffset;
	//printf ("pos: %.5g, %.5g, %.5g\n", pos[0], pos[1], pos[2]);
	Vec3f spacing = _volumeSpacing;
	pos = pos / spacing;
	//printf ("pos: %.5g, %.5g, %.5g\n", pos[0], pos[1], pos[2]);
	Vec3f offset;
	offset.setValue(_volumeOrigin._x, _volumeOrigin._y, _volumeOrigin._z);
	pos -= offset;
	//printf ("pos: %.5g, %.5g, %.5g\n", pos[0], pos[1], pos[2]);
	pos = pos / pow(2, newLevel);
	//printf ("pos: %.5g, %.5g, %.5g\n", pos[0], pos[1], pos[2]);

	Vec3i coordsFloor;
	coordsFloor.setValue(floor(pos[0]), floor(pos[1]), floor(pos[2]) );
	float ratio[3] = { pos[0] - coordsFloor[0], pos[0] - coordsFloor[0], pos[0] - coordsFloor[0] };
	Vec3i coordsGrid;
	float ret = 0;
	int xUpper, yUpper, zUpper;
	int ra = pow(2, newLevel);
	xUpper = _x / ra;
	yUpper = _y / ra;
	zUpper = _z / ra;
	float skippedRatio = 0;
	for (int i = 0; i < 8; i++) // 8 neighboring grid
	{
		float factor = 1.0;
		for (int j = 0; j < 3; j++) // interpolate in 3 direction
		{
			if ( (((int) pow(2, j)) & i) != 0 )
			{
				coordsGrid[j] = coordsFloor[j]+1;
				factor *= ratio[j];
			} else {
				coordsGrid[j] = coordsFloor[j];
				factor *= (1.0 - ratio[j]);
			}
		}
		if ( coordsGrid[0] >= xUpper || coordsGrid[1] >= yUpper || coordsGrid[2] >= zUpper) {
			skippedRatio += factor;
			continue;
		}
		float value = getVolumeOfDetail( newLevel, coordsGrid);
		//cout << " iteration: " << i << " factor: " << factor << " value: " << value <<endl;
		ret += (value*factor);
	}
	if ( skippedRatio != 0) {
		ret /= ( 1.0 - skippedRatio);
	}
	return ret;
}
