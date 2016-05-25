#ifndef _ATTRIBUTES_HPP_
#define _ATTRIBUTES_HPP_

namespace Attributes {
     namespace Tracking {
         static         int     start_frame =   0;
         static         int     end_frame   =   std::numeric_limits<int>::max();
         static         int     min_distance=   5;
         static         float   quality     =   .001f;
         static         int     init_gap    =   1;
         static         int     traj_length =   15;
     }

     namespace Descriptor {
         namespace MultiScale {
             static         int     scale_num   =   8;
             const static   float   scale_stride=   std::sqrt(2.f);
         }

         namespace Cube {
             static         int     patch_size  =   32;
             static         int     nxCells     =   2;
             static         int     nyCells     =   2;
             static         int     ntCells     =   3;
         }

         namespace Hof {
             static         float   epsilon     =   .05f;
             static         float   min_float   =   0.4 * 0.4;
         }
     }

     namespace Saliency {
         static         float       static_ratio    =   .5f;
         static         float       dynamic_ratio   =   .5f;
         static         float       salient_ratio   =   1.5f;
     }

     inline void normalizedRatios() {
         float _ratioSum = Saliency::static_ratio + Saliency::dynamic_ratio;
         Saliency::static_ratio /= _ratioSum;
         Saliency::dynamic_ratio /= _ratioSum;
     }

     namespace Mode {
         static         bool        toDisplay       =   false;
         static         bool        toPreserve      =   false;
     }
}

#endif// ! _ATTRIBUTES_HPP_
