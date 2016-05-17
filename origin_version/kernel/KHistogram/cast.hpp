#ifndef _CAST_H_
#define _CAST_H_

#include "DenseTrack.h"

template < class T >
void Mat2IplImageWrapper(const cv::Mat & image, IplImageWrapper & wrapper, const int type, const int channel) {
    wrapper = IplImageWrapper(cvSize(image.rows, image.cols), type, channel);
    for(int iHeight = 0; iHeight < image.cols; ++ iHeight) {
        const T * pSrc = image.ptr<T>(iHeight);
        T * pDst = (T *)(wrapper->imageData + wrapper->widthStep *  iHeight);
        for(int iWidth = 0; iWidth < image.rows; ++ iWidth) {
            for(int iChannel = 0; iChannel < channel; ++ iChannel) {
                pDst[iWidth * channel + iChannel] = pSrc[iWidth * channel + iChannel];
            }
        }
    }
}

#endif// ! _CAST_H_
