#include <iostream>
#include <string.h>
#include <math.h>
#include <opencv/cv.h>
#include <vector>
#include <list>

#include "cluster/Cluster.h"

//#include "countLine.h"
int main(int argc, char** argv)
{

	Cluster(argc, argv);

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



