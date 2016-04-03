#ifndef _TRACK_HPP_
#define _TRACK_HPP_

#include "Attributes.hpp"
#include "Structures.hpp"

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
            SeqInfo _seqInfo = fromVideo(_capture);
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
                    }
                }

                _frame_idx ++;
            }
        }
    }
}

#endif// ! _TRACK_HPP_
