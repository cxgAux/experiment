#ifndef _AFX_HPP_
#define _AFX_HPP_

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>
#include <opencv2/video.hpp>

#include <algorithm>
#include <vector>
#include <list>
#include <limits>
#include <cmath>
#include <cstdarg>
#include <cstdlib>
#include <string>

#define _DEBUG_

#if defined(_DEBUG_)
    #include <iostream>
    #include <fstream>
    #include <cstdio>
    #define _log(x) fprintf(stdout, "\033[34mLOG >>\033[32m%s\033[m", x);
    #define _err(x) fprintf(stderr, "\033[33mERR >>\033[31m%s\033[m", x);

#else
    #define _log(x)
    #define _err(x)

    void log(char *, va_list) {}
    void err(char *, va_list) {}
#endif// _DEBUG_

#endif// ! _AFX_HPP_
