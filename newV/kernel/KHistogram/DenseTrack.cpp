#include "DenseTrack.h"
#include "Descriptors.h"
#include "Initialize.h"
#include "Saliency.h"
#include "Pyramid.hpp"
#include "VisualTask.hpp"
#include "DescComputation.hpp"
#include "AppearanceSaliency.hpp"
#include "MotionSaliency.hpp"

int show_track = 0; // set show_track = 1, if you want to visualize the trajectories

bool DenseTrack(int argc, char** argv) {
	int frameNum = 0;
	TrackerInfo tracker;
	DescInfo hogInfo;
	DescInfo hofInfo;
	DescInfo mbhInfo;
	Saliency sal;
	float staticRatio = 0.5;
	float dynamicRatio = 0.5;
	float threshold = 1.5;
	// kernel matrix of densely sampled angles
	CvMat* kernelMatrix = cvCreateMat(3600, 8, CV_32FC1);

	char* video = argv[1];
	arg_parse(argc, argv);
	cv::VideoCapture capture(video);

	InitTrackerInfo(&tracker, track_length, init_gap);
	InitDescInfo(&hogInfo, 8, 0, 1, patch_size, nxy_cell, nt_cell);
	InitDescInfo(&hofInfo, 9, 1, 1, patch_size, nxy_cell, nt_cell);
	InitDescInfo(&mbhInfo, 8, 0, 1, patch_size, nxy_cell, nt_cell);
	InitKernelMatrix(kernelMatrix, 5);
	cv::Mat kMat = cv::cvarrToMat(kernelMatrix);

	if( show_track == 1 )
		cv::namedWindow( "DenseTrack", 0 );

	std::vector<std::list<Track> > xyScaleTracks;
	std::vector<float> fscales;
	std::vector<cv::Size> sizes;
	cv::Mat frame, grey;
	std::vector<cv::Mat> greys_pyramid, prev_greys_pyramid, flow_pyramid;
	int init_counter = 0; // indicate when to detect new feature points
	while( true ) {
		capture >> frame;
		if(frame.empty()) {
			break;
		}

		int i, j, c;

		cxgAlleria::initPyramid(frame.cols, frame.rows, scale_num, scale_stride, fscales, sizes);
		xyScaleTracks.resize(scale_num);
		if( frameNum >= start_frame && frameNum <= end_frame ) {
			if( frameNum == start_frame ) {
				// initailize all the buffers
				grey.create(frame.size(), CV_8UC1);
				cxgAlleria::buildPyramid(sizes, greys_pyramid, CV_8UC1);
				cxgAlleria::buildPyramid(sizes, prev_greys_pyramid, CV_8UC1);
				cxgAlleria::buildPyramid(sizes, flow_pyramid, CV_32FC2);

				cv::cvtColor(frame, grey, CV_BGR2GRAY);

				cxgAlleria::setPyramid(grey, prev_greys_pyramid);

				for( int ixyScale = 0; ixyScale < scale_num; ++ixyScale ) {
					std::list<Track>& tracks = xyScaleTracks[ixyScale];

					// find good features at each scale separately
					std::vector<CvPoint2D32f> points(0);
					cxgAlleria::DenseSample(prev_greys_pyramid[ixyScale], points, quality, min_distance);
					// save the feature points
					for( i = 0; i < points.size(); i++ ) {
						Track track(tracker.trackLength);
						PointDesc point(hogInfo, hofInfo, mbhInfo, points[i], 0, 0);
						track.addPointDesc(point);
						tracks.push_back(track);
					}
				}
				frameNum ++;
				continue;
			}

			// build the image pyramid for the current frame
			cv::cvtColor(frame, grey, CV_BGR2GRAY );
			cxgAlleria::setPyramid(grey, greys_pyramid);

			if( frameNum > start_frame ) {
				init_counter++;
				for( int ixyScale = 0; ixyScale < scale_num; ++ixyScale ) {
				// track feature points in each scale separately
					std::vector<CvPoint2D32f> points_in(0);
					std::list<Track>& tracks = xyScaleTracks[ixyScale];
					for (std::list<Track>::iterator iTrack = tracks.begin(); iTrack != tracks.end(); ++iTrack) {
						CvPoint2D32f point = iTrack->pointDescs.back().point;
						points_in.push_back(point); // collect all the feature points
					}

					// compute the optical flow
					cv::calcOpticalFlowFarneback(prev_greys_pyramid[ixyScale], greys_pyramid[ixyScale], flow_pyramid[ixyScale],
									sqrt(2)/2.0, 5, 10, 2, 7, 1.5, cv::OPTFLOW_FARNEBACK_GAUSSIAN );
					// prevImg(y, x) = nextImg(y + flow(y, x)[1], x + flow(y, x)[0])?
					// the point (y,x) of previous image move to (y + flow(y, x)[1], x + flow(y, x)[0]) of next image

					int width = greys_pyramid[ixyScale].cols, height = greys_pyramid[ixyScale].rows;
					//--------------------------------begin--------------------------------
					//-------------------------modified by Yikun Lin-----------------------
					// compute the integral histograms
					DescMat* hogMat = InitDescMat(height, width, hogInfo.nBins);
					//HogComp(prev_grey_temp, hogMat, hogInfo, kernelMatrix);
					cxgAlleria::HogComp(prev_greys_pyramid[ixyScale], hogMat, hogInfo, kMat);

					DescMat* hofMat = InitDescMat(height, width, hofInfo.nBins);
					//HofComp(flow, hofMat, hofInfo, kernelMatrix);
					cxgAlleria::HofComp(flow_pyramid[ixyScale], hofMat, hofInfo, kMat);

					DescMat* mbhMatX = InitDescMat(height, width, mbhInfo.nBins);
					DescMat* mbhMatY = InitDescMat(height, width, mbhInfo.nBins);
					//MbhComp(flow, mbhMatX, mbhMatY, mbhInfo, kernelMatrix);
					cxgAlleria::MbhComp(flow_pyramid[ixyScale], mbhMatX, mbhMatY, mbhInfo, kMat);

					// calculate static saliency map

					//hog to calc the staticsaliencymap?
					// function api for calcstaticsaliency...

					//float staticSaliency = sal.CalcStaticSaliencyMap(prev_grey_temp, staticSalMap, true);
					cv::Mat staticSalMap(height, width, CV_32F, 1);
					float staticSaliency = cxgAlleria::CalcStaticSaliencyMap(prev_greys_pyramid[ixyScale], staticSalMap);

					/*if( show_track == 1 ) {
						IplImage* pImg = cvCreateImage(cvGetSize(staticSalMap), IPL_DEPTH_32F, 1);
						cvConvert(staticSalMap, pImg);
						char str[20];
						sprintf(str, "saliency1/%d.jpg", frameNum);// what's this mean??
						if (ixyScale == 0)
							cvSaveImage(str, pImg);
						cvReleaseImage( &pImg );
					}*/

					// calculate dynamic saliency map
					cv::Mat dynamicSalMap(height, width, CV_32F, 1);
					float dynamicSaliency = cxgAlleria::CalcMotionSaliencyMap(flow_pyramid[ixyScale], hofInfo, kMat, dynamicSalMap);
					/*if( show_track == 1 ) {
						IplImage* pImg = cvCreateImage(cvGetSize(dynamicSalMap), IPL_DEPTH_32F, 1);
						cvConvert(dynamicSalMap, pImg);
						char str[20];
						sprintf(str, "saliency2/%d.jpg", frameNum);
						if (ixyScale == 0)
							cvSaveImage(str, pImg);
						cvReleaseImage( &pImg );
					}*/

					// calculate combined saliency map
					cv::Mat salMap(height, width, CV_32F, 1);
					cv::addWeighted(staticSalMap, staticRatio, dynamicSalMap, dynamicRatio, 0.f, salMap);
					/*if( show_track == 1 ) {
						IplImage* pImg = cvCreateImage(cvGetSize(salMap), IPL_DEPTH_32F, 1);
						cvConvert(salMap, pImg);
						char str[20];
						sprintf(str, "saliency3/%d.jpg", frameNum);
						if (ixyScale == 0)
							cvSaveImage(str, pImg);
						cvReleaseImage( &pImg );
					}*/
					int count = points_in.size();
					float averageSaliency = staticRatio * staticSaliency + dynamicRatio * dynamicSaliency;
					std::vector<bool> status(count);
					std::vector<CvPoint2D32f> points_out(count);
					std::vector<float> saliency(count);

					// track feature points by median filtering
					cxgAlleria::OpticalFlowTracker(flow_pyramid[ixyScale], salMap, points_in, points_out, status, saliency);
					//---------------------------------end---------------------------

					i = 0;
					for (std::list<Track>::iterator iTrack = tracks.begin(); iTrack != tracks.end(); ++i) {
						if( status[i] ) { // if the feature point is successfully tracked
							PointDesc& pointDesc = iTrack->pointDescs.back();
							CvPoint2D32f prev_point = points_in[i];
							// get the descriptors for the feature point
							//CvScalar rect = getRect(prev_point, cvSize(width, height), hogInfo);
							cxgAlleria::RectInfo rectInfo(prev_point, cvSize(width, height), hogInfo);
							//pointDesc.hog = getDesc(hogMat, rect, hogInfo);
							cxgAlleria::getDesc(hogMat, rectInfo, hogInfo, epsilon, pointDesc.hog);
							//pointDesc.hof = getDesc(hofMat, rect, hofInfo);
							cxgAlleria::getDesc(hofMat, rectInfo, hofInfo, epsilon, pointDesc.hof);
							//pointDesc.mbhX = getDesc(mbhMatX, rect, mbhInfo);
							cxgAlleria::getDesc(mbhMatX, rectInfo, mbhInfo, epsilon, pointDesc.mbhX);
							//pointDesc.mbhY = getDesc(mbhMatY, rect, mbhInfo);
							cxgAlleria::getDesc(mbhMatY, rectInfo, mbhInfo, epsilon, pointDesc.mbhY);

							PointDesc point(hogInfo, hofInfo, mbhInfo, points_out[i], saliency[i], averageSaliency);
							iTrack->addPointDesc(point);
							++iTrack;
						}
						else // remove the track, if we lose feature point
							iTrack = tracks.erase(iTrack);
					}
					ReleDescMat(hogMat);
					ReleDescMat(hofMat);
					ReleDescMat(mbhMatX);
					ReleDescMat(mbhMatY);
				}
				for( int ixyScale = 0; ixyScale < scale_num; ++ixyScale ) {
					std::list<Track>& tracks = xyScaleTracks[ixyScale]; // output the features for each scale
					for( std::list<Track>::iterator iTrack = tracks.begin(); iTrack != tracks.end(); ) {
						if( iTrack->pointDescs.size() >= tracker.trackLength+1 ) { // if the trajectory achieves the length we want
							std::vector<CvPoint2D32f> trajectory(tracker.trackLength+1);
							std::vector<float> saliency(tracker.trackLength+1);
							std::vector<float> averageSaliency(tracker.trackLength+1);
							std::list<PointDesc>& descs = iTrack->pointDescs;
							std::list<PointDesc>::iterator iDesc = descs.begin();

							for (int count = 0; count <= tracker.trackLength; ++iDesc, ++count) {
								trajectory[count].x = iDesc->point.x*fscales[ixyScale];
								trajectory[count].y = iDesc->point.y*fscales[ixyScale];
								saliency[count] = iDesc->saliency;
								averageSaliency[count] = iDesc->averageSaliency;
							}

							if( isValid(trajectory, saliency, averageSaliency, threshold) == 1 ) {

								iDesc = descs.begin();
								int t_stride = cvFloor(tracker.trackLength/hogInfo.ntCells);
								for( int n = 0; n < hogInfo.ntCells; n++ ) {
									std::vector<float> vec(hogInfo.dim);
									for( int t = 0; t < t_stride; t++, iDesc++ )
										for( int m = 0; m < hogInfo.dim; m++ )
											vec[m] += iDesc->hog[m];
									for( int m = 0; m < hogInfo.dim; m++ )
										printf("%f\t", vec[m]/float(t_stride));
								}

								iDesc = descs.begin();
								t_stride = cvFloor(tracker.trackLength/hofInfo.ntCells);
								for( int n = 0; n < hofInfo.ntCells; n++ ) {
									std::vector<float> vec(hofInfo.dim);
									for( int t = 0; t < t_stride; t++, iDesc++ )
										for( int m = 0; m < hofInfo.dim; m++ )
											vec[m] += iDesc->hof[m];
									for( int m = 0; m < hofInfo.dim; m++ )
										printf("%f\t", vec[m]/float(t_stride));//do not know the form of it's output...
								}

								iDesc = descs.begin();
								t_stride = cvFloor(tracker.trackLength/mbhInfo.ntCells);
								for( int n = 0; n < mbhInfo.ntCells; n++ ) {
									std::vector<float> vec(mbhInfo.dim);
									for( int t = 0; t < t_stride; t++, iDesc++ )
										for( int m = 0; m < mbhInfo.dim; m++ )
											vec[m] += iDesc->mbhX[m];
									for( int m = 0; m < mbhInfo.dim; m++ )
										printf("%f\t", vec[m]/float(t_stride)); //t_stride 's number to be one average point of it.
								}

								iDesc = descs.begin();
								t_stride = cvFloor(tracker.trackLength/mbhInfo.ntCells);
								for( int n = 0; n < mbhInfo.ntCells; n++ ) {
									std::vector<float> vec(mbhInfo.dim);
									for( int t = 0; t < t_stride; t++, iDesc++ )
										for( int m = 0; m < mbhInfo.dim; m++ )
											vec[m] += iDesc->mbhY[m];
									for( int m = 0; m < mbhInfo.dim; m++ )
										printf("%f\t", vec[m]/float(t_stride));
								}

								printf("\n");
								/*if( show_track == 1 ) {
									std::list<PointDesc>& descs = iTrack->pointDescs;
									std::list<PointDesc>::iterator iDesc = descs.begin();
									float length = descs.size();
									CvPoint2D32f point0 = iDesc->point;
									point0.x *= fscales[ixyScale]; // map the point to first scale
									point0.y *= fscales[ixyScale];

									float j = 0;
									for (iDesc++; iDesc != descs.end(); ++iDesc, ++j) {
										CvPoint2D32f point1 = iDesc->point;
										point1.x *= fscales[ixyScale];
										point1.y *= fscales[ixyScale];

										cvLine(image, cvPointFrom32f(point0), cvPointFrom32f(point1),
										   	CV_RGB(0,cvFloor(255.0*(j+1.0)/length),0), 2, 8,0);
										point0 = point1;
									}
									cvCircle(image, cvPointFrom32f(point0), 2, CV_RGB(255,0,0), -1, 8,0);
								}*/
							}
							iTrack = tracks.erase(iTrack);
						}
						else
							iTrack++;
					}
				}

				if( init_counter == tracker.initGap ) { // detect new feature points every initGap frames
					init_counter = 0;
					for (int ixyScale = 0; ixyScale < scale_num; ++ixyScale) {
						std::list<Track>& tracks = xyScaleTracks[ixyScale];
						std::vector<CvPoint2D32f> points_in(0);
						std::vector<CvPoint2D32f> points_out(0);
						for(std::list<Track>::iterator iTrack = tracks.begin(); iTrack != tracks.end(); iTrack++, i++) {
							std::list<PointDesc>& descs = iTrack->pointDescs;
							CvPoint2D32f point = descs.back().point; // the last point in the track
							points_in.push_back(point);
						}
						cxgAlleria::DenseSample(greys_pyramid[ixyScale], points_in, quality, min_distance);
						points_out = points_in;

						// save the new feature points
						for( i = 0; i < points_out.size(); i++) {
							Track track(tracker.trackLength);
							PointDesc point(hogInfo, hofInfo, mbhInfo, points_out[i], 0, 0);
							track.addPointDesc(point);
							tracks.push_back(track);
						}
					}
				}
			}
			frameNum ++;

			for (int ixyScale = 0; ixyScale < scale_num; ++ixyScale) {
				greys_pyramid[ixyScale].copyTo(prev_greys_pyramid[ixyScale]);
			}
		}
		else {
			frameNum ++;
		}

		/*if( show_track == 1 ) {
			char str[20];
			sprintf(str, "traj/%d.jpg", frameNum);
			cvSaveImage(str, image);

			cvShowImage( "DenseTrack", image);
			c = cvWaitKey(3);
			if((char)c == 27) break;
		}*/
	}

	cvReleaseMat(&kernelMatrix);
	return true;
}
