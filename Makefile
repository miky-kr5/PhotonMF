TARGET = ray
HEADERS = ray.hpp figure.hpp sphere.hpp plane.hpp light.hpp tracer.hpp
OBJECTS = main.o sphere.o plane.o tracer.o
CXX = g++
CXXFLAGS = -ansi -pedantic -Wall -g -DGLM_FORCE_RADIANS -fopenmp
LDLIBS = -lm

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJECTS) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

main.o: main.cpp $(HEADERS)

sphere.o: sphere.cpp $(HEADERS)

plane.o: plane.cpp $(HEADERS)

tracer.o: tracer.cpp $(HEADERS)

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJECTS)
