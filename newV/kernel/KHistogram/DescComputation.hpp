#ifndef _DESCCOMPUTATION_HPP_
#define _DESCCOMPUTATION_HPP_

namespace cxgAlleria {
    #include "afx.hpp"

    typedef struct RectInfo {
        int x, y, width, height, nBins;
        RectInfo (int x, int y, int w, int h, int b)
            : x(x), y(y), width(w), height(h) , nBins(b) {}
        RectInfo (const CvPoint2D32f & point, const CvSize & size, const DescInfo & descInfo) {
            const int _xMin = descInfo.blockWidth / 2,
                _yMin = descInfo.blockHeight / 2,
                _xMax = size.width - descInfo.blockWidth,
                _yMax = size.height - descInfo.blockHeight;
            this->x = std::min<int>(std::max<int>(point.x - _xMin, 0), _xMax);
            this->y = std::min<int>(std::min<int>(point.y - _yMin, 0), _yMax);
            this->width = descInfo.blockWidth;
            this->height = descInfo.blockHeight;
            this->nBins = descInfo.nBins;
        }
        ~RectInfo() {}
    };

    void buildDescMat(
        const cv::Mat & xComponent,
        const cv::Mat & yComponent,
        DescMat * const descMat,
        const DescInfo & descInfo,
        const cv::Mat & kernelMatrix
    ) {
        const int _width = descMat->width,
            _height = descMat->height,
            _histDim = descMat->nBins,
            _nBins = descInfo.flagThre ? (descInfo.nBins - 1) : descInfo.nBins;
        const float _denseBase = (2 * M_PI) / float(kernelMatrix.rows);
        int _idx = 0;

        HREP(_height) {
            const float * _xComp = xComponent.ptr<float>(iHeight),
                * _yComp = yComponent.ptr<float>(iHeight);
            std::vector<float> _kHist(_histDim, 0.f);
            WREP(_width) {
                const float _xShift = _xComp[iWidth],
                    _yShift = _yComp[iWidth],
                    _magnitude = std::sqrt(_xShift * _xShift + _yShift * _yShift);
                if(1 == descInfo.flagThre && _magnitude <= descInfo.threshold) {
                    _kHist[_nBins] += 1.f;
                }
                else {
                    float _orientation = std::atan2(_yShift, _xShift);
                    if(_orientation < 0) {
                        _orientation += 2 * M_PI;
                    }

                    int _iDense = static_cast<int>(roundf(_orientation / _denseBase));
                    if(_iDense >= kernelMatrix.rows) {
                        _iDense = 0;
                    }

                    const float * _pK = kernelMatrix.ptr<float>(_iDense);
                    for(int iBin = 0; iBin < _nBins; ++ iBin) {
                        _kHist[iBin] += _magnitude * _pK[iBin];
                    }
                }
                int _realIdx = _idx * _histDim;
                if(0 == iHeight) {
                    for (int iBin = 0; iBin < _histDim; ++ iBin, ++ _realIdx) {
                        descMat->desc[_realIdx] = _kHist[iBin];
                    }
                }
                else {
                    int _prevIdx = (_idx - _width) * _histDim;
                    for (int iBin = 0; iBin < _histDim; ++ iBin, ++ _realIdx, ++ _prevIdx) {
                        descMat->desc[_realIdx] = descMat->desc[_prevIdx] + _kHist[iBin];
                    }
                }
                ++ _idx;
            }
        }
    }

    float getDesc(const DescMat * const descMat, const RectInfo & rectInfo, const DescInfo & descInfo, const float epsilon, std::vector<float> & desc) {
        const int _width = descMat->width, _height = descMat->height,
            _xOff = rectInfo.x, _yOff = rectInfo.y,
            _xCells = descInfo.nxCells, _yCells = descInfo.nyCells,
            _xStride = rectInfo.width / _xCells, _yStride = rectInfo.height / _yCells,
            _descDim = descInfo.dim, _nBins = descInfo.nBins;

        desc.clear();

        for (int iX = 0; iX < _xCells; ++ iX) {
            for (int iY = 0; iY < _yCells; ++ iY) {
                const int
                    _l = _xOff + iX * _xStride - 1,
                    _r = std::min<int>(_l + _xStride, _width - 1),
                    _t = _yOff + iY * _yStride - 1,
                    _b = std::min<int>(_t + _yStride, _height - 1);

                const int
                    _TL = (_t * _width + _l) * _nBins,
                    _TR = (_t * _width + _r) * _nBins,
                    _BL = (_b * _width + _l) * _nBins,
                    _BR = (_b * _width + _r) * _nBins;
                for (int iBin = 0; iBin < _nBins; ++ iBin) {
                    float _sTL(0.f), _sTR(0.f), _sBL(0.f), _sBR(0.f);
                    if (_t >= 0) {
                        if (_l >= 0) {
                            _sTL = descMat->desc[_TL + iBin];
                        }
                        if (_r >= 0) {
                            _sTR = descMat->desc[_TR + iBin];
                        }
                    }
                    if (_b >= 0) {
                        if (_l >= 0) {
                            _sBL = descMat->desc[_BL + iBin];
                        }
                        if (_r >= 0) {
                            _sBR = descMat->desc[_BR + iBin];
                        }
                    }
                    desc.push_back(std::max<float>(_sTL + _sBR - _sTR - _sBL, 0) + epsilon);
                }
            }
        }

        if(1 == descInfo.norm) {
            float _absSum = 0.f;
            for (std::vector<float>::const_iterator it = desc.begin(); it != desc.end(); ++ it) {
                _absSum += std::fabs(*it);
            }
            for (std::vector<float>::iterator it = desc.begin(); it != desc.end(); ++ it) {
                *it /= _absSum;
            }
        }
        else {
            float _powSum = 0.f;
            for (std::vector<float>::const_iterator it = desc.begin(); it != desc.end(); ++ it) {
                _powSum += (*it) * (*it);
            }
            _powSum = std::sqrt(_powSum);
            for (std::vector<float>::iterator it = desc.begin(); it != desc.end(); ++ it) {
                *it /= _powSum;
            }
        }

        float _response = 0.f;
        for (std::vector<float>::const_iterator it = desc.begin(); it != desc.end(); ++ it) {
            _response += (*it) * (*it);
        }
        return _response;
    }

    void HogComp(const cv::Mat & img, DescMat * descMat, const DescInfo & descInfo, const cv::Mat & kernelMatrix) {
        cv::Mat imgf(img.size(), CV_32FC1), _imgX, _imgY;
        cv::Sobel(img, _imgX, CV_32F, 1, 0, 1);
        cv::Sobel(img, _imgY, CV_32F, 0, 1, 1);
        buildDescMat(_imgX, _imgY, descMat, descInfo, kernelMatrix);
    }

    void HofComp(const cv::Mat & flow, DescMat * descMat, const DescInfo & descInfo, const cv::Mat & kernelMatrix) {
        cv::Mat flows[2];
        cv::split(flow, flows);
        buildDescMat(flows[0], flows[1], descMat, descInfo, kernelMatrix);
    }

    void MbhComp(const cv::Mat & flow, DescMat * descMatX, DescMat * descMatY, const DescInfo & descInfo, const cv::Mat & kernelMatrix) {
        cv::Mat flows[2];
        cv::split(flow, flows);
        HREP (flow.rows) {
            float
                * _ptrX = flows[0].ptr<float>(iHeight),
                * _ptrY = flows[1].ptr<float>(iHeight);
            WREP (flow.cols) {
                _ptrX[iWidth] *= 100.f;
                _ptrY[iWidth] *= 100.f;
            }
        }
        HogComp(flows[0], descMatX, descInfo, kernelMatrix);
        HogComp(flows[1], descMatY, descInfo, kernelMatrix);
    }
}

#endif
