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
        std::vector<CvPoint2D32f> & points,
        std::vector<bool> & status,
        std::vector<float> & saliency
    ) {
        
    }
}

#endif// ! _VISUALTASK_HPP
