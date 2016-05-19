#include <iostream>
#include <string.h>
#include <math.h>
#include <opencv/cv.h>
#include <vector>
#include <list>

#include "kernel/KHistogram/DenseTrack.h"

int main(int argc, char** argv)
{

	// step 1. extract trajectories and compute kernel histograms.
	DenseTrack(argc, argv);
	
	// step 2. select features to compute k-means.提取特征 进行K-means算法
    // SelectFeatures(argc, argv); // 因为不太了解g++ compile 和link 的机制。今晚好好看怎么编译和链接的...现在了解了
		
	// step 3. compute k-means clustering.
	//Cluster(argc, argv);

	// step 4. encode each video to <label> <index1>:<TF1> <index2>:<TF2>
	//EncodeFeatures(argc, argv);
	
	// step 5. compute the x^2 distance of video pairs.
	//ComputeDistance(argc, argv);
	
	// step 6. compute the kernel matrix of video pairs.
	//ComputeKernelMatrix(argc, argv);

	// step 7. train and test with svm.
	//TrainAndTest(argc, argv);
	
	// extra. count the lines of files extracted by step 2.
	//countLine(argc, argv);	
	return 0;
}



