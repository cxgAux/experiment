//--------------------------------begin--------------------------------
//-----------------------------by Yikun Lin----------------------------
#include "ComputeKernelMatrix.h"
bool ComputeKernelMatrix(int argc, char** argv)
{
	assert(argc == 3);
	char* mbInFile = argv[1];
	char* mbOutFile = argv[2];
	FILE* mbIn = fopen(mbInFile, "r");
	FILE* mbOut = fopen(mbOutFile, "w");
	
	float averageMb;
	int nTrains;
	int nTests;
	
	// read the average distance of three descriptors
	fscanf(mbIn, "%f", &averageMb);
	
	// read the number of training and testing videos
	fscanf(mbIn, "%d", &nTrains);
	fscanf(mbIn, "%d", &nTests);
	fprintf(mbOut, "%d %d\n", nTrains, nTests);

	for (int iRow = 0; iRow < nTrains + nTests; iRow++)
	{
		int label;
		int id;
		// read and write the class label and id of video
		fscanf(mbIn, "%d", &label);
		fscanf(mbIn, "%d", &id);
		fprintf(mbOut, "%d 0:%d ", label, id);
		
		for (int iCol = 0; iCol < nTrains; iCol++)
		{
			float mbValue;
			fscanf(mbIn, "%f", &mbValue);
			mbValue /= averageMb;
			mbValue = exp(-mbValue);
			fprintf(mbOut, "%d:%f ", iCol + 1, mbValue);
		}
		fprintf(mbOut, "\n");
	}
	
	fclose(mbIn);
	fclose(mbOut);
}
//---------------------------------end---------------------------------
