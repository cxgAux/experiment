#ifndef DENSETRACK_H_
#define DENSETRACK_H_

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <ctype.h>
#include <unistd.h>

#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <list>
#include <string>

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>


// parameters for descriptors
static int patch_size = 32;
static int nxy_cell = 2;
static int nt_cell = 3;
static bool fullOrientation = true;
static float epsilon = 0.05;
const float min_flow = 0.4*0.4;
//const float PI = 3.14159;

// parameters for tracking
static int start_frame = 0;
static int end_frame = 1000000;
static double quality = 0.001;
static double min_distance = 5;
static int init_gap = 1;
static int track_length = 15;

// parameters for the trajectory descriptor
const float min_var = sqrt(3);
const float max_var = 50;
const float max_dis = 20;
//const int max_points = 100;

// parameters for multi-scale
static int scale_num = 8;
const float scale_stride = sqrt(2);

typedef struct TrackerInfo
{
    int trackLength; // length of the trajectory
    int initGap; // initial gap for feature detection
}TrackerInfo;

typedef struct DescInfo
{
    int nBins; // number of bins for vector quantization
    int fullOrientation; // 0: 180 degree; 1: 360 degree
    int norm; // 1: L1 normalization; 2: L2 normalization
    float threshold; //threshold for normalization
	int flagThre; // whether thresholding or not
    int nxCells; // number of cells in x direction
    int nyCells;
    int ntCells;
	int dim; // dimension of the descriptor
    int blockHeight; // size of the block for computing the descriptor
    int blockWidth;
}DescInfo;

typedef struct DescMat
{
    int height;
    int width;
    int nBins;
    float* desc;
}DescMat;

class PointDesc
{
public:
    std::vector<float> hog;
    std::vector<float> hof;
    std::vector<float> mbhX;
    std::vector<float> mbhY;
    CvPoint2D32f point;
    float saliency; // the saliency value
    float averageSaliency;

    PointDesc(const DescInfo& hogInfo, const DescInfo& hofInfo, const DescInfo& mbhInfo, const CvPoint2D32f& point_, float saliency_, float averageSaliency_)
        : hog(hogInfo.nxCells * hogInfo.nyCells * hogInfo.nBins),
        hof(hofInfo.nxCells * hofInfo.nyCells * hofInfo.nBins),
        mbhX(mbhInfo.nxCells * mbhInfo.nyCells * mbhInfo.nBins),
		mbhY(mbhInfo.nxCells * mbhInfo.nyCells * mbhInfo.nBins),
        point(point_), saliency(saliency_), averageSaliency(averageSaliency_)
    {}
};

class Track
{
public:
    std::list<PointDesc> pointDescs;
    int maxNPoints;

    Track(int maxNPoints_)
        : maxNPoints(maxNPoints_)
    {}

    void addPointDesc(const PointDesc& point)
    {
        pointDescs.push_back(point);
        if (pointDescs.size() > maxNPoints + 2) {
            pointDescs.pop_front();
		}
    }
};
bool DenseTrack(int argc, char** argv);
#endif /*DENSETRACK_H_*/
