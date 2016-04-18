CC := g++
CFLAGS := -O3 -ggdb -pipe -Wall -std=c++11
BUILD := debug
INCLUDES := -I/usr/local/include -I./ -I./3rd/ -I./util
LIBS := -L/usr/local/lib \
		-lopencv_core -lopencv_highgui -lopencv_video \
		-lopencv_videoio -lopencv_imgproc -lopencv_imgcodecs
MODE := -D_DEBUG
TARGET := $(BUILD)/Main
OBJS := main.o
.PHONY : all
all : $(TARGET)
$(TARGET) : $(OBJS)
	$(CC) $(MODE) $(CFLAGS) $^ -o $@ $(INCLUDES) $(LIBS)
%.o : %.cpp
	$(CC) $(MODE) $(CFLAGS) -c $^ $(INCLUDES) $(LIBS)
.PHONY : _cleanObjs
 _cleanObjs :
	rm -rf *.o
.PHONY : _cleanTraget
 _cleanTraget:
	rm -rf $(TARGET)
.PHONY : clean
clean : _cleanObjs _cleanTraget
