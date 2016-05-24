#ifndef _VISUALTASK_HPP
#define _VISUALTASK_HPP

namespace cxgAlleria {
    #include "afx.hpp"

    void DenseSample(const cv::Mat & image, std::vector<CvPoint2D32f> & points, const float quality, const float min_distance) {
        const int _width = image.cols, _height = image.rows,
            _widthStep = cvFloor(_width / min_distance), _heightStep = cvFloor(_height / min_distance);

        cv::Mat _eigenMat;
        cv::cornerMinEigenVal(image, _eigenMat, 3, 3);
        double _maxVal = 0;
        cv::minMaxLoc(_eigenMat, 0, & _maxVal);
        const float _threshold = float(_maxVal) * quality;

        std::vector<bool> _isEmpty(_widthStep * _heightStep, true);
        const float _xMax = min_distance * _widthStep,
            _yMax = min_distance * _heightStep;
        for (std::vector<CvPoint2D32f>::const_iterator it = points.begin(); it != points.end(); ++ it) {
            if( it->x >= _xMax || it->y >= _yMax) {
                continue;
            }
            int _xIdx = cvFloor(it->x / min_distance),
                _yIdx = cvFloor(it->y / min_distance);
            _isEmpty[_yIdx * _widthStep + _xIdx] = false;
        }
        points.clear();

        int _offset = cvFloor(min_distance / 2), _idx = 0;
        HREP(_heightStep) {
            WREP(_widthStep) {
                if(_isEmpty[_idx ++]) {
                    int _cx = cvFloor(iWidth * min_distance + _offset),
                        _cy = cvFloor(iHeight * min_distance + _offset);
                    if (_eigenMat.at<float>(_cy, _cx) > _threshold) {
                        points.push_back(CvPoint2D32f(_cx, _cy));
                    }
                }
            }
        }
    }

    void OpticalFlowTracker(
        const cv::Mat & flow,
        const cv::Mat & salMap,
        const std::vector<CvPoint2D32f> & points_in,
        std::vector<CvPoint2D32f> & points_out,
        std::vector<bool> & status,
        std::vector<float> & saliency
    ) {
        points_out.clear();
        status.clear();
        saliency.clear();

        int _width = flow.cols, _height = flow.rows;
        std::vector<float> _xFilter, _yFilter, _sFilter;
        for(const auto & _point : points_in) {
            _xFilter.clear(); _yFilter.clear(); _sFilter.clear();
            const int _x = cvFloor(_point.x), _y = cvFloor(_point.y);
            for (int _xOff = -1; _xOff <= 1; ++ _xOff) {
                for (int _yOff = -1; _yOff <= 1; ++ _yOff) {
                    const int _x_ = std::min<int>(std::max<int>(_x + _xOff, 0), _width - 1),
                        _y_ = std::min<int>(std::max<int>(_y + _yOff, 0), _height - 1);
                    const float * _pFlow = flow.ptr<float>(_y_);
                    _xFilter.push_back(_pFlow[2 * _x_]);
                    _yFilter.push_back(_pFlow[2 * _x_ + 1]);
                    _sFilter.push_back(salMap.at<float>(_y_, _x_));
                }
            }
            std::sort(_xFilter.begin(), _xFilter.end());
            std::sort(_yFilter.begin(), _yFilter.end());
            std::sort(_sFilter.begin(), _sFilter.end());

            const size_t _midIdx = _xFilter.size() / 2;
            const float _xMid = _point.x + _xFilter[_midIdx], _yMid = _point.y + _yFilter[_midIdx];
            points_out.push_back(cvPoint2D32f(_xMid, _yMid));
            saliency.push_back(_sFilter[_midIdx]);

            if(_xMid > 0 && _xMid < _width && _yMid > 0 && _yMid < _height) {
                status.push_back(true);
            }
            else {
                status.push_back(false);
            }
        }
    }
}

#endif// ! _VISUALTASK_HPP
