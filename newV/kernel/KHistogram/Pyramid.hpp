#ifndef _PYRAMID_HPP_
#define _PYRAMID_HPP_

namespace cxgAlleria {
    #include "afx.hpp"
    void initPyramid(int width, int height, int & scale_num, const float scale_stride, std::vector<float> & fscales, std::vector<CvSize> & sizes) {
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
        sizes[0] = CvSize(width, height);
        for (int iScale = 1; iScale < scale_num; ++ iScale) {
            fscales[iScale] = fscales[iScale - 1] * scale_stride;
            sizes[iScale] = CvSize(round(width / fscales[iScale]), round(height / fscales[iScale]));
        }
    }

    //void buildPyramid(const std::vector<CvSize> sizes, )
}

#endif// ! _PYRAMID_HPP_
