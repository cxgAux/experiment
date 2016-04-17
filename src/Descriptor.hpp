#ifndef _DESCRIPTOR_HPP_
#define _DESCRIPTOR_HPP_

#include "afx.hpp"
#include "Structures.hpp"


void BuildIntegralImage(
    const cv::Mat & xComp,/*x component*/
    const cv::Mat & yComp,/*y component*/
    float * desc,/*the descriptor content*/
    const DescInfo & descInfo,
    const cv::Mat kernelMatrix
) {
    static const float __2pi = 2 * M_PI;
    int _dims = descInfo._nBins;
    //0-bin for too weak flow
    int _bins = _dims - descInfo._isHof ? 1 : 0;
    const float _denseBase = __2pi / float(kernelMatrix.rows);
    const int _height = xComp.rows, _width = xComp.cols;
    int _idx = 0;
    for(int iHeight = 0; iHeight < _height; ++ iHeight) {
        const float * xcomp = xComp.ptr<float>(iHeight),
            * ycomp = yComp.ptr<float>(iHeight);
        //summarize current line
        std::vector<float> _sum(_dims);
        for(int iWidth = 0; iWidth < _width; ++ iWidth, ++ _idx) {
            float _xShift = xcomp[iWidth], _yShift = ycomp[iWidth],
                _magnitude = std::sqrt(_xShift * _xShift + _yShift * _yShift);

            if(descInfo._isHof == true && _magnitude <= __min_flow) {
                int _bin = _bins;
                _magnitude = 1;
                _sum[_bin] += _magnitude;
            }
            else {
                float _orientation = cv::fastAtan2(_yShift, _xShift);
                while(_orientation >= __2pi) {
                    _orientation -= __2pi;
                }

                int _iDense = static_cast<int>(cvRound(_orientation / _denseBase));
                if(_iDense >= kernelMatrix.rows) {
                    _iDense = 0;
                }

                //apply kernel histogram
                const float * _kernel = kernelMatrix.ptr<float>(_iDense);
                for(int iBin = 0; iBin < _bins; ++ iBin) {
                    _sum[iBin] += _magnitude * _kernel[iBin];
                }
            }

            //integrate
            int _tmpf = _idx * _dims, _tmps = 0;
            if(iHeight == 0) {
                for(int iBin = 0; iBin < _dims; ++ iBin) {
                    desc[_tmpf ++] = _sum[iBin];
                }
            }
            else {
                _tmps = (_idx - _width) * _dims;
                for(int iBin = 0; iBin < _dims; ++ iBin) {
                    desc[_tmpf ++] = desc[_tmps ++] + _sum[iBin];
                }
            }
        }
    }
}


void getDesc(const DescMat * const mat, const DescInfo & info, const RectInfo & rect, std::vector<float> & desc, int idx) {
    int _descDim = info._dim, _height = mat->_height, _width = mat->_width,
        _xoff = rect._x, _yoff = rect._y,
        _xStride = rect._width / info._nxCells, _yStride = rect._height / info._nyCells,
        _nBins = info._nBins;

    int _iDesc = 0;
    std::vector<float> _tmp(_descDim);
    float _norm = 0;
    for(int iX = 0; iX < info._nxCells; ++ iX) {
        for(int iY = 0; iY < info._nyCells; ++ iY) {
            int _left = _xoff + iX * _xStride - 1,
                _right = std::min<int>(_left + _xStride, _width - 1),
                _top = _yoff + iY * _yStride - 1,
                _bottom = std::min<int>(_top + _yStride, _height - 1);

            int _TL = (_top * _width + _left) * _nBins,
                _TR = (_top * _width + _right) * _nBins,
                _BL = (_bottom * _width + _left) * _nBins,
                _BR = (_bottom * _width + _right) * _nBins;

            for(int iB = 0; iB < _nBins; ++ iB, ++ _iDesc) {
                float _sTL(0), _sTR(0), _sBL(0), _sBR(0);
                if(_top >= 0) {
                    if(_left >= 0) {
                        _sTL = mat->_desc[_TL + iB];
                    }
                    if(_right >= 0) {
                        _sTR = mat->_desc[_TR + iB];
                    }
                }
                if(_bottom >= 0) {
                    if(_left >= 0) {
                        _sBL = mat->_desc[_BL + iB];
                    }
                    if(_right >= 0) {
                        _sBR = mat->_desc[_BR + iB];
                    }
                }
                _tmp[_iDesc] = std::max<int>(_sBR - _sBL - _sTR + _sTL, 0) + __epsilon;
                _norm += _tmp[_iDesc];
            }
        }
        if(_norm > 0) {
            _norm = 1.f / _norm;
        }

        int _iPos = idx * _descDim;
        for(int iPos = 0; iPos < _descDim; ++ iPos) {
            desc[_iPos ++] = std::sqrt(_tmp[iPos] * _norm);
        }
    }
}

/**
 *  @brief  functions for hog, hof, mbh
 */
//for HOG descriptor
void HogComp(const cv::Mat & grey, float * desc, const DescInfo & hogInfo, const cv::Mat & kernelMatrix) {
    cv::Mat _greyX, _greyY;
    cv::Sobel(grey, _greyX, CV_32F, 1, 0, 1);
    cv::Sobel(grey, _greyY, CV_32F, 0, 1, 1);
    BuildIntegralImage(_greyX, _greyY, desc, hogInfo, kernelMatrix);
}

//for hof descriptor
void HofComp(const cv::Mat & flow, float * desc, const DescInfo & hofInfo, const cv::Mat & kernelMatrix) {
    cv::Mat _flows[2];
    cv::split(flow, _flows);
    BuildIntegralImage(_flows[0], _flows[1], desc, hofInfo, kernelMatrix);
}

//for mbh descriptor
void MbhComp(const cv::Mat & flow, float * xDesc, float * yDesc, const DescInfo & mbhInfo, const cv::Mat & kernelMatrix) {
    cv::Mat _flows[2];
    cv::split(flow, _flows);
    HogComp(_flows[0], xDesc, mbhInfo, kernelMatrix);
    HogComp(_flows[1], yDesc, mbhInfo, kernelMatrix);
}
#endif// ! _DESCRIPTOR_HPP_
