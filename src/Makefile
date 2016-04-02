CC := g++
CFLAGS := -O3 -ggdb -Wall -std=c++11
BUILD := debug
INCLUDE := -I/usr/local/include
LIBS := -L/usr/local/lib -lopencv_core -lopencv_highgui -lopencv_video -lopencv_videoio
TARGET := $(BUILD)/Main
OBJS := main.o
.PHONY : all
all : $(TARGET)
$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(INCLUDES) $(LIBS)
%.o : %.cpp
	$(CC) $(CFLAGS) -c $^
.PHONY : _cleanObjs
 _cleanObjs :
	rm -rf *.o
.PHONY : _cleanTraget
 _cleanTraget:
	rm -rf $(TARGET)
.PHONY : clean
clean : _cleanObjs _cleanTraget
