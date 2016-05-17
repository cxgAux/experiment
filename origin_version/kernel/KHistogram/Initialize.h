#ifndef INITIALIZE_H_
#define INITIALIZE_H_

#include "DenseTrack.h"

void InitTrackerInfo(TrackerInfo* tracker, int track_length, int init_gap)
{
	tracker->trackLength = track_length;
	tracker->initGap = init_gap;
}

DescMat* InitDescMat(int height, int width, int nBins)
{
	DescMat* descMat = (DescMat*)malloc(sizeof(DescMat));
	descMat->height = height;
	descMat->width = width;
	descMat->nBins = nBins;
	descMat->desc = (float*)malloc(height*width*nBins*sizeof(float));
	memset( descMat->desc, 0, height*width*nBins*sizeof(float));
	return descMat;
}

void ReleDescMat( DescMat* descMat)
{
	free(descMat->desc);
	free(descMat);
}

void InitDescInfo(DescInfo* descInfo, int nBins, int flag, int orientation, int size, int nxy_cell, int nt_cell)
{
	descInfo->nBins = nBins;
	descInfo->fullOrientation = orientation;
	descInfo->norm = 2;
	descInfo->threshold = min_flow;
	descInfo->flagThre = flag;
	descInfo->nxCells = nxy_cell;
	descInfo->nyCells = nxy_cell;
	descInfo->ntCells = nt_cell;
	descInfo->dim = descInfo->nBins*descInfo->nxCells*descInfo->nyCells;
	descInfo->blockHeight = size;
	descInfo->blockWidth = size;
}

//--------------------------------begin--------------------------------
//-----------------------------by Yikun Lin----------------------------
void InitKernelMatrix(CvMat* kernelMatrix, float kernelParam)
{
	int nDense = kernelMatrix->height;
	int nBins = kernelMatrix->width;
	float denseBase = (2 * M_PI) / float(nDense);
	float binBase = (2 * M_PI) / float(nBins);
	for (int iRow = 0; iRow < nDense; iRow++)
	{
		float* ptr = (float*)(kernelMatrix->data.ptr + iRow * kernelMatrix->step);
		for (int iCol = 0; iCol < nBins; iCol++)
		{
			float diff1 = cos(iRow * denseBase) - cos(iCol * binBase);
			float diff2 = sin(iRow * denseBase) - sin(iCol * binBase);
			ptr[iCol] = exp(-kernelParam * (pow(diff1, 2) + pow(diff2, 2)));
			
		}

	}
	
}
//---------------------------------end---------------------------

void usage()
{
	fprintf(stderr, "Extract dense trajectories from a video\n\n");
	fprintf(stderr, "Usage: DenseTrack video_file [options]\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -h                        Display this message and exit\n");
	fprintf(stderr, "  -S [start frame]          The start frame to compute feature (default: S=0 frame)\n");
	fprintf(stderr, "  -E [end frame]            The end frame for feature computing (default: E=last frame)\n");
	fprintf(stderr, "  -L [trajectory length]    The length of the trajectory (default: L=15 frames)\n");
	fprintf(stderr, "  -W [sampling stride]      The stride for dense sampling feature points (default: W=5 pixels)\n");
	fprintf(stderr, "  -N [neighborhood size]    The neighborhood size for computing the descriptor (default: N=32 pixels)\n");
	fprintf(stderr, "  -s [spatial cells]        The number of cells in the nxy axis (default: nxy=2 cells)\n");
	fprintf(stderr, "  -t [temporal cells]       The number of cells in the nt axis (default: nt=3 cells)\n");
}

void arg_parse(int argc, char** argv)
{
	int c;
	char* executable = basename(argv[0]);
	while((c = getopt (argc, argv, "hS:E:L:W:N:s:t:")) != -1)
	switch(c) {
		case 'S':
		start_frame = atoi(optarg);
		break;
		case 'E':
		end_frame = atoi(optarg);
		break;
		case 'L':
		track_length = atoi(optarg);
		break;
		case 'W':
		min_distance = atoi(optarg);
		break;
		case 'N':
		patch_size = atoi(optarg);
		break;
		case 's':
		nxy_cell = atoi(optarg);
		break;
		case 't':
		nt_cell = atoi(optarg);
		break;

		case 'h':
		usage();
		exit(0);
		break;

		default:
		fprintf(stderr, "error parsing arguments at -%c\n  Try '%s -h' for help.", c, executable );
		abort();
	}
}

#endif /*INITIALIZE_H_*/
