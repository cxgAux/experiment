#include "cluster_Structs.hpp"

namespace Structs {
    GZFileReader::GZFileReader (int initLen)
        : m_iSize (initLen),  m_fd (nullptr), m_line (new char [this->m_iSize]) {

    }

    GZFileReader::~GZFileReader () {
        this->close ();
        if (this->m_line != nullptr) {
            delete this->m_line;
        }
    }

    void GZFileReader::open (const char * gzFile) throw (std::exception) {
        this->m_fd = gzopen (gzFile, "r");
        if (this->m_fd == nullptr) {
            throw std::logic_error ("open gzFile failed!");
        }
    }

    bool GZFileReader::is_open () const {
        return this->m_fd != nullptr;
    }

    void GZFileReader::close () {
        if (this->is_open ()) {
            gzclose (this->m_fd);
            this->m_fd = nullptr;
        }
    }

    bool GZFileReader::readLine () throw (std::exception) {
        if (gzgets (this->m_fd, this->m_line, this->m_iSize) == nullptr) {
            return false;
        }
        while (strrchr (this->m_line, '\n') == nullptr) {
            this->m_iSize *= 2;
            this->m_line = (char *) realloc (this->m_line, this->m_iSize);
            int _len = (int) strlen (this->m_line);
            if (gzgets (this->m_fd, this->m_line + _len, this->m_iSize - _len) == nullptr) {
                throw std::logic_error ("GZFileReader::readLine: error format!");
            }
        }
        return true;
    }

    int GZFileReader::countLines (const char * gzFile) throw (std::exception) {
        GZFileReader _gzfd;
        _gzfd.open (gzFile);
        int _res = 0;
        while (_gzfd.readLine ()) {
            _res ++;
        }
        return _res;
    }

    namespace Feature {
        int split (char * cptr, float * fptr) {
            int _res = 0;
        	char * _sptrf = NULL, * _part = NULL;
        	while ( (_part = strtok_r (cptr, " \t\n", & _sptrf)) != NULL) {
        		fptr[_res ++] = atof (_part);
        		cptr = NULL;
        	}
        	return _res;
        }

        void read (std::istream & is, float * fptr, const int size) {
            REP_COL (size) {
                is >> fptr[iCol];
            }
        }

        void write (std::ostream & os, float * fptr, const int size) {
            REP_COL (size) {
                os << fptr[iCol] << '\t';
            }
        }

        void writeLine (std::ostream & os, float * fptr, const int size) {
            write (os, fptr, size);
            os << '\n';
        }
    }

    namespace CodeBook {
        void read (std::istream & is, float * fptr, const int size) {
            REP_COL (size) {
                is >> fptr[iCol];
            }
        }

        void write (std::ostream & os, float * fptr, const int size) {
            REP_COL (size) {
                os << fptr[iCol] << '\t';
            }
        }

        void writeLine (std::ostream & os, float * fptr, const int size) {
            write (os, fptr, size);
            os << '\n';
        }
    }
}
