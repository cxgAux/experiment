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
#include <fstream>
#include <functional>

#if defined(DEBUG)
    #include <iostream>
    #include <cstdio>
    #define _log(x) fprintf(stdout, "\033[34mLOG >>\033[32m%s\033[m", x);
    #define _err(x) fprintf(stderr, "\033[33mERR >>\033[31m%s\033[m", x);

#else
    #define _log(x)
    #define _err(x)

    void log(char *, va_list) {}
    void err(char *, va_list) {}
#endif// DEBUG

#endif// ! _AFX_HPP_
