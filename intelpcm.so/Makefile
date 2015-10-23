# Copyright (c) 2014 Intel Corporation

all: libintelpcm.so libintelpcm.a

OPT= -g -O3
CXXFLAGS+= -Wall -fPIC $(OPT) -I../
vpath %.cpp ..

OBJS=msr.o cpucounters.o pci.o client_bw.o c_wrap_intelpcm.o


libintelpcm.so: $(OBJS)
	$(CXX) $(CXXFLAGS) -shared $^ -lpthread -o $@

libintelpcm.a: $(OBJS)
	ar rcs $@ $^

clean:
	rm -rf *.x *.o *~ *.so


