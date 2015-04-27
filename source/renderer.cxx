/*	CS 595 Assignment 10
 *	Jie Jiang
 *	
 */

#include "renderer.h"
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <stdio.h>

using namespace std;
using namespace AJParallelRendering;

Renderer::Renderer()
{
	_volumeData = NULL;
	
	_imgWidth = 0;
	_imgHeight = 0;
	_globalSize.setValue(0, 0, 0);
	
	_rayType = UNDEFINED;
}

Renderer::~Renderer()
{

}

void Renderer::addTransferFunction(float val, float r, float g, float b, float a)
{
	float rgba[4];
	rgba[0] = r;
	rgba[1] = g;
	rgba[2] = b;
	rgba[3] = a;
	_transferFn.addAnchorPoint(val, rgba);
}

void Renderer::setVolume(LODVolume *volume)
{
	_volumeData = volume;
}

void Renderer::setGlobalOffset ( int x, int y, int z)
{
	_globalOffset.setValue(x, y, z);
}

void Renderer::setGlobalSize(int x, int y, int z)
{
	_globalSize.setValue(x, y, z);	
}

void Renderer::setGlobalMinMax( float min, float max)
{
	_globalMin = min;
	_globalMax = max;
}

void Renderer::setOrthogonalDirection( int dim, bool dir, int min, int max )
{
	_dim = dim;
	_direction = dir;
	_minDim = min;
	_maxDim = max;
	_rayType = ORTHOGONAL;
}

void Renderer::updateMetaInfo()
{
	switch(_rayType)
	{
	case ORTHOGONAL:
		if (_globalSize._x == 0 || _globalSize._y == 0 || _globalSize._z == 0 )
		{
			cout << "no global size yet" << endl;
			return;
		}
		if ( _volumeData == NULL )
		{
			cout << "no volumes" << endl;
			return;
		}
		if ( _globalMin == 0 || _globalMax == 0)
		{
			cout << "no global min/max yet" << endl;
			return;
		}
		normalizeVolume();
		break;
	case ARBITRARY:
		_image.initiateData(_imgWidth, _imgHeight, NULL);
		break;

	case UNDEFINED:
	default:
		cout << "undefined rendering scheme" << endl;
		break;
	}
	
}

void Renderer::normalizeVolume()
{
	_volumeData->normalize(_globalMin, _globalMax);
	//_volumeData->updateMetaInfo();
}

void Renderer::updateImageSize()
{
	int width, height;
	switch ( _dim )
	{
	case 0:
		width = _globalSize._y;
		height = _globalSize._z;
		break;
	case 1:
		width = _globalSize._x;
		height = _globalSize._z;
		break;
	case 2:
		width = _globalSize._x;
		height = _globalSize._y;
		break;
	}
	_image.initiateData(width, height, NULL);
}

void Renderer::render()
{
	switch(_rayType)
	{
	case ORTHOGONAL:
		int width;
		int height;
		int coord[2];
		
		switch ( _dim )
		{
		case 0:
			coord[0] = 1;
			coord[1] = 2;
			width = _globalSize._y;
			height = _globalSize._z;
			break;
		case 1:
			coord[0] = 0;
			coord[1] = 2;
			width = _globalSize._x;
			height = _globalSize._z;
			break;
		case 2:
			coord[0] = 0;
			coord[1] = 1;
			width = _globalSize._x;
			height = _globalSize._y;
			break;
		}

		int volumeSize[3];
		_volumeData->getSize(volumeSize);
		
		int frontPage, endPage; // local range of visible volumes
		frontPage = _minDim - _globalOffset[_dim];
		endPage = _maxDim -  _globalOffset[_dim];
		
/*		printf ("minDim: %d, maxDim: %d, offset: %d %d %d, size: %d %d %d, targetDim: %d, start: %d, end: %d\n",
						_minDim, _maxDim, _globalOffset[0], _globalOffset[1], _globalOffset[2],
						volumeSize[0], volumeSize[1], volumeSize[2], _dim, frontPage, endPage
				);*/
		//_image.setSize(width, height);
		_image.initiateData(width, height, NULL);

		if ( frontPage >= volumeSize[_dim] || endPage <= 0) // current node does not contain any visible volume
		{
			cout << "no data for rendering" << endl;
		} else { // visible volume
			int seq[3];
			frontPage = frontPage < 0 ? 0 : frontPage;
			endPage = endPage > volumeSize[_dim] ? volumeSize[_dim] : endPage;
			if ( _direction == true )
			{
				seq[0] = frontPage;
				seq[1] = endPage-1;
				seq[2] = 1;
			} else {
				seq[0] = endPage-1;
				seq[1] = frontPage;
				seq[2] = -1;
			}
			int pos[3];

			printf ("minDim: %d, maxDim: %d, offset: %d %d %d, size: %d %d %d, targetDim: %d, start: %d, end: %d, start: %d, end: %d\n",
							_minDim, _maxDim, _globalOffset[0], _globalOffset[1], _globalOffset[2],
							volumeSize[0], volumeSize[1], volumeSize[2], _dim, frontPage, endPage,
							seq[0], seq[1]
					);

			for ( int y = 0; y < volumeSize[coord[1]]; y++)
			{
				pos[coord[1]] = y;
				int imageY = y+_globalOffset[coord[1]];
				for ( int x = 0; x < volumeSize[coord[0]]; x++)
				{
					pos[coord[0]] = x;
					int imageX = x+_globalOffset[coord[0]];
					for ( int i = seq[0]; i != seq[1]; i += seq[2])
					{
						pos[_dim] = i;
						float val = _volumeData->getVolume(pos);
						float rgba[5];
						float rgbaX[4];
						_image.getRGBA ( imageX, imageY, rgba);
						_transferFn.getValue(val, rgbaX);
						rgba[0] = rgba[0] + rgbaX[0] * rgbaX[3] * ( 1- rgba[3]);
						rgba[1] = rgba[1] + rgbaX[1] * rgbaX[3] * ( 1- rgba[3]);
						rgba[2] = rgba[2] + rgbaX[2] * rgbaX[3] * ( 1- rgba[3]);
						rgba[3] = rgba[3] + rgbaX[3] * ( 1 - rgba[3]);
						rgba[4] = i + _globalOffset[_dim];
						//rgba[4] = _globalOffset[_dim];
						_image.setRGBA( imageX, imageY, rgba);
						if (rgba[3] > 0.97)
							break;
					}
				}
			}
		}
		break;
	case ARBITRARY:
	{
		float Le = .25f;           // Emission coefficient
		float sigma_a = 10;        // Absorption coefficient
		float sigma_s = 10;        // Scattering coefficient
		float stepDist = 0.025f;   // Ray step amount
		float lightIntensity = 40; // Light source intensity
	    float tau = 0.f;  // accumulated beam transmittance
	    float L = 0;      // radiance along the ray

		Ray ray = _ray; // new ray
		Vec3f origin = _ray.origin;
		Vec3f xDir = _ray.dir.cross(_cameraUpDir);
		xDir.normalize();
		Vec3f yDir = _cameraUpDir;
		yDir.normalize();
		int index = 0;
		_image.initiateData(_imgWidth, _imgHeight, NULL);
		for ( int j = 0; j < _imgHeight; j++) {
			for ( int i = 0; i < _imgWidth; i++) {
				ray.origin = origin;

				float xOffset = _imgWidth;
				xOffset = _pixelWidth * ( j - xOffset/2.0 );
				float yOffset = _imgHeight;
				yOffset = _pixelHeight * ( i - yOffset/2.0 );

				ray.origin += xDir * xOffset;
				ray.origin += yDir * yOffset;

				if (rayMarching(ray, i, j)) {
					index++;
				}
			}
		}
		printf("\nx: %.5g, %.5g, %.5g\ny:%.5g, %.5g, %.5g \ndirection: %.5g, %.5g, %.5g effective ray: %d\ncameraUp: %.5g, %.5g, %.5g\n", xDir._x, xDir._y, xDir._z, yDir._x, yDir._y, yDir._z, ray.dir._x, ray.dir._y, ray.dir._z, index, _cameraUpDir._x, _cameraUpDir._y, _cameraUpDir._z);
	}
		break;

	case UNDEFINED:
	default:
		cout << "undefined rendering scheme" << endl;
		break;
	}
}

bool Renderer::rayMarching (Ray ray, int imageX, int imageY)
{
	float t0, t1;
	if ( _volumeData->intercept(ray, _clipNear, _clipFar, t0, t1) )
	{
		Vec3f pos;
		int start = ceil(t0 / _stepSize);
		int end = floor(t1 / _stepSize);


		for ( int i = start; i < end; i++ )	{
			//printf("\nray: %.5g, %.5g, %.5g\ndir:%.5g, %.5g, %.5g \n range: %.5g, %.5g\n", ray.origin._x, ray.origin._y, ray.origin._z, ray.dir._x, ray.dir._y, ray.dir._z, t0, t1);
			pos = ray.origin + ray.dir * i * _stepSize;

			float val = _volumeData->getVolumeOfDetail( _level, pos);
			if ( val < 0) {
				printf ("level: %d pos: %.5g, %.5g, %.5g data value: %.5g\n", _level, pos[0], pos[1], pos[2], val);
				continue;
			}
			//float val = 0.5;
			float rgba[5];
			float rgbaX[4];
			_image.getRGBA ( imageX, imageY, rgba);
			_transferFn.getValue(val, rgbaX);
			rgba[0] = rgba[0] + rgbaX[0] * rgbaX[3] * ( 1- rgba[3]);
			rgba[1] = rgba[1] + rgbaX[1] * rgbaX[3] * ( 1- rgba[3]);
			rgba[2] = rgba[2] + rgbaX[2] * rgbaX[3] * ( 1- rgba[3]);
			rgba[3] = rgba[3] + rgbaX[3] * ( 1 - rgba[3]);
			rgba[4] = i * _stepSize;
			_image.setRGBA( imageX, imageY, rgba);
			if (rgba[3] > 0.97)
				break;
		}

		return true;
	} else {
		return false;
	}
}

void Renderer::setLevel(int level)
{
	_level = level;
}

void Renderer::updateRay( Ray ray, Vec3f cameraUpDir)
{
	_ray = ray;
	_ray.dir.normalize();
	float upDir = cameraUpDir.dot(_ray.dir);
	if ( upDir != 0 ) {
		_cameraUpDir = cameraUpDir - _ray.dir * upDir;
	} else {
		_cameraUpDir = cameraUpDir;
	}
	_cameraUpDir.normalize();
	//printf("\n1: %.5g, %.5g, %.5g\n2: %.5g, %.5g, %.5g\n%.5g", _cameraUpDir._x, _cameraUpDir._y, _cameraUpDir._z, cameraUpDir._x, cameraUpDir._y, cameraUpDir._z, upDir);
}

void Renderer::setClippingPlane(float nearPlane, float farPlane, float stepSize)
{
	_clipNear = nearPlane;
	_clipFar = farPlane;
	_stepSize = stepSize;
}

void Renderer::setResolution(int width, int height, float pixelWidth, float pixelHeight)
{
	_imgWidth = width;
	_imgHeight = height;
	_pixelWidth = pixelWidth;
	_pixelHeight = pixelHeight;
	_rayType = ARBITRARY;
}

void Renderer::saveImage(const char* filename)
{
	_image.saveImage(filename);
}

ImageIO* Renderer::getImage()
{
	return &_image;
}

void Renderer::setArbitraryRay( Ray ray, Vec3f cameraUpDir,
								int width, int height, float pixelWidth, float pixelHeight,
								float nearPlane, float farPlane, float stepSize	)
{
	_ray = ray;
	_ray.dir.normalize();
	_imgWidth = width;
	_imgHeight = height;
	_pixelWidth = pixelWidth;
	_pixelHeight = pixelHeight;
	_clipNear = nearPlane;
	_clipFar = farPlane;
	_stepSize = stepSize;


	float upDir = cameraUpDir.dot(_ray.dir);
	if ( upDir != 0 )
		_cameraUpDir = cameraUpDir - _ray.dir * upDir;
	_cameraUpDir.normalize();


	_level = 0;

	_rayType = ARBITRARY;
}

