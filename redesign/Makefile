# set the binaries that have to be built
TARGETS := Track SelectFeatures Cluster EncodeFeatures ComputeDistance ComputeKernelMatrix TrainAndTest 

# set the build configuration set
#BUILD := release
BUILD := debug

# set bin and build dirs
BUILDDIR := .build_$(BUILD)
BINDIR := $(BUILD)

# libraries
LDLIBS = $(addprefix -l, $(LIBS) $(LIBS_$(notdir $*)))
LIBS := \
	boost_program_options boost_regex boost_system boost_filesystem \
	opencv_core opencv_imgproc opencv_video opencv_features2d opencv_flann \
	opencv_calib3d opencv_objdetect opencv_imgcodecs opencv_videoio  \
	opencv_highgui avformat avutil avcodec swscale z
#LIBS_hog2D := boost_python-mt

# set some flags and compiler/linker specific commands
CXXFLAGS = -D __STDC_CONSTANT_MACROS -D STD=std -pipe $(CXXFLAGS_$(BUILD)) $(IDFLAGS)
CXXFLAGS_base 		:= 		-Wl,--no-as-need, -Wall -std=c++11 -O3
CXXFLAGS_debug 		:= 		$(CXXFLAGS_base) -ggdb -D_ENABLE_ASSERT_
CXXFLAGS_release 	:= 		$(CXXFLAGS_base)
IDFLAGS := -I. -I/usr/local/include -I./include -ITrack -ITrack/include -ICluster -ICluster/include
LDFLAGS := -L/usr/local/lib

include make/generic.mk
