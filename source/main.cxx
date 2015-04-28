/*
 *	CS 595 Assignment 10
 *	Jie Jiang
 *	
 */
#include <mpi.h>
#include <stdlib.h>
#include <time.h>
#include <cmath>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <fstream>
#include <cmath>
#include <vector>
#include <cstring>

//#include "volume.h"
//#include "renderer.h"
//#include "compositor.h"
#include "reader.h"
#include "dataManager.h"
#include "renderManager.h"
#include "compositeManager.h"
#include <dirent.h>
#include <vector>
//using AJParallelRendering::Volume;
//using AJParallelRendering::Renderer;
//using AJParallelRendering::Compositor;
//using AJParallelRendering::SDFReader;
//using AJParallelRendering::DataManager;
//using AJParallelRendering::Vec3i;
using namespace AJParallelRendering;

#define FILENAME_DISTRIBUTION 1

using namespace std;

int main(int argc, char** argv)
{
	int myrank;
	int npes;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &npes);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

	char *fileDir;
	char *filePattern;
	fileDir = argv[1];
	filePattern = argv[2];

	if ( argc != 23 )
	{
		cout << "usage ..." << endl;
		cout << "./demo [data_dir] [data_file_pattern] [gridX] [gridY] [gridZ] [imageWidth] [imageHeight] [cameraX] [cameraY] [cameraZ] [cameraDirX] [cameraDirY] [cameraDirZ] [cameraUpDirX] [cameraUpDirY] [cameraUpDirZ] [nearPlane] [farPlane] [level] [imageName] [ifuseemission] [imageResolution]" << endl;
		MPI_Finalize();
		return -1;
	}

	int gridX = atoi(argv[3]);
	int gridY = atoi(argv[4]);
	int gridZ = atoi(argv[5]);
	int imgWidth = atoi(argv[6]);
	int imgHeight = atoi(argv[7]);
	float cameraPosX = atof(argv[8]);
	float cameraPosY = atof(argv[9]);
	float cameraPosZ = atof(argv[10]);
	float cameraDirX = atof(argv[11]);
	float cameraDirY = atof(argv[12]);
	float cameraDirZ = atof(argv[13]);
	float cameraUpDirX = atof(argv[14]);
	float cameraUpDirY = atof(argv[15]);
	float cameraUpDirZ = atof(argv[16]);
	float nearPlane = atof(argv[17]);
	float farPlane = atof(argv[18]);
	int level = atoi(argv[19]);
	int emission = atoi(argv[21]);
	float imgResolution = atoi(argv[22]);

	int d = 0;
	for ( int i = 0; pow(2, i) <= npes - 1; i++)
	{
		if ( pow(2, i) == npes - 1)
		{
			d = i;
			break;
		}
	}
	if ( d == 0 )
	{
		if ( myrank == 0)
		{
			cout << "npes number of proper. Use a npes = pow(2,i)+1" << endl;
		}
		return 1;
	}

	if (myrank == 0)
	{
		vector<string> fileList;

		DIR *dir;
		struct dirent *ent;
		if ((dir = opendir(argv[1])) != NULL ) {
			string file;
			while ((ent = readdir(dir)) != NULL) {
				file = ent->d_name;
				//cout << file << endl;
				size_t found = file.find(filePattern);
				if ( found != string::npos) {
					file.insert(0,"/");
					file.insert(0,fileDir);
					fileList.push_back(file);
				}
			}
			closedir(dir);
		} else {
			perror("");
			return -1;
		}

		for ( int i = 0; i < fileList.size(); i++) {
			cout << i << "   " << fileList[i] << endl;
		}

		{
			char buffer[1000];

			int offset = 0;
			int nOfFile = fileList.size() / (npes-1);
			//cout << fileList.size() << "     " << nOfFile << "      " << npes << endl;
			for ( int i = 1; i < npes; i++) {
				if ( i == npes -1) {
					nOfFile += fileList.size() % (npes-1);
				}
				int index = 0;

				// get the filename string into buffer
				for ( int j = offset; j < offset + nOfFile; j++) {
					strcpy(buffer+index, fileList[j].c_str());
					index += strlen( fileList[j].c_str() );
					index += 1;
					//printf(" %d: index is %d \n" , i, index);
				}
				offset += nOfFile;
				index -= 1;
				int length[2];
				length[0] = index;
				length[1] = nOfFile;
				MPI_Send (&length, 2, MPI_INT, i, FILENAME_DISTRIBUTION, MPI_COMM_WORLD );
				MPI_Send (buffer, index, MPI_INT, i, FILENAME_DISTRIBUTION, MPI_COMM_WORLD );
			}
		}
		MPI_Barrier(MPI_COMM_WORLD);	// barrier_1;
		MPI_Barrier(MPI_COMM_WORLD);	// barrier_2;


		RenderManager myRenderManager;
		myRenderManager.setRenderingParameter(imgWidth, imgHeight, 500.0, 500.0, 20.0, 50000.0, 500.0);
		//myRenderManager.setRenderingParameter(11, 11, 10.0, 10.0, 20.0, 6000.0, 30.0);
		//cameraUp.setValue(1, 0, 0);

		CompositeManager myCompositeManager;
		myCompositeManager.setImage( myRenderManager.getImage() );
		myCompositeManager.composite(myrank, d);
		myCompositeManager.saveImage(myrank, level, argv[20]);
		MPI_Barrier(MPI_COMM_WORLD);	// barrier_3;
	} else {
		DataManager myDataManager;
		MPI_Status status;
		{
			int length[2];
			char buffer[1000];
			MPI_Recv (&length, 2, MPI_INT, 0, FILENAME_DISTRIBUTION, MPI_COMM_WORLD, &status);
			MPI_Recv (buffer, length[0], MPI_INT, 0, FILENAME_DISTRIBUTION, MPI_COMM_WORLD, &status);
			printf ("processor: %d, files: %d, length: %d;", myrank, length[1], length[0]);
			myDataManager.setFileList(buffer, length[1]);
			myDataManager.findLocalBoundary();
			//myDataManager.testPrintVolume(myrank);
			MPI_Barrier(MPI_COMM_WORLD);	// barrier_1;
			float minDim = myDataManager.syncGlobalBoundary(d, 1, myrank);

			float step = max(gridX, gridY);
			step = step > gridZ ? step : gridZ;
			step = minDim / step / 2;
			float pixelSize = step / imgResolution;

			MPI_Barrier(MPI_COMM_WORLD);	// barrier_2;

			Vec3i volumeGrid;
			//volumeGrid.setValue(512, 512, 512);
			//volumeGrid.setValue(128, 128, 128);
			volumeGrid.setValue(gridX, gridY, gridZ);
			myDataManager.particle2Volume(3, volumeGrid);
//myDataManager.testPrintVolume(myrank);
			//myDataManager.accumulateGlobalVolume(d, 1, myrank);
//myDataManager.testPrintVolume(myrank);
			//myDataManager.normalizeVolume();	// normalize volume
			myDataManager.testPrintVolume(myrank);
			myDataManager.shrinkToLocalVolume(d, 1, myrank);

			RenderManager myRenderManager;
			myRenderManager.getVolumeInput( myDataManager.getVolumeOutput() );

			if ( emission == 0)
				myRenderManager.setEmissionOff();
			else
				myRenderManager.setEmissionOn();

			myRenderManager.initiateTransferFunction();
			myRenderManager.setRenderingParameter(imgWidth, imgHeight, pixelSize, pixelSize, nearPlane, farPlane, step);
			//myRenderManager.setRenderingParameter(11, 11, 10.0, 10.0, 20.0, 6000.0, 30.0);

			//if (myrank == 1) {
			Ray testRay;
			//testRay.origin.setValue(-3000, -50000, -50000);
			testRay.origin.setValue(cameraPosX, cameraPosY, cameraPosZ);
			//testRay.origin.setValue(0, 0, 0);
			//testRay.dir.setValue(0.7, 0.7, 0.7);
			testRay.dir.setValue(cameraDirX, cameraDirY, cameraDirZ);
			//testRay.dir.setValue(0.6, 1, 1);
			Vec3f cameraUp;
			cameraUp.setValue(cameraUpDirX, cameraUpDirY, cameraUpDirZ);
			//cameraUp.setValue(1, 0, 0);
			myRenderManager.updateCamera(level, testRay, cameraUp);
			myRenderManager.render();

			string q(argv[20]);
			myRenderManager.saveImage(myrank, level, q.c_str());
			q.append("_comp");
			CompositeManager myCompositeManager;
			myCompositeManager.setImage( myRenderManager.getImage() );
			myCompositeManager.composite(myrank, d);
			myCompositeManager.saveImage(myrank, level, q.c_str());
			//}
			//myRenderManager.test(myrank);


			MPI_Barrier(MPI_COMM_WORLD);	// barrier_3;
// This looks weird
/*
			if ( myrank == 1)
			{
				//float val = myRenderManager._volume->getVolumeOfDetail(0, -559.72f, -35.701f, -2151.4f);
				//printf ( "%.5g\n", val);
				//val = myRenderManager._volume->getVolumeOfDetail(1, -559.72f, -35.701f, -2151.4f);
				//printf ( "%.5g\n", val);
				//val = myRenderManager._volume->getVolumeOfDetail(0, 4, 56, 126);
				//printf ( "%.5g\n", val);
				//val = myRenderManager._volume->getVolumeOfDetail(1, 2, 28, 63);
				//printf ( "%.5g\n", val);
				DataManager hisDataManager;
				hisDataManager.setFileList(buffer, length[1]);
				hisDataManager.findLocalBoundary();
				volumeGrid.setValue(128, 128, 128);
				hisDataManager.particle2Volume(3, volumeGrid);
				hisDataManager.testPrintVolume(50);
				hisDataManager.normalizeVolume();	// normalize volume
				hisDataManager.testPrintVolume(50);
				printf("50 1.*************data: %.5f\n", hisDataManager._volume->getVolume(20, 20, 20));
				hisDataManager._volume->subVolume(0, 0, 0, 128, 128, 128);
				printf("50 2.*************data: %.5f\n", hisDataManager._volume->getVolume(20, 20, 20));
				RenderManager hisRenderManager;
				hisRenderManager.getVolumeInput( hisDataManager.getVolumeOutput() );
				hisRenderManager.initiateTransferFunction();
				hisRenderManager.setRenderingParameter(256, 256, 500.0, 500.0, 20.0, 100000.0, 500.0);
				//myRenderManager.setRenderingParameter(11, 11, 10.0, 10.0, 20.0, 6000.0, 30.0);

				//if (myrank == 1) {
				Ray testRay;
				testRay.origin.setValue(0, 0, -50000);
				//testRay.origin.setValue(0, 0, 0);
				testRay.dir.setValue(0, 0, 1);
				Vec3f cameraUp;
				cameraUp.setValue(0, 1, 0);
				//cameraUp.setValue(1, 0, 0);
				int level = 0;
				hisRenderManager.updateCamera(level, testRay, cameraUp);
				hisRenderManager.render();
				hisRenderManager.saveImage(50, level, "compare");
				//level = 1;
				//hisRenderManager.updateCamera(level, testRay, cameraUp);
				//hisRenderManager.render();
				//hisRenderManager.saveImage(50, level, "compare");
			}
			if ( myrank == 2)
			{
				//float val = myRenderManager._volume->getVolumeOfDetail(0, -559.72f, -35.701f, -2151.4f);
				//printf ( "%.5g\n", val);
				//val = myRenderManager._volume->getVolumeOfDetail(1, -559.72f, -35.701f, -2151.4f);
				//printf ( "%.5g\n", val);
				//val = myRenderManager._volume->getVolumeOfDetail(0, 4, 56, 126);
				//printf ( "%.5g\n", val);
				//val = myRenderManager._volume->getVolumeOfDetail(1, 2, 28, 63);
				//printf ( "%.5g\n", val);
				DataManager hisDataManager;
				hisDataManager.setFileList(buffer, length[1]);
				hisDataManager.findLocalBoundary();
				volumeGrid.setValue(128, 128, 128);
				hisDataManager.particle2Volume(3, volumeGrid);
				hisDataManager.testPrintVolume(51);
				hisDataManager.normalizeVolume();	// normalize volume
				hisDataManager.testPrintVolume(51);
				printf("51 1.*************data: %.5f\n", hisDataManager._volume->getVolume(20, 20, 20));
				hisDataManager._volume->subVolume(0, 0, 0, 64, 64, 128);
				hisDataManager.testPrintVolume(51);
				printf("51 2.*************data: %.5f\n", hisDataManager._volume->getVolume(20, 20, 20));
				//hisDataManager._volume->_volumeOrigin.setValue(0,0,0);

				RenderManager hisRenderManager;
				hisRenderManager.getVolumeInput( hisDataManager.getVolumeOutput() );
				hisRenderManager.initiateTransferFunction();
				hisRenderManager.setRenderingParameter(256, 256, 500.0, 500.0, 20.0, 100000.0, 500.0);
				//myRenderManager.setRenderingParameter(11, 11, 10.0, 10.0, 20.0, 6000.0, 30.0);

				//if (myrank == 1) {
				Ray testRay;
				testRay.origin.setValue(0, 0, -50000);
				//testRay.origin.setValue(0, 0, 0);
				testRay.dir.setValue(0, 0, 1);
				Vec3f cameraUp;
				cameraUp.setValue(0, 1, 0);
				//cameraUp.setValue(1, 0, 0);
				int level = 0;
				hisRenderManager.updateCamera(level, testRay, cameraUp);
				hisRenderManager.render();
				hisRenderManager.saveImage(51, level, "compare");

			}
			if ( myrank == 3)
			{
				//float val = myRenderManager._volume->getVolumeOfDetail(0, -559.72f, -35.701f, -2151.4f);
				//printf ( "%.5g\n", val);
				//val = myRenderManager._volume->getVolumeOfDetail(1, -559.72f, -35.701f, -2151.4f);
				//printf ( "%.5g\n", val);
				//val = myRenderManager._volume->getVolumeOfDetail(0, 4, 56, 126);
				//printf ( "%.5g\n", val);
				//val = myRenderManager._volume->getVolumeOfDetail(1, 2, 28, 63);
				//printf ( "%.5g\n", val);
				DataManager hisDataManager;
				hisDataManager.setFileList(buffer, length[1]);
				hisDataManager.findLocalBoundary();
				volumeGrid.setValue(128, 128, 128);
				hisDataManager.particle2Volume(3, volumeGrid);
				hisDataManager.testPrintVolume(52);
				hisDataManager.normalizeVolume();	// normalize volume
				hisDataManager.testPrintVolume(52);

				float *testData = (float*)malloc(64*64*128*sizeof(float));
				hisDataManager._volume->createSubVolume(0, 64, 0, 64, 64, 128, testData);
				hisDataManager._volume->setAllVolume(64, 64, 128, testData);
				hisDataManager._volume->_volumeOrigin.setValue(0,64,0);
				free(testData);
				printf("52 *************data: %.5f\n", hisDataManager._volume->getVolume(20, 20, 20));
				hisDataManager.testPrintVolume(52);
				//hisDataManager._volume->_volumeOrigin.setValue(0,0,0);

				RenderManager hisRenderManager;
				hisRenderManager.getVolumeInput( hisDataManager.getVolumeOutput() );
				hisRenderManager.initiateTransferFunction();
				hisRenderManager.setRenderingParameter(256, 256, 500.0, 500.0, 20.0, 100000.0, 500.0);
				//myRenderManager.setRenderingParameter(11, 11, 10.0, 10.0, 20.0, 6000.0, 30.0);

				//if (myrank == 1) {
				Ray testRay;
				testRay.origin.setValue(0, 0, -50000);
				//testRay.origin.setValue(0, 0, 0);
				testRay.dir.setValue(0, 0, 1);
				Vec3f cameraUp;
				cameraUp.setValue(0, 1, 0);
				//cameraUp.setValue(1, 0, 0);
				int level = 0;
				hisRenderManager.updateCamera(level, testRay, cameraUp);
				hisRenderManager.render();
				hisRenderManager.saveImage(52, level, "compare");

			}

			*/
		}
	}
	MPI_Finalize();

}

