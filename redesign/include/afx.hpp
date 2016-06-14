#ifndef _AFX_HPP_
#define _AFX_HPP_

#ifdef _ENABLE_OPENCV_
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
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
#ifdef _ENABLE_ASSERT_
#include <cassert>
#define DEBUG_ASSERT(msg) assert (msg);
#else
#define DEBUG_ASSERT(msg)
#endif// _ENABLE_ASSERT_
#include <cstdlib>
#include <cstring>

#ifdef _ENABLE_ZLIB_
#include <zlib.h>
#endif// _ENABLE_ZLIB_

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
#include <stdexcept>
#include <random>
#include <chrono>
namespace AFX {

    namespace Parser {
        struct ArgParser {
        public:
            typedef int ArgNo;
            typedef char * Args;
            ~ArgParser () {}
            inline virtual void operator () (ArgNo, Args const [])  = 0;
        protected:
            ArgParser () {}
            ArgParser (const ArgParser &) = delete;
            ArgParser & operator= (const ArgParser &) = delete;
            inline virtual void errorHandler (int) = 0;
            inline virtual void usage () = 0;
        };
    }

    namespace Measure {
        template < class ResultType, class ParameterType >
        struct Distance {
            typedef std::vector<ParameterType> Vec;
            inline virtual ResultType operator() (const Vec &, const Vec &) const = 0;
            inline virtual ResultType operator() (ParameterType *, ParameterType *, const int) const = 0;
            inline virtual bool isBetter (const ResultType, const ResultType) const;
            inline virtual ResultType getWorst () const;
        };

        template < class ResultType, class ParameterType >
        struct Similarity : public Distance<ResultType, ParameterType> {
            inline virtual bool isBetter (const ResultType, const ResultType) const final;
            inline virtual ResultType getWorst () const final;
        };

        template < class ResultType, class ParameterType >
        struct EuclideanDistance : public Distance<ResultType, ParameterType> {
            typedef typename Distance<ResultType, ParameterType>::Vec Vec;
            inline virtual ResultType operator() (const Vec &, const Vec &) const final;
            inline virtual ResultType operator() (ParameterType *, ParameterType *, const int) const final;
        };

        template < class ResultType, class ParameterType >
        struct ChiSquareDistance : public Distance<ResultType, ParameterType> {
            typedef typename Distance<ResultType, ParameterType>::Vec Vec;
            inline virtual ResultType operator() (const Vec &, const Vec &) const final;
            inline virtual ResultType operator() (ParameterType *, ParameterType *, const int) const final;
        };

        template < class ResultType, class ParameterType >
        struct ManhattanDistance : public Distance<ResultType, ParameterType> {
            typedef typename Distance<ResultType, ParameterType>::Vec Vec;
            inline virtual ResultType operator() (const Vec &, const Vec &) const final;
            inline virtual ResultType operator() (ParameterType *, ParameterType *, const int) const final;
        };

        template < class ResultType, class ParameterType >
        struct CosineSimilarity : public Similarity<ResultType, ParameterType> {
            typedef typename Similarity<ResultType, ParameterType>::Vec Vec;
            inline virtual ResultType operator() (const Vec &, const Vec &) const final;
            inline virtual ResultType operator() (ParameterType *, ParameterType *, const int) const final;
        };
    }
}

namespace AFX {
    namespace Parser {

    }

    namespace Measure {
        template < class ResultType, class ParameterType >
        bool Distance<ResultType, ParameterType>::isBetter (const ResultType _prev, const ResultType _post) const {
            return _prev <= _post;
        }

        template < class ResultType, class ParameterType >
        ResultType Distance<ResultType, ParameterType>::getWorst () const {
            return std::numeric_limits<ResultType>::max ();
        }

        template < class ResultType, class ParameterType >
        bool Similarity<ResultType, ParameterType>::isBetter (const ResultType _prev, const ResultType _post) const {
            return _prev >= _post;
        }

        template < class ResultType, class ParameterType >
        ResultType Similarity<ResultType, ParameterType>::getWorst () const {
            return ResultType(0);
        }

        template < class ResultType, class ParameterType >
        ResultType EuclideanDistance<ResultType, ParameterType>::operator() (const EuclideanDistance::Vec & _prev, const EuclideanDistance::Vec & _post) const {
            DEBUG_ASSERT (_prev.size () == _post.size ())
            ResultType _res (0), _Two (2);
            int _sz = int (_prev.size ());
            for (int _idx = 0; _idx < _sz; ++ _idx) {
                _res += std::pow (_prev[_idx] - _post[_idx], _Two);
            }
            return std::sqrt (_res);
        }

        template < class ResultType, class ParameterType >
        ResultType EuclideanDistance<ResultType, ParameterType>::operator() (ParameterType * _prev, ParameterType * _post, const int size) const{
            ResultType _res (0), _Two (2);
            for (int _idx = 0; _idx < size; ++ _idx) {
                _res += std::pow (_prev[_idx] - _post[_idx], _Two);
            }
            return std::sqrt (_res);
        }

        template < class ResultType, class ParameterType >
        ResultType ChiSquareDistance<ResultType, ParameterType>::operator() (const ChiSquareDistance::Vec & _prev, const ChiSquareDistance::Vec & _post) const {
            DEBUG_ASSERT (_prev.size () == _post.size ())
            ResultType _res (0), _Zero (0), _Half (0.5);
            int _sz = int (_prev.size ());
            for (int _idx = 0; _idx < _sz; ++ _idx) {
                const ParameterType _diff = _prev[_idx] - _post[_idx], _sum = _prev[_idx] + _post[_idx];
                if (_sum > _Zero) {
                    _res += _Half * _diff * _diff / _sum;
                }
            }
            return _res;
        }

        template < class ResultType, class ParameterType >
        ResultType ChiSquareDistance<ResultType, ParameterType>::operator() (ParameterType * _prev, ParameterType * _post, const int size) const {
            ResultType _res (0), _Zero (0), _Half (0.5);
            for (int _idx = 0; _idx < size; ++ _idx) {
                const ParameterType _diff = _prev[_idx] - _post[_idx], _sum = _prev[_idx] + _post[_idx];
                if (_sum > _Zero) {
                    _res += _Half * _diff * _diff / _sum;
                }
            }
            return _res;
        }

        template < class ResultType, class ParameterType >
        ResultType ManhattanDistance<ResultType, ParameterType>::operator() (const ManhattanDistance::Vec & _prev, const ManhattanDistance::Vec & _post) const {
            DEBUG_ASSERT (_prev.size () == _post.size ())
            ResultType _res (0);
            int _sz = int (_prev.size ());
            for (int _idx = 0; _idx < _sz; ++ _idx) {
                _res += std::abs (_prev[_idx] - _post[_idx]);
            }
            return _res;
        }

        template < class ResultType, class ParameterType >
        ResultType ManhattanDistance<ResultType, ParameterType>::operator() (ParameterType * _prev, ParameterType * _post, const int size) const {
            ResultType _res (0);
            for (int _idx = 0; _idx < size; ++ _idx) {
                _res += std::abs (_prev[_idx] - _post[_idx]);
            }
            return _res;
        }

        template < class ResultType, class ParameterType >
        ResultType CosineSimilarity<ResultType, ParameterType>::operator() (const CosineSimilarity::Vec & _prev, const CosineSimilarity::Vec & _post) const {
            DEBUG_ASSERT (_prev.size () == _post.size ())
            ResultType _ff (0), _ss (0), _fs (0);
            int _sz = int (_prev.size ());
            for (int _idx = 0; _idx < _sz; ++ _idx) {
                _ff += _prev[_idx] * _prev[_idx];
                _ss += _post[_idx] * _post[_idx];
                _fs += _prev[_idx] * _post[_idx];
            }
            return _fs / (std::sqrt (_ff) * std::sqrt (_ss));
        }

        template < class ResultType, class ParameterType >
        ResultType CosineSimilarity<ResultType, ParameterType>::operator() (ParameterType * _prev, ParameterType * _post, const int size) const {
            ResultType _ff (0), _ss (0), _fs (0);
            for (int _idx = 0; _idx < size; ++ _idx) {
                _ff += _prev[_idx] * _prev[_idx];
                _ss += _post[_idx] * _post[_idx];
                _fs += _prev[_idx] * _post[_idx];
            }
            return _fs / (std::sqrt (_ff) * std::sqrt (_ss));
        }

        static EuclideanDistance<float, float> _euclideanDistanceHandler;
        static ChiSquareDistance<float, float> _chisquareDistanceHandler;
        static ManhattanDistance<float, float> _manhattanDistanceHandler;
        static CosineSimilarity<float, float> _cosineSimilarityHandler;
    }
}

#endif// ! _AFX_HPP_
