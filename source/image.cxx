/*	CS 595 Assignment 10
 *	Jie Jiang
 *	
 */

#include "image.h"
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <stdio.h>

using namespace std;
using namespace AJParallelRendering;

ImageIO::ImageIO()
{
	_width = 0;
	_height = 0;
	_rgbadData = NULL;
}

ImageIO::ImageIO(const ImageIO &image)
{
	cout << "copying" << endl;
	initiateData(image._width, image._height, image._rgbadData);
}

ImageIO& ImageIO::operator = (const ImageIO &image)
{
	cout << "assign" << endl;
	initiateData(image._width, image._height, image._rgbadData);
}

ImageIO::~ImageIO()
{
	if (_rgbadData != NULL)
	{
		free( _rgbadData );
	}	
}

void ImageIO::setSize(int width, int height)
{
	if (width == _width && height == height)
		return;
	
	if (width > 0 || height > 0)
	{
		_width = width;
		_height = height;
		if ( _rgbadData == NULL )
		{
			_rgbadData = (float *) malloc ( 5 * width * height * sizeof(float));
		} else {
			if ( sizeof ( _rgbadData ) < 5 * width * height * sizeof(float) )
			{
				free (_rgbadData);
				_rgbadData = (float *) malloc ( 5 * width * height * sizeof(float));
			}
		}
	} else {
		cout << "setSize for image must be > 0" << endl;
	}
}

void ImageIO::setRGBA(int x, int y, float r, float g, float b, float a, float depth)
{
	int index = x + y * _width;
	index *= 5;
	_rgbadData[index] = r;
	_rgbadData[index+1] = g;
	_rgbadData[index+2] = b;
	_rgbadData[index+3] = a;
	_rgbadData[index+4] = depth;
}

void ImageIO::setRGBA(int x, int y, float *rgba)
{
	int index = x + y * _width;
	index *= 5;
	for ( int i = 0; i <= 5; i++)
		_rgbadData[index+i] = rgba[i];
}

void ImageIO::getRGBA(int x, int y, float &r, float &g, float &b, float &a, float &depth)
{
	int index = x + y * _width;
	index *= 5;
	r = _rgbadData[index];
	g = _rgbadData[index+1];
	b = _rgbadData[index+2];
	a = _rgbadData[index+3];
	depth = _rgbadData[index+4];
}

void ImageIO::getRGBA(int x, int y, float *rgba)
{
	int index = x + y * _width;
	index *= 5;
	for ( int i = 0; i <= 5; i++)
		 rgba[i] = _rgbadData[index+i];
}


void ImageIO::initiateData(int width, int height, float *rgbadData)
{
	setSize(width, height);
	if (rgbadData != NULL)
	{
		memcpy(_rgbadData, rgbadData, 5*width*height*sizeof(float));
	} else {
		memset(_rgbadData, 0, 5*width*height*sizeof(float));
	}
}

void ImageIO::saveImage(const char* filename)
{
	_bmpImage.SetBitDepth(8);
	printf ( "size: %d x %d \n", _width, _height);
	_bmpImage.SetSize(_width, _height);
	RGBApixel tmp;
	for ( int i = 0; i < _height; i++)
	{
		for ( int j = 0; j < _width; j++)
		{
			int index = i*_width + j;
			index *= 5;
			tmp.Red = _rgbadData[index] * 255;
			tmp.Green = _rgbadData[index+1] * 255;
			tmp.Blue = _rgbadData[index+2] * 255;
			tmp.Alpha = 0;//(1 - _rgbadData[index+3]) * 255;

			/*tmp.Red = 0.5 * 255;
			tmp.Green = 0.5 * 255;
			tmp.Blue = 0.5 * 255;
			tmp.Alpha = 1;//(1 - _rgbadData[index+3]) * 255;*/
			_bmpImage.SetPixel(j,i,tmp);
		}
	}
	_bmpImage.WriteToFile(filename);
}

float* ImageIO::getDataPointer(int offset )
{
	float *ret;
	ret = _rgbadData;
	ret += 5 * offset;
	return ret;
}

int ImageIO::getDataLength( int size )
{
	return size*5;
}

void TransferFunction::getValue ( float val, float *rgba)
{
	vector<float>::iterator iter;
	vector<Vec4f>::iterator iterRGBA = controlRGBAList.begin();
	for ( iter = controlPointList.begin(); iter != controlPointList.end(); iter++)
	{
		if ((*iter) >= val)
			break;
		iterRGBA++;
	}
	if ( iter == controlPointList.end() )
	{
		cout << "value: " << val << " not in TransferFunction"; //<< endl;
		return;
	}
	if ( (*iter) == val ) // if we found the vale
	{
		(*iterRGBA).getValue(rgba);
		return;
	}
	if ( iter == controlPointList.begin() )
	{
		cout << "value: " << val << " not in TransferFunction" << endl;
		return;
	} else { // we did not find the value bue could interpolate it.
		float anchorR = (*iter);
		iter--;
		float anchorL = (*iter);
		float weightL = ( val - anchorL )/( anchorR - anchorL );
		float weightR = 1 - weightL;

		float rgbaL[4];
		float rgbaR[4];
		(*iterRGBA).getValue(rgbaR);
		iterRGBA--;
		(*iterRGBA).getValue(rgbaL);
		for ( int i = 0; i <= 4; i++)
		{
			rgba[i] = rgbaL[i] * weightL + rgbaR[i] * weightR;
		}
	}
}

int TransferFunction::addAnchorPoint( float val, float *rgba)
{
	vector<float>::iterator iter = controlPointList.begin();
	vector<Vec4f>::iterator iterRGBA = controlRGBAList.begin();
	if ( controlPointList.size() == 0)
	{
		controlPointList.insert(iter, val);
		Vec4f vec4;
		vec4.setValue(rgba);
		controlRGBAList.insert(iterRGBA, vec4);
		return 1;
	}

	for ( iter = controlPointList.begin(); iter != controlPointList.end(); iter++)
	{
		if ((*iter) >= val)
			break;
		iterRGBA++;
	}
	if ( (*iter) == val ) // if we found the vale, change it.
	{
		(*iterRGBA).setValue(rgba);
	} else { // if it's not there. we just insert it
		controlPointList.insert(iter, val);
		Vec4f vec4;
		vec4.setValue(rgba);
		controlRGBAList.insert(iterRGBA, vec4);
	}
	return controlPointList.size(); // return the new size.
}

void TransferFunction::resetList()
{
	controlRGBAList.empty();
	controlPointList.empty();
}

void TransferFunction::print()
{
	for ( int i = 0; i <= controlPointList.size(); i++)
	{
		float val = controlPointList[i];
		float rgba[4];
		controlRGBAList[i].getValue(rgba);
		printf ("value: %f, r: %f, g: %f, b: %f, a: %f\n", val, rgba[0], rgba[1], rgba[2], rgba[3] );
	}

}
