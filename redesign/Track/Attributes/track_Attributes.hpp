#ifndef _TRACK_ATTRIBUTES_HPP_
#define _TRACK_ATTRIBUTES_HPP_

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

         const static       int     hogBins     =   8;
         const static       int     hofBins     =   9;
         const static       int     mbhBins     =   8;

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
             static         float   min_flow    =   0.4f * 0.4f;
         }
     }

     namespace Saliency {
         static         float       static_ratio    =   .5f;
         static         float       dynamic_ratio   =   .5f;
		 static			float		alpha			=	.5f;
         static         float       salient_ratio   =   1.5f;
     }

     inline void normalizedRatios() {
         float _ratioSum = Saliency::static_ratio + Saliency::dynamic_ratio;
         Saliency::static_ratio /= _ratioSum;
         Saliency::dynamic_ratio /= _ratioSum;
     }

     namespace Mode {
         static         bool        toDisplay       =   false;
         const static   int         delay           =   30;
         static         bool        toPreserve      =   false;
     }

    namespace KernelMatrix {
        static          float       radius          =   5.f;
        const static    int         denseBins       =   3600;
    }
}

#endif// ! _TRACK_ATTRIBUTES_HPP_
