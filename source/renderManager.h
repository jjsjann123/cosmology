/*!	darkSkySim raw data renderManager.h
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

#ifndef __RENDERMANAGER_H
#define __RENDERMANAGER_H

#include "LODvolume.h"
#include "renderer.h"
#include "vec.h"

using namespace std;

namespace AJParallelRendering {

	class RenderManager {
	public:
		RenderManager();
		~RenderManager();
		
		void getVolumeInput(LODVolume *volume); /// accept new volume as output. Delete old volume if possessed.
		void setRenderingParameter(int width, int height, float	pixelWidth, float pixelHeight, float nearPlane, float farPlane, float stepSize); /// setup parameters for the output image.
		void updateCamera(int level, Ray ray, Vec3f cameraUpDir); /// setRendering
		void render();
		void initiateTransferFunction();
		void saveImage(int myrank, int level, const char *ptr);
		ImageIO* getImage();
		void test(int n);

		void setEmissionOn() {_emissionMode = true;};
		void setEmissionOff() {_emissionMode = false;};

	protected:
		LODVolume* _volume;
		Renderer _renderer;

		bool _emissionMode;

	private:

	};

};

#endif //__RENDERMANAGER_H

