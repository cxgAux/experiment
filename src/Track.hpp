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

        }
    }
}

#endif// ! _TRACK_HPP_
