CXX?=g++
CFLAGS?=
CXXFLAGS+=-std=c++17 $(CFLAGS)
LDFLAGS?=

OBJECTS=
MAIN_OBJECTS=dft.o

DEPENDENCY_RULES=$(OBJECTS:=.d) $(MAIN_OBJECTS:=.d)

TARGETS=dft fft

all: dft fft

dft: dft.o $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cc compiler-flags
	$(CXX) $(CXXFLAGS)  -c  $< -o $@
	@$(CXX) $(CXXFLAGS) -MM $< > $@.d

-include $(DEPENDENCY_RULES)

clean:
	rm -rf $(TARGETS) $(OBJECTS) $(MAIN_OBJECTS) $(DEPENDENCY_RULES)

compiler-flags: FORCE
	@echo '$(CXX) $(CXXFLAGS) | cmp -s - $@ || echo '$(CXX) $(CXXFLAGS) > $@

.PHONY: FORCE

