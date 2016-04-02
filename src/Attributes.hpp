#ifndef _ATTRIBUTES_HPP_
#define _ATTRIBUTES_HPP_

#include "afx.hpp"

/**
 *  @brief  parameters for spacial-temporal cube construction
 */
static int __start_frame = 0;
static int __end_frame = std::numeric_limits<int>::max();
static int __scale_nums = 8;
static float __scale_stride = std::sqrt(2.0);

/**
 *  @brief  parameters for descriptors
 */
static int __patch_size = 32;
static int __nxy_cells = 2;
static int __nt_cells = 3;
static int __epsilon = 0.05;
/**
 *  @warn   <2016/04/01 22:02> in version 1 this is 0.4 * 0.4, but here is 0.4?
 */
const float __min_flow = 0.4f;

/**
 *  @brief  parameters for tracking
 */
static double __quality = 0.001;
static int __min_distance = 5;
static int __init_gap = 1;
static int __track_length = 15;

/**
 *  @brief  kernel matrix attributes
 */
static int __preset_bins = 3600;
static int __GaussSmooth = 5;

#endif// ! _ATTRIBUTES_HPP_
