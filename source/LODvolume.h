/*!	darkSkySim volume data LODvolume.h
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

#ifndef __LODVOLUME_H
#define __LODVOLUME_H

#include "volume.h"
#include "WaveletAnalysis.h"	// Library used to handle multi-dimensional DWT
#include <vector>

namespace AJParallelRendering {

	/*!
	 *    Class Volume is storing current volume data of a given LOD.
	 *    volume data stored on LODVolume is the original volume data after wavelet transform
	 *    every time when changing LOD, LODVolume update the _data of parent class.
	 */
	class LODVolume : public Volume
	{
	public:
		LODVolume(int levels, int *size);
		LODVolume(int levels, int x, int y, int z);
		LODVolume(int levels, int x, int y, int z, float *val);
		~LODVolume();

		// methods reimplemented from Volume
		void setSize(int x, int y, int z);
		bool setVolume(int x, int y, int z, float val);

		void normalize(float min, float max);

		float getVolumeOfDetail(int newLevel, Vec3i pos);
		float getVolumeOfDetail(int newLevel, int x, int y, int z);
		float getVolumeOfDetail(int newLevel, float x, float y, float z);
		float getVolumeOfDetail(int newLevel, Vec3f pos);

	protected:
		int _maxLevel;
		int _curLevel;

		bool _dirty;	// flag for data modification

		WaveletAnalysis *_wa;	// wavelet analysis
		vector<double> _dataRAW;	// original volume data for wavelet transform
		vector<unsigned int> _dim;	// dimension vector used for wavelet transform

		void updateData();

	private:
		vector<double> _h0;	// Haar filter coefficients
		vector<double> _h1;	// Haar filter coefficients
		vector<double> _h2;	// Haar filter coefficients
		vector<double> _h3;	// Haar filter coefficients

		void initiateHaarFilter(); // Defines Haar filter
	};

}

#endif //__LODVOLUME_H
