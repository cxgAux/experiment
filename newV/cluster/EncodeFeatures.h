//--------------------------------begin--------------------------------
//-----------------------------by Yikun Lin----------------------------
#ifndef ENCODEFEATURES_H_
#define ENCODEFEATURES_H_

#include <zlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define Malloc(type,n) (type *)malloc((n)*sizeof(type))

static const int GR_START = 0;
static const int GR_DI = 96; // dimension: 96 for gr
static const int OF_START = 96;
static const int OF_DI = 108; // dimension: 108 for of
static const int MB_START = 204;
static const int MB_DI = 192; // dimension: 192 for mb
static int k = 4000; // number of centers
static char *line = NULL;
static int max_line_len;

char* ReadLine(gzFile gzfp);
int Codewords(float** pCodeBook, float* pPoint, int nRows, int nCols);
bool EncodeFeatures(int argc, char** argv);

#endif /*ENCODEFEATURES_H_*/
//---------------------------------end---------------------------------
