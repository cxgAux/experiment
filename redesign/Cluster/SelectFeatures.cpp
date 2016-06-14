#include "SelectFeatures.hpp"

int getVoted (int);
std::vector<int> randPerm (int, int) throw (std::exception);
void opt_out (float *, unsigned int);

void SelectFeatures(int argc, char * const argv[]) {
	DEBUG_ASSERT (argc == 3)
	float * _line = new float [Attributes::Dimension::hog + Attributes::Dimension::hof + Attributes::Dimension::mbh];
	const int _cnt = Structs::GZFileReader::countLines (argv[1]),_vct = getVoted (_cnt), _opts = atoi (argv[2]);
	std::vector<int> _voted = randPerm (_vct, _cnt);
	const int _sz = _voted.size ();
	Structs::GZFileReader _gzfd;
	_gzfd.open (argv[1]);
	int _iLine = 0, _idx = 0;
	while (_idx < _sz) {
		while (_iLine <= _voted[_idx]) {
			_gzfd.readLine ();
			_iLine ++;
		}
		_idx ++;
		Structs::Feature::split (_gzfd.m_line, _line);
		opt_out (_line, _opts);
	}
	_gzfd.close ();
	delete [] _line;
}

int getVoted (int cnt) {
	if (cnt <= Attributes::SelectFeatures::min_line) {
		return cnt;
	}
	else {
		return int(Attributes::SelectFeatures::min_line + (cnt - Attributes::SelectFeatures::min_line) * Attributes::SelectFeatures::ratio);
	}
}

std::vector<int> randPerm (int p, int q) throw (std::exception) {
	if (p < 0 || q < 0 || p > q) {
		throw std::logic_error ("randPerm: error paras!");
	}
	unsigned int _seed = std::chrono::system_clock::now ().time_since_epoch ().count ();
	std::vector<int> _res;
	for (int i = 0; i < q; ++ i) {
		_res.push_back (i);
	}
	std::shuffle (_res.begin (), _res.end (), std::default_random_engine (_seed));
	_res.resize (p);
	std::sort(_res.begin  (), _res.end ());
	return _res;
}

void opt_out (float * ptr, unsigned int opt) {
	DEBUG_ASSERT (opt <= 0x111)
	int _st = 0;
	if ((opt >> 2) & 0x0001) {
		Structs::Feature::write (std::cout, ptr + _st, Attributes::Dimension::hog);
	}
	else {
		_st += Attributes::Dimension::hog;
	}
	if ((opt >> 1) & 0x001) {
		Structs::Feature::write (std::cout, ptr + _st, Attributes::Dimension::hof);
	}
	else {
		_st += Attributes::Dimension::hof;
	}
	if ((opt >> 0) & 0x001) {
		Structs::Feature::write (std::cout, ptr + _st, Attributes::Dimension::mbh);
	}
	else {
		_st += Attributes::Dimension::mbh;
	}
	std::cout << "\n";
}
