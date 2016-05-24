#ifndef _PYRAMID_HPP_
#define _PYRAMID_HPP_

namespace cxgAlleria {
    #include "afx.hpp"
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

    void buildPyramid(const std::vector<cv::Size> & sizes, std::vector<cv::Mat> & pyramid, const int type) {
        int scaleNum = int(sizes.size());
        pyramid.resize(scaleNum);
        for (int iScale = 0; iScale < scaleNum; ++ iScale) {
            pyramid[iScale].create(sizes[iScale], type);
        }
    }

    void setPyramid(const cv::Mat & img, std::vector<cv::Mat> & pyramid) {
        if(! pyramid.empty()) {
            img.copyTo(pyramid[0]);
            int scaleNum = int(pyramid.size());
            for (int iScale = 1; iScale < scaleNum; ++ iScale) {
                cv::resize(img, pyramid[iScale], pyramid[iScale].size(), 0, 0, cv::INTER_AREA);
            }
        }
    }
}

#endif// ! _PYRAMID_HPP_
