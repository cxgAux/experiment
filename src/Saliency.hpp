#ifndef _SALIENCY_HPP_
#define _SALIENCY_HPP_

#include "afx.hpp"
//to test
#include "VisualProc.hpp"

float Normalize(cv::Mat & saliencyMap, const int & normRange) {
    int _width = saliencyMap.cols, _height = saliencyMap.rows;
    float _maxVal(0), _minVal(FLT_MAX);
    for(int iHeight = 0; iHeight < _height; ++ iHeight) {
        float * _pSaliencyMap = saliencyMap.ptr<float>(iHeight);
        for(int iWidth = 0; iWidth < _width; ++ iWidth) {
            if(_maxVal < _pSaliencyMap[iWidth]) {
                _maxVal = _pSaliencyMap[iWidth];
            }
            if(_minVal > _pSaliencyMap[iWidth]) {
                _minVal = _pSaliencyMap[iWidth];
            }
        }
    }
    float _range = _maxVal - _minVal, _avgSaliency = 0;
    if(0 == _range) {
        _range = 1;
    }
    /**
     *  @warn   _averageSaliency overflow?
     */
    for(int iHeight = 0; iHeight < _height; ++ iHeight) {
        float * _pSaliencyMap = saliencyMap.ptr<float>(iHeight);
        for(int iWidth = 0; iWidth < _width; ++ iWidth) {
            _pSaliencyMap[iWidth] = normRange * (_pSaliencyMap[iWidth] - _minVal) / _range;
            _avgSaliency += _pSaliencyMap[iWidth];
        }
    }
    return _avgSaliency / (_width * _height);
}

void GaussianSmoothC1(const cv::Mat & originImg, cv::Mat & _smoothImg, const std::vector<float> & kernel) {
    int _width = originImg.cols, _height = originImg.rows, _center = int(kernel.size()) / 2, _realIdx = 0;
    _smoothImg.create(_height, _width, CV_32FC1);
    cv::Mat _buffer(originImg.size(), CV_32FC1);
    //blur in x direction
    for(int iHeight = 0; iHeight < _height; ++ iHeight) {
        for(int iWidth = 0; iWidth < _width; ++ iWidth) {
            float _weightedSum = 0, _kernelSum = 0;
            const uchar *pOrigin = originImg.ptr<uchar>(iHeight);
            float *pBuffer = _buffer.ptr<float>(iHeight);
            for(int jWidth = - _center; jWidth <= _center; ++ jWidth) {
                _realIdx = jWidth + iWidth;
                if(_realIdx >= 0 && _realIdx < _width) {
                    _weightedSum += kernel[_center + jWidth] * pOrigin[_realIdx];
                    _kernelSum += kernel[_center + jWidth];
                }
            }
            pBuffer[iWidth] = _weightedSum / _kernelSum;
        }
    }
    //blur in y direction
    for(int iHeight = 0; iHeight < _height; ++ iHeight) {
        for(int iWidth = 0; iWidth < _width; ++ iWidth) {
            float _weightedSum = 0, _kernelSum = 0;
            float *pSmooth = _smoothImg.ptr<float>(iHeight);
            for(int jHeight = - _center; jHeight <= _center; ++ jHeight) {
                _realIdx = jHeight + iHeight;
                if(_realIdx >= 0 && _realIdx < _height) {
                    _weightedSum += kernel[_center + jHeight] * _buffer.ptr<float>(_realIdx)[iWidth];
                    _kernelSum += kernel[_center + jHeight];
                }
            }
            pSmooth[iWidth] = _weightedSum / _kernelSum;
        }
    }
}

void integralC1(const cv::Mat & originImg, std::vector<float> & _res) {
    int _width = originImg.cols, _height = originImg.rows, _idx = 0;
    for(int iHeight = 0; iHeight < _height; ++ iHeight) {
        const float *pOrigin = originImg.ptr<float>(iHeight);
        float _rowSum(0);
        for(int iWidth = 0; iWidth < _width; ++ iWidth, ++ _idx) {
            _rowSum += pOrigin[iWidth];
            if(0 == iHeight) {
                _res[_idx] = _rowSum;
            }
            else {
                _res[_idx] = _res[_idx - _width] + _rowSum;
            }
        }
    }
}

float getIntegralSum(const std::vector<float> & intImg, int xf, int yf, int xs, int ys, int height, int width) {
    yf = std::max<int>(0, yf);
    xf = std::max<int>(0, xf);
    ys = std::min<int>(height - 1, ys);
    xs = std::min<int>(width - 1, xs);

    /*
    (0, 0)         (0, 1)       ....                   (0, width - 1)
    (1, 0)
      .
      .                     (xf, yf)
      .
      .                                      (xs, ys)
      .
    (height - 1, 0)                                    (height - 1, width - 1)
    */
    int _idxTL = (yf - 1) * width + (xf - 1),/*(xf, yf) is inclusive*/
        _idxTR = (yf - 1) * width + xs,
        _idxBL = ys * width + (xf - 1),
        _idxBR = ys * width + xs;
    float _intTL(0), _intTR(0), _intBL(0), _intBR(0);
    if(yf >= 1) {
        if(xf >= 1) {
            _intTL = intImg[_idxTL];
        }
        if(xs >= 0) {
            _intTR = intImg[_idxTR];
        }
    }
    if(ys >= 0){
        if(xf >= 1) {
            _intBL = intImg[_idxBL];
        }
        if(xs >= 0) {
            _intBR = intImg[_idxBR];
        }
    }

    float _regionSum = _intBR - _intBL - _intTR + _intTL, _area = (xs - xf + 1) * (ys - yf + 1);
    _regionSum /= _area;
    return _regionSum;
}

float calculateAppearcanceSaliencyMap(const cv::Mat & grey, cv::Mat & saliencyMap) {
    int _width = grey.cols, _height = grey.rows;
    saliencyMap.create(_height, _width, CV_32FC1);
    std::vector<float> _kernel{1.0f, 2.0f, 1.0f};
    cv::Mat _smoothImg;
    GaussianSmoothC1(grey, _smoothImg, _kernel);
    std::vector<float> _intImg(_width * _height);
    integralC1(_smoothImg, _intImg);
    //maximal symmetric region
    for(int iHeight = 0; iHeight < _height; ++ iHeight) {
        int _yoff = std::min<int>(iHeight, _height - iHeight);
        float * _pSaliencyMap = saliencyMap.ptr<float>(iHeight);
        for(int iWidth = 0; iWidth < _width; ++ iWidth) {
            int _xoff = std::min<int>(iWidth, _width - iWidth);
            //maximal symmetric area
            float _msa = getIntegralSum(_intImg, iWidth - _xoff, iHeight - _yoff, iWidth + _xoff, iHeight + _yoff, _height, _width);
            //3*3 patch to represent the center
            _yoff = std::min<int>(_yoff, 1);
            _xoff = std::min<int>(_xoff, 1);
            float _center = getIntegralSum(_intImg, iWidth - _xoff, iHeight - _yoff, iWidth + _xoff, iHeight + _yoff, _height, _width);
            _pSaliencyMap[iWidth] = std::pow(_msa - _center, 2.0f);
        }
    }
    return Normalize(saliencyMap, 255);
}

#endif// ! _SALIENCY_HPP_
