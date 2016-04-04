#ifndef _TRACK_HPP_
#define _TRACK_HPP_

#include "Attributes.hpp"
#include "Structures.hpp"
#include "VisualProc.hpp"
#include "OpticalFlow.h"
#include "Descriptor.hpp"

bool __toDisplay = false;

int Track(int argc, char ** argv) {
    if(argc < 2) {
        _err("Pls provide videos to process\n");
        return EXIT_FAILURE;
    }
    else {
        cv::VideoCapture _capture;
        /**
         *  @warn   <2016/04/02 15:09> bind the videoname as the first trivial argument
         */
        cv::String _videoName = argv[1];
        _capture.open(_videoName);
        if(_capture.isOpened() == false) {
            _err("Error file is provided!\n");
            return EXIT_FAILURE;
        }
        else {
            int _frame_idx = 0;
            TrackInfo _trackInfo = createWith_TrajLength_SampleGap(__track_length, __init_gap);
            DescInfo _hogInfo = createWith_isHofFlag_cubePartition_xy_t_b_crossSection_w_h(false, __nxy_cells, __nt_cells, 8, __patch_size, __patch_size),
                _hofInfo = createWith_isHofFlag_cubePartition_xy_t_b_crossSection_w_h(true, __nxy_cells, __nt_cells, 9, __patch_size, __patch_size),
                _mbhInfo = createWith_isHofFlag_cubePartition_xy_t_b_crossSection_w_h(false, __nxy_cells, __nt_cells, 8, __patch_size, __patch_size);
            //sequence info getter provided, but useless in the experiment
            //SeqInfo _seqInfo = fromVideo(_capture);
            cv::Mat _kernelMatrix = createWith_bins_nBins_kernelRadius(__preset_bins, 8, __GaussSmooth);

            if(__toDisplay == true) {
                cv::namedWindow("Track", cv::WINDOW_AUTOSIZE);
            }

            //buffers
            cv::Mat _frame, _image, _prev_grey, _grey;

            //for pixel pyramid generation and restoration
            std::vector<float> _fscales(0);
            std::vector<cv::Size> _sizes(0);

            //pyramids
            std::vector<cv::Mat> _prev_grey_pyr(0), _grey_pyr(0), _flow_pyr(0);
            //aux pyramids
            std::vector<cv::Mat> _prev_poly_pyr(0), _poly_pyr(0);
            //tracker
            std::vector< std::list<Trajectory> > _xyScaleTracks;
            //counter to perform sampling
            int _counter2Sample = 0;
            while(true) {
                //get a new frame
                _capture >> _frame;
                if(_frame.empty()) {
                    static int __count = 200;
                    char * __buff = new char [__count];
                    /**
                     *  @warn   <2016/04/02 15:09> bind the videoname as the first trivial argument
                     */
                    sprintf(__buff, "Video %s has meet its end!\n", argv[1]);
                    _log(__buff);
                    delete [] __buff;
                    break;
                }

                if(_frame_idx >= __start_frame && _frame_idx <= __end_frame) {
                    if(_frame_idx == __start_frame) {
                        _image.create(_frame.size(), CV_8UC3);
                        _grey.create(_frame.size(), CV_8UC1);
                        _prev_grey.create(_frame.size(), CV_8UC1);

                        InitPry(_frame.rows, _frame.cols, _fscales, _sizes);
                        BuildPyr(_sizes, CV_8UC1, _prev_grey_pyr);
                        BuildPyr(_sizes, CV_8UC1, _grey_pyr);

                        BuildPyr(_sizes, CV_32FC2, _flow_pyr);
                        BuildPyr(_sizes, CV_32FC(5), _prev_poly_pyr);
                        BuildPyr(_sizes, CV_32FC(5), _poly_pyr);

                        _xyScaleTracks.resize(__scale_nums);

                        _frame.copyTo(_image);
                        cvtColor(_image, _prev_grey, CV_BGR2GRAY);
                        for(int iScale = 0; iScale < __scale_nums; ++ iScale) {
                            if(iScale == 0) {
                                _prev_grey.copyTo(_prev_grey_pyr[0]);
                            }
                            else {
                                cv::resize(_prev_grey_pyr[iScale - 1], _prev_poly_pyr[iScale], _prev_grey_pyr[iScale].size(), 0, 0, cv::INTER_LINEAR);
                            }
                            //find good features to track defined in "VisualProc.hpp"
                            std::vector<cv::Point2f> _points(0);
                            DenseSample(_prev_grey_pyr[iScale], _points, __quality, __min_distance);

                            //save features inits
                            std::list<Trajectory> & _iScaleTrajList = _xyScaleTracks[iScale];
                            for(auto & initPoint : _points) {
                                _iScaleTrajList.push_back(Trajectory(initPoint, _trackInfo, _hogInfo, _hofInfo, _mbhInfo, _frame_idx, 0, 0));
                            }

                            //compute polinomial expansion
                            my::FarnebackPolyExpPyr(_prev_grey, _prev_poly_pyr, _fscales, 7, 1.5);
                        }// end of for iScale
                    }// end of if(_frame_idx == __start_frame)
                    else {
                        _counter2Sample ++;
                        _frame.copyTo(_image);
                        cvtColor(_image, _grey, CV_BGR2GRAY);

                        //in the journal version of "Dense Trajectory"
                        //compute optical flow for all scales once
                        my::FarnebackPolyExpPyr(_grey, _poly_pyr, _fscales, 7, 1.5);
                        my::calcOpticalFlowFarneback(_prev_poly_pyr, _poly_pyr, _flow_pyr, 10, 2, __scale_stride);

                        for(int iScale = 0; iScale < __scale_nums; ++ iScale) {
                            //build up pixel pyramid
                            if(iScale == 0) {
                                _grey.copyTo(_grey_pyr[0]);
                            }
                            else {
                                cv::resize(_grey_pyr[iScale - 1], _grey_pyr[iScale], _grey_pyr[iScale].size(), 0, 0, cv::INTER_LINEAR);
                            }

                            int _width = _grey_pyr[iScale].cols, _height = _grey_pyr[iScale].rows;
                            //compute all releated information
                                //compute integral histograms
                            DescMat * _hogImg = createWith_w_h_b(_width, _height, _hogInfo._nBins);
                            HogComp(_prev_grey_pyr[iScale], _hogImg->_desc, _hogInfo, _kernelMatrix);

                            DescMat * _hofImg = createWith_w_h_b(_width, _height, _hofInfo._nBins);
                            HofComp(_flow_pyr[iScale], _hofImg->_desc, _hofInfo, _kernelMatrix);

                            DescMat * _xMbhImg = createWith_w_h_b(_width, _height, _mbhInfo._nBins),
                                * _yMbhImg = createWith_w_h_b(_width, _height, _mbhInfo._nBins);
                            MbhComp(_flow_pyr[iScale], _xMbhImg->_desc, _yMbhImg->_desc, _mbhInfo, _kernelMatrix);
                            //track

                            //resample

                            //garbage collection
                            release(_hogImg);
                            release(_hofImg);
                            release(_xMbhImg);
                            release(_yMbhImg);
                        }// end for(int iScale = 0; iScale < __scale_nums; ++ iScale)
                    } // end of if(_frame_idx == __start_frame) else
                }//end of if(_frame_idx >= __start_frame && _frame_idx <= __end_frame)

                _frame_idx ++;
            }//end of while(true)
        }// else int _frame_idx = 0;
    }//end of else cv::VideoCapture _capture;
}//end of int Track(int argc, char ** argv)

#endif// ! _TRACK_HPP_
