TARGET = ray
HEADERS = ray.hpp sphere.hpp figure.hpp light.hpp tracer.hpp
OBJECTS = main.o sphere.o tracer.o
CXX = g++
CXXFLAGS = -ansi -pedantic -Wall -g -DGLM_FORCE_RADIANS -fopenmp
LDLIBS = -lm

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJECTS) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

main.o: main.cpp $(HEADERS)

sphere.o: sphere.cpp $(HEADERS)

tracer.o: tracer.cpp $(HEADERS)

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJECTS)
