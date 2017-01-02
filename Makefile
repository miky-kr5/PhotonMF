TARGET = ray
HEADERS = ray.hpp figure.hpp sphere.hpp plane.hpp disk.hpp material.hpp light.hpp directional_light.hpp point_light.hpp tracer.hpp 
OBJECTS = main.o sphere.o plane.o disk.o directional_light.o point_light.o tracer.o 
CXX = g++
CXXFLAGS = -ansi -pedantic -Wall -g -DGLM_FORCE_RADIANS -fopenmp
LDLIBS = 

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJECTS) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

main.o: main.cpp $(HEADERS)

sphere.o: sphere.cpp $(HEADERS)

plane.o: plane.cpp $(HEADERS)

disk.o: disk.cpp $(HEADERS)

tracer.o: tracer.cpp $(HEADERS)

directional_light.o: directional_light.cpp $(HEADERS)

point_light.o: point_light.cpp $(HEADERS)

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJECTS)
