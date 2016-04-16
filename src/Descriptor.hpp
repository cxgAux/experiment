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
    int _dim = info._dim, _nBins = info._nBins, _width = mat->_width;
    int _xStride = rect._width / info._nxCells, _yStride = rect._height / info._nyCells, _xStep = _xStride * _nBins, _yStep = _yStride * _nBins;

    //iterator over relative cells
    int iDesc = 0;
    std::vector<float> _v(_dim);
    for(int xPos = rect._x, x = 0; x < info._nxCells; xPos += _xStride, ++ x) {
        for(int yPos = rect._y, y = 0; y < info._nyCells; yPos += _yStride, ++ y) {
            const float * _TL = mat->_desc + (yPos * _width + xPos) * _nBins,
                * _TR = _TL + _xStep,
                * _BL = _TL + _yStep,
                * _BR = _BL + _xStep;
            for(int iBin = 0; iBin < _nBins; ++ iBin) {
                float _area = _BR[iBin] + _TL[iBin] - _BL[iBin] - _TR[iBin];
                _v[iDesc ++] = std::max<float>(_area, 0) + __epsilon;
            }
        }
    }
    float _norm = 0.f;
    for(int iDim = 0; iDim < _dim; ++ iDim) {
        _norm += _v[iDim];
    }
    if(_norm > 0) {
        _norm = 1.f / _norm;
    }

    int _pos = idx * _dim;
    for(int iDim = 0; iDim < _dim; ++ iDim) {
        desc[_pos ++] = std::sqrt(_v[iDim] * _norm);
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
