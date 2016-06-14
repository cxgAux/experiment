#ifndef _MOTIONSALIENCY_HPP_
#define _MOTIONSALIENCY_HPP_

#include "Saliency.hpp"

namespace MotionSaliency {

    //==============================================================================
    ///	CreateMotionIntegralImage
    //==============================================================================
    void CreateMotionIntegralImage (
    	const cv::Mat & xFlow, const cv::Mat & yFlow,
    	const Structs::DescInfo & descInfo,
    	const cv::Mat & kernelMatrix,
    	std::vector<float> & intImg
    ) {
    	const int _width = xFlow.cols,
                _height = xFlow.rows,
                _nBins = descInfo.getBin ();
    	const float _denseBase = (2 * M_PI) / float (kernelMatrix.rows);
    	int _idx = 0;
        intImg.resize (_width * _height * _nBins);

    	REP_HEIGHT (_height) {
            const float * _pX = xFlow.ptr<float> (iHeight), * _pY = yFlow.ptr<float> (iHeight);
    		// the histogram accumulated in the current line
    		std::vector<float> _khof (_nBins, 0.f);
    		REP_WIDTH (_width) {
    			const float _xShift = _pX[iWidth], _yShift = _pY[iWidth],
                    _magnitude = std::sqrt (_xShift * _xShift + _yShift * _yShift);

    			if(descInfo.isValid (_magnitude)) {
    				float _orientation = std::atan2 (_yShift, _xShift);
    				if (_orientation < 0) {
    					_orientation += 2 * M_PI;
    				}
    				int _iDense = roundf (_orientation / _denseBase);
    				if (_iDense >= kernelMatrix.rows) {
    					_iDense = 0;
    				}
    				// directly apply kernel histograms
    				const float * _pK = kernelMatrix.ptr<float>( _iDense);
    				REP_BIN (_nBins) {
    					_khof[iBin] += _magnitude * _pK[iBin];
    				}
    			}

    			int _realIdx = _idx * _nBins;
    			if (0 == iHeight) { // for the first line
    				REP_BIN (_nBins) {
    					intImg[_realIdx ++] = _khof[iBin];
                    }
    			}
    			else {
    				int _prevIdx = (_idx - _width) * _nBins;
    				REP_BIN (_nBins) {
    					intImg[_realIdx ++] = intImg[_prevIdx ++] + _khof[iBin];
                    }
    			}

                ++ _idx;
    		}
    	}
    }

    float CalcMotionSaliencyMap (
    	const cv::Mat & flow,
    	const Structs::DescInfo& descInfo,
    	const cv::Mat & kernelMatrix,
    	cv::Mat & salMap
    ) {

        const int _width = flow.cols, _height = flow.rows, _nBins = descInfo.getBin ();
    	std::vector<float> kernel (0);kernel.push_back (1.f);kernel.push_back (2.f);kernel.push_back (1.f);

    	cv::Mat flows[2], sflows[2];
        cv::split (flow, flows);
        Saliency::GaussianSmooth<float> (flows[0], kernel, sflows[0]);
        Saliency::GaussianSmooth<float> (flows[1], kernel, sflows[1]);

    	std::vector<float> intImg;
    	CreateMotionIntegralImage (sflows[0], sflows[1], descInfo, kernelMatrix, intImg);

    	// calculate motion saliency for each pixel
    	REP_HEIGHT (_height) {
    		int _yoff = std::min<int> (iHeight, _height - iHeight);
    		float * _pSalMap = salMap.ptr<float> (iHeight);
    	       REP_WIDTH (_width) {
    			int _xoff = std::min<int> (iWidth, _width - iWidth);
    			std::vector<float> _surround;
    			Saliency::GetIntegralSum (intImg, iWidth - _xoff, iHeight - _yoff, iWidth + _xoff, iHeight + _yoff, _height, _width, _nBins, _surround);
    			std::vector<float> _point;
    			_yoff = std::min<int> (_yoff, 0);
    			_xoff = std::min<int> (_xoff, 0);
    			_pSalMap[iWidth] = 0.f;
    			if (Saliency::GetIntegralSum (intImg, iWidth - _xoff, iHeight - _yoff, iWidth + _xoff, iHeight + _yoff, _height, _width, _nBins, _point) > 0) {
    				_pSalMap[iWidth] = AFX::Measure::_chisquareDistanceHandler (_point, _surround);
    			}
    		}
    	}

    	return Saliency::minMaxNormalize(salMap, 0.f, 255.f);
    }

}

#endif
