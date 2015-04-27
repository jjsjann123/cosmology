/*!	darkSkySim raw data renderManager.cxx
 * 
 * 	This data manager will handle: 
 *		1. render data from different perspective.
 *		2. render data at different level of detail.
 * 
 *	Author: Jie Jiang
 *	Date:	2015 Spring
 *	Class:	CS595 High Performance Simulation
 *	Project:DarkSkySimViz ( Partly from Scivis 2015 Contest )	
 */

#include "renderManager.h"
#include <sstream>

#include <cstdlib>

using namespace AJParallelRendering;
using namespace std;


RenderManager::RenderManager()
{
	setEmissionOff();
	_volume = NULL;
}

RenderManager::~RenderManager() 
{
	if (_volume != NULL)
	{
		delete(_volume);
	}
}

void RenderManager::getVolumeInput(LODVolume *volume)
{
	if ( _volume != NULL )
	{
		delete(_volume);
	}
	_volume = volume;
	_renderer.setVolume(_volume);
}

void RenderManager::test(int n)
{
	Ray testRay;
	testRay.origin.setValue(100, 200, -3300);
	//testRay.dir.setValue(0.1, 0.1, .8);
	testRay.dir.setValue(0, 0, 1);
	testRay.dir.normalize();
	
	float hit0, hit1;
	if (_volume->intercept(testRay, 0.1, 2000, hit0, hit1))
	{
		printf ( "PE %d range: %f, %f\n", n, hit0, hit1 );
	} else {
		printf ( "PE %d not hit", n);
	}
}

void RenderManager::setRenderingParameter(int width, int height, 
				float pixelWidth, float pixelHeight, float nearPlane, float farPlane,
				float stepSize)
{
	_renderer.setResolution(width, height, pixelWidth, pixelHeight);
	_renderer.setClippingPlane(nearPlane, farPlane, stepSize);
	_renderer.updateMetaInfo();
}

void RenderManager::updateCamera(int level, Ray ray, Vec3f cameraUpDir)
{
	_renderer.setLevel(level);
	_renderer.updateRay(ray, cameraUpDir);
}

void RenderManager::render()
{
	//_volume->test();	// print debug information
	if (_emissionMode )
		_renderer.setEmissionOn();
	else
		_renderer.setEmissionOff();
	_renderer.render();
}

void RenderManager::saveImage(int myrank, int level, const char *ptr)
{
	ostringstream convert;
	convert << level;
	convert << string("_level_");
	convert << myrank;
	convert << string("_");
	convert << string(ptr);
	string q = convert.str();
	q.append(".bmp");
	
	_renderer.saveImage(q.c_str());

}

void RenderManager::initiateTransferFunction()
{

	if ( _emissionMode )
	{
		_renderer.addTransferFunction(0, 0, 0, 0, 0);
		_renderer.addTransferFunction(0.5, 0, 0.2, 0.2, 0.0);
		_renderer.addTransferFunction(1, 0.05, 0.12, 0.02, 0.2);
		_renderer.addTransferFunction(2, 0.3, 0.2, 0.5, 0.2);
		_renderer.addTransferFunction(20, 0.7, 0, 0.6, 0.3);
		_renderer.addTransferFunction(100, 1, 0.0, 0.0, 1);
		_renderer.addTransferFunction(1000, 0.6, 0.2, 0.1, 0.3);
		_renderer.addTransferFunction(10000, 1, 1, 1, 1);
	} else {
		_renderer.addTransferFunction(0, 0, 0, 0, 0);
		_renderer.addTransferFunction(0.5, 0, 0.2, 0.2, 0.0);
		_renderer.addTransferFunction(1, 0.5, 0.2, 0.2, 0.04);
		_renderer.addTransferFunction(100, 1, 0.6, 0.6, 0.06);
		_renderer.addTransferFunction(1000, 0.6, 0.2, 0.1, 0.3);
		_renderer.addTransferFunction(10000, 1, 1, 1, 1);
	}

/*
	_renderer.addTransferFunction(0, 0, 0, 0, 0.0);
	_renderer.addTransferFunction(0.01, 0, 1, 0, 0.0);
	_renderer.addTransferFunction(0.02, 0.5, 0.4, 0, 0.3);
	_renderer.addTransferFunction(0.1, 1, 0, 0.5, 0.3);
	_renderer.addTransferFunction(0.2, 0, 1, 0.5, 0.3);
	_renderer.addTransferFunction(0.5, 1, 0.5, 0.5, 0.3);
	_renderer.addTransferFunction(0.7, 1, 1, 1, 0.5);
	_renderer.addTransferFunction(1, 1, 1, 1, 1);
*/

}

ImageIO* RenderManager::getImage()
{
	return _renderer.getImage();
}
