#ifndef _INITIALIZE_HPP_
#define _INITIALIZE_HPP_

#include "afx_track.hpp"

namespace Initialize {
    cv::Mat InitKernelMatrix (const int denseBins, const int bins, const float radius) {
        cv::Mat _kernelMat (denseBins, bins, CV_32FC1);
        const float _denseBase = (2 * M_PI) / float (denseBins),
            _binBase = (2 * M_PI) / float (bins);
        REP_ROW (denseBins) {
            float * _pK = _kernelMat.ptr<float> (iRow);
            REP_COL (bins) {
                const float _diff1 = std::cos (iRow * _denseBase) - std::cos (iCol * _binBase),
                    _diff2 = std::sin (iRow * _denseBase) - std::sin (iCol * _binBase);
                _pK[iCol] = std::exp(-radius * (std::pow(_diff1, 2.f) + std::pow(_diff2, 2.f)));
            }
        }
        return _kernelMat;
    }

    void initPyramid(const int width, const int height, int & scale_num, const float scale_stride, std::vector<float> & fscales, std::vector<cv::Size> & sizes) {
        scale_num = std::min<int>(
            static_cast<int>(
                cvFloor(
                    std::log(std::min<int>(width, height)) - std::log(1)
                ) / std::log(scale_stride)
            ),
            scale_num
        );

        if(scale_num == 0) {
            scale_num ++;
        }

        fscales.resize(scale_num);
        sizes.resize(scale_num);

        fscales[0] = 1.f;
        sizes[0] = cv::Size(width, height);
        for (int iScale = 1; iScale < scale_num; ++ iScale) {
            fscales[iScale] = fscales[iScale - 1] * scale_stride;
            sizes[iScale] = cv::Size(round(width / fscales[iScale]), round(height / fscales[iScale]));
        }
    }
}

#endif// ! _INITIALIZE_HPP_
