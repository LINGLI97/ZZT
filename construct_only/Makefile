CXX       ?= g++
SDSL_ROOT ?= ../third_party/sdsl-install
CXXFLAGS  ?= -D_USE_64 -DCONSTRUCT_ONLY -O3 -fopenmp -fomit-frame-pointer -funroll-loops -msse3 -ffunction-sections -fdata-sections -std=c++17
CPPFLAGS  ?= -I$(SDSL_ROOT)/include
LDFLAGS   ?= -Wl,--gc-sections
LDLIBS    ?= -L$(SDSL_ROOT)/lib -lsdsl -ldivsufsort -ldivsufsort64

.PHONY: all clean

all: zzt_construct_only

zzt_construct_only: CC.o SA_LCP_LCE.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

CC.o: CC.cpp SA_LCP_LCE.h cmdline.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ CC.cpp

SA_LCP_LCE.o: SA_LCP_LCE.cpp SA_LCP_LCE.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ SA_LCP_LCE.cpp

clean:
	rm -f zzt_construct_only CC.o SA_LCP_LCE.o
