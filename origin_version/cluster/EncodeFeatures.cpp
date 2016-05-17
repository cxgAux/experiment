//--------------------------------begin--------------------------------
//-----------------------------by Yikun Lin----------------------------
#include "EncodeFeatures.h"
char* ReadLine(gzFile gzfp)
{
	int len;
	
	if(gzgets(gzfp, line, max_line_len) == NULL)
		return NULL;

	while(strrchr(line, '\n') == NULL)
	{
	
		max_line_len *= 2;
		line = (char*) realloc(line, max_line_len);
		len = (int) strlen(line);
		if(gzgets(gzfp, line + len, max_line_len - len) == NULL)
			break;
	}
	
	return line;
}


// get the closest codewords
int Codewords(float** pCodeBook, float* pPoint, int nRows, int nCols)
{
	float dist = 0;
	float minDist = 0;
	int minIndex = 0;
	int iRow, iCol;
	for (iCol = 0; iCol < nCols; iCol++)
	{
		float term = pCodeBook[0][iCol] - pPoint[iCol];
		minDist += term * term;
	}
	for (iRow = 1; iRow < nRows; iRow++)
	{
		dist = 0;
		for (iCol = 0; iCol < nCols; iCol++)
		{
			float term = pCodeBook[iRow][iCol] - pPoint[iCol];
			dist += term * term;
			if (dist >= minDist)
			{
				break;
			}
		}
		if (dist < minDist)
		{
			minDist = dist;
			minIndex = iRow;
		}
	}
	return minIndex;
}


bool EncodeFeatures(int argc, char** argv)
{

	// codebooks of trajectory centers
	char* grInFile = "./codewords/grCenters.txt";
	char* ofInFile = "./codewords/ofCenters.txt";
	char* mbInFile = "./codewords/mbCenters.txt";
	FILE* grIn = fopen(grInFile, "r");
	FILE* ofIn = fopen(ofInFile, "r");
	FILE* mbIn = fopen(mbInFile, "r");
	
	float **pGr = new float*[k];
	float **pOf = new float*[k];
	float **pMb = new float*[k];
	int iPoint;
	int iDim;
	float value; // read each entry from files
	for (iPoint = 0; iPoint < k; iPoint++)
	{
		pGr[iPoint] = new float[GR_DI];
	}
	for (iPoint = 0; iPoint < k; iPoint++)
	{
		pOf[iPoint] = new float[OF_DI];
	}
	for (iPoint = 0; iPoint < k; iPoint++)
	{
		pMb[iPoint] = new float[MB_DI];
	}
	
	// get the codebook of KDE-G from the file
	for (iPoint = 0; iPoint < k; iPoint++)
    	{
    		for (iDim = 0; iDim < GR_DI; iDim++)
    		{
    			
    			fscanf(grIn, "%f", &pGr[iPoint][iDim]);
    		}
    	}
    	// get the codebook of KDE-OF from the file
	for (iPoint = 0; iPoint < k; iPoint++)
    	{
    		for (iDim = 0; iDim < OF_DI; iDim++)
    		{
    			
    			fscanf(ofIn, "%f", &pOf[iPoint][iDim]);
    		}
    	}
    	// get the codebook of KDE-MB from the file
	for (iPoint = 0; iPoint < k; iPoint++)
    	{
    		for (iDim = 0; iDim < MB_DI; iDim++)
    		{
    			
    			fscanf(mbIn, "%f", &pMb[iPoint][iDim]);
    		}
    	}
    	
	fclose(grIn);
	fclose(ofIn);
	fclose(mbIn);
	
	
	char* fileName = argv[1];
	char* grOutFile = argv[2];
	char* ofOutFile = argv[3];
	char* mbOutFile = argv[4];
	int label = atoi(argv[5]) + 1;
	gzFile file = gzopen(fileName,"r");
	FILE* grOut = fopen(grOutFile, "a");
	FILE* ofOut = fopen(ofOutFile, "a");
	FILE* mbOut = fopen(mbOutFile, "a");
	float* pGrPoint = new float[GR_DI];
	float* pOfPoint = new float[OF_DI];
	float* pMbPoint = new float[MB_DI];
	// the term frequency 
	int* pGrTF = new int[k];
	int* pOfTF = new int[k];
	int* pMbTF = new int[k];
	// initiate variables
	for (iPoint = 0; iPoint < k; iPoint++)
	{
		pGrTF[iPoint] = 0;
		pOfTF[iPoint] = 0;
		pMbTF[iPoint] = 0;
	} 
	if (!file)
	{
		return false;
	}
	
	int nLines = 0;
	max_line_len = 1024;
	line = Malloc(char, max_line_len);
	// parse each trajectory
	while (ReadLine(file) != NULL)
	{	
		int iToken;
		char* feature;
		/*strtok(line," \t");
		for (iToken = 2; iToken < GR_START; iToken++)
		{
			feature = strtok(NULL," \t");
		}*/
		feature = strtok(line," \t");
		value = atof(feature);
		pGrPoint[0] = value;
		for (iToken = GR_START + 1; iToken < GR_START + GR_DI; iToken++)
		{
			feature = strtok(NULL," \t");
			value = atof(feature);
			pGrPoint[iToken - GR_START] = value;
		}
		
		for (iToken = OF_START; iToken < OF_START + OF_DI; iToken++)
		{
			feature = strtok(NULL," \t");
			value = atof(feature);
			pOfPoint[iToken - OF_START] = value;
		}
		
		for (iToken = MB_START; iToken < MB_START + MB_DI; iToken++)
		{
			feature = strtok(NULL," \t");
			value = atof(feature);
			pMbPoint[iToken - MB_START] = value;
		}
		int codeword = Codewords(pGr, pGrPoint, k, GR_DI);
		pGrTF[codeword]++;
		codeword = Codewords(pOf, pOfPoint, k, OF_DI);	
		pOfTF[codeword]++;
		codeword = Codewords(pMb, pMbPoint, k, MB_DI);
		pMbTF[codeword]++;
		nLines++;
	}
	
	fprintf(grOut, "%d\t", label);
	fprintf(ofOut, "%d\t", label);
	fprintf(mbOut, "%d\t", label);
	
	// output term frequency to files
	for (iPoint = 0; iPoint < k; iPoint++)
	{
		if (pGrTF[iPoint] > 0)
		{
			fprintf(grOut, "%d:%d\t", iPoint, pGrTF[iPoint]);
		}
		if (pOfTF[iPoint] > 0)
		{
			fprintf(ofOut, "%d:%d\t", iPoint, pOfTF[iPoint]);
		}
		if (pMbTF[iPoint] > 0)
		{
			fprintf(mbOut, "%d:%d\t", iPoint, pMbTF[iPoint]);
		}
		
	}
	fprintf(grOut, "\n");
	fprintf(ofOut, "\n");
	fprintf(mbOut, "\n");

	gzclose(file);
	fclose(grOut);
	fclose(ofOut);
	fclose(mbOut);
	
	// free memory
	free(line);
	delete[] pGrPoint;
	delete[] pOfPoint;
	delete[] pMbPoint;
	delete[] pGrTF;
	delete[] pOfTF;
	delete[] pMbTF;
	
	for (iPoint = 0; iPoint < k; iPoint++)
	{
		delete[] pGr[iPoint];
	}
	delete[] pGr;
	for (iPoint = 0; iPoint < k; iPoint++)
	{
		delete[] pOf[iPoint];
	}
	delete[] pOf;
	for (iPoint = 0; iPoint < k; iPoint++)
	{
		delete[] pMb[iPoint];
	}
	delete[] pMb;
	return true;
}
//---------------------------------end---------------------------------
