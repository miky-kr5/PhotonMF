CXX = g++
TARGET = ray
OBJECTS = main.o sampling.o camera.o environment.o disk.o plane.o sphere.o \
          phong_brdf.o hsa_brdf.o directional_light.o point_light.o \
          spot_light.o sphere_area_light.o disk_area_light.o scene.o tracer.o \
          path_tracer.o whitted_tracer.o rgbe.o kd_tree.o photon_tracer.o
DEPENDS = $(OBJECTS:.o=.d)
CXXFLAGS = -ansi -pedantic -Wall -DGLM_FORCE_RADIANS -fopenmp
LDLIBS = -lfreeimage -ljson_spirit

.PHONY: all
all: CXXFLAGS += -O2 -DNDEBUG
all: $(TARGET)

.PHONY: debug
debug: CXXFLAGS += -g
debug: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) -o $(TARGET) $(OBJECTS) $(CXXFLAGS) $(LDLIBS)

-include $(DEPENDS)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $*.cpp -o $*.o
	$(CXX) -MM $(CXXFLAGS) $*.cpp > $*.d

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJECTS) $(DEPENDS)
