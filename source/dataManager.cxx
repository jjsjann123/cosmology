/*!	darkSkySim raw data dataManager.h
 * 
 * 	This data manager will handle: 
 *		1. convert particle data into a volumetric data.
 *		2. parallel data distribute/gather.
 * 
 *	Author: Jie Jiang
 *	Date:	2015 Spring
 *	Class:	CS595 High Performance Simulation
 *	Project:DarkSkySimViz ( Partly from Scivis 2015 Contest )	
 */

#include "dataManager.h"
#include <iostream>
#include <cmath>

using namespace AJParallelRendering;
using namespace std;

#define SYNC_BOUNDARY 21
#define ACCUMULATE_VOLUME 22

DataManager::DataManager() : _reader(BUFFER_SIZE)
{
	_volume = NULL;
}

DataManager::~DataManager()
{
	if (_volume != NULL)
	{
		delete(_volume);
	}
}

void DataManager::setFileList(const char* fileList, int length)
{
	int index = 0;
	for ( int i = 0; i < length; i++) {
		_fileList.push_back(fileList + index);
		index++;
		if ( i == length - 1 )
		{
			break;
		} else {
			while ( fileList[index] != '\0' ) {
				index++;
			}
			index++;
		}
	}
}

void DataManager::setFileList(vector<string> fileList)
{
	_fileList = fileList;
}

void DataManager::findLocalBoundary()
{
	float globalMinMax3[6] = {0, 0, 0, 0, 0, 0};
	for ( vector<string>::iterator iter = _fileList.begin(); iter != _fileList.end(); iter++) {
		vector<string> att;
		att.push_back("x");
		att.push_back("y");
		att.push_back("z");
		float minMax3[6] = {0, 0, 0, 0, 0, 0};
		long length = _reader.readFile(iter->c_str(), att);
		_reader.load();
		_reader.getDataRange3f(minMax3);
		for ( int i = 0; i < 3; i++ )
		{
			if ( globalMinMax3[2*i] > minMax3[2*i]) {
				globalMinMax3[2*i] = minMax3[2*i];
			}
			if ( globalMinMax3[2*i+1] < minMax3[2*i+1]) {
				globalMinMax3[2*i+1] = minMax3[2*i+1];
			}
		}
	}
	_volumeOrigin.setValue(globalMinMax3[0], globalMinMax3[2], globalMinMax3[4]);
	_volumeSpan.setValue(globalMinMax3[1], globalMinMax3[3], globalMinMax3[5]);
}

void DataManager::syncGlobalBoundary(int d, int offset, int id)
{
	int flag = id - offset; // this will be the id for each node while compositing

	float minMax[6];
	MPI_Status status;
	int mask = 0;
	for ( int i = d-1; i >= 0; i--)
	{
		int activeBit = pow(2,i);
		if ( (mask & flag) == 0 )
		{
			int comm = (flag ^ activeBit) + offset; // the other node to communicate with.
			if ( (activeBit & flag ) != 0)
			{
				for ( int j = 0; j < 3; j++) {
					minMax[2*j] = _volumeOrigin[j];
					minMax[2*j+1] = _volumeSpan[j];
				}
				printf ( "%d sending to %d\n", id, comm);
				MPI_Send( minMax, 6, MPI_INT, comm, SYNC_BOUNDARY, MPI_COMM_WORLD);
			} else {
				printf ( "%d waiting from %d\n", id, comm);
				MPI_Recv( minMax, 6, MPI_INT, comm, SYNC_BOUNDARY, MPI_COMM_WORLD, &status );
				for ( int j = 0; j < 3; j++) {
					if ( _volumeOrigin[j] > minMax[2*j] )
						_volumeOrigin[j] = minMax[2*j];
					if ( _volumeSpan[j] < minMax[2*j+1] )
						_volumeSpan[j] = minMax[2*j+1];
				}
			}
		}
		mask = activeBit | mask;
	}

	if ( flag == 0 ) {
		for ( int j = 0; j < 3; j++) {
			minMax[2*j] = _volumeOrigin[j];
			minMax[2*j+1] = _volumeSpan[j];
		}
	}

	mask = pow(2,d) - 1;
	for ( int i = 0; i < d; i++)
	{
		int activeBit = pow(2,i);
		mask = activeBit ^ mask;
		if ( (mask & flag) == 0 )
		{
			int comm = (flag ^ activeBit) + offset; // the other node to communicate with.
			if ( (activeBit & flag) == 0)
			{
				printf ( "phase 2: %d sending to %d\n", id, comm);
				MPI_Send( minMax, 6, MPI_INT, comm, SYNC_BOUNDARY, MPI_COMM_WORLD);
			} else {
				printf ( "phase 2: %d waiting from %d\n", id, comm);
				MPI_Recv( minMax, 6, MPI_INT, comm, SYNC_BOUNDARY, MPI_COMM_WORLD, &status );
				for ( int j = 0; j < 3; j++) {
					_volumeOrigin[j] = minMax[2*j];
					_volumeSpan[j] = minMax[2*j+1];
				}
			}
		}
	}
}

void DataManager::testPrintVolume(int id)
{
	_volume->updateMetaInfo();
	printf ( "processor: %d   %.5g, %.5g, %.5g : %.5g, %.5g, %.5g\n   spacing: %.5g, %.5g, %.5g\n data Range: %.5g %.5g\n",
			id, _volumeOrigin[0], _volumeOrigin[1], _volumeOrigin[2],_volumeSpan[0], _volumeSpan[1], _volumeSpan[2],
			_volumeSpacing[0], _volumeSpacing[1], _volumeSpacing[2], _volume->_min, _volume->_max
			);
}

void DataManager::particle2Volume(int levels, Vec3i grid)
{
	_gridSize = grid;
	_volume = new LODVolume(levels, grid[0], grid[1], grid[2]);

	_volumeSpacing = _volumeSpan - _volumeOrigin;
	_volumeSpacing[0] /= (grid[0]-1);
	_volumeSpacing[1] /= (grid[1]-1);
	_volumeSpacing[2] /= (grid[2]-1);
	_volume->_volumeSpacing = _volumeSpacing;
	_volume->_volumeOffset = _volumeOrigin; // Yes it looks weird.
	float pos[3];
	_reader.load(0, BUFFER_SIZE);
	do {
		for (int i = 0; i < BUFFER_SIZE; i++) {
			_reader.getData3f(i, pos);
			for ( int j = 0; j < 3; j++) {
				pos[j] = ( pos[j] - _volumeOrigin[j] ) / _volumeSpacing[j];
			}
			//cout << "reading data " << endl;
			_volume->addVolume(pos, 1.0);
		}
	} while ( _reader.load() != 0 );
}

void DataManager::accumulateGlobalVolume(int d, int offset, int id)
{
	int flag = id - offset; // this will be the id for each node while compositing
	MPI_Status status;
	int mask = 1;
	int size = _gridSize[0] * _gridSize[1] * _gridSize[2];
	float *buffer = (float *) malloc ( size * sizeof (float));
	for ( int i = d-1; i >= 0; i--)
	{
		int comm = (flag ^ mask) + offset; // the other node to communicate with.
		if ( (mask & flag ) != 0)
		{
			//printf ( "%d sending to %d\n", id, comm);
			MPI_Send( _volume->_valArray, size, MPI_FLOAT, comm, ACCUMULATE_VOLUME, MPI_COMM_WORLD);
			MPI_Recv( buffer, size, MPI_FLOAT, comm, ACCUMULATE_VOLUME, MPI_COMM_WORLD, &status);
			_volume->addWholeVolume(buffer);
		} else {
			//printf ( "%d waiting from %d\n", id, comm);
			MPI_Recv( buffer, size, MPI_FLOAT, comm, ACCUMULATE_VOLUME, MPI_COMM_WORLD, &status);
			MPI_Send( _volume->_valArray, size, MPI_FLOAT, comm, ACCUMULATE_VOLUME, MPI_COMM_WORLD);
			_volume->addWholeVolume(buffer);

		}
		mask = mask << 1;
	}
	free(buffer);
}

void DataManager::normalizeVolume()
{
	_volume->updateMetaInfo();
	_volume->normalize(_volume->_min, _volume->_max);
}

void DataManager::shrinkToLocalVolume(int d, int offset, int id)
{

	int offsetX, offsetY, offsetZ; // offset for each dimension of current PE
	int nParX, nParY, nParZ; // total number of partitions for each dimension
	int sizeX, sizeY, sizeZ;
	int voxelOffsetX, voxelOffsetY, voxelOffsetZ;
	int volumeSizeX, volumeSizeY, volumeSizeZ;

	nParX = d/3+1;
	nParX = d%3 >= 1 ? nParX + 1 : nParX;
	nParY = d/3+1;
	nParY = d%3 >= 2 ? nParY + 1 : nParY;
	nParZ = d/3+1;
	sizeX = _volume->_x / nParX;
	sizeY = _volume->_y / nParY;
	sizeZ = _volume->_z / nParZ;

	int flag = id - offset;
	int bitFlag = flag;
	int bitWeight = 1;

	offsetX = 0;
	offsetY = 0;
	offsetZ = 0;

	// find out offset for each dimension
	while ( bitFlag != 0 ) {
		if ((bitFlag % 8 & 1) != 0) {
			offsetX += bitWeight;
		}
		if ((bitFlag % 8 & 2) != 0) {
			offsetY += bitWeight;
		}
		if ((bitFlag % 8 & 4) != 0) {
			offsetZ += bitWeight;
		}
		bitWeight = bitWeight << 1;
		bitFlag /= 8;
	}
	voxelOffsetX = sizeX * offsetX;
	voxelOffsetY = offsetY * sizeY;
	voxelOffsetZ = offsetZ * sizeZ;

	volumeSizeX = offsetX == nParX ? sizeX + _volume->_x % nParX : sizeX;
	volumeSizeY = offsetY == nParY ? sizeY + _volume->_y % nParY : sizeY;
	volumeSizeZ = offsetZ == nParZ ? sizeZ + _volume->_z % nParZ : sizeZ;
	printf("id: %d, x: %d, y: %d, z: %d, volX: %d, volY: %d, volZ: %d\n", id, voxelOffsetX, voxelOffsetY, voxelOffsetZ, volumeSizeX, volumeSizeY, volumeSizeZ );
	_volume->subVolume(voxelOffsetX, voxelOffsetY, voxelOffsetZ, volumeSizeX, volumeSizeY, volumeSizeZ );
}

LODVolume* DataManager::getVolumeOutput()
{
	if (_volume == NULL) {
		printf("empty volume. Not able to provide output");
	} else {
		LODVolume* ret = _volume;
		_volume = NULL;
		return ret;
	}
}
