#include <iostream>
#include <string.h>
#include <math.h>
#include <opencv/cv.h>
#include <vector>
#include <list>

#include "classification/TrainAndTest.h"

//#include "countLine.h"
int main(int argc, char** argv)
{

	
	TrainAndTest(argc, argv);
	
	// extra. count the lines of files extracted by step 2.
	//countLine(argc, argv);	
	return 0;
}



