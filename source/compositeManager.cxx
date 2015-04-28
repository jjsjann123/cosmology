
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



#include "compositeManager.h"
#include <cstdlib>
#include <string>
#include <sstream>

using namespace AJParallelRendering;
using namespace std;

void CompositeManager::saveImage(int myrank, int level, const char *ptr)
{
	ostringstream convert;
	string q;
	if ( myrank != 0 ) {
		convert << string(ptr);
		convert << string("_node_");
		convert << myrank;
		convert << string("_level_");
		convert << level;
	} else {
		convert << string(ptr) << "_output";
	}
	q = convert.str();
	q.append(".bmp");

	_compositor.saveImage(q.c_str());
}

void CompositeManager::setImage( ImageIO *image)
{
	_compositor.setImage(image);
}

void CompositeManager::composite( int id, int d)
{
	_compositor.composite(id, d);
}
