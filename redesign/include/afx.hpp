#ifndef _AFX_HPP_
#define _AFX_HPP_

#ifdef _ENABLE_OPENCV_
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/video.hpp>
#include <opencv2/videoio.hpp>

#endif// ! _ENABLE_OPENCV_

#ifdef _ENABLE_BOOST_
#include <boost/filesystem.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>
#endif// ! _ENABLE_BOOST_

#ifdef _ENABLE_MACROS_
#define REP_HEIGHT(Height)  for (int iHeight = 0; iHeight < (Height); ++ iHeight)
#define REP_WIDTH(Width)    for (int iWidth = 0; iWidth < (Width); ++ iWidth)
#define REP_SCALE(Scales)   for (int iScale = 0; iScale < (Scales); ++ iScale)
#define REP_ROW(Rows)       for (int iRow = 0; iRow < (Rows); ++ iRow)
#define REP_COL(Cols)       for (int iCol = 0; iCol < (Cols); ++ iCol)
#define REP_BIN(Bins)       for (int iBin = 0; iBin < (Bins); ++ iBin)
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
 *  @brief  Safety
 */
#include <memory>

/**
 *  @brief  Type checking
 */
#include <type_traits>


/**
 *  @brief  Others
 */
#include <algorithm>
#include <string>
#include <cmath>
#include <limits>
#include <functional>


#endif// ! _AFX_HPP_
