#include "track_Structs.hpp"

namespace Structs {
    TrackerInfo::TrackerInfo (const int trajLength, const int initGap) : m_iTrajLength (trajLength), m_iInitGap (initGap) {}

    TrackerInfo::~TrackerInfo() {}
}

namespace Structs {
    DescInfo::CubeInfo::CubeInfo (const int xCells, const int yCells, const int tCells, const int width, const int height)
        : m_iXCells (xCells), m_iYCells (yCells), m_iTCells (tCells), m_iBlockWidth (width), m_iBlockHeight (height) {}

    DescInfo::CubeInfo::CubeInfo (const CubeInfo & cubeInfo)
        : CubeInfo (cubeInfo.m_iXCells, cubeInfo.m_iYCells, cubeInfo.m_iTCells, cubeInfo.m_iBlockWidth, cubeInfo.m_iBlockHeight) {}

    DescInfo::CubeInfo & DescInfo::CubeInfo::operator= (const CubeInfo & cubeInfo) {
        this->m_iXCells = cubeInfo.m_iXCells;
        this->m_iYCells = cubeInfo.m_iYCells;
        this->m_iTCells = cubeInfo.m_iTCells;
        this->m_iBlockWidth = cubeInfo.m_iBlockWidth;
        this->m_iBlockHeight = cubeInfo.m_iBlockHeight;
        return *this;
    }

    DescInfo::CubeInfo::~CubeInfo () {}


    DescInfo::DescInfo (const int xCells, const int yCells, const int tCells, const int width, const int height, const int bins, const int normType)
        : m_cubeInfo (xCells, yCells, tCells, width, height), m_iBin (bins), m_iNormType (normType) {}

    DescInfo::DescInfo (const DescInfo & descInfo)
        : m_cubeInfo (descInfo.m_cubeInfo), m_iBin (descInfo.m_iBin), m_iNormType (descInfo.m_iNormType) {}

    DescInfo & DescInfo::operator= (const DescInfo & descInfo) {
        this->m_cubeInfo = descInfo.m_cubeInfo;
        this->m_iBin = descInfo.m_iBin;
        this->m_iNormType = descInfo.m_iNormType;
        return *this;
    }

    DescInfo::~DescInfo () {}

    int DescInfo::getDim () const {
        return this->m_cubeInfo.m_iXCells * this->m_cubeInfo.m_iYCells * this->m_iBin;
    }

    bool DescInfo::isValid (const float value) const {
        return true;
    }

    int DescInfo::getBin () const {
        return this->m_iBin;
    }


    HofInfo::HofInfo (const int xCells, const int yCells, const int tCells, const int width, const int height, const int bins, const int normType, const float threshold)
        : DescInfo (xCells, yCells, tCells, width, height, bins, normType), m_fThreshold (threshold) {}

    HofInfo::HofInfo (const HofInfo & hofInfo)
        : DescInfo(hofInfo), m_fThreshold(hofInfo.m_fThreshold) {}

    HofInfo & HofInfo::operator= (const HofInfo & hofInfo) {
        DescInfo::operator= (*this);
        this->m_fThreshold = hofInfo.m_fThreshold;
        return *this;
    }

    HofInfo::~HofInfo () {}

    bool HofInfo::isValid(const float value) const {
        return value > this->m_fThreshold;
    }

    int HofInfo::getBin () const {
        return DescInfo::getBin () - 1;
    }
}

namespace Structs {
    DescMat::DescMat (const int width, const int height, const int bin)
        : m_iWidth (width), m_iHeight (height), m_iBin (bin), m_desc (width * height * bin, 0.f) {}

    DescMat::~DescMat() {}
}

namespace Structs {
    PointDesc::PointDesc () {}

    PointDesc::PointDesc (const cv::Point2f & point)
        : m_point (point) {}

    PointDesc::PointDesc (const PointDesc & point_desc)
        : m_point (point_desc.m_point), m_hog (point_desc.m_hog), m_hof (point_desc.m_hof), m_mbhX (point_desc.m_mbhX), m_mbhY (point_desc.m_mbhY) {}

    PointDesc::~PointDesc () {}

    bool PointDesc::isValid (const HogInfo & hogInfo, const HofInfo & hofInfo, const MbhInfo & mbhInfo) const {
        return
            int (this->m_hog.size ()) == hogInfo.getDim () &&
            int (this->m_hof.size ()) == hofInfo.getDim () &&
            int (this->m_mbhX.size ()) == mbhInfo.getDim () &&
            int (this->m_mbhY.size ()) == mbhInfo.getDim ();
    }

    Trajectory::Trajectory (const int capcacity)
        : m_iCapacity (capcacity), m_pointDescs (0) {}

    Trajectory::Trajectory (const Trajectory & traj)
        :  m_iCapacity (traj.m_iCapacity), m_pointDescs (traj.m_pointDescs) {}

    Trajectory::~Trajectory() {}

    void Trajectory::addPoint (const cv::Point2f & point) {
        this->m_pointDescs.push_back (PointDesc (point));
    }

    bool Trajectory::isValid (const HogInfo & hogInfo, const HofInfo & hofInfo, const MbhInfo & mbhInfo) const {
        bool _isValid = this->isEnded ();
        if (_isValid)
        std::for_each (this->m_pointDescs.begin(), this->m_pointDescs.end(), [& _isValid, hogInfo, hofInfo, mbhInfo] (const PointDesc & _point_desc) {_isValid &= _point_desc.isValid(hogInfo, hofInfo, mbhInfo);});
        return _isValid;
    }

    bool Trajectory::isEnded () const {
        return (int (this->m_pointDescs.size()) == this->m_iCapacity);
    }

    void Trajectory::print (const HogInfo & hogInfo, const HofInfo & hofInfo, const MbhInfo & mbhInfo) const {
        int _dim = hogInfo.getDim (), _tCells = hogInfo.m_cubeInfo.m_iTCells, _tStride = cvFloor (this->m_iCapacity / _tCells);
        std::vector <float> _vec;
        std::list<PointDesc>::const_iterator _iDesc = this->m_pointDescs.begin();

        for (int _iCell = 0; _iCell < _tCells; ++ _iCell) {
            _vec.clear ();
            _vec.resize (_dim);
            for (int _iT = 0; _iT < _tStride; ++ _iT, ++ _iDesc) {
                for (int iDim = 0; iDim < _dim; ++ iDim) {
                    _vec[iDim] += _iDesc->m_hog[iDim];
                }
            }
            for (const auto & _desc : _vec) {
                std::cout << _desc / _tStride << m_cDelimiter;
            }
        }

        _dim = hofInfo.getDim (); _tCells = hofInfo.m_cubeInfo.m_iTCells; _tStride = cvFloor (this->m_iCapacity / _tCells);
        _iDesc = this->m_pointDescs.begin();
        for (int _iCell = 0; _iCell < _tCells; ++ _iCell) {
            _vec.clear ();
            _vec.resize (_dim);
            for (int _iT = 0; _iT < _tStride; ++ _iT, ++ _iDesc) {
                for (int iDim = 0; iDim < _dim; ++ iDim) {
                    _vec[iDim] += _iDesc->m_hof[iDim];
                }
            }
            for (const auto & _desc : _vec) {
                std::cout << _desc / _tStride << m_cDelimiter;
            }
        }

        _dim = mbhInfo.getDim (); _tCells = mbhInfo.m_cubeInfo.m_iTCells; _tStride = cvFloor (this->m_iCapacity / _tCells);
        _iDesc = this->m_pointDescs.begin();
        for (int _iCell = 0; _iCell < _tCells; ++ _iCell) {
            _vec.clear ();
            _vec.resize (_dim);
            for (int _iT = 0; _iT < _tStride; ++ _iT, ++ _iDesc) {
                for (int iDim = 0; iDim < _dim; ++ iDim) {
                    _vec[iDim] += _iDesc->m_mbhX[iDim];
                }
            }
            for (const auto & _desc : _vec) {
                std::cout << _desc / _tStride << m_cDelimiter;
            }
        }

        _dim = mbhInfo.getDim (); _tCells = mbhInfo.m_cubeInfo.m_iTCells; _tStride = cvFloor (this->m_iCapacity / _tCells);
        _iDesc = this->m_pointDescs.begin();
        for (int _iCell = 0; _iCell < _tCells; ++ _iCell) {
            _vec.clear ();
            _vec.resize (_dim);
            for (int _iT = 0; _iT < _tStride; ++ _iT, ++ _iDesc) {
                for (int iDim = 0; iDim < _dim; ++ iDim) {
                    _vec[iDim] += _iDesc->m_mbhY[iDim];
                }
            }
            for (const auto & _desc : _vec) {
                std::cout << _desc / _tStride << m_cDelimiter;
            }
        }
        std::cout << "\n";
    }

    void Trajectory::drawOn (cv::Mat & image, const cv::Point2f & endPoint, const float fscale) const {
        int _len = int(m_pointDescs.size()), _idx = 0;
        cv::Point _prev, _cur;
        for (const auto & val : this->m_pointDescs) {
            _cur = cv::Point2f (val.m_point.x * fscale, val.m_point.y * fscale);
            if (_idx != 0) {
                cv::line (image, _prev, _cur, CV_RGB (0, cvFloor (255.0 * _idx / _len), 0), 2, 8, 0);
            }
            _idx ++;
            _prev = _cur;
        }
        _cur = cv::Point2f (endPoint.x * fscale, endPoint.y * fscale);
        cv::line (image, _prev, _cur, CV_RGB (0, cvFloor (255.0 * _idx / _len), 0), 2, 8, 0);
        cv::circle (image, endPoint, 2, CV_RGB (255, 0, 0), -1, 8, 0);
    }


    SalientTrajectory::SalientTrajectory (const int capacity, const float ratio)
        : Trajectory (capacity), m_fSaliency (0.f), m_fRatio (ratio) {}

    SalientTrajectory::SalientTrajectory (const SalientTrajectory & st)
        : Trajectory (st), m_fSaliency (st.m_fSaliency), m_fRatio (st.m_fRatio) {}

    SalientTrajectory::~SalientTrajectory () {}

    void SalientTrajectory::addPoint (const cv::Point2f & point, const float trajSaliency, const float frameSaliency) {
        Trajectory::addPoint (point);
        this->m_fSaliency += trajSaliency - this->m_fRatio * frameSaliency;
    }

    bool SalientTrajectory::isSalient () const {
        return this->m_fSaliency >= 0;
    }
}
