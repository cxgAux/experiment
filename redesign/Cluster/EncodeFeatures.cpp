#include "EncodeFeatures.hpp"

namespace Parser {
	struct ArgParser_Encode : public AFX::Parser::ArgParser {
	public:
		enum Measure {
			Eu,
			Ma,
			Co
		};
		typedef AFX::Parser::ArgParser::ArgNo ArgNo;
		typedef AFX::Parser::ArgParser::Args Args;
		explicit ArgParser_Encode ();
		~ArgParser_Encode ();
		inline void operator () (ArgNo, Args const []) final;
		inline AFX::Measure::Distance<float, float> & getMeasureHandler ();
	public:
		char * m_codeBookSrc;
		char * m_featureSrc;
		int	m_codeBookDim;
		int m_iTag;
		int m_iStart;
		Measure m_measure;
	protected:
		inline void usage () final;
		inline void errorHandler (int) final;
	private:
		char * m_executable;
	};
}

void EncodeFeatures (int argc, char * const argv []) {
	Parser::ArgParser_Encode _argParser;
	_argParser (argc, argv);

	AFX::Measure::Distance<float, float> & _measurer = _argParser.getMeasureHandler ();
	Structs::GZFileReader _gzfd;
	_gzfd.open (_argParser.m_featureSrc);
	std::ifstream _if (_argParser.m_codeBookSrc, std::ios::in);
	DEBUG_ASSERT (_if.is_open ())
	const int _szcb = Attributes::Cluster::K, _opt = _argParser.m_iStart, _dim =_argParser.m_codeBookDim;
	float **_codebook = new float * [_szcb];
	REP_ROW (_szcb) {
		_codebook[iRow] = new float [_dim];
		Structs::CodeBook::read (_if, _codebook[iRow], _dim);
	}

	float * _line = new float [Attributes::Dimension::hog + Attributes::Dimension::hof + Attributes::Dimension::mbh];
	int * _tfv = new int [_szcb];
	REP_ROW (_szcb) {
		_tfv[iRow] = 0;
	}

	while (_gzfd.readLine ()) {
		Structs::Feature::split (_gzfd.m_line, _line);
		int _bestIdx = 0;
		float _bestDis = _measurer.getWorst ();
		REP_ROW (_szcb) {
			int _ptr = 0, _realPtr = 0;
			if ((_opt >> 2) & 0x0001) {
				REP_BIN (Attributes::Dimension::hog) {
					_line[_realPtr ++] = _line[_ptr ++];
				}
			}
			else {
				_ptr += Attributes::Dimension::hog;
			}
			if ((_opt >> 1) & 0x001) {
				REP_BIN (Attributes::Dimension::hof) {
					_line[_realPtr ++] = _line[_ptr ++];
				}
			}
			else {
				_ptr += Attributes::Dimension::hof;
			}
			if ((_opt >> 0) & 0x001) {
				REP_BIN (Attributes::Dimension::mbh) {
					_line[_realPtr ++] = _line[_ptr ++];
				}
			}
			else {
				_ptr += Attributes::Dimension::mbh;
			}
			float _dis = _measurer (_codebook[iRow], _line, _realPtr);
			if (_measurer.isBetter (_dis, _bestDis)) {
				_bestIdx = iRow;
				_bestDis = _dis;
			}
		}
		_tfv[_bestIdx] += 1;
	}

	std::cout << _argParser.m_iTag + 1 << '\t';
	REP_ROW (_szcb) {
		if (_tfv[iRow] > 0) {
			std::cout << iRow << ':' << _tfv[iRow] << '\t';
		}
	}
	std::cout << '\n';

	delete [] _line;
	delete [] _tfv;
	_gzfd.close ();
	REP_ROW (_szcb) {
		delete [] _codebook[iRow];
	}
	delete [] _codebook;
	return;
}

namespace Parser {
	ArgParser_Encode::ArgParser_Encode () {}

	ArgParser_Encode::~ArgParser_Encode () {}

	void ArgParser_Encode::operator () (ArgParser_Encode::ArgNo argc, ArgParser_Encode::Args const argv[]) {
		int c = -1;
		this->m_executable = basename(argv[0]);
		this->m_codeBookSrc = nullptr;
		this->m_featureSrc = nullptr;
		this->m_codeBookDim = -1;
		this->m_measure = ArgParser_Encode::Measure::Eu;
		while((c = getopt (argc, argv, "hf:c:s:d:m:t:")) != -1) {
			switch(c) {
				case 'h':
					this->usage ();
					exit (0);
				case 'f':
					this->m_featureSrc = optarg;
					break;
				case 'c':
					this->m_codeBookSrc = optarg;
					break;
				case 's':
					this->m_iStart = atoi (optarg);
					break;
				case 'd':
					this->m_codeBookDim = atoi (optarg);
					break;
				case 'm':
					if (strcmp (optarg, "eu") == 0 || strcmp (optarg, "Euclidean_distance") == 0) {
						this->m_measure = ArgParser_Encode::Measure::Eu;
					}
					else if (strcmp (optarg, "ma") == 0 || strcmp (optarg, "Manhattan_distance") == 0) {
						this->m_measure = ArgParser_Encode::Measure::Ma;
					}
					else if (strcmp (optarg, "co") == 0 || strcmp (optarg, "Cosine_similarity") == 0) {
						this->m_measure = ArgParser_Encode::Measure::Co;
					}
					else {
						this->errorHandler (c);
					}
					break;
				case 't':
					this->m_iTag = atoi (optarg);
					break;
				default:
					this->errorHandler (c);
			}
		}
	}

	void ArgParser_Encode::usage () {
		std::cerr << "Encode features with providing CodeBook\n\n";
		std::cerr << "Usage: EncodeFeatures -f [feature src formatted *.gz] -c [CodeBook] -s [feature options] -d [codebook dim] -t [video class tag] [options]\n";
		std::cerr << "Options:\n";
		std::cerr << "  -h                        Display this message and exit\n";
		std::cerr << "  -m                        Measure type [default euclidean distance, alternative: eu(Euclidean_distance) | ma(Manhattan_distance) | co(Cosine_similarity)]\n";
	}

	void ArgParser_Encode::errorHandler (int opt) {
		std::cerr << "error parsing arguments at -"<< opt << "\n  Try '" << this->m_executable << " -h' for help." << std::endl;
		abort();
	}

	AFX::Measure::Distance<float, float> & ArgParser_Encode::getMeasureHandler () {
		if (this->m_measure == Measure::Eu) {
			return AFX::Measure::_euclideanDistanceHandler;
		}
		else if (this->m_measure == Measure::Ma) {
			return AFX::Measure::_euclideanDistanceHandler;
		}
		if (this->m_measure == Measure::Co) {
			return AFX::Measure::_euclideanDistanceHandler;
		}
	}
}
