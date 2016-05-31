#ifndef _SALIENCY_HPP_
#define _SALIENCY_HPP_

#include "afx_track.hpp"

namespace Saliency {

    float minMaxNormalize (
    	cv::Mat & salMap,
    	const float lowerBound, const float upperBound
    ) {
    	const int _width = salMap.cols, _height = salMap.rows;
    	float _maxVal (0), _minVal = std::numeric_limits<float>::max ();

    	REP_HEIGHT (_height) {
            const float * pMap = salMap.ptr<float> (iHeight);
            REP_WIDTH(_width) {
    			_maxVal = std::max<float> (_maxVal, pMap[iWidth]);
    			_minVal = std::min<float> (_minVal, pMap[iWidth]);
    		}
    	}

    	float _range = _maxVal - _minVal;
    	if (0 == _range) {
    		_range = 1;
    	}

    	float _averageSaliency = 0, _normRange = upperBound - lowerBound;
        REP_HEIGHT (_height) {
            float * pMap = salMap.ptr<float> (iHeight);
            REP_WIDTH (_width) {
    			pMap[iWidth] =  _normRange * (pMap[iWidth] - _minVal) / _range + lowerBound;
    			_averageSaliency += pMap[iWidth];
    		}
    	}
    	return _averageSaliency / (_width * _height);
    }

    template < typename T >
    void GaussianSmooth (
    		const cv::Mat & inputImg,
            const std::vector<float> & kernel,
    		cv::Mat & smoothImg
    ) {
    	const int _width = inputImg.cols, _height = inputImg.rows, _center = kernel.size () / 2;
        cv::Mat _tempImg (inputImg.size (), CV_32FC1);
    	smoothImg.create (inputImg.size (), CV_32FC1);

    	//--------------------------------------------------------------------------
       	// Blur in the x direction.
       	//---------------------------------------------------------------------------
    	REP_HEIGHT (_height) {
            const T * pInput = inputImg.ptr<T> (iHeight);
            float * pTemp = _tempImg.ptr<float> (iHeight);
            REP_WIDTH (_width) {
    			float _kernelSum (0), _weightSum (0);
    			for (int ww = (- _center), _curW = iWidth + ww, _curK = _center + ww; ww <= _center; ++ ww, ++ _curW, ++ _curK) {
    				if( (_curW >= 0) && (_curW < _width)) {
    					_weightSum += ( (float)pInput[_curW]) * kernel[_curK];
    					_kernelSum += kernel[_curK];
    				}
    			}
    			pTemp[iWidth] = _weightSum / _kernelSum;
    		}
    	}

    	//--------------------------------------------------------------------------
       	// Blur in the y direction.
       	//---------------------------------------------------------------------------
        REP_HEIGHT (_height) {
            float * pSmooth = smoothImg.ptr<float> (iHeight);
            REP_WIDTH (_width) {
    			float _kernelSum (0), _weightSum (0);
    			for (int hh = (- _center), _curH = iHeight + hh, _curK = _center + hh; hh <= _center; ++ hh, ++ _curH, ++ _curK ) {
    				if( (_curH >= 0) && (_curH < _height))
    				{
    					_weightSum += _tempImg.at<float> (_curH, iWidth) * kernel[_curK];
    					_kernelSum += kernel[_curK];
    				}
    			}
    			pSmooth[iWidth] = _weightSum / _kernelSum;
    		}
    	}
    }

    //==============================================================================
    ///	GetIntegralSum
    //==============================================================================
    float GetIntegralSum (
    	const std::vector<float> & intImg,
    	int xf, int yf,
    	int xs, int ys,
    	const int height,
    	const int width,
        const int bins,
        std::vector<float> & hist
    ) {
        hist.resize (bins);

    	xf = std::max<int> (xf, 0);
    	yf = std::max<int> (yf, 0);
    	xs = std::min<int> (xs, width - 1);
    	ys = std::min<int> (ys, height - 1);
    	const int _TL = ( (yf - 1) * width + (xf - 1)) * bins,
                _TR = ( (yf - 1) * width + xs) * bins,
                _BL = (ys * width + (xf - 1)) * bins,
                _BR = (ys * width + xs) * bins;

        const float _area = (xs - xf + 1) * (ys - yf + 1);
        float _response = 0.f;
        REP_BIN (bins) {
    	    float _sTL (0.f), _sTR (0.f), _sBL (0.f), _sBR (0.f);
        	if (yf >= 1)
        	{
        		if (xf >= 1)
        			_sTL = intImg[_TL + iBin];
        		if (xs >= 0)
        			_sTR = intImg[_TR + iBin];
        	}
        	if (ys >= 0)
        	{
        		if (xf >= 1)
        			_sBL = intImg[_BL + iBin];
        		if (xs >= 0)
        			_sBR = intImg[_BR + iBin];
        	}

            hist[iBin] = std::max<float> (0.f, _sBR + _sTL - _sTR - _sBL);
            hist[iBin] /= _area;
        	_response += hist[iBin] * hist[iBin];
        }
        return _response;
    }

    void addWeight (const cv::Mat & as, const float fas, const cv::Mat & ms, const float fms, cv::Mat & s, float & fs) {
        cv::addWeighted (as, Attributes::Saliency::static_ratio, ms, Attributes::Saliency::dynamic_ratio, 0.f, s);
        fs = Attributes::Saliency::static_ratio * fas + Attributes::Saliency::dynamic_ratio * fms;
    }
}

#endif
