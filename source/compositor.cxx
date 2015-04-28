#include "compositor.h"
#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define IMAGE_COMPOSITION 0

using namespace AJParallelRendering;
using std::pow;

void Compositor::saveImage(const char* filename)
{
	_image->saveImage(filename);
}

void Compositor::setImage(ImageIO *image)
{
	_image = image;
}

void Compositor::composite(int id, int d)
{
	int dataSize, width, height;
	_image->getSize(width, height);
	dataSize = width*height;
	
	int flag = id - 1; // this will be the id for each node while compositing
	_tempBuffer = (float*) malloc (_image->getDataLength(dataSize/2)*sizeof(float));
	int curDataSize = dataSize;
	if ( id != 0)
	{
		int mask = 1;
		int offset = 0;
		for ( int i = 0; i < d; i++)
		{
			int comm = (flag ^ mask) + 1; // the other node to communicate with.
			curDataSize /= 2; // size of chunk to exchange.
			
			if ( (mask & flag) == 0 )
			{
				//printf ( "node: %d waiting for node %d offset: %d, size: %d\n", id, comm, offset, curDataSize); 
				compositeArray (false, comm, offset, curDataSize);
				//printf ( "node: %d sending to node %d offset: %d, size: %d\n", id, comm, offset, curDataSize);
				compositeArray (true, comm, offset+curDataSize, curDataSize);
			} else {
				//printf ( "before increasing node: %d sending to node %d offset: %d, size: %d\n", id, comm, offset, curDataSize);
				compositeArray (true, comm, offset, curDataSize);
				offset += curDataSize;
				//printf ( "increased node: %d waiting for node %d offset: %d, size: %d\n", id, comm, offset, curDataSize); 
				compositeArray (false, comm, offset, curDataSize);
			}
			mask = mask << 1;
		}
		int bufferLength = _image->getDataLength(curDataSize);
		int offsetSize;
		offsetSize = offset;
		printf ("sending to 0 from: %d", id);
		MPI_Send( &offsetSize, 1, MPI_INT, 0, IMAGE_COMPOSITION, MPI_COMM_WORLD);
		MPI_Send( _image->getDataPointer(offset), bufferLength, MPI_FLOAT, 0, IMAGE_COMPOSITION, MPI_COMM_WORLD);
		printf ("send completed to 0 from: %d", id);
	} else {
		int numOfNodes = pow(2,d);
		MPI_Status status;
		int bufferLength = dataSize*5 / pow(2,d);
		for ( int i = 1; i <= numOfNodes; i++)
		{
			int offset;
			printf ("receiving: %d", i);
			MPI_Recv(&offset, 1, MPI_INT, i, IMAGE_COMPOSITION, MPI_COMM_WORLD, &status );
			MPI_Recv(_image->getDataPointer(offset), bufferLength, MPI_FLOAT, i, IMAGE_COMPOSITION, MPI_COMM_WORLD, &status);
			printf ("received: %d", i);
		}
	}
	free(_tempBuffer);
}

void Compositor::compositeArray( bool send, int commID, int offset, int length)
{
	int bufferLength = _image->getDataLength(length);
	if ( send )
	{
		MPI_Send( _image->getDataPointer(offset), bufferLength, MPI_FLOAT, commID, IMAGE_COMPOSITION, MPI_COMM_WORLD);  
	} else {
		MPI_Status status;
		MPI_Recv( _tempBuffer, bufferLength, MPI_FLOAT, commID, IMAGE_COMPOSITION, MPI_COMM_WORLD, &status);
		float *data;
		data = _image->getDataPointer(offset);
		for (int i = 0; i < length; i++ )
		{
			bool beneath;
			//if ( _direction )
			{
				if ( _tempBuffer[i*5+4] > data[i*5+4] )
				{
					beneath = true;
					//beneath = false;
				} else {
					beneath = false;
					//beneath = true;
				}
//			} else {
//				if ( _tempBuffer[i*5+4] > data[i*5+4] )
//				{
//					beneath = false;
//				} else {
//					beneath = true;
//				}
			}
			if ( beneath )
			{
				if (data[i*5+3] != 0)
				{
					//data[i*5] = data[i*5] + _tempBuffer[i*5] * _tempBuffer[i*5*3] * ( 1- data[i*5+3]);
					//data[i*5+1] = data[i*5+1] + _tempBuffer[i*5+1] * _tempBuffer[i*5+3] * ( 1- data[i*5+3]);
					//data[i*5+2] = data[i*5+2] + _tempBuffer[i*5+2] * _tempBuffer[i*5+3] * ( 1- data[i*5+3]);
					//data[i*5+3] = data[i*5+3] + _tempBuffer[i*5+3] * ( 1 - data[i*5+3]);
					data[i*5] = data[i*5] + _tempBuffer[i*5] * ( 1- data[i*5+3]);
					data[i*5+1] = data[i*5+1] + _tempBuffer[i*5+1] * ( 1- data[i*5+3]);
					data[i*5+2] = data[i*5+2] + _tempBuffer[i*5+2] * ( 1- data[i*5+3]);
					data[i*5+3] = data[i*5+3] + _tempBuffer[i*5+3] * ( 1 - data[i*5+3]);
					//data[i*5+3] = data[i*5+3] + data[i*5+3];
					data[i*5+4] = _tempBuffer[i*5+4];
				} else {
					memcpy ( data+i*5, _tempBuffer+i*5, 4*sizeof(float));
				}
			} else {
				if (_tempBuffer[i*5+3] == 0)
					continue;
				//data[i*5] = _tempBuffer[i*5] + data[i*5] * data[i*5*3] * ( 1- _tempBuffer[i*5+3]);
				//data[i*5+1] = _tempBuffer[i*5+1] + data[i*5+1] * data[i*5+3] * ( 1- _tempBuffer[i*5+3]);
				//data[i*5+2] = _tempBuffer[i*5+2] + data[i*5+2] * data[i*5+3] * ( 1- _tempBuffer[i*5+3]);
				//data[i*5+3] = _tempBuffer[i*5+3] + data[i*5+3] * ( 1 - _tempBuffer[i*5+3]);
				data[i*5] = _tempBuffer[i*5] + data[i*5] * ( 1- _tempBuffer[i*5+3]);
				data[i*5+1] = _tempBuffer[i*5+1] + data[i*5+1] * ( 1- _tempBuffer[i*5+3]);
				data[i*5+2] = _tempBuffer[i*5+2] + data[i*5+2] * ( 1- _tempBuffer[i*5+3]);
				data[i*5+3] = _tempBuffer[i*5+3] + data[i*5+3] * ( 1 - _tempBuffer[i*5+3]);
				//data[i*5+3] = _tempBuffer[i*5+3] + data[i*5+3];
			}
		}
	}
}
