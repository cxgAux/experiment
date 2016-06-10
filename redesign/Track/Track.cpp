#include "Track.hpp"
#include "track_argParse.hpp"
#include "Initialize.hpp"
#include "Visual.hpp"
#include "Descriptor.hpp"
#include "AppearanceSaliency.hpp"
#include "MotionSaliency.hpp"

void Track (int argc, char * const argv[]) {
	/**
	 *	@brief	parse arguments
	 */
	const char * const videoName = arg_parse(argc, argv);
	assert(videoName != nullptr);
	cv::VideoCapture _capture(videoName);
	if(_capture.isOpened() == false) {
		std::cerr << "Invalid video file : " << videoName << std::endl;
		abort();
	}

	Attributes::normalizedRatios();
	Structs::TrackerInfo _trackerInfo(Attributes::Tracking::traj_length, Attributes::Tracking::init_gap);
	Structs::HogInfo _hogInfo (Attributes::Descriptor::Cube::nxCells, Attributes::Descriptor::Cube::nyCells, Attributes::Descriptor::Cube::ntCells, Attributes::Descriptor::Cube::patch_size, Attributes::Descriptor::Cube::patch_size, Attributes::Descriptor::hogBins, 2);
	Structs::HofInfo _hofInfo (Attributes::Descriptor::Cube::nxCells, Attributes::Descriptor::Cube::nyCells, Attributes::Descriptor::Cube::ntCells, Attributes::Descriptor::Cube::patch_size, Attributes::Descriptor::Cube::patch_size, Attributes::Descriptor::hofBins, 2, Attributes::Descriptor::Hof::min_flow);
	Structs::MbhInfo _mbhInfo (Attributes::Descriptor::Cube::nxCells, Attributes::Descriptor::Cube::nyCells, Attributes::Descriptor::Cube::ntCells, Attributes::Descriptor::Cube::patch_size, Attributes::Descriptor::Cube::patch_size, Attributes::Descriptor::mbhBins, 2);
	cv::Mat _kernelMatrix = Initialize::InitKernelMatrix(Attributes::KernelMatrix::denseBins, Attributes::Descriptor::hogBins, Attributes::KernelMatrix::radius);

	if (Attributes::Mode::toDisplay) {
		cv::namedWindow("Track", cv::WINDOW_AUTOSIZE);
		cv::moveWindow("Track", 0, 0);
		cv::namedWindow("Origin", cv::WINDOW_AUTOSIZE);
		cv::moveWindow("Origin", 200, 100);
		cv::namedWindow("Static", cv::WINDOW_AUTOSIZE);
		cv::moveWindow("Static", 400, 200);
		cv::namedWindow("Dynamic", cv::WINDOW_AUTOSIZE);
		cv::moveWindow("Dynamic", 600, 300);
		cv::namedWindow("Salient", cv::WINDOW_AUTOSIZE);
		cv::moveWindow("Salient", 800, 400);
	}

	/**
	 *	@brief	data declaration
	 */
	std::vector<std::list<Structs::SalientTrajectory>> _tracker;
	std::vector<float> _fscales;
	std::vector<cv::Size> _vSizes;
	cv::Mat _frame, _grey;
	std::vector<cv::Mat> _greys_pyramid, _prev_greys_pyramid, _flows_pyramid;
	std::vector<std::vector<cv::Point2f>> _points(0);
	std::vector<bool> _status;
	std::vector<cv::Point2f> _points_out;
	std::vector<float> _saliencies;

	int _count_to_sample = 0, _frameNum = 0;

	/**
	 *	@brief	main process
	 */
	while (true) {
		_capture >> _frame;
		if (_frame.empty ()) {
			break;
		}

		if (_frameNum >= Attributes::Tracking::start_frame) {

			_grey.create (_frame.size(), CV_8UC1);
			cv::cvtColor (_frame, _grey, cv::COLOR_BGR2GRAY);
			DISP ("Origin", _grey, 0);
			/**
			 *	@brief	1.	initialize data
			 			2.	sample
			 */
			if (_frameNum == Attributes::Tracking::start_frame) {

				Initialize::initPyramid (_grey.cols, _grey.rows, Attributes::Descriptor::MultiScale::scale_num, Attributes::Descriptor::MultiScale::scale_stride, _fscales, _vSizes);

				Visual::buildPyramid (_vSizes, _greys_pyramid, CV_8UC1);
				Visual::buildPyramid (_vSizes, _prev_greys_pyramid, CV_8UC1);
				Visual::buildPyramid (_vSizes, _flows_pyramid, CV_32FC2);

				Visual::setPyramid (_grey, _prev_greys_pyramid);

				_tracker.resize (Attributes::Descriptor::MultiScale::scale_num);
				_points.resize (Attributes::Descriptor::MultiScale::scale_num);

				REP_SCALE (Attributes::Descriptor::MultiScale::scale_num) {
					std::list<Structs::SalientTrajectory> & _iTracker = _tracker[iScale];

					Visual::DenseSample(_prev_greys_pyramid[iScale], _points[iScale], Attributes::Tracking::quality, Attributes::Tracking::min_distance);
					for (const auto & _point : _points[iScale]) {
						Structs::SalientTrajectory _newTraj (Attributes::Tracking::traj_length, Attributes::Saliency::salient_ratio);
						_iTracker.push_back (_newTraj);
					}
				}
			}
			else {
				Visual::setPyramid (_grey, _greys_pyramid);

				_count_to_sample ++;

				REP_SCALE (Attributes::Descriptor::MultiScale::scale_num) {
					/**
					 *	@brief	calculate optical flow
					 *	@warn	see Wang's improved version
					 */
					cv::calcOpticalFlowFarneback (_prev_greys_pyramid[iScale], _greys_pyramid[iScale], _flows_pyramid[iScale],
						std::sqrt(2) / 2.0, 5, 10, 2, 7, 1.5, cv::OPTFLOW_FARNEBACK_GAUSSIAN);

					/**
					 *	@brief	compute integral histograms
					 */
					const int _width = _greys_pyramid[iScale].cols, _height = _greys_pyramid[iScale].rows;

					Structs::DescMat _hogMat (_width, _height, _hogInfo.m_iBin),
						_hofMat (_width, _height, _hofInfo.m_iBin),
						_mbhXMat (_width, _height, _mbhInfo.m_iBin),
						_mbhYMat (_width, _height, _mbhInfo.m_iBin);

					Descriptor::HogComp (_prev_greys_pyramid [iScale], _hogMat, _hogInfo, _kernelMatrix);
					Descriptor::HofComp (_flows_pyramid [iScale], _hofMat, _hofInfo, _kernelMatrix);
					Descriptor::MbhComp (_flows_pyramid [iScale], _mbhXMat, _mbhYMat, _mbhInfo, _kernelMatrix);

					cv::Mat _asm(_height, _width, CV_32FC1), _msm(_height, _width, CV_32FC1), _sm(_height, _width, CV_32FC1);
					float _as = AppearanceSaliency::CalcStaticSaliencyMap(_prev_greys_pyramid[iScale], _asm),
						_ms = MotionSaliency::CalcMotionSaliencyMap(_flows_pyramid[iScale], _hofInfo, _kernelMatrix, _msm),
						_s = 0.f;

					Saliency::addWeight (_asm, _as, _msm, _ms, _sm, _s);

					DISP ("Static", _asm, iScale);
					DISP ("Dynamic", _msm, iScale);
					DISP ("Salient", _sm, iScale);

					/**
					 *	@brief	track by medium-filtered optical flow
					 */
					_status.clear ();
					_points_out.clear ();
					_saliencies.clear ();

					Visual::OpticalFlowTracker (_flows_pyramid[iScale], _sm, _points[iScale], _points_out, _status, _saliencies);

					std::list<Structs::SalientTrajectory> & _iTracker = _tracker[iScale];
					const int _ct = int (_points_out.size ());
					int _idx = 0;
					for (std::list<Structs::SalientTrajectory>::iterator _itt = _iTracker.begin (); _itt != _iTracker.end (); ++ _idx) {
						if (_status[_idx]) {// if successfully tracked
							cv::Point2f & _prev_point = _points[iScale][_idx];
							float _xOff = _points_out[_idx].x * _fscales[_idx] - _prev_point.x * _fscales[_idx],
								_yOff = _points_out[_idx].y * _fscales[_idx] - _prev_point.y * _fscales[_idx];
							if (std::isnan (_xOff)) {
								_xOff = 0;
							}
							if (std::isnan (_yOff)) {
								_yOff = 0;
							}
							cv::Point2f _offset (_xOff, _yOff);
							Descriptor::RectInfo _rectInfo (_prev_point, cv::Size (_width, _height), _hogInfo);
							_itt->addPoint (_prev_point, _offset, _saliencies[_idx], _s);
							Structs::PointDesc & _point_desc = _itt->m_pointDescs.back ();
							Descriptor::getDesc (_hogMat, _rectInfo, _hogInfo, Attributes::Descriptor::Hof::epsilon, _point_desc.m_hog);
							Descriptor::getDesc (_hofMat, _rectInfo, _hofInfo, Attributes::Descriptor::Hof::epsilon, _point_desc.m_hof);
							Descriptor::getDesc (_mbhXMat, _rectInfo, _mbhInfo, Attributes::Descriptor::Hof::epsilon, _point_desc.m_mbhX);
							Descriptor::getDesc (_mbhYMat, _rectInfo, _mbhInfo, Attributes::Descriptor::Hof::epsilon, _point_desc.m_mbhY);

							if (_itt->isEnded ()) {// if the trajectory comes to its end
								_status[_idx] = false;
								if (_itt->isSalient ()) {// if it is salient
									_itt->print(_hogInfo, _hofInfo, _mbhInfo);
									if (Attributes::Mode::toDisplay) {
										_itt->drawOn (_frame, _points_out[_idx], _fscales[_idx]);
									}
								}
								//remove all ended trajectories
								_itt = _iTracker.erase (_itt);
							}
							else {
								//continue to track it
								++ _itt;
							}
						}
						else {// remove trajs which are fail to track
							_itt = _iTracker.erase (_itt);
						}
					}

					/**
					 *	@brief	update candidates
					 */
					_points[iScale].clear ();
					for (_idx = 0; _idx < _ct; ++ _idx) {
						if (_status[_idx]) {
							_points[iScale].push_back (_points_out[_idx]);
						}
					}
				}

				DISP ("Track", _frame, 0);


				if (_count_to_sample == Attributes::Tracking::init_gap) {
					_count_to_sample = 0;
					REP_SCALE (Attributes::Descriptor::MultiScale::scale_num) {
						std::vector<cv::Point2f> _occupied = _points[iScale];
						Visual::DenseSample (_greys_pyramid[iScale], _occupied, Attributes::Tracking::quality, Attributes::Tracking::min_distance);
						for (const auto & _point : _occupied) {
							_points[iScale].push_back (_point);
							_tracker[iScale].push_back (Structs::SalientTrajectory (Attributes::Tracking::traj_length, Attributes::Saliency::salient_ratio));
						}
					}
				}

				REP_SCALE (Attributes::Descriptor::MultiScale::scale_num) {
					_greys_pyramid[iScale].copyTo (_prev_greys_pyramid[iScale]);
				}
			}
		}

		++ _frameNum;

	}

	return;
}
