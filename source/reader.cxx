/*!	darkSkySim raw data reader.h
 * 
 * 	This data reader will load a SDF raw data. Convert it into a volumetric data.
 * 
 *	Author: Jie Jiang
 *	Date:	2015 Spring
 *	Class:	CS595 High Performance Simulation
 *	Project:DarkSkySimViz ( Partly from Scivis 2015 Contest )	
 */

#include "reader.h"
#include <iostream>

using namespace AJParallelRendering;
using namespace std;

SDFReader::SDFReader(int bufferHeightCap)
{
	_data = NULL;
	_width = 0;
	_height = 0;
	_bufferHeight = bufferHeightCap;
}

SDFReader::~SDFReader()
{
	if ( _data != NULL){
		for ( int i = 0; i < _width; i++)
			free (_data[i]);
		free(_data);

		Free(nread);
		Free(acnt);
		Free(strides);
		Free(types);
	}
}

float SDFReader::getFloat(int width, int height)
{
	if ( _data == NULL){
		cout << "No data loaded" << endl;
		return -1;
	}
	if ( height >= _length - _anchor || width >= _width){
		cout << "getData(...) out of range" << endl;
		return -1;
	}
	return (((float*)_data[width])[height] );
}

int SDFReader::getInt(int width, int height)
{
	if ( _data == NULL)
	{
		cout << "No data loaded" << endl;
		return -1;
	}
	if ( height >= _length - _anchor || width >= _width)
	{
		cout << "getData(...) out of range" << endl;
		return -1;
	}
	return (((int*)_data[width])[height] );
}

int64_t SDFReader::getInt64(int width, int height)
{
	if ( _data == NULL)
	{
		cout << "No data loaded" << endl;
		return -1;
	}
	if ( height >= _length - _anchor || width >= _width)
	{
		cout << "getData(...) out of range" << endl;
		return -1;
	}
	return (((int64_t*)_data[width])[height] );
}

int64_t SDFReader::readFile(const char *filename, std::vector<std::string> attribute)
{
	_sdfp = SDFopen(NULL, filename);
	if( _sdfp == NULL ){
	  printf("Could not SDFopen(%s), %s\n", filename, SDFerrstring);
	  return 0;
	}

	_length = SDFnrecs( attribute[0].c_str(), _sdfp);

	if ( _length < _bufferHeight ){
		_bufferHeight = _length;
	}

	if ( attribute.size() > _width ) { // if we need to change buffer size
		if ( _data != NULL )	// delete previous buffer
		{
			for ( int i = 0; i < _width; i++)
				Free (_data[i]);
			Free(_data);
			_data == NULL;

			Free(nread);
			Free(acnt);
			Free(strides);
			Free(types);
		}

		_attr.empty();
		_attr = attribute;
		_width = attribute.size();

		nread = (int*) malloc(_width*sizeof(int));
		types = (enum SDF_type_enum*) malloc(_width*sizeof(enum SDF_type_enum));
		strides = (int*) malloc(_width*sizeof(int));
		acnt = (int*) malloc(_width*sizeof(int));
	    // Get variable counts if the variable is itself an array.
	    for (int var=0; var<_width; var++){
	    	acnt[var] = SDFarrcnt(_attr[var].c_str(), _sdfp);
	    }
	    // Get variable types
	    for (int var=0; var<_width; var++){
	    	types[var] = SDFtype(_attr[var].c_str(), _sdfp);
	    }
	    // Set the strides for each of the variables
	    for (int var=0; var<_width; var++){
	    	strides[var] = SDFtype_sizes[types[var]]*acnt[var];
	    }
	    for (int var=0; var<_width; var++){
			nread[var] = _bufferHeight;
		}
	    // Allocate mem for _data buffer
	    _data = (void**) malloc(_width*sizeof(void*));
	    for (int var=0; var<_width; var++){
	    	_data[var] = (void*)malloc(strides[var] * nread[var]);
	    }

	}
	_anchor = 0;
	return _length;
}

int SDFReader::load()
{
	if ( _length == _anchor)
		return 0;

	char **names;
	names = (char**)malloc(_width*sizeof(char*));
	for (int var=0; var<_width; var++){
		SDFseek(_attr[var].c_str(), _anchor, SDF_SEEK_SET, _sdfp);
		int length = _attr[var].length();
		names[var] = (char*) malloc((length+1)*sizeof(char));
		_attr[var].copy(names[var], length, 0);
		names[var][length] = '\0';
	}

	for (int var=0; var<_width; var++){
		nread[var] = _length - _anchor < _bufferHeight? _length - _anchor : _bufferHeight;
	}

	// Read the data
	if (SDFrdvecsarr(_sdfp, _width, names, nread, _data, strides))
		printf("SDFrdvecsarr failed n=%d: %s\n", nread[0], SDFerrstring);
	_anchor += nread[0];
	for (int var=0; var<_width; var++){
		free(names[var]);
	}
	free(names);
	return nread[0];
}

int SDFReader::load(int start, int length)
{
	char **names;
	names = (char**)malloc(_width*sizeof(char*));
	for (int var=0; var<_width; var++){
		SDFseek(_attr[var].c_str(), start, SDF_SEEK_SET, _sdfp);
		int length = _attr[var].length();
		names[var] = (char*) malloc((length+1)*sizeof(char));
		_attr[var].copy(names[var], length, 0);
		names[var][length] = '\0';
	}
	for (int var=0; var<_width; var++){
		nread[var] = length;
	}

	// Read the data
	if (SDFrdvecsarr(_sdfp, _width, names, nread, _data, strides))
		printf("SDFrdvecsarr failed n=%d: %s\n", nread[0], SDFerrstring);

	_anchor = start + length;
	for (int var=0; var<_width; var++){
		free(names[var]);
	}
	free(names);
	return nread[0];

}

void SDFReader::getData3f(int index, float *dataArray)
{
	if ( _data == NULL){
		cout << "No data loaded" << endl;
		return;
	}
	if ( index < 0 || index >= _bufferHeight){
		cout << "getData(...) out of range" << endl;
		return;
	}
	for ( int i = 0; i < _width; i++){
		dataArray[i] = ((float*)_data[i])[index] ;
	}
}

void SDFReader::getDataRange3f(float *minMax)
{
	if ( _data == NULL){
		cout << "No data loaded" << endl;
		return;
	}

	long iteration = _length / _bufferHeight;

	for (int var=0; var<_width; var++){
		SDFseek(_attr[var].c_str(), 0, SDF_SEEK_SET, _sdfp);
	}

	for (int i = 0; i < 3; i++){
		minMax[2*i] = 0;
		minMax[2*i + 1] = -1;
	}

	float xyz[3];
	for ( int index = 0; index < iteration; index++)
	{
		load();
		for (int i = 0; i < _bufferHeight; i++) {
			getData3f(i, xyz);
			for (int j = 0; j < 3; j++) {
				if ( minMax[2*j] > minMax[2*j+1] ) {
					minMax[2*j] = xyz[j];
					minMax[2*j+1] = xyz[j];
				} else if ( minMax[2*j] > xyz[j]) {
					minMax[2*j] = xyz[j];
				} else if ( minMax[2*j+1] < xyz[j]) {
					minMax[2*j+1] = xyz[j];
				}
			}
		}
	}
	return;
}
