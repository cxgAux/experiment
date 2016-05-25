#ifndef _AFX_HPP_
#define _AFX_HPP_

#ifdef _ENABLE_OPENCV_
#include <opencv2/opencv.hpp>
#endif// ! _ENABLE_OPENCV_

#ifdef _ENABLE_BOOST_
#include <boost/filesystem.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>
#endif// ! _ENABLE_BOOST_

#ifndef _ENABLE_MACROS_
#define REP_Height(Height) for (int iHeight = 0; iHeight < (Height); ++ iHeight)
#define REP_Width(Width) for (int iWidth = 0; iWidth < (Width); ++ iWidth)
#define REP_SCALE(Scales) for (int iScale = 0; iScale < (Scales); ++ iScale)
#endif// ! _ENABLE_MACROS_

/**
 *  @brief C-extended libraries
 */
#include <cassert>
#include <cstdlib>

/**
 *  @brief  IO
 */
#include <iostream>
#include <fstream>

/**
 *  @brief  Containers
 */
#include <vector>
#include <list>

/**
 *  @brief  Others
 */
#include <algorithm>
#include <string>
#include <cmath>
#include <limits>
#include <functional>


#endif// ! _AFX_HPP_
