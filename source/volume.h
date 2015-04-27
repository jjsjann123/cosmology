/*!
 *
 *	CS 595 Assignment 10
 *	Jie Jiang
 *	
 */
#ifndef	__VOLUME_H
#define __VOLUME_H

#include "vec.h"

namespace AJParallelRendering {

	class Renderer;
	class DataManager;
	class RenderManager;

	class Volume
	{
	public:
		Volume(int *size);
		Volume(int x, int y, int z);
		Volume(int x, int y, int z, float *val);
		~Volume();
		
		void setSize(int x, int y, int z);
		void getSize(int &x, int &y, int &z);
		void getSize(int *size);

		void setAllVolume(int x, int y, int z, float *val);
		bool setVolume(int x, int y, int z, float val);
		bool addVolume(int x, int y, int z, float val);	/// add val into (x, y, z)
		bool addVolume(float x, float y, float z, float val);	/// for each (x, y, z) find 8 neighboring vertices and accumulate interpolated value.
		bool addVolume(float *xyz, float val);	/// for each (x, y, z) find 8 neighboring vertices and accumulate interpolated value.

		float getVolume(int x, int y, int z);
		float getVolume(int *xyz);
		float getVolume(const Vec3i &vec);

		void addWholeVolume( float *_data );	/// assuming data has the same size of _valArray. Adding each element to _valArray;
		bool addSubVolume(	int x, int y, int z,
							int sizeX, int sizeY, int sizeZ,
							float *data);
		bool subVolume(	int x, int y, int z,	/// shrink data to a sub block, starting voxel (x,y,z) will be stored in _volumeOrigin;
						int sizeX, int sizeY, int sizeZ );
		bool createSubVolume(	int x, int y, int z,
								int sizeX, int sizeY, int sizeZ,
								float *data);
		bool createSubVolume(	int x, int y, int z, 
			 					int sizeX, int sizeY, int sizeZ,
								Volume *volume);
		void normalize(float min, float max);
		void updateMetaInfo();
		
		bool intercept(Ray ray, float nearPlane, float farPlane, float &hit0, float &hit1);	/// check if current ray intercept the volume

	//protected:
		int _x;
		int _y;
		int _z;
		
		float _max;
		float _min;
		
		bool isWithinRange( int x, int y, int z);
		bool isWithinRange( float x, float y, float z);
		float *_valArray;
		
		Vec3i _volumeOrigin;	// volume offset
		Vec3f _volumeSpacing;	// space between each voxel
		Vec3f _volumeOffset;	// This is actually the coordinate origin

	private:
		float interpolate(float val, float min, float max);

		void test();
		friend class RenderManager;
		friend class Renderer;
		friend class DataManager;
	};
	
};

#endif // __VOLUME_H
