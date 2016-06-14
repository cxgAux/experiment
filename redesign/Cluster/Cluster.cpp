#include "Cluster.hpp"

void Cluster (int argc, char * const argv[]) {
	DEBUG_ASSERT (argc == 4)
	//std::ifstream _if (argv[1], std::ios::in);
	FILE * fp = fopen (argv[1], "r");
	//DEBUG_ASSERT (_if.is_open ())
    const int _K = 4000,
		_ps = atoi (argv[2]),
		_dim = atoi (argv[3]),
		_stages = 100;
	cv::Mat _pts(_ps, _dim, CV_32FC1), labels, _cts(_K, _dim, _pts.type());

	REP_ROW (_ps) {
		float * pRow = _pts.ptr<float> (iRow);
    	//Structs::Feature::read (_if, pRow, _dim);
		REP_COL (_dim) {
			fscanf (fp, "%f", pRow + iCol);
		}
    }

	cv::kmeans(_pts, _K, labels,
		cv::TermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, _stages, 0.1),
		3, cv::KMEANS_PP_CENTERS, _cts);

	REP_ROW (_K) {
		float * pRow = _cts.ptr<float> (iRow);
		//Structs::CodeBook::writeLine (std::cout, pRow, _dim);
		REP_COL (_dim) {
			fprintf (stdout, "%f\t", pRow[iCol]);
		}
		fprintf(stdout, "\n");
    }
	//_if.close ();
}
