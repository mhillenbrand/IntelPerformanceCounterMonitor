/*
   Copyright (c) 2009-2013, Intel Corporation
some parts Copyright (c) 2015 Marius Hillenbrand, Karlsruhe Institute of Technology
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
// written by Patrick Lu
// modified by Marius Hillenbrand


/*!     \file pcm-llc.cpp
  \brief Use CBox counters for monitoring LLC lookups
  */
#define HACK_TO_REMOVE_DUPLICATE_ERROR
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <iomanip>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <assert.h>
#include "cpucounters.h"
#include "utils.h"

#define PCM_DELAY_DEFAULT 2.0 // in seconds
#define PCM_DELAY_MIN 0.015 // 15 milliseconds is practical on most modern CPUs
#define PCM_CALIBRATION_INTERVAL 50 // calibrate clock only every 50th iteration

using namespace std;

const uint32 max_sockets = 4;

void printInfoLine(uint64 counters[18], int cboxes, int csv)
{
	// determine hot cboxes
	uint64 sum = 0;
	uint64 avg;

	for(int j=0; j < cboxes; j++)
	    sum += counters[j];
	
	avg = sum / cboxes;

	// print counter values
        if(csv)
        {
	    for(int j=0; j < cboxes; j++)
		cout << "," << counters[j];
        }
        else
        {
	    for(int j=0; j < cboxes; j++) {
		// show whether cbox is hot
		if( counters[j] > avg )
		    cout << " | *" << counters[j] << "*";
		else
		    cout << " | " << counters[j];
	    }
        }

        cout << "\n";
}

int main(int argc, char * argv[])
{
    set_signal_handlers();

    std::cout.flags ( std::ios::showbase );

#ifdef PCM_FORCE_SILENT
    null_stream nullStream1, nullStream2;
    std::cout.rdbuf(&nullStream1);
    std::cerr.rdbuf(&nullStream2);
#endif

    cerr << endl;
    cerr << " Intel(r) Performance Counter Monitor: LLC Monitoring Utility "<< endl;
    cerr << "   modified to report LLC activity per CBox (is each CBox a slice?!?!" << endl;
    cerr << endl;
    cerr << " Copyright (c) 2013-2014 Intel Corporation" << endl;
    cerr << " Copyright (c) 2015 Marius Hillenbrand, Karlsruhe Institute of Technology" << endl;
    cerr << endl;

    double delay = PCM_DELAY_DEFAULT;
    bool csv = false;
    string program = string(argv[0]);

    PCM * m = PCM::getInstance();

    m->disableJKTWorkaround();
    PCM::ErrorCode status = m->program();
    switch (status)
    {
        case PCM::Success:
            break;
        case PCM::MSRAccessDenied:
            cerr << "Access to Intel(r) Performance Counter Monitor has denied (no MSR or PCI CFG space access)." << endl;
            exit(EXIT_FAILURE);
        case PCM::PMUBusy:
            cerr << "Access to Intel(r) Performance Counter Monitor has denied (Performance Monitoring Unit is occupied by other application). Try to stop the application that uses PMU." << endl;
            cerr << "Alternatively you can try to reset PMU configuration at your own risk. Try to reset? (y/n)" << endl;
            char yn;
            std::cin >> yn;
            if ('y' == yn)
            {
                m->resetPMU();
                cerr << "PMU configuration has been reset. Try to rerun the program again." << endl;
            }
            exit(EXIT_FAILURE);
        default:
            cerr << "Access to Intel(r) Performance Counter Monitor has denied (Unknown error)." << endl;
            exit(EXIT_FAILURE);
    }
    
    cerr << "\nDetected "<< m->getCPUBrandString() << " \"Intel(r) microarchitecture codename "<<m->getUArchCodename()<<"\""<<endl;
    if(!(m->hasPCICFGUncore()))
    {
        cerr << "Jaketown, Ivytown, Haswell Server CPU is required for this tool! Program aborted" << endl;
        exit(EXIT_FAILURE);
    }

    if(m->getNumSockets() > max_sockets)
    {
        cerr << "Only systems with up to "<<max_sockets<<" sockets are supported! Program aborted" << endl;
        exit(EXIT_FAILURE);
    }
  
    if(m->getNumCores() !=  m->getNumOnlineCores())
    {
        cerr << "Core offlining is not supported yet. Program aborted" << endl;
        exit(EXIT_FAILURE);
    }

    m->setBlocked(false);

    cerr << "Update every "<<delay<<" seconds"<< endl;

#define NUM_SAMPLES (1)

    uint32 i;
    uint32 delay_ms = uint32(delay * 1000);
    int coreId = -1;

    // sample_t sample[max_sockets];
    cerr << "delay_ms: " << delay_ms << endl;

	// ================================== Begin Printing Output ==================================

    PCM::LLCRequestType rqt = PCM::DataRead;
    PCM::CBoxOpcode     opc = PCM::RFO;
	
    while(1)
    {
	int cboxes = m->getMaxNumOfCBoxes();

	// setup counters
	m->freezeServerUncoreCounters();
	m->programLLCCounters(rqt, opc, coreId, 0);

	//m->unfreezeServerUncoreCounters(); // should not run now

	// wait
	MySleepUs(delay_ms*1000);
 
        
        if(m->getCPUModel() == PCM::HASWELLX) // Haswell Server
        {
	    // TODO: completely remodel this stuff
            for(i=0;i<NUM_SAMPLES;i++)
            {
		// TODO replace with getLLCEvents
                   }
            
            if(csv) {
                cout << "Skt,Cnt,Core,Type";
		for(int j=0; j < cboxes; j++)
		    cout << ",Cbo" << j;
		cout << endl;

            } else {
                cout << "Skt | Cnt | Core | Type";
		for(int j=0; j < cboxes; j++)
		    cout << " | CBo" << j;
		cout << endl;
	    }

            //report LLC activity per socket using the data from the sample
            for(i=0; i<m->getNumSockets(); ++i)
            {
		LLCCounterState cnt;

		m->freezeServerUncoreCounters();
		cnt = m->getLLCCounterState(i);

		// print start of line for LLC lookups
		if(csv)
	        {
	            cout << i << "," << "lookup" << ",";
		    cout << coreId << ",";
		    cout << std::hex << rqt << std::dec;
	        }
	        else
	        {
	            cout << " " << i << "  | " "lkp" << " | ";
		    cout << "  " << coreId << "  | ";
		    cout << std::hex << rqt << std::dec << " ";
	        }

		// print counter values
		printInfoLine(cnt.lookups, cboxes, csv);

		// print start of line for LLC requests
		// TODO add opcode, if filtered
		if(csv)
	        {
	            cout << i << "," << "req" << ",";
		    cout << coreId << ",";
		    cout << std::hex << opc << std::dec;
	        }
	        else
	        {
	            cout << " " << i << "  | " << "req" << " | ";
		    cout << " " << coreId << "  | ";
		    cout << std::hex << opc << std::dec << " ";
	        }
		
		// print counter values
		printInfoLine(cnt.requests, cboxes, csv);

            }
            if(!csv)
            {
                cout << "-----------------------------------------------------------------------\n";
                cout << " * ";
                cout << "\n\n";
            }
        }
        else // Ivytown and Older Architectures
        {
	    cerr << "We do not support older architectures than Haswell." << endl;
	    break;
        }
	
	// iterate request type forward
	switch(rqt) {
	    case PCM::DataRead: rqt = PCM::Write; break;
	    case PCM::Write: rqt = PCM::RemoteSnoop; // do not do break;
	    case PCM::RemoteSnoop: rqt = PCM::Any; break;
	    case PCM::Any: rqt = PCM::Read; break;
	    case PCM::Read: rqt = PCM::DataRead; break;
	    case PCM::Nid: rqt = PCM::DataRead; break; // unused, wrap over
	}
	// nope, stick to Read
	//rqt = PCM::Read;

	// iterate cbox opcode forward
#define ITER(p) opc = PCM::p; break; \
	case PCM::p: 

	switch(opc) {
	    case PCM::RFO: 
	    ITER(CRd);
	    ITER(DRd);
	    ITER(PRd);
	    ITER(WCiLF);
	    ITER(WCiL);
	    ITER(WiL);
	    ITER(WbMtoI);
	    ITER(WbMtoE);
	    ITER(ItoM);
	    ITER(AnyOp);
	    ITER(WB); opc=PCM::RFO; break;
	    default: opc = PCM::RFO;
	}
#undef ITER
	// nope, stick to RFO
        //opc = PCM::RFO;

	// iterate core Id
	//coreId++;
	//if(coreId > 7) coreId = 0;

    }
	
	// ================================== End Printing Output ==================================
	
    exit(EXIT_SUCCESS);
}

