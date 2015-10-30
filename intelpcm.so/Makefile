# Copyright (c) 2014 Intel Corporation

all: libintelpcm.so libintelpcm.a

OPT= -g -O3
CXXFLAGS+= -Wall -fPIC $(OPT) -I../
vpath %.cpp ..

libintelpcm.so: msr.o cpucounters.o pci.o client_bw.o utils.o c_wrap_intelpcm.o
	$(CXX) $(CXXFLAGS) -shared $^ -lpthread -o $@

libintelpcm.a: $(OBJS)
	ar rcs $@ $^

clean:
	rm -rf *.x *.o *~ *.so


