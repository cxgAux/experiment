#ifndef _CLUSTER_STRUCTS_HPP_
#define _CLUSTER_STRUCTS_HPP_

#ifndef _ENABLE_MACROS_
#define _ENABLE_MACROS_
#endif// ! _ENABLE_MACROS_

#ifndef _ENABLE_ZLIB_
#define _ENABLE_ZLIB_
#endif// ! _ENABLE_ZLIB_

#include "afx.hpp"

namespace Structs {

    struct GZFileReader {
    private:
        int m_iSize;
        gzFile m_fd;
    public:
        char * m_line;
        static const int INIT_LEN = 1024;
        explicit GZFileReader (int = INIT_LEN);
        GZFileReader (const GZFileReader &) = delete;
        GZFileReader & operator= (const GZFileReader &) = delete;
        ~GZFileReader ();
        void open (const char *) throw (std::exception);
        bool is_open () const;
        void close ();
        bool readLine () throw (std::exception);
    public:
        static int countLines (const char *) throw (std::exception);
    };

    namespace Feature {
        int split (char *, float *);
        void read (std::istream &, float *, const int);
        void write (std::ostream &, float *, const int);
        void writeLine (std::ostream &, float *, const int);
    }

    namespace CodeBook {
        void read (std::istream &, float *, const int);
        void write (std::ostream &, float *, const int);
        void writeLine (std::ostream &, float *, const int);
    }
}

#endif// ! _CLUSTER_STRUCTS_HPP_
