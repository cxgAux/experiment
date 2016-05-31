#ifndef _VISUAL_HPP_
#define _VISUAL_HPP_

#define DISP(wn, img, scaleIdx)   Visual::display((wn), (img), Attributes::Mode::delay, scaleIdx)

namespace Visual {
    inline void display (const char * windowName, const cv::Mat & image, const int delay, const int scaleIdx) {
        if (Attributes::Mode::toDisplay && 0 == scaleIdx) {
            cv::imshow(windowName, image);
            if (cv::waitKey(delay) == 27) {
                abort();
            }
        }
    }

    inline void buildPyramid(const std::vector<cv::Size> & sizes, std::vector<cv::Mat> & pyramid, const int type) {
        int _scaleNum = int(sizes.size());
        pyramid.resize(_scaleNum);
        REP_SCALE (_scaleNum) {
            pyramid[iScale].create(sizes[iScale], type);
        }
    }

    inline void setPyramid(const cv::Mat & img, std::vector<cv::Mat> & pyramid) {
        if(! pyramid.empty()) {
            for (auto & _img : pyramid) {
                cv::resize(img, _img, _img.size(), 0, 0, cv::INTER_AREA);
            }
            /**
             *  @brief  to sguarantee exactly same
             */
            img.copyTo(pyramid[0]);
        }
    }

    void DenseSample(const cv::Mat & image, std::vector<cv::Point2f> & points, const float quality, const float min_distance) {
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

        for (const auto & _point : points) {
            if( _point.x >= _xMax || _point.y >= _yMax) {
                continue;
            }
            int _xIdx = cvFloor(_point.x / min_distance),
                _yIdx = cvFloor(_point.y / min_distance);
            _isEmpty[_yIdx * _widthStep + _xIdx] = false;
        }
        points.clear();

        int _offset = cvFloor(min_distance / 2), _idx = 0;
        REP_HEIGHT(_heightStep) {
            REP_WIDTH(_widthStep) {
                if(_isEmpty[_idx ++]) {
                    int _cx = cvFloor(iWidth * min_distance + _offset),
                        _cy = cvFloor(iHeight * min_distance + _offset);
                    if (_eigenMat.at<float>(_cy, _cx) > _threshold) {
                        points.push_back(cv::Point2f(_cx, _cy));
                    }
                }
            }
        }
    }

    void OpticalFlowTracker(
        const cv::Mat & flow,
        const cv::Mat & salMap,
        const std::vector<cv::Point2f> & points_in,
        std::vector<cv::Point2f> & points_out,
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
            points_out.push_back(cv::Point2f(_xMid, _yMid));
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

#endif// ! _VISUAL_HPP_
