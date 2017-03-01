TARGET = pviewer
OBJECTS = main.o
CXXFLAGS = -ansi -pedantic -std=c++11 -g -Wall -DGLM_FORCE_RADIANS -I/usr/include/GL
LDLIBS = -lglut -lGLU -lGL

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

main.o: main.cpp

.PHONY: clean
clean:
	$(RM) $(TARGET) *.o
