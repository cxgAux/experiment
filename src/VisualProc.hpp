#ifndef _VISUALPROC_HPP_
#define _VISUALPROC_HPP_

#include "afx.hpp"
#include "Descriptor.hpp"

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

void MedianFilterOpticalFlowTracker(
    const cv::Mat & flow,
    std::list<Trajectory> & tracker,
    std::vector<cv::Point2f> & points,
    const cv::Mat & saliencyMap,
    const DescMat * const hogImg, const DescInfo & hogInfo,
    const DescMat * const hofImg, const DescInfo & hofInfo,
    const DescMat * const xMbhImg, const DescMat * const yMbhImg, const DescInfo & mbhInfo,
    const TrackInfo & trackInfo,
    float avgFrameSaliency,
    std::ostream & salientTrajDelegator,
    std::ostream & unSalientTrahDelegator
) {
    TrajectorySerializable _ts;
    int _width = flow.cols, _height = flow.rows;
    points.clear();
    std::vector<float> _vXFlow, _vYFlow, _vSaliency;
    for(auto _traj = tracker.begin(); _traj != tracker.end();/* ++ _traj*/) {
        cv::Point2f _point = _traj->_points[_traj->_idx];
        _vXFlow.clear(); _vYFlow.clear(); _vSaliency.clear();
        int _x = cvRound(_point.x), _y = cvRound(_point.y);
        for(int iX = -1; iX <= 1; ++ iX) {
            for(int iY = -1; iY <= 1; ++ iY) {
                int _candidateX = std::min<int>(std::max<int>(_x + iX, 0), _width - 1), _candidateY = std::min<int>(std::max<int>(_y + iY, 0), _height - 1);
                const float * pFlow = flow.ptr<float>(_candidateY);
                _vXFlow.push_back(pFlow[2 * _candidateX]);
                _vYFlow.push_back(pFlow[2 * _candidateX + 1]);
                _vSaliency.push_back(saliencyMap.ptr<float>(_candidateY)[_candidateX]);//_candidateY --> row _candidateX --> col
            }
        }
        std::sort(_vXFlow.begin(), _vXFlow.end());
        std::sort(_vYFlow.begin(), _vYFlow.end());
        std::sort(_vSaliency.begin(), _vSaliency.end());

        float _candidateX = _point.x + _vXFlow[_vXFlow.size() / 2], _candidateY = _point.y + _vYFlow[_vYFlow.size() / 2];
        if(_candidateX > 0 && _candidateX < _width && _candidateY > 0 && _candidateY < _height) {
            RectInfo _rect = createWith_center_w_h_descInfo(_point, _width, _height, hogInfo);
            getDesc(hogImg, hogInfo, _rect, _traj->_hog, _traj->_idx);
            getDesc(hofImg, hofInfo, _rect, _traj->_hof, _traj->_idx);
            getDesc(xMbhImg, mbhInfo, _rect, _traj->_mbhx, _traj->_idx);
            getDesc(yMbhImg, mbhInfo, _rect, _traj->_mbhy, _traj->_idx);
            points.push_back(cv::Point2f(_candidateX, _candidateY));
            _traj->addPoint(cv::Point2f(_candidateX, _candidateY), _vSaliency[_vSaliency.size() / 2], avgFrameSaliency);
            /**
             *  @warn  the comparative operator should be >=, or getDesc will arise an iterator out of bound error, while the compiler reports it as an "invalid pointer"  error
             */
            if(_traj->_idx >= trackInfo._length) {
                if(_traj->_saliency / _traj->_averageSaliency >=  __ratio) {
                    //salient trajectories
                    _log("\t\tsalinet!\n")
                    _ts(salientTrajDelegator, * _traj);
                }
                else {
                    _log("\t\tunsalinet!\n")
                    _ts(unSalientTrahDelegator, * _traj);
                }
                //remove completed trajectories
                //<2016/04/16 18:18> free() corruption here!
                _traj = tracker.erase(_traj);
            }
            else {
                ++ _traj;
            }
        }
        else {
            _log("\t\tinvalid!\n")
            //remove the incomplete trajectories when fail to track
            _traj = tracker.erase(_traj);
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
