CXX = g++
TARGET = ray pviewer
PVDIR = PhotonViewer
OBJECTS = main.o sampling.o camera.o environment.o disk.o plane.o sphere.o \
          phong_brdf.o hsa_brdf.o directional_light.o point_light.o \
          spot_light.o sphere_area_light.o disk_area_light.o scene.o tracer.o \
          path_tracer.o whitted_tracer.o rgbe.o kd_tree.o photon_tracer.o
DEPENDS = $(OBJECTS:.o=.d)
CXXFLAGS = -ansi -pedantic -Wall -DGLM_FORCE_RADIANS -fopenmp -std=c++11
LDLIBS = -lfreeimage -ljson_spirit

.PHONY: all
all: CXXFLAGS += -O3 -DNDEBUG
all: $(TARGET)

.PHONY: debug
debug: CXXFLAGS += -g
debug: $(TARGET)

ray: $(OBJECTS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDLIBS)

-include $(DEPENDS)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $*.cpp -o $*.o
	$(CXX) -MM $(CXXFLAGS) $*.cpp > $*.d

.PHONY: pviewer
pviewer:
	$(MAKE) $(MFLAGS) -C $(PVDIR)

.PHONY: clean
clean:
	$(RM) ray $(OBJECTS) $(DEPENDS)
	$(MAKE) $(MFLAGS) -C $(PVDIR) clean
