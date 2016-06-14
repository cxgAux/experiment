//--------------------------------begin--------------------------------
//-----------------------------by Yikun Lin----------------------------
#include "ComputeDistance.h"
char* ReadLine(FILE* input)
{
	int len;
	
	if(fgets(line, max_line_len, input) == NULL)
		return NULL;

	while(strrchr(line, '\n') == NULL)
	{
		max_line_len *= 2;
		line = (char *) realloc(line, max_line_len);
		len = (int) strlen(line);
		if(fgets(line + len, max_line_len - len, input) == NULL)
			break;
	}
	return line;
}


bool ReadTrainProblem(char** trainFiles, int nTrains)
{
	FILE* trainIn;
	int elements;
	train_prob.l = 0;
	elements = 0;
	int iTrain;
	// record the quantity of lines and elements of training videos
	for (iTrain = 0; iTrain < nTrains; iTrain++)
	{
		trainIn = fopen(trainFiles[iTrain], "r");
		while (ReadLine(trainIn))
		{
			char* feature = strtok(line, " \t"); // label
			
			// features
			while (1)
			{
				feature = strtok(NULL, " \t");
				if (feature == NULL || *feature == '\n')
				{
					break;
				}
				++elements;
			}
			++elements;
			++train_prob.l;
		}
		fclose(trainIn);
		
	}
	
	// allocate the memory for all the training videos
	train_prob.y = Malloc(double, train_prob.l);
	train_prob.x = Malloc(struct svm_node *, train_prob.l);
	train_x_space = Malloc(struct svm_node, elements);
	
	int iEle, iLine, maxIndex; 
	char *endptr;
	char *index, *value, *label;
	iEle = 0;
	iLine = 0;
	// read all the video features
	for (iTrain = 0; iTrain < nTrains; iTrain++)
	{
		trainIn = fopen(trainFiles[iTrain], "r");
		
		while (ReadLine(trainIn))
		{
			maxIndex = -1;
			train_prob.x[iLine] = &train_x_space[iEle];
			label = strtok(line, " \t\n"); // label
			if (label == NULL)
			{
				return false;
			}
			train_prob.y[iLine] = strtod(label, &endptr);
			if (endptr == label || *endptr != '\0')
			{
				return false;
			}
			
			// features
			while (1)
			{
				index = strtok(NULL, ":");
				value = strtok(NULL, " \t");
				
				if (value == NULL)
				{
					break;
				}
				
				train_x_space[iEle].index = (int) strtol(index, &endptr, 10);
				// make sure the index in ascending order
				if (endptr == index || *endptr != '\0' || train_x_space[iEle].index <= maxIndex)
				{
					return false;
				}
				else
				{
					maxIndex = train_x_space[iEle].index;
				}
				
				train_x_space[iEle].value = strtod(value, &endptr);
				if (endptr == index || (*endptr != '\0' && !isspace(*endptr)))
				{
					return false;
				}
				++iEle;
			}
			train_x_space[iEle++].index = -1;
			++iLine;
		}
				
		fclose(trainIn);
	}
	return true;
}


bool ReadTestProblem(char* testFile)
{
	FILE* testIn;
	int elements;
	test_prob.l = 0;
	elements = 0;
	testIn = fopen(testFile, "r");
	// record the quantity of lines and elements of testing videos
	while (ReadLine(testIn))
	{
		char* feature = strtok(line, " \t");
		
		while (1)
		{
			feature = strtok(NULL, " \t");
			if (feature == NULL || *feature == '\n')
			{
				break;
			}
			++elements;
		}
		++elements;
		++test_prob.l;
	}
	rewind(testIn);
	// allocate the memory for all the testing videos
	test_prob.y = Malloc(double, test_prob.l);
	test_prob.x = Malloc(struct svm_node *, test_prob.l);
	test_x_space = Malloc(struct svm_node, elements);
	
	int iEle, iLine, maxIndex; 
	char *endptr;
	char *index, *value, *label;
	iEle = 0;
	iLine = 0;
	
	// read all the video features
	while (ReadLine(testIn))
	{
		maxIndex = -1;
		test_prob.x[iLine] = &test_x_space[iEle];
		label = strtok(line, " \t\n"); // label
		if (label == NULL)
		{
			return false;
		}
		test_prob.y[iLine] = strtod(label, &endptr);
		if (endptr == label || *endptr != '\0')
		{
			return false;
		}
		
		// features
		while (1)
		{
			index = strtok(NULL, ":");
			value = strtok(NULL, " \t");
			
			if (value == NULL)
			{
				break;
			}
			test_x_space[iEle].index = (int) strtol(index, &endptr, 10);
			// make sure the index in ascending order
			if (endptr == index || *endptr != '\0' || test_x_space[iEle].index <= maxIndex)
			{
				return false;
			}
			else
			{
				maxIndex = test_x_space[iEle].index;
			}
			
			test_x_space[iEle].value = strtod(value, &endptr);
			if (endptr == index || (*endptr != '\0' && !isspace(*endptr)))
			{
				return false;
			}
			++iEle;
		}
		test_x_space[iEle++].index = -1;
		++iLine;
	}

	fclose(testIn);
	return true;
}


// with term frequency
// CF : Collection Frequency, details in "Supervised and Traditional Term Weighting Methods for Automatic Text Categorization"
void ComputeWeight(char* file, float* CF)
{
	float averageDistance = 0;
	// normalization factors for each video
	float* norFactor = new float[train_prob.l + test_prob.l];
	float** distance = new float*[train_prob.l + test_prob.l];
	int iLine, iEle, index;
	int iRow, iCol, iRowEle, iColEle, rowIndex, colIndex;
	float rowValue, colValue;
	FILE* TFCFOut = fopen(file, "w");
	
	// get the normalization factors for training videos
	for (iLine = 0; iLine < train_prob.l; iLine++)
	{
		iEle = 0;
		norFactor[iLine] = 0;
		distance[iLine] = new float[train_prob.l];
		// index = -1 indicates the end of one vector
		while ((index = train_prob.x[iLine][iEle].index) != -1)
		{
			norFactor[iLine] += CF[index] * log(train_prob.x[iLine][iEle++].value + 1);
			//norFactor[iLine] += CF[index] * train_prob.x[iLine][iEle++].value;
		}
		assert(norFactor[iLine] > 0);
	}
	// get the normalization factors for testing videos
	for (iLine = train_prob.l; iLine < (train_prob.l + test_prob.l); iLine++)
	{
		iEle = 0;
		norFactor[iLine] = 0;
		distance[iLine] = new float[train_prob.l];
		// index = -1 indicates the end of one vector
		while ((index = test_prob.x[iLine - train_prob.l][iEle].index) != -1)
		{
			norFactor[iLine] += CF[index] * log(test_prob.x[iLine - train_prob.l][iEle++].value + 1);
			//norFactor[iLine] += CF[index] * test_prob.x[iLine - train_prob.l][iEle++].value;
		}
		assert(norFactor[iLine] > 0);
	}
	
	// compute the x^2 distances between training videos
	for (iRow = 0; iRow < train_prob.l; iRow++)
	{
		
		for (iCol = 0; iCol < train_prob.l; iCol++)
		{
			iRowEle = 0;
			iColEle = 0;
			distance[iRow][iCol] = 0;
			while (1)
			{
				rowIndex = train_prob.x[iRow][iRowEle].index;
				colIndex = train_prob.x[iCol][iColEle].index;
				// both videos arrive at the end
				if (rowIndex == -1 && colIndex == -1)
				{
					break;
				}
				// calculate and move forward the first video
				if ((rowIndex > -1 && rowIndex < colIndex) || colIndex == -1)
				{
					rowValue = CF[rowIndex] * log(train_prob.x[iRow][iRowEle].value + 1) / norFactor[iRow];
					//rowValue = CF[rowIndex] * train_prob.x[iRow][iRowEle].value / norFactor[iRow];
					distance[iRow][iCol] += 0.5 * rowValue;
					++iRowEle;
				}
				// calculate and move forward the second video
				else if ((colIndex > -1 && colIndex < rowIndex) || rowIndex == -1)
				{
					colValue = CF[colIndex] * log(train_prob.x[iCol][iColEle].value + 1) / norFactor[iCol];
					//colValue = CF[colIndex] * train_prob.x[iCol][iColEle].value / norFactor[iCol];
					distance[iRow][iCol] += 0.5 * colValue;
					++iColEle;
				}
				// calculate and move forward both videos
				else
				{
					rowValue = CF[rowIndex] * log(train_prob.x[iRow][iRowEle].value + 1) / norFactor[iRow];
					colValue = CF[colIndex] * log(train_prob.x[iCol][iColEle].value + 1) / norFactor[iCol];
					//rowValue = CF[rowIndex] * train_prob.x[iRow][iRowEle].value / norFactor[iRow];
					//colValue = CF[colIndex] * train_prob.x[iCol][iColEle].value / norFactor[iCol];
					if (rowValue != colValue)
					{
						distance[iRow][iCol] += 0.5 * pow((rowValue -  colValue), 2) / (rowValue + colValue);
					}
					++iRowEle;
					++iColEle;
				}
			}
			averageDistance += distance[iRow][iCol];
		}
	}
	// calculate the average x^2 distances between training videos
	averageDistance /= ((float)pow(train_prob.l, 2) - train_prob.l);
	
	// compute the x^2 distances between testing and training videos
	for (iRow = train_prob.l; iRow < train_prob.l + test_prob.l; iRow++)
	{
		for (iCol = 0; iCol < train_prob.l; iCol++)
		{
			iRowEle = 0;
			iColEle = 0;
			distance[iRow][iCol] = 0;
			while (1)
			{
				rowIndex = test_prob.x[iRow - train_prob.l][iRowEle].index;
				colIndex = train_prob.x[iCol][iColEle].index;
				// both videos arrive at the end
				if (rowIndex == -1 && colIndex == -1)
				{
					break;
				}
				// calculate and move forward the first video
				if ((rowIndex > -1 && rowIndex < colIndex) || colIndex == -1)
				{
					rowValue = CF[rowIndex] * log(test_prob.x[iRow -  train_prob.l][iRowEle].value + 1) / norFactor[iRow];
					//rowValue = CF[rowIndex] * test_prob.x[iRow -  train_prob.l][iRowEle].value / norFactor[iRow];
					distance[iRow][iCol] += 0.5 * rowValue;
					++iRowEle;
				}
				// calculate and move forward the second video
				else if ((colIndex > -1 && colIndex < rowIndex) || rowIndex == -1)
				{
					colValue = CF[colIndex] * log(train_prob.x[iCol][iColEle].value + 1) / norFactor[iCol];
					//colValue = CF[colIndex] * train_prob.x[iCol][iColEle].value / norFactor[iCol];
					distance[iRow][iCol] += 0.5 * colValue;
					++iColEle;
				}
				// calculate and move forward both videos
				else
				{
					rowValue = CF[rowIndex] * log(test_prob.x[iRow -  train_prob.l][iRowEle].value + 1) / norFactor[iRow];
					colValue = CF[colIndex] * log(train_prob.x[iCol][iColEle].value + 1) / norFactor[iCol];
					//rowValue = CF[rowIndex] * test_prob.x[iRow -  train_prob.l][iRowEle].value / norFactor[iRow];
					//colValue = CF[colIndex] * train_prob.x[iCol][iColEle].value / norFactor[iCol];
					if (rowValue != colValue)
					{
						distance[iRow][iCol] += 0.5 * pow((rowValue -  colValue), 2) / (rowValue + colValue);
					}
					++iRowEle;
					++iColEle;
				}
			}
			
		}
	}
	
	// print the average distance between training videos, 
	// number of training and testing videos
	fprintf(TFCFOut, "%f\t%d\t%d\t\n", averageDistance, train_prob.l, test_prob.l);
	
	// print the x^2 distances between videos
	for (iRow = 0; iRow < train_prob.l + test_prob.l; iRow++)
	{
		if (iRow < train_prob.l)
		{
			// print the class label and id of video
			fprintf(TFCFOut, "%d\t%d\t", (int)train_prob.y[iRow], iRow + 1);
		}
		else
		{
			fprintf(TFCFOut, "%d\t%d\t", (int)test_prob.y[iRow - train_prob.l], iRow + 1);
		}
		for (iCol = 0; iCol < train_prob.l; iCol++)
		{
			fprintf(TFCFOut, "%f\t", distance[iRow][iCol]);
		}
		fprintf(TFCFOut, "\n");
	}
	fclose(TFCFOut);
	delete[] norFactor;
	for (iLine = 0; iLine < train_prob.l + test_prob.l; iLine++)
	{
		delete[] distance[iLine];
	}
	delete[] distance;
}


//-----------------------------------------------------------------------------
// weight1: TF
//-----------------------------------------------------------------------------
void Compute_TF(float* TF)
{
	int index;
	for (index = 0; index < k; index++)
	{
		TF[index] = 1;
	}
}


//-----------------------------------------------------------------------------
// weight2: TF-IDF
//-----------------------------------------------------------------------------
void Compute_IDF(float* IDF)
{
	int index;
	for (index = 0; index < k; index++)
	{
		IDF[index] = 0;
	}
	
	for (int iLine = 0; iLine < train_prob.l; iLine++)
	{
		int iEle = 0;
		// index = -1 indicates the end of one vector
		while ((index = train_prob.x[iLine][iEle++].index) != -1)
		{
			assert(index < k);
			IDF[index]++;
		}
	}
	/*for (int iLine = 0; iLine < test_prob.l; iLine++)
	{
		int iEle = 0;
		// index = -1 indicates the end of one vector
		while ((index = test_prob.x[iLine][iEle++].index) != -1)
		{
			assert(index < k);
			IDF[index]++;
		}
	}*/
	for (index = 0; index < k; index++)
	{
		
		if (IDF[index] > 0)
		{
			//IDF[index] = log((float)(train_prob.l + test_prob.l) / IDF[index]);
			IDF[index] = log((float)(train_prob.l) / IDF[index]);
		}
	}
}


//-----------------------------------------------------------------------------
// weight3: TF-CHI
//-----------------------------------------------------------------------------
void Compute_CHI(float* CHI)
{
	int index;
	int iClass;
	// the last column and last row contain global supports
	float** featureSupports = new float*[k + 1];
	
	for (index = 0; index < (k + 1); index++)
	{
		if (index < k)
		{
			CHI[index] = 0;
		}
		featureSupports[index] = new float[nClasses + 1];
		for (iClass = 0; iClass < (nClasses + 1); iClass++)
		{
			featureSupports[index][iClass] = 0;
		}
	}

	for (int iLine = 0; iLine < train_prob.l; iLine++)
	{
		int iEle = 0;
		int label = (int)train_prob.y[iLine] - 1;
		featureSupports[k][label]++; // the number of documents in a class
		
		while ((index = train_prob.x[iLine][iEle++].index) != -1)
		{
			featureSupports[index][nClasses]++; // the number of documents where a feature occurs
			featureSupports[index][label]++; // the number of classes where a feature occurs
		}
	}
	
	for (index = 0; index < k; index++)
	{
		if (featureSupports[index][nClasses] > 0)
		{
			for (iClass = 0; iClass < nClasses; iClass++)
			{
				float tp = featureSupports[index][iClass];
				float fp = featureSupports[k][iClass] - tp;
				float fn = featureSupports[index][nClasses] - tp;
				float tn = train_prob.l - featureSupports[k][iClass] - fn;
				float value = train_prob.l * pow(tp * tn - fp * fn, 2) / ((tp + fp) * (fn + tn) * (tp + fn) * (fp + tn));
				CHI[index] = (value > CHI[index]) ? (value) : CHI[index];
			}
		}
	}
	
	for (index = 0; index < (k + 1); index++)
	{
		delete[] featureSupports[index];
	}
	delete[] featureSupports;
}


//-----------------------------------------------------------------------------
// weight4: TF-OR
//-----------------------------------------------------------------------------
void Compute_OR(float* OR)
{
	int index;
	int iClass;
	// the last column and last row contain global supports
	float** featureSupports = new float*[k + 1];
	
	for (index = 0; index < (k + 1); index++)
	{
		if (index < k)
		{
			OR[index] = 0;
		}
		featureSupports[index] = new float[nClasses + 1];
		for (iClass = 0; iClass < (nClasses + 1); iClass++)
		{
			featureSupports[index][iClass] = 0;
		}
	}

	for (int iLine = 0; iLine < train_prob.l; iLine++)
	{
		int iEle = 0;
		int label = (int)train_prob.y[iLine] - 1;
		featureSupports[k][label]++; // the number of documents in a class
		
		while ((index = train_prob.x[iLine][iEle++].index) != -1)
		{
			featureSupports[index][nClasses]++; // the number of documents where a feature occurs
			featureSupports[index][label]++; // the number of classes where a feature occurs
		}
	}
	
	for (index = 0; index < k; index++)
	{
		if (featureSupports[index][nClasses] > 0)
		{
			for (iClass = 0; iClass < nClasses; iClass++)
			{
				float tp = featureSupports[index][iClass];
				float fp = featureSupports[k][iClass] - tp;
				float fn = featureSupports[index][nClasses] - tp;
				float tn = train_prob.l - featureSupports[k][iClass] - fn;
				float value = 0;
				if (tp * tn > 0 && fp * fn > 0)
				{
					value = log((tp * tn) / (fp * fn));
				}
				OR[index] = (value > OR[index]) ? (value) : OR[index];
			}
		}
	}
	
	for (index = 0; index < (k + 1); index++)
	{
		delete[] featureSupports[index];
	}
	delete[] featureSupports;
}


//-----------------------------------------------------------------------------
// weight5: TF-RF
//-----------------------------------------------------------------------------
void Compute_RF(float* RF)
{
	int index;
	int iClass;
	// the last column and last row contain global supports
	float** featureSupports = new float*[k + 1];
	
	for (index = 0; index < (k + 1); index++)
	{
		if (index < k)
		{
			RF[index] = 0;
		}
		featureSupports[index] = new float[nClasses + 1];
		for (iClass = 0; iClass < (nClasses + 1); iClass++)
		{
			featureSupports[index][iClass] = 0;
		}
	}

	for (int iLine = 0; iLine < train_prob.l; iLine++)
	{
		int iEle = 0;
		int label = (int)train_prob.y[iLine] - 1;
		featureSupports[k][label]++; // the number of documents in a class
		
		while ((index = train_prob.x[iLine][iEle++].index) != -1)
		{
			featureSupports[index][nClasses]++; // the number of documents where a feature occurs
			featureSupports[index][label]++; // the number of classes where a feature occurs
		}
	}
	
	for (index = 0; index < k; index++)
	{
		if (featureSupports[index][nClasses] > 0)
		{
			for (iClass = 0; iClass < nClasses; iClass++)
			{
				float tp = featureSupports[index][iClass];
				float fp = featureSupports[k][iClass] - tp;
				float fn = featureSupports[index][nClasses] - tp;
				float tn = train_prob.l - featureSupports[k][iClass] - fn;
				float value = 0;
				if (fn <= 0)
				{
					fn = 1;
				}
				if (tp > 0 && fn > 0)
				{
					value = log(2 + tp / fn);
				}
				RF[index] = (value > RF[index]) ? (value) : RF[index];
			}
		}
	}
	
	for (index = 0; index < (k + 1); index++)
	{
		delete[] featureSupports[index];
	}
	delete[] featureSupports;
}


//-----------------------------------------------------------------------------
// weight6: TF-VRF
//-----------------------------------------------------------------------------
void Compute_VRF(float* VRF)
{
	int index;
	int iClass;
	// the last column and last row contain global supports
	float** featureSupports = new float*[k + 1];
	
	for (index = 0; index < (k + 1); index++)
	{
		if (index < k)
		{
			VRF[index] = 0;
		}
		featureSupports[index] = new float[nClasses + 1];
		for (iClass = 0; iClass < (nClasses + 1); iClass++)
		{
			featureSupports[index][iClass] = 0;
		}
	}

	for (int iLine = 0; iLine < train_prob.l; iLine++)
	{
		int iEle = 0;
		int label = (int)train_prob.y[iLine] - 1;
		featureSupports[k][label]++; // the number of documents in a class
		
		while ((index = train_prob.x[iLine][iEle++].index) != -1)
		{
			featureSupports[index][nClasses]++; // the number of documents where a feature occurs
			featureSupports[index][label]++; // the number of classes where a feature occurs
		}
	}
	
	for (index = 0; index < k; index++)
	{
		if (featureSupports[index][nClasses] > 0)
		{
			for (iClass = 0; iClass < nClasses; iClass++)
			{
				float tp = featureSupports[index][iClass];
				float fp = featureSupports[k][iClass] - tp;
				float fn = featureSupports[index][nClasses] - tp;
				float tn = train_prob.l - featureSupports[k][iClass] - fn;
				float value = 0;
				if (fn <= 0)
				{
					fn = 1;
				}
				if (tp > 0 && fn > 0)
				{
					value = log(1 + tp) / log(fn + 1);
				}
				VRF[index] = (value > VRF[index]) ? (value) : VRF[index];
			}
		}
	}
	
	for (index = 0; index < (k + 1); index++)
	{
		delete[] featureSupports[index];
	}
	delete[] featureSupports;
}


//-----------------------------------------------------------------------------
// weight7: TF-PRF
//-----------------------------------------------------------------------------
void Compute_PRF(float* PRF)
{
	int index;
	int iClass;
	// the last column and last row contain global supports
	float** featureSupports = new float*[k + 1];
	
	for (index = 0; index < (k + 1); index++)
	{
		if (index < k)
		{
			PRF[index] = 0;
		}
		featureSupports[index] = new float[nClasses + 1];
		for (iClass = 0; iClass < (nClasses + 1); iClass++)
		{
			featureSupports[index][iClass] = 0;
		}
	}

	for (int iLine = 0; iLine < train_prob.l; iLine++)
	{
		int iEle = 0;
		int label = (int)train_prob.y[iLine] - 1;
		
		while ((index = train_prob.x[iLine][iEle].index) != -1)
		{
			featureSupports[index][nClasses] += train_prob.x[iLine][iEle].value; // the number of documents where a feature occurs
			featureSupports[index][label] += train_prob.x[iLine][iEle].value; // the number of classes where a feature occurs
			iEle++;
		}
	}
	
	for (index = 0; index < k; index++)
	{
		if (featureSupports[index][nClasses] > 0)
		{
			for (iClass = 0; iClass < nClasses; iClass++)
			{
				float PTF = featureSupports[index][iClass]; // positive term frequency
				float NTF = featureSupports[index][nClasses] - PTF; // negative term frequency
				float value = 0;
				if (NTF <= 0)
				{
					NTF = 1;
				}
				if (PTF > 0 && NTF > 0)
				{
					value = log(1 + PTF) / log(NTF / (nClasses - 1) + 1);
				}
				PRF[index] = (value > PRF[index]) ? (value) : PRF[index];
			}
		}
	}
	
	for (index = 0; index < (k + 1); index++)
	{
		delete[] featureSupports[index];
	}
	delete[] featureSupports;
}

// Filter out those features with low weights
void FeatureSelection(float* weight)
{
	std::vector<float> weight_values;
	int index;
	int stop = (int)(r * k);
	float maxIG = 0;
	float threshold = 0;
	
	for (index = 0; index < k; index++)
	{
		weight_values.push_back(weight[index]);
		if (weight[index] > maxIG)
		{
			maxIG = weight[index];
		}
	}
	sort(weight_values.begin(), weight_values.end());
	threshold = weight_values.at(stop);
	
	for (index = 0; index < k; index++)
	{
		if (weight[index] >= threshold)
		{
			weight[index] *= 1;
		}
		else 
		{	
			weight[index] *= 0.001;
		}
	}
	
}


bool ComputeDistance(int argc, char** argv)
{
	
	assert(argc > 4);
	char* testFile = argv[1];
	char* weightFile = argv[2];
	int nTrains = argc - 3;
	char** trainFiles = new char*[nTrains];
	
	int iTrain;
	for (iTrain = 0; iTrain < nTrains; iTrain++)
	{
		trainFiles[iTrain] = argv[iTrain + 3];
	}
	
	max_line_len = 1024;
	line = Malloc(char,max_line_len);
	
	// read train and test videos separately
	assert(ReadTrainProblem(trainFiles, nTrains));
	assert(ReadTestProblem(testFile));
	
	//---------------------------------------------------------------------
	// weight1: TF
	//---------------------------------------------------------------------
	/*float* TF = new float[k];
	Compute_TF(TF);
	ComputeWeight(weightFile, TF);
	delete[] TF;*/
	
	//---------------------------------------------------------------------
	// weight2: TF-IDF
	//---------------------------------------------------------------------
	/*float* IDF = new float[k];
	Compute_IDF(IDF);
	FeatureSelection(IDF);
	ComputeWeight(weightFile, IDF);
	delete[] IDF;*/
	
	//-----------------------------------------------------------------------------
	// weight3: TF-CHI
	//-----------------------------------------------------------------------------
	/*float* CHI = new float[k];
	Compute_CHI(CHI);
	FeatureSelection(CHI);
	ComputeWeight(weightFile, CHI);
	delete[] CHI;*/
	
	//-----------------------------------------------------------------------------
	// weight4: TF-OR
	//-----------------------------------------------------------------------------
	/*float* OR = new float[k];
	Compute_OR(OR);
	FeatureSelection(OR);
	ComputeWeight(weightFile, OR);
	delete[] OR;*/
	
	//-----------------------------------------------------------------------------
	// weight5: TF-RF
	//-----------------------------------------------------------------------------
	/*float* RF = new float[k];
	Compute_RF(RF);
	FeatureSelection(RF);
	ComputeWeight(weightFile, RF);
	delete[] RF;*/
	
	//-----------------------------------------------------------------------------
	// weight6: TF-VRF
	//-----------------------------------------------------------------------------
	/*float* VRF = new float[k];
	Compute_VRF(VRF);
	FeatureSelection(VRF);
	ComputeWeight(weightFile, VRF);
	delete[] VRF;*/
	
	//-----------------------------------------------------------------------------
	// weight7: TF-PRF (pseudo relevance frequency)
	//-----------------------------------------------------------------------------
	float* PRF = new float[k];
	Compute_PRF(PRF);
	FeatureSelection(PRF);
	ComputeWeight(weightFile, PRF);
	delete[] PRF;
	
	
	free(train_prob.y);
	free(train_prob.x);
	free(train_x_space);
	free(test_prob.y);
	free(test_prob.x);
	free(test_x_space);
	free(line);
	delete[] trainFiles;
	
	return true;
}
//---------------------------------end---------------------------------
