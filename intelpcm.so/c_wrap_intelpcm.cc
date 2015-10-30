// wrap_intelpcm.cc
// Intel library is C++ only, so we need to wrap it up nicely
// Copyright 2015 Marius Hillenbrand, Karlsruhe Institute of Technology

#include "c_wrap_intelpcm.h"
#include <cpucounters.h>

pcm_handle_t getInstance() {

    PCM * m = PCM::getInstance();

    m->setBlocked(false);

    return (pcm_handle_t) m;
}

int getNumSockets(pcm_handle_t instance) {

    PCM * m = (PCM *) instance;

    return m->getNumSockets();
}

int getMaxNumOfCBoxes(pcm_handle_t instance) {

    PCM * m = (PCM *) instance;

    return m->getMaxNumOfCBoxes();
}


void programLLCCounters(pcm_handle_t instance) {

    PCM * m = (PCM *) instance;

    m->programLLCCounters();
}

// void freezeUncoreCounters(pcm_handle_t instance);
//void unfreezeUncoreCounters(pcm_handle_t instance);

struct LLCCounters getLLCCounterState(pcm_handle_t instance, int socket) {

    PCM * m = (PCM *) instance;

    LLCCounterState res = m->getLLCCounterState(socket);
    struct LLCCounters ret;

    memset(&ret, 0, sizeof(struct LLCCounters));

    memcpy(&ret, &res, sizeof(ret));

    return ret;
}


void programPCIeCounters(pcm_handle_t instance, enum CBoxOpc opc, uint32 tid, uint32 miss) {

    PCM::CBoxOpcode event = PCM::CBoxOpcode(opc);

    PCM * m = (PCM *) instance;

    m->programPCIeCounters(event, tid, miss);
}

uint64_t getPCIeCounters(pcm_handle_t instance, int socket) {

    PCM * m = (PCM *) instance;

    PCIeCounterState s = m->getPCIeCounterState(socket);

    return s.data;
}

