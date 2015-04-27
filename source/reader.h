/*!	darkSkySim raw data reader.h
 * 
 * 	This data reader will load a SDF raw data. Convert it into a volumetric data.
 * 
 *	Author: Jie Jiang
 *	Date:	2015 Spring
 *	Class:	CS595 High Performance Simulation
 *	Project:DarkSkySimViz ( Partly from Scivis 2015 Contest )	
 */

#ifndef __READER_H
#define __READER_H

#include <vector>
#include <string>
#include "SDF.h"
#include "utils.h"

namespace AJParallelRendering {

	class SDFReader {
	public:
		SDFReader(int bufferHeightCap);
		~SDFReader();
		float getFloat(int width, int height); /// load one single data from current buffer
		int getInt(int width, int height);
		int64_t getInt64(int width, int height);

		int64_t readFile(const char *filename, std::vector<std::string> attribute); /// open SDF raw file
		int load();	/// load all remaining data into data buffer
		int load(int start, int length);	/// load more data ( offset, length ) into buffer

		void getDataRange3f(float* minMax); /// returns bounding box for the particles. [minX, maxX, minY, maxY, minZ, maxZ]
		void getData3f(int index, float* dataArray); /// returns all data assuming they are float. Used to get position efficiently

	protected:
		SDF	*_sdfp;
		void **_data;	/// data buffer
		std::vector<std::string> _attr;	/// data attribute vector
		int	_width;		/// data width in _data buffer
		int _height;	/// current data length in _data buffer
		int _bufferHeight; /// current buffer height
		int64_t _length;	/// global data length in _data buffer
		int64_t _anchor;	/// current data offset
		
	private:
		int *nread;
		int *acnt;
		int *strides;
		enum SDF_type_enum *types;
		
	};

}

#endif //__READER_H
