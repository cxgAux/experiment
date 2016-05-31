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

    typedef struct DescInfo {
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
        virtual bool isValid (const float) const;
        virtual int getBin () const;
    } HogInfo, MbhInfo;

    struct HofInfo : public DescInfo {
        float m_fThreshold;
        explicit HofInfo (const int, const int, const int, const int, const int, const int, const int, const float);
        explicit HofInfo (const HofInfo &);
        HofInfo & operator= (const HofInfo &);
        ~HofInfo ();
        virtual bool isValid (const float) const final;
        virtual int getBin () const final;
    };

    struct DescMat {
        int m_iWidth, m_iHeight, m_iBin;
        std::vector<float> m_desc;
        explicit DescMat (const int, const int, const int);
        DescMat (const DescMat &) = delete;
        DescMat & operator= (const DescMat &) = delete;
        ~DescMat ();
    };

    struct PointDesc {
        cv::Point2f m_point;
        std::vector<float> m_hog, m_hof, m_mbhX, m_mbhY;
        explicit PointDesc ();
        explicit PointDesc (const cv::Point2f &);
        explicit PointDesc (const PointDesc &);
        PointDesc & operator= (const PointDesc &) = delete;
        ~PointDesc ();
        bool isValid (const HogInfo &, const HofInfo &, const MbhInfo &) const;
    };

    struct Trajectory {
    public:
        const static char m_cDelimiter = '\t';
        int m_iCapacity;
        std::list<PointDesc> m_pointDescs;
        explicit Trajectory (const int);
        explicit Trajectory (const Trajectory &);
        Trajectory & operator= (const Trajectory &) = delete;
        virtual ~Trajectory ();
        void addPoint (const cv::Point2f &);
        virtual bool isValid (const HogInfo &, const HofInfo &, const MbhInfo &) const;
        bool isEnded () const;
        void print (const HogInfo &, const HofInfo &, const MbhInfo &) const;
        void drawOn (cv::Mat &, const cv::Point2f &, const float) const;
    };

    struct SalientTrajectory : public Trajectory {
    public:
        float m_fSaliency, m_fRatio;
        explicit SalientTrajectory (const int, const float);
        explicit SalientTrajectory (const SalientTrajectory &);
        SalientTrajectory & operator= (const SalientTrajectory &) = delete;
        ~SalientTrajectory ();
        void addPoint (const cv::Point2f &, const float, const float);
        bool isSalient () const;
    };
}

#endif// ! _STRUCTS_HPP_
