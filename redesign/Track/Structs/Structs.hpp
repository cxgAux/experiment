#ifndef _STRUCTS_HPP_
#define _STRUCTS_HPP_

#ifndef _ENABLE_OPENCV_
#define _ENABLE_OPENCV_
#endif// ! _ENABLE_OPENCV_

#ifndef _ENABLE_BOOST_
#define _ENABLE_BOOST_
#endif// ! _ENABLE_BOOST_

#ifndef _ENABLE_MACROS_
#define _ENABLE_MACROS_
#endif// ! _ENABLE_MACROS_

#include "afx.hpp"

namespace Structs {
    struct TrackerInfo {
        int m_iTrajLength, m_iInitGap;
        explicit TrackerInfo (const int, const int);
        TrackerInfo (const TrackerInfo &) = delete;
        TrackerInfo & operator= (const TrackerInfo &) = delete;
        ~TrackerInfo();
    };

    struct DescInfo {
        struct CubeInfo {
            int m_iXCells, m_iYCells, m_iTCells, m_iBlockWidth, m_iBlockHeight;
            CubeInfo (const int, const int, const int, const int, const int);
            CubeInfo (const CubeInfo &);
            CubeInfo & operator= (const CubeInfo &);
            ~CubeInfo();
        } m_cubeInfo;
        int m_iBin, m_iNormType;
        explicit DescInfo (const int, const int, const int, const int, const int, const int, const int);
        explicit DescInfo (const DescInfo &);
        DescInfo & operator= (const DescInfo &);
        virtual ~DescInfo ();
        int getDim () const;
    };

    struct DescMat {
        int m_iWidth, m_iHeight, m_iBin;
        std::vector<float> m_desc;
        explicit DescMat (const int, const int, const int);
        DescMat (const DescMat &) = delete;
        DescMat & operator= (const DescMat &) = delete;
        ~DescMat ();
    };

    struct Points {
        std::vector<cv::Point2f> m_points;
        explicit Points ();
        Points (const Points &) = delete;
        Points & operator= (const Points &) = delete;
        virtual ~Points ();
    };

    struct Histogram {
        std::vector<float> m_hog, m_hof, m_mbhX, m_mbhY;
        explicit Histogram ();
        virtual ~Histogram ();
    };

    struct Trajectory : public Points, public Histogram {
        int m_iLength, m_iCapacity;
        explicit Trajectory (const int, const int);
        virtual ~Trajectory ();
    };

    struct SalientTrajectory : public Trajectory {
        int m_iSaliency;
        explicit SalientTrajectory (const int, const int);
        ~SalientTrajectory ();
    };
}

#endif// ! _STRUCTS_HPP_
