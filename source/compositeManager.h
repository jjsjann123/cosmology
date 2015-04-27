/*!	darkSkySim compositeManager.cxx
 * 
 * 	This  compositeManager will handle: 
 *		1. composite rendered image from each PE.
 *		2. distribute image to specific displaying node.
 * 
 *	Author: Jie Jiang
 *	Date:	2015 Spring
 *	Class:	CS595 High Performance Simulation
 *	Project:DarkSkySimViz ( Partly from Scivis 2015 Contest )	
 */

#ifndef __COMPOSITEMANAGER_H
#define __COMPOSITEMANAGER_H

#include <mpi.h>
#include <vector>
#include <string>

#include "compositor.h"

namespace AJParallelRendering {

	/*!
	 *    Class compositeManager is used to:
	 *    	1. composite rendered images using binary swap via class compositor
	 *    	2. distribute sub image to each displaying nodes
	 *
	 */
	class CompositeManager
	{
	public:
		void setImage ( ImageIO *image );
		void composite( int id, int d);
		void saveImage( int id, int level, const char* filename);
	protected:
		Compositor	_compositor;

	private:

	};
};

#endif //__COMPOSITEMANAGER_H
