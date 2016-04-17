#ifndef _STRUCTURES_HPP_
#define _STRUCTURES_HPP_

#include "afx.hpp"

/**
 *  @brief  top-left corner index and size to represent a rectangle
 */
typedef struct RectInfo{
    int _x, _y;
    int _width, _height;
    RectInfo (int x, int y, int w, int h)
        : _x(x), _y(y), _width(w), _height(h) {
        static int __count = 200;
        char * __buff = new char [__count];
        sprintf(__buff, "Rect\n[\n\tx:\t%d\n\ty:\t%d\n\twidth:\t%d\n\theight:\t%d\n]\n", _x, _y, _width, _height);
        _log(__buff);
        delete [] __buff;
    }
    RectInfo(const RectInfo & rect) : RectInfo(rect._x, rect._y, rect._width, rect._height) {}
} RectInfo;

/**
 *  @brief  resolution and the number of frames to represent an video
 */
typedef struct SeqInfo {
    int _width, _height, _length;
    SeqInfo (int w, int h, int len)
        : _width(w), _height(h), _length(len) {
            static int __count = 200;
            char * __buff = new char [__count];
            sprintf(__buff, "Seq\n[\n\twidth:\t%d\n\theight:\t%d\n\tlength:\t%d\n]\n", _width, _height, _length);
            _log(__buff);
            delete [] __buff;
    }
    SeqInfo(const SeqInfo & seq) : SeqInfo(seq._width, seq._height, seq._length) {}
} SeqInfo;

SeqInfo createWith_resolution_w_h_frames_length (int w, int h, int len) {
    return SeqInfo(w, h, len);
}

SeqInfo fromVideo(cv::VideoCapture & _video) {
    cv::Mat _frame;
    int _w = 0, _h = 0, _fc = 0;
    while(true) {
        _video >> _frame;
        if(_frame.empty()) {
            break;
        }
        if(_fc == 0) {
            _w = _frame.cols;
            _h = _frame.rows;
        }
        _fc ++;
    }
    _video.set(cv::CAP_PROP_POS_FRAMES , 0);
    return SeqInfo(_w, _h, _fc);
}

/**
 *  @brief  parameters for tracking
 */
typedef struct TrackInfo {
    int _length, _gap;
    TrackInfo (int len, int gap)
        : _length(len), _gap(gap) {
        static int __count = 200;
        char * __buff = new char [__count];
        sprintf(__buff, "Tracking parameters\n[\n\ttrajectory length:\t%d\n\tSample gap:\t%d\n]\n", _length, _gap);
        _log(__buff);
        delete [] __buff;
    }
    TrackInfo(const TrackInfo & track) : TrackInfo(track._length, track._gap) {}
} TrackInfo;

TrackInfo createWith_TrajLength_SampleGap (int len, int gap) {
    return TrackInfo(len, gap);
}

/**
 *  @brief  parameters for descriptor
 */
typedef struct DescInfo {
    bool _isHof;// flag to indicate is hof desc
    int _nxCells, _nyCells, _ntCells, _nBins;// hwo to partition the spacial-temporal cube
    int _dim;// the dimension(Without temporal dimension)
    int _width, _height;// the size of the cross-section
    DescInfo (bool isHof, int nxCells, int nyCells, int ntCells, int nBins, int w, int h)
        : _isHof(isHof), _nxCells(nxCells), _nyCells(nyCells), _ntCells(ntCells), _nBins(nBins), _dim(_nxCells * _nyCells * _nBins), _width(w), _height(h) {
        static int __count = 200;
        char * __buff = new char [__count];
        sprintf(__buff, "Descriptor infomation\n[\n\tis type of Hof:\t%d\n\t(nxCells, nyCells, ntCells, nBins):\t(%d, %d, %d, %d)-->dim: %d\n\tcross-section size(w, h):\t(%d, %d)\n]\n",
            _isHof, _nxCells, _nyCells, _ntCells, _nBins, _dim, _width, _height);
        _log(__buff);
        delete [] __buff;
    }
    DescInfo(const DescInfo & desc) : DescInfo(desc._isHof, desc._nxCells, desc._nyCells, desc._ntCells, desc._nBins, desc._width, desc._height) {}
} DescInfo;

DescInfo createWith_isHofFlag_cubePartition_xy_t_b_crossSection_w_h (bool isHof, int nxyCells, int ntCells, int nBins, int w, int h) {
    return DescInfo(isHof, nxyCells, nxyCells, ntCells, nBins, w, h);
}

RectInfo createWith_center_w_h_descInfo (const cv::Point2f & point, int width, int height, const DescInfo & descInfo) {
    int _minX = descInfo._width / 2, _minY = descInfo._height / 2, _maxX = width - descInfo._width, _maxY = height - descInfo._height;
    return RectInfo(std::min<int>(std::max<int>(cvRound(point.x) - _minX, 0), _maxX), std::min<int>(std::max<int>(cvRound(point.y) - _minY, 0), _maxY), descInfo._width, descInfo._height);
}

/**
 *  @brief  descriptor structure
 */
typedef struct DescMat {
    int _width, _height, _nBins;
    float * _desc;
    DescMat (int w, int h, int bins)
        : _width(w), _height(h), _nBins(bins), _desc(new float[_width * _height * _nBins]) {
        static int __count = 200;
        char * __buff = new char [__count];
        sprintf(__buff, "New Descriptor\n[\n\twidth:\t%d\n\theight:\t%d\n\tnBins:\t%d\n]\n", _width, _height, _nBins);
        _log(__buff);
        delete [] __buff;
    }
    DescMat (const DescMat & mat) : DescMat(mat._width, mat._height, mat._nBins) {
        std::copy_n(mat._desc, _width * _height * _nBins, _desc);
    }
} DescMat;

DescMat * createWith_w_h_b (int w, int h, int b) {
    return new DescMat(w, h, b);
}

void release(DescMat * descMat) {
    if(descMat != nullptr) {
        _log("descMat release\n")
        if(descMat->_desc != nullptr) {
            delete [] descMat->_desc;
            descMat->_desc = nullptr;
        }
        delete descMat;
        descMat = nullptr;
    }
}

/**
 *  @brief  trajectory structure
 */
class SamplePoints {
public:
    int _idx;
    std::vector<cv::Point2f> _points;
    SamplePoints (const TrackInfo & trackInfo, const cv::Point2f & point)
        : _idx(0), _points(trackInfo._length + 1) {
            /**
             *  @warn   In this version we constraint the size of the points so push_back will rise a logical error!
             */
            _points[_idx] = point;
    }
    SamplePoints (const SamplePoints & sample) : _idx(sample._idx), _points(sample._points) {}
    SamplePoints & operator=(const SamplePoints & sample) {
        this->_idx = sample._idx;
        this->_points = sample._points;
        return *this;
    }
    void _addPoint (const cv::Point2f & point) {
        _points[++ _idx] = point;
    }
    virtual ~SamplePoints() {}
    //<2016/04/01 22:46> remain unimplemted
    void computeTraj ();
};

class Histogram : public SamplePoints {
public:
    std::vector<float> _hog, _hof, _mbhx, _mbhy;
    Histogram (const TrackInfo & trackInfo, const cv::Point2f & point, const DescInfo & hogInfo, const DescInfo & hofInfo, const DescInfo & mbhInfo)
        : SamplePoints(trackInfo, point), _hog(hogInfo._dim * trackInfo._length), _hof(hofInfo._dim * trackInfo._length), _mbhx(mbhInfo._dim * trackInfo._length), _mbhy(mbhInfo._dim * trackInfo._length){}
    Histogram (const Histogram & hist) : SamplePoints(hist), _hog(hist._hog), _hof(hist._hof), _mbhx(hist._mbhx), _mbhy(hist._mbhy) {}
    Histogram & operator=(const Histogram & hist) {
        SamplePoints::operator=(hist);
        this->_hog = hist._hog;
        this->_hof = hist._hof;
        this->_mbhx = hist._mbhx;
        this->_mbhy = hist._mbhy;
        return *this;
    }
    virtual ~Histogram() {}
};

class Trajectory : public Histogram {
public:
    int _start_frame;
    float _saliency, _averageSaliency;
    Trajectory (const cv::Point2f & point, const TrackInfo & trackInfo, const DescInfo & hogInfo, const DescInfo & hofInfo, const DescInfo & mbhInfo, int start_frame, float saliency = 0.f, float averageSaliency = 0.f)
        : Histogram(trackInfo, point, hogInfo, hofInfo, mbhInfo), _start_frame(start_frame), _saliency(saliency), _averageSaliency(averageSaliency) {

    }
    Trajectory (const Trajectory & traj)
        : Histogram(traj), _start_frame(traj._start_frame), _saliency(traj._saliency), _averageSaliency(traj._averageSaliency) {}
    Trajectory & operator=(const Trajectory & traj) {
        Histogram::operator=(traj);
        this->_start_frame = traj._start_frame;
        this->_saliency = traj._saliency;
        this->_averageSaliency = traj._averageSaliency;
        return *this;
    }
    ~Trajectory() {}
    void addPoint (const cv::Point2f & point, float saliency, float avgSaliency) {
        _addPoint(point);
        this->_saliency += saliency;
        this->_averageSaliency += avgSaliency;
    }
};

/**
 *  @brief  kernel Matrix base creation
 */
cv::Mat createWith_bins_nBins_kernelRadius (int binCount, int nBins, int guassSmooth) {
    cv::Mat _res(binCount, nBins, CV_32FC1);
    float _denseBase = (2 * M_PI) / float(binCount), \
        _binBase = (2 * M_PI) / float(nBins);
    for(int iRow = 0; iRow < binCount; ++ iRow) {
        float * _pRow = _res.ptr<float>(iRow);
        for(int iCol = 0; iCol < nBins; ++ iCol) {
            float _diff_f = std::cos(iRow * _denseBase) - std::cos(iCol * _binBase), \
                _diff_s = std::sin(iRow * _denseBase) - std::sin(iCol * _binBase);
            _pRow[iCol] = std::exp(- guassSmooth * (std::pow(_diff_f, 2.0f) + std::pow(_diff_s, 2.0f)));
        }
    }
    return _res;
}

/**
 *  @brief  pixel pyramid construction and restoration
 */
void InitPry (int rows, int cols, std::vector<float> & fscales, std::vector<cv::Size> & sizes) {
    float _min_size = std::min<int>(rows, cols);
    int _nLayers = 0;
    while (_min_size >= __patch_size) {
        _min_size /= __scale_stride;
        _nLayers ++;
    }

    if(_nLayers == 0) {
        _nLayers ++;
    }

    __scale_nums = std::min<int>(__scale_nums, _nLayers);

    fscales.resize(__scale_nums);
    sizes.resize(__scale_nums);

    fscales[0] = 1.f;
    sizes[0] = cv::Size(cols, rows);

    for(int iLayer = 1; iLayer < __scale_nums; ++ iLayer) {
        fscales[iLayer] = fscales[iLayer - 1] * __scale_stride;
        sizes[iLayer] = cv::Size(cvRound(cols / fscales[iLayer]), cvRound(rows / fscales[iLayer]));
    }
}

/**
 *  @brief  consturct pyramid
 */
void BuildPyr(std::vector<cv::Size> sizes, int type, std::vector<cv::Mat> & pyramid) {
    int _nLayers = (int)sizes.size();
    pyramid.resize(_nLayers);

    for(int iLayer = 0; iLayer < _nLayers; ++ iLayer) {
        pyramid[iLayer].create(sizes[iLayer], type);
    }
}

/**
 *  @brief  trajectory delegator
 */
struct TrajectorySerializable {
    void operator()(std::ostream & os, const Trajectory & trajectory){
        os << trajectory._start_frame << __delimiter;
        for(const auto & val : trajectory._hog) {
            os << val << __delimiter;
        }
        for(const auto & val : trajectory._hof) {
            os << val << __delimiter;
        }
        for(const auto & val : trajectory._mbhx) {
            os << val << __delimiter;
        }
        for(const auto & val : trajectory._mbhy) {
            os << val << __delimiter;
        }
        os << __endofLine;
    }
    static const char __delimiter = '\t';
    static const char __endofLine = '\n';
};

#endif// ! _STRUCTURES_HPP_
