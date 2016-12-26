TARGET = ray
HEADERS = ray.hpp sphere.hpp figure.hpp
OBJECTS = main.o sphere.o
CXX = g++
CXXFLAGS = -ansi -pedantic -Wall -g -DGLM_FORCE_RADIANS -fopenmp
LDLIBS = -lm

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJECTS) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

main.o: main.cpp $(HEADERS)

sphere.o: sphere.cpp $(HEADERS)

.PHONY: clean
clean:
	$(RM) $(TARGET) *.o
