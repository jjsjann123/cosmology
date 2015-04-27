/*!
 *
 *	CS 595 Assignment 10
 *	Jie Jiang
 *	
 */

#ifndef __COMPOSITOR_H
#define __COMPOSITOR_H

#include "renderer.h"

namespace AJParallelRendering {

	
	class Compositor
	{
	public:
		void setImage(ImageIO *image);
		void setDirection(bool direction) { _direction = direction; };	/// deprecated

		void composite(int id, int d);

		void saveImage(const char* filename);
	protected:
		void compositeArray ( bool send, int commID, int offset, int length);
		ImageIO *_image;
		bool _direction; /// deprecated
		
	private:
		float* _tempBuffer;
		
	};
	
};

#endif // __COMPOSITOR_H
