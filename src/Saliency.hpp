#ifndef _SALIENCY_HPP_
#define _SALIENCY_HPP_

#include "afx.hpp"
//to test
#include "VisualProc.hpp"
#include "Structures.hpp"

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
    _avgSaliency /= (_width * _height);
    return _avgSaliency;
}


/**
 *  @brief  Apperance Saliency Part
 */
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

/**
 *  @brief  Motion Saliency Part
 */

 void GaussianSmoothC2(const cv::Mat & originImg, cv::Mat & _smoothImg, const std::vector<float> & kernel) {
     int _width = originImg.cols, _height = originImg.rows, _center = int(kernel.size()) / 2, _realIdx = 0;
     _smoothImg.create(_height, _width, CV_32FC2);
     cv::Mat _buffer(originImg.size(), CV_32FC2);
     //blur in x direction
     for(int iHeight = 0; iHeight < _height; ++ iHeight) {
         for(int iWidth = 0; iWidth < _width; ++ iWidth) {
             float _xWeightedSum = 0, _yWeightedSum = 0, _kernelSum = 0;
             const float *pOrigin = originImg.ptr<float>(iHeight);
             float *pBuffer = _buffer.ptr<float>(iHeight);
             for(int jWidth = - _center; jWidth <= _center; ++ jWidth) {
                 _realIdx = jWidth + iWidth;
                 if(_realIdx >= 0 && _realIdx < _width) {
                     _xWeightedSum += kernel[_center + jWidth] * pOrigin[2 * _realIdx];
                     _yWeightedSum += kernel[_center + jWidth] * pOrigin[2 * _realIdx + 1];
                     _kernelSum += kernel[_center + jWidth];
                 }
             }
             pBuffer[2 * iWidth] = _xWeightedSum / _kernelSum;
             pBuffer[2 * iWidth + 1] = _yWeightedSum / _kernelSum;
         }
     }
     //blur in y direction
     for(int iHeight = 0; iHeight < _height; ++ iHeight) {
         for(int iWidth = 0; iWidth < _width; ++ iWidth) {
             float _xWeightedSum = 0, _yWeightedSum = 0, _kernelSum = 0;
             float *pSmooth = _smoothImg.ptr<float>(iHeight);
             for(int jHeight = - _center; jHeight <= _center; ++ jHeight) {
                 _realIdx = jHeight + iHeight;
                 if(_realIdx >= 0 && _realIdx < _height) {
                     _xWeightedSum += kernel[_center + jHeight] * _buffer.ptr<float>(_realIdx)[2 * iWidth];
                     _yWeightedSum += kernel[_center + jHeight] * _buffer.ptr<float>(_realIdx)[2 * iWidth + 1];
                     _kernelSum += kernel[_center + jHeight];
                 }
             }
             pSmooth[2 * iWidth] = _xWeightedSum / _kernelSum;
             pSmooth[2 * iWidth + 1] = _yWeightedSum / _kernelSum;
         }
     }
 }

 void integral(const cv::Mat & flow, int height, int width, int nBins, const cv::Mat & kernelMatrix, std::vector<float> & intImg) {
     const float __2pi = 2 * M_PI, _denseBase = __2pi / float(kernelMatrix.rows);
     int _idx = 0;

     for(int iHeight = 0; iHeight < height; ++ iHeight) {
         const float * pFlow = flow.ptr<float>(iHeight);
         std::vector<float> _sum(nBins, 0.f);
         for(int iWidth = 0; iWidth < width; ++ iWidth, ++ _idx) {
             float _xflow = pFlow[2 * iWidth], _yflow = pFlow[2 * iWidth + 1], _magnitude = std::sqrt(_xflow * _xflow + _yflow * _yflow);
             if(_magnitude > __min_flow) {
                 float _orientation = cv::fastAtan2(_yflow, _xflow);
                 while(_orientation >= __2pi) {
                     _orientation -= __2pi;
                 }
                 while(_orientation < 0) {
                     _orientation += __2pi;
                 }

                 int _iDense = static_cast<int>(cvRound(_orientation / _denseBase));
                 if(_iDense >= kernelMatrix.rows) {
                     _iDense = 0;
                 }

                 //apply kernel histogram
                 const float * _kernel = kernelMatrix.ptr<float>(_iDense);
                 for(int iBin = 0; iBin < nBins; ++ iBin) {
                     _sum[iBin] += _magnitude * _kernel[iBin];
                 }
             }
             int _tmpf = _idx * nBins, _tmps = 0;
             if(iHeight == 0) {
                 for(int iBin = 0; iBin < nBins; ++ iBin) {
                     intImg[_tmpf ++] = _sum[iBin];
                 }
             }
             else {
                 _tmps = (_idx - width) * nBins;
                 for(int iBin = 0; iBin < nBins; ++ iBin) {
                     intImg[_tmpf ++] = intImg[_tmps ++] + _sum[iBin];
                 }
             }
         }
     }
 }

void integral(const cv::Mat & xFlow, const cv::Mat & yFlow, int height, int width, int nBins, const cv::Mat & kernelMatrix, std::vector<float> & intImg) {
    const float __2pi = 2 * M_PI, _denseBase = __2pi / float(kernelMatrix.rows);
    int _idx = 0;

    for(int iHeight = 0; iHeight < height; ++ iHeight) {
        const float * pXFlow = xFlow.ptr<float>(iHeight), * pYFlow = yFlow.ptr<float>(iHeight);
        std::vector<float> _sum(nBins, 0.f);
        for(int iWidth = 0; iWidth < width; ++ iWidth, ++ _idx) {
            float _xflow = pXFlow[iWidth], _yflow = pYFlow[iWidth], _magnitude = std::sqrt(_xflow * _xflow + _yflow * _yflow);
            if(_magnitude > __min_flow) {
                float _orientation = cv::fastAtan2(_yflow, _xflow);
                while(_orientation >= __2pi) {
                    _orientation -= __2pi;
                }
                while(_orientation < 0) {
                    _orientation += __2pi;
                }

                int _iDense = static_cast<int>(cvRound(_orientation / _denseBase));
                if(_iDense >= kernelMatrix.rows) {
                    _iDense = 0;
                }

                //apply kernel histogram
                const float * _kernel = kernelMatrix.ptr<float>(_iDense);
                for(int iBin = 0; iBin < nBins; ++ iBin) {
                    _sum[iBin] += _magnitude * _kernel[iBin];
                }
            }
            int _tmpf = _idx * nBins, _tmps = 0;
            if(iHeight == 0) {
                for(int iBin = 0; iBin < nBins; ++ iBin) {
                    intImg[_tmpf ++] = _sum[iBin];
                }
            }
            else {
                _tmps = (_idx - width) * nBins;
                for(int iBin = 0; iBin < nBins; ++ iBin) {
                    intImg[_tmpf ++] = intImg[_tmps ++] + _sum[iBin];
                }
            }
        }
    }
}

float getIntegralSum(const std::vector<float> & intImg, int xf, int yf, int xs, int ys, int height, int width, int nBins, std::vector<float> & hist) {
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
    int _idxTL = ((yf - 1) * width + (xf - 1)) * nBins,/*(xf, yf) is inclusive*/
        _idxTR = ((yf - 1) * width + xs) * nBins,
        _idxBL = (ys * width + (xf - 1)) * nBins,
        _idxBR = (ys * width + xs) * nBins;
    float _area = (xs - xf + 1) * (ys - yf + 1), sum = 0;
    for(int iBin = 0; iBin < nBins; ++ iBin) {
        float _intTL(0), _intTR(0), _intBL(0), _intBR(0);
        if(yf >= 1) {
            if(xf >= 1) {
                _intTL = intImg[_idxTL + iBin];
            }
            if(xs >= 0) {
                _intTR = intImg[_idxTR + iBin];
            }
        }
        if(ys >= 0){
            if(xf >= 1) {
                _intBL = intImg[_idxBL + iBin];
            }
            if(xs >= 0) {
                _intBR = intImg[_idxBR + iBin];
            }
        }
        hist[iBin] = _intBR - _intBL - _intTR + _intTL;
        hist[iBin] = std::max<int>(hist[iBin], 0);
        hist[iBin] /= _area;
        sum += std::pow(hist[iBin], 2.0);
    }
    return sum;
}

float calculateMotionSaliencyMap(const cv::Mat & flow, cv::Mat & saliencyMap, const DescInfo & descInfo, const cv::Mat & kernelMatrix) {
    int _width = flow.cols, _height = flow.rows, _nBins = descInfo._isHof ? descInfo._nBins - 1 : descInfo._nBins;
    saliencyMap.create(_height, _width, CV_32FC1);
    std::vector<float> _kernel{1.0f, 2.0f, 1.0f};
    //cv::Mat _smoothFlow;
    //GaussianSmoothC2(flow, _smoothFlow, _kernel);
    //cv::Mat _smoothXFlow, _smoothYFlow, _flows[2];
    //cv::split(flow, _flows);
    /**
     *  @warn   GaussianSmoothC1 is specail designed for mat of 8UC1 type
     */
    //GaussianSmoothC1(_flows[0], _smoothXFlow, _kernel);
    //GaussianSmoothC1(_flows[1], _smoothYFlow, _kernel);
    std::vector<float> _intImg(_width * _height * _nBins);
    integral(flow, _height, _width, _nBins, kernelMatrix, _intImg);
    //integral(_smoothXFlow, _smoothYFlow, _height, _width, _nBins, kernelMatrix, _intImg);
    //integral(_flows[0], _flows[1], _height, _width, _nBins, kernelMatrix, _intImg);
    for(int iHeight = 0; iHeight < _height; ++ iHeight) {
        int _yoff = std::min<int>(iHeight, _height - iHeight);
        float * _pSaliencyMap = saliencyMap.ptr<float>(iHeight);
        for(int iWidth = 0; iWidth < _width; ++ iWidth) {
            int _xoff = std::min<int>(iWidth, _width - iWidth);
            std::vector<float> _msa(_nBins), _center(_nBins);
            getIntegralSum(_intImg, iWidth - _xoff, iHeight - _yoff, iWidth + _xoff, iHeight + _yoff, _height, _width, _nBins, _msa);
            _yoff = std::min<int>(_yoff, 1);
            _xoff = std::min<int>(_xoff, 1);
            _pSaliencyMap[iWidth] = 0;
            if(getIntegralSum(_intImg, iWidth - _xoff, iHeight - _yoff, iWidth + _xoff, iHeight + _yoff, _height, _width, _nBins, _center) > 0) {
                for(int iBin = 0; iBin < _nBins; ++ iBin) {
                    float _sum = _msa[iBin] + _center[iBin], _diff = _msa[iBin] - _center[iBin];
                    if(_sum > 0) {
                        _pSaliencyMap[iWidth] += .5f * std::pow(_diff, 2.f) / _sum;
                    }
                }
            }
        }
    }
    return Normalize(saliencyMap, 255);
}

/**
 *  @brief  Apperance Saliency Part
 */


#endif// ! _SALIENCY_HPP_
