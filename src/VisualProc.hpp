#ifndef _VISUALPROC_HPP_
#define _VISUALPROC_HPP_

#include "afx.hpp"

void DenseSample(const cv::Mat & grey, std::vector<cv::Point2f> & points, const double quality, const int min_distance) {
    int _width = grey.cols / min_distance, _height = grey.rows / min_distance;
    cv::Mat _eigen;
    cv::cornerMinEigenVal(grey, _eigen, 3, 3);

    double _maxEigenValue = 0;
    cv::minMaxLoc(_eigen, 0, &_maxEigenValue);
    const double __threshold = _maxEigenValue * quality;

    std::vector<bool> occupied(_width * _height, false);
    int _x_max = min_distance * _width, _y_max = min_distance * _height;
    for(auto & val : points) {
        int _x = cvFloor(val.x), _y = cvFloor(val.y);
        if(_x >= _x_max || _y >= _y_max) {
            continue;
        }
        else {
            _x /= min_distance;
            _y /= min_distance;
            occupied[_y * _width + _x] = true;
        }
    }

    points.clear();
    int _idx = 0, _offset = cvFloor(min_distance >> 1);
    for(int iHeight = 0; iHeight < _height; ++ iHeight) {
        for(int iWidth = 0; iWidth < _width; ++ iWidth, ++ _idx) {
            if(occupied[_idx] == false) {
                int _x = iWidth * min_distance + _offset, _y = iHeight * min_distance + _offset;
                if(_eigen.at<float>(_y, _x) > __threshold) {// good features to track
                    points.push_back(cv::Point2f(float(_x), float(_y)));
                }
            }
        }
    }
}

enum Idx{TK = 0, AS = 1, MS = 2, TS = 3, JS = 4};
static const std::string __displayName[] = {"Track", "Appearance Saliency", "Motion Salinecy", "Temporal Salinecy", "Joint Salinecy"};

void Display(const cv::Mat & image, Idx idx) {
    cv::Mat _toDisplay(image.size(), CV_8U);
    for(int iRow = 0; iRow < image.rows; ++ iRow) {
        const float * pImage = image.ptr<float>(iRow);
        uchar * pDisplay = _toDisplay.ptr<uchar>(iRow);
        for(int iCol = 0; iCol < image.cols; ++ iCol) {
            pDisplay[iCol] = uchar(pImage[iCol]);
        }
    }
    cv::imshow(__displayName[idx], _toDisplay);
    cv::waitKey(1);
}

static std::string __dir[] =  {"Track", "Appearance Saliency", "Motion Salinecy", "Temporal Salinecy", "Joint Salinecy"};
void Save(const cv::Mat & image, Idx idx, int frameNo) {
    cv::Mat __toSave(image.size(), CV_8U);
    for(int iRow = 0; iRow < image.rows; ++ iRow) {
        const float * pImage = image.ptr<float>(iRow);
        uchar * pSave = __toSave.ptr<uchar>(iRow);
        for(int iCol = 0; iCol < image.cols; ++ iCol) {
            pSave[iCol] = uchar(pImage[iCol]);
        }
    }
    std::string _buff = __dir[idx] + "/" + std::to_string(frameNo) + (".jpg");
    cv::imwrite(_buff, __toSave);
}

#endif// ! _VISUALPROC_HPP_
