//--------------------------------begin--------------------------------
//-----------------------------by Yikun Lin----------------------------
#include "Cluster.h"
bool Cluster(int argc, char** argv)
{
	FILE* srcFile = fopen(argv[1],"r"); //source , destination , dim...
	FILE* destFile = fopen(argv[2],"w");
	int dim = atoi(argv[3]); // dimension: 96 for gr, 108 for of, 192 for mb

    int k = 4000; // number of centers 聚类中心有K个
	int maxPts = 100063; // max number of data points 最多多少个点
	int stages = 100; // number of stages
	float *ptr;
	cv::Mat points(maxPts, dim, CV_32FC1), labels;
	cv::Mat centers(k, dim, points.type());
	
	// input the points from the file
	for (int iPoint = 0; iPoint < points.rows; iPoint++)
    	{
    		for (int iDim = 0; iDim < points.cols; iDim++)
    		{
    			//ptr = (float*)points.data + iPoint * points.cols + iDim;
    			ptr = (float *)(points.data + iPoint * points.step[0] + iDim * points.step[1]);  
    			fscanf(srcFile, "%f", ptr);
    		}
    		
    	}
    	
	kmeans(points, k, labels, 
		cv::TermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, stages, 0.1),
		3, cv::KMEANS_PP_CENTERS, centers);
	
	// output the codewords to file 
	for (int iPoint = 0; iPoint < centers.rows; iPoint++)
    	{
    		for (int iDim = 0; iDim < centers.cols; iDim++)
    		{
    			//ptr = (float*)centers.data + iPoint * centers.cols + iDim;
    			ptr = (float *)(centers.data + iPoint * centers.step[0] + iDim * centers.step[1]);  
    			fprintf(destFile, "%f\t", *ptr);
    		}
    		fprintf(destFile, "\n");
    	}
    	
    	fclose(srcFile);
    	fclose(destFile);
	return true;
}
//---------------------------------end---------------------------------
