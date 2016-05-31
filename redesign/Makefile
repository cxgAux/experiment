# set the binaries that have to be built
TARGETS := Track

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
CXXFLAGS = -D __STDC_CONSTANT_MACROS -D STD=std -pipe $(CXXFLAGS_$(BUILD)) -I. -I/usr/local/include -I./include $(addprefix -I, $(TARGETS)) $(addprefix -I, $(TARGETS:%=%/include))
CXXFLAGS_base 		:= 		-Wl,--no-as-need, -Wall -std=c++11 -O3
CXXFLAGS_debug 		:= 		$(CXXFLAGS_base) -ggdb
CXXFLAGS_release 	:= 		$(CXXFLAGS_base) -DNDEBUG
LDFLAGS = -L/usr/local/lib

include make/generic.mk
