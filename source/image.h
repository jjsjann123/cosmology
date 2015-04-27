/*!
 *
 *	CS 595 Assignment 10
 *	Jie Jiang
 *	
 */
#ifndef __IMAGE_H
#define __IMAGE_H

#include "volume.h"
#include "vec.h"
#include "EasyBMP.h"

#include <vector>


namespace AJParallelRendering {

	class ImageIO
	{
	public:
		ImageIO();
		ImageIO(const ImageIO &image);
		ImageIO& operator = (const ImageIO& image); // no assign constructor
		~ImageIO();

		void initiateData(int width, int height, float *rgbadData);

		void setSize(int width, int height);
		void getSize(int &width, int &height) { width = _width; height = _height; };
		void setRGBA(int x, int y, float r, float g, float b, float a, float depth);
		void setRGBA(int x, int y, float *rgba);
		void getRGBA(int x, int y, float &r, float &g, float &b, float &a, float &depth);
		void getRGBA(int x, int y, float *rgba);

		void saveImage(const char* filename);

		float* getDataPointer( int offset );
		int getDataLength( int size );

	protected:
		int _width;
		int _height;
		float *_rgbadData;
		BMP _bmpImage;
	};
	

	class TransferFunction
	{
	public:
		void getValue ( float val, float *rgba);
		int addAnchorPoint ( float val, float *rgba);
		void resetList ();
		void print();

	protected:
		std::vector<float> controlPointList;
		std::vector<Vec4f> controlRGBAList; // Sorted vector.
	};

}

#endif //__IMAGE_H