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

#ifndef __DATAMANAGER_H
#define __DATAMANAGER_H

#include <mpi.h>
#include <vector>
#include <string>

#include "reader.h"
#include "LODvolume.h"
#include "vec.h"

using namespace std;

#define BUFFER_SIZE 5000		/// page size of each sdf read

namespace AJParallelRendering {

	class DataManager {
	public:
		DataManager();
		~DataManager();

		void setFileList(vector<string> fileList);
		void setFileList(const char* fileList, int length);

		void findLocalBoundary();
		float syncGlobalBoundary(int d, int offset, int id);	/// returns the maximum span

		void particle2Volume(int levels, Vec3i grid);
		void accumulateGlobalVolume(int d, int offset, int id);
		void normalizeVolume();
		void shrinkToLocalVolume(int d, int offset, int id);

		LODVolume* getVolumeOutput();
		Vec3f getOrigin() { return _volumeOrigin; };

		void testPrintVolume(int id);
	//protected:
		LODVolume *_volume;
		SDFReader _reader;
		vector<string> _fileList;
		Vec3i _gridSize;
		Vec3f _volumeSpacing;
		Vec3f _volumeOrigin;	// This is the (-X,-Y,-Z) for particle
		Vec3f _volumeSpan;	// This is the (X,Y,Z) for particle


	private:

	};

}

#endif //__DATAMANAGER_H
