#include <iostream>
#include <string.h>
#include <math.h>
#include <opencv/cv.h>
#include <vector>
#include <list>
#include "classification/ComputeDistance.h"
//#include "classification/ComputeKernelMatrix.h"
//#include "classification/TrainAndTest.h"

//#include "countLine.h"
int main(int argc, char** argv)
{

	ComputeDistance(argc, argv);
	
	// step 6. compute the kernel matrix of video pairs.
	//ComputeKernelMatrix(argc, argv);

	// step 7. train and test with svm.
	//TrainAndTest(argc, argv);
	
	// extra. count the lines of files extracted by step 2.
	//countLine(argc, argv);	
	return 0;
}



