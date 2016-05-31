#ifndef _APPEARANCESALIENCY_HPP_
#define _APPEARANCESALIENCY_HPP_

#include "Saliency.hpp"

namespace AppearanceSaliency {

    /**
     *  @brief  	CreateStaticIntegralImage
     */
    void CreateStaticIntegralImage (
    	const cv::Mat & inputImg,
    	std::vector<float> & intImg
    ) {
    	const int _width = inputImg.cols, _height = inputImg.rows;
    	int _idx = 0;
        intImg.resize (_width * _height);
        REP_HEIGHT (_height) {
            const float * pImg = inputImg.ptr<float> (iHeight);
            float _sum(0);
            REP_WIDTH (_width) {
    			_sum += pImg[iWidth];
    			if (0 == iHeight) { // for the first line
    				intImg[_idx] = _sum;
    			}
    			else {
    				intImg[_idx] = intImg[_idx - _width] + _sum;
    			}
                ++ _idx;
    		}
    	}
    }

    //===========================================================================
    ///	CalcStaticSaliencyMap
    ///
    /// Outputs a static saliency map with a value assigned per pixel. The values are
    /// normalized in the interval [0,255] if normflag is set true (default value).
    //===========================================================================
    float CalcStaticSaliencyMap (
    		const cv::Mat & grey,
    		cv::Mat & salMap
    ) {
    	const int _width = grey.cols, _height = grey.rows;

    	std::vector<float> kernel (0);kernel.push_back (1.f);kernel.push_back (2.f);kernel.push_back (1.f);

    	cv::Mat smoothImg; Saliency::GaussianSmooth<uchar> (grey, kernel, smoothImg);

    	std::vector<float> intImg; CreateStaticIntegralImage(smoothImg, intImg);

    	// calculate static saliency for each pixel
    	REP_HEIGHT (_height) {
    		int yoff = std::min<int> (iHeight, _height - iHeight);
    		float * pMap = salMap.ptr<float> (iHeight);
    		REP_WIDTH (_width) {
    			int xoff = std::min<int> (iWidth, _width -iWidth);
                std::vector<float> _msa;
    			Saliency::GetIntegralSum (intImg, iWidth - xoff, iHeight - yoff, iWidth + xoff, iHeight + yoff, _height, _width, 1, _msa);
    			yoff = std::min<int>(yoff, 0);
    			xoff = std::min<int>(xoff, 0);
                std::vector<float> _point;
    			Saliency::GetIntegralSum(intImg, iWidth - xoff, iHeight - yoff, iWidth + xoff, iHeight + yoff, _height, _width, 1, _point);
    			pMap[iWidth] = (_msa[0] - _point[0]) * (_msa[0] - _point[0]);
    		}
    	}

    	return Saliency::minMaxNormalize(salMap, 0.f, 255.f);

    }

}

#endif
