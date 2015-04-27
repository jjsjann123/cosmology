/*!
 *
 *	CS 595 Assignment 10
 *	Jie Jiang
 *	
 */
#ifndef __RENDERER_H
#define __RENDERER_H

//#include "volume.h"
#include "LODvolume.h"
#include "vec.h"
#include "EasyBMP.h"
#include "image.h"

#include <vector>

namespace AJParallelRendering {

	class Renderer
	{
		enum rayType { UNDEFINED, ORTHOGONAL, ARBITRARY, CYLINDER };

	public:
		Renderer();
		~Renderer();

		void addTransferFunction(float val, float r, float g, float b, float a);

		void setVolume(LODVolume *volume);

		void setGlobalOffset(int x, int y, int z);	 /// used for axis aligned ray casting
		void setGlobalSize(int x, int y, int z);	/// used for axis aligned ray casting
		void setGlobalMinMax( float min, float max);

		void setOrthogonalDirection( int dim, bool dir, int min, int max);	/// used only for axis aligned ray casting

		void setArbitraryRay( Ray ray, Vec3f cameraUpDir,
				int width, int height, float pixelWidth, float pixelHeight,
				float nearPlane, float farPlane, float stepSize); /// used for arbitrary ray casting

		void setLevel(int level);
		void updateRay(Ray ray, Vec3f cameraUpDir);
		void setClippingPlane(float nearPlane, float farPlane, float stepSize);
		void setResolution(int width, int height, float pixelWidth, float pixelHeight);

		void updateMetaInfo();
		void updateImageSize();
		void normalizeVolume();
		void render();
		void saveImage(const char* filename);

		ImageIO* getImage();

		void setEmissionOn() {_emissionMode = true;};
		void setEmissionOff() {_emissionMode = false;};

	//protected:
		LODVolume *_volumeData;
		float _globalMin;
		float _globalMax;
		Vec3i _globalSize;
		Vec3i _globalOffset;

		// used for axis aligned ray casting
		int _dim;
		int _minDim;
		int _maxDim;
		bool _direction;

		// used for arbitrary ray casting
		Ray _ray;
		Vec3f _cameraUpDir;
		float _clipNear;
		float _clipFar;
		float _stepSize;
		float _pixelWidth;
		float _pixelHeight;
		bool _emissionMode;

		int _level;

		// general field;
		int _rayType;

		int _imgWidth;
		int _imgHeight;

		ImageIO	_image;
		TransferFunction _transferFn;

		bool rayMarching(Ray ray, int imageX, int imageY);

	private:
		
	};
	
};

#endif // __RENDERER_H
