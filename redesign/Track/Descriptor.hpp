#ifndef _DESCRIPTOR_HPP_
#define _DESCRIPTOR_HPP_

#include "afx_track.hpp"

namespace Descriptor {
    struct RectInfo {
        int m_iX, m_iY, m_iWidth, m_iHeight;
        explicit RectInfo (int x, int y, int w, int h)
            : m_iX (x), m_iY (y), m_iWidth (w), m_iHeight (h)  {}
        explicit RectInfo (const cv::Point2f & point, const cv::Size & size, const Structs::DescInfo & descInfo) {
            const int _xMin = descInfo.m_cubeInfo.m_iBlockWidth / 2,
                _yMin = descInfo.m_cubeInfo.m_iBlockHeight / 2,
                _xMax = size.width - descInfo.m_cubeInfo.m_iBlockWidth,
                _yMax = size.height - descInfo.m_cubeInfo.m_iBlockHeight;
            this->m_iX = static_cast<int> (std::min<float> (std::max<float> (point.x - _xMin, 0), _xMax));
            this->m_iY = static_cast<int> (std::min<float> (std::max<float> (point.y - _yMin, 0), _yMax));
            this->m_iWidth = descInfo.m_cubeInfo.m_iBlockWidth;
            this->m_iHeight = descInfo.m_cubeInfo.m_iBlockHeight;
        }
        RectInfo (const RectInfo &) = delete;
        RectInfo & operator= (const RectInfo &) = delete;
        ~RectInfo () {}
    };

    void buildDescMat (
        const cv::Mat & xComponent,
        const cv::Mat & yComponent,
        Structs::DescMat & descMat,
        const Structs::DescInfo & descInfo,
        const cv::Mat & kernelMatrix
    ) {
        const int _width = descMat.m_iWidth,
            _height = descMat.m_iHeight,
            _histDim = descMat.m_iBin,
            _nBins = descInfo.getBin();
        const float _denseBase = (2 * M_PI) / float (kernelMatrix.rows);
        int _idx = 0;

        REP_HEIGHT (_height) {
            const float * _xComp = xComponent.ptr<float> (iHeight),
                * _yComp = yComponent.ptr<float> (iHeight);
            std::vector<float> _kHist (_histDim, 0.f);
            REP_WIDTH (_width) {
                const float _xShift = _xComp[iWidth],
                    _yShift = _yComp[iWidth],
                    _magnitude = std::sqrt (_xShift * _xShift + _yShift * _yShift);
                if (descInfo.isValid (_magnitude)) {
                    float _orientation = std::atan2 (_yShift, _xShift);
                    if (_orientation < 0) {
                        _orientation += 2 * M_PI;
                    }

                    int _iDense = static_cast<int> (roundf (_orientation / _denseBase));
                    if (_iDense >= kernelMatrix.rows) {
                        _iDense = 0;
                    }

                    const float * _pK = kernelMatrix.ptr<float> (_iDense);
                    REP_BIN (_nBins) {
                        _kHist[iBin] += _magnitude * _pK[iBin];
                    }
                }
                else {
                    _kHist[_nBins] += 1.f;
                }

                int _realIdx = _idx * _histDim;
                if(0 == iHeight) {
                    REP_BIN (_histDim) {
                        descMat.m_desc[_realIdx ++] = _kHist[iBin];
                    }
                }
                else {
                    int _prevIdx = (_idx - _width) * _histDim;
                    REP_BIN (_histDim) {
                        descMat.m_desc[_realIdx ++] = descMat.m_desc[_prevIdx ++] + _kHist[iBin];
                    }
                }
                ++ _idx;
            }
        }
    }

    float getDesc (
        const Structs::DescMat & descMat,
        const RectInfo & rectInfo,
        const Structs::DescInfo & descInfo,
        const float epsilon,
        std::vector<float> & desc
    ) {
        const int _width = descMat.m_iWidth, _height = descMat.m_iHeight,
            _xOff = rectInfo.m_iX, _yOff = rectInfo.m_iY,
            _xCells = descInfo.m_cubeInfo.m_iXCells, _yCells = descInfo.m_cubeInfo.m_iYCells,
            _xStride = rectInfo.m_iWidth / _xCells, _yStride = rectInfo.m_iHeight / _yCells,
            _nBins = descInfo.m_iBin;

        desc.clear ();

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
                REP_BIN (_nBins) {
                    float _sTL (0.f), _sTR (0.f), _sBL (0.f), _sBR (0.f);
                    if (_t >= 0) {
                        if (_l >= 0) {
                            _sTL = descMat.m_desc[_TL + iBin];
                        }
                        if (_r >= 0) {
                            _sTR = descMat.m_desc[_TR + iBin];
                        }
                    }
                    if (_b >= 0) {
                        if (_l >= 0) {
                            _sBL = descMat.m_desc[_BL + iBin];
                        }
                        if (_r >= 0) {
                            _sBR = descMat.m_desc[_BR + iBin];
                        }
                    }
                    desc.push_back (std::max<float> (_sTL + _sBR - _sTR - _sBL, 0) + epsilon);
                }
            }
        }

        /**
         *  @brief  modify to lamda expr
         */
        float _response = 0.f;
        if (1 == descInfo.m_iNormType) {
            float _absSum = 0.f;
            for (std::vector<float>::const_iterator it = desc.begin(); it != desc.end(); ++ it) {
                _absSum += std::fabs(*it);
            }
            for (std::vector<float>::iterator it = desc.begin(); it != desc.end(); ++ it) {
                *it /= _absSum;
                _response += (*it) * (*it);
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
                _response += (*it) * (*it);
            }
        }
        return _response;
    }

    void HogComp(const cv::Mat & img, Structs::DescMat & descMat, const Structs::DescInfo & descInfo, const cv::Mat & kernelMatrix) {
        cv::Mat _imgX, _imgY;
        cv::Sobel(img, _imgX, CV_32F, 1, 0, 1);
        cv::Sobel(img, _imgY, CV_32F, 0, 1, 1);
        buildDescMat(_imgX, _imgY, descMat, descInfo, kernelMatrix);
    }

    void HofComp(const cv::Mat & flow, Structs::DescMat & descMat, const Structs::DescInfo & descInfo, const cv::Mat & kernelMatrix) {
        cv::Mat flows[2];
        cv::split(flow, flows);
        buildDescMat(flows[0], flows[1], descMat, descInfo, kernelMatrix);
    }

    void MbhComp(const cv::Mat & flow, Structs::DescMat & descMatX, Structs::DescMat & descMatY, const Structs::DescInfo & descInfo, const cv::Mat & kernelMatrix) {
        cv::Mat flows[2];
        cv::split(flow, flows);
        REP_HEIGHT (flow.rows) {
            float
                * _ptrX = flows[0].ptr<float>(iHeight),
                * _ptrY = flows[1].ptr<float>(iHeight);
            REP_WIDTH (flow.cols) {
                _ptrX[iWidth] *= 100.f;
                _ptrY[iWidth] *= 100.f;
            }
        }
        HogComp(flows[0], descMatX, descInfo, kernelMatrix);
        HogComp(flows[1], descMatY, descInfo, kernelMatrix);
    }
}

#endif// ! _DESCRIPTOR_HPP_
