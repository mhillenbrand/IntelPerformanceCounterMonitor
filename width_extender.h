/*
Copyright (c) 2009-2012, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
// written by Roman Dementiev
//            Austen Ott

#ifndef WIDTH_EXTENDER_HEADER_
#define WIDTH_EXTENDER_HEADER_ 

/*!     \file width_extender.h
        \brief Provides 64-bit "virtual" counters from underlying 32-bit HW counters
*/

#ifdef _MSC_VER
#include <windows.h>
#else
#include <pthread.h>
#endif

#include <stdlib.h>
#include "cpucounters.h"
#include "client_bw.h"
#include "mutex.h"
#include <memory>

#ifdef _MSC_VER
DWORD WINAPI WatchDogProc(LPVOID state);
#else
void * WatchDogProc(void * state);
#endif

class CounterWidthExtender
{
public:
   struct AbstractRawCounter
   {
       virtual uint64 operator() () = 0;
       virtual ~AbstractRawCounter() {}
   };

   struct MsrHandleCounter : public AbstractRawCounter
   {
      std::shared_ptr<SafeMsrHandle>  msr;
      uint64 msr_addr;
      MsrHandleCounter(std::shared_ptr<SafeMsrHandle> msr_, uint64 msr_addr_) : msr(msr_), msr_addr(msr_addr_) {}
      uint64 operator() ()
      {
         uint64 value = 0;
         msr->read(msr_addr,&value);
         return value;
      } 
   };

   struct ClientImcReadsCounter : public AbstractRawCounter
   {
      std::shared_ptr<ClientBW> clientBW;
      ClientImcReadsCounter(std::shared_ptr<ClientBW>  clientBW_) : clientBW(clientBW_) {}
      uint64 operator() () { return clientBW->getImcReads(); }
   };

   struct ClientImcWritesCounter : public AbstractRawCounter
   {
      std::shared_ptr<ClientBW> clientBW;
      ClientImcWritesCounter(std::shared_ptr<ClientBW> clientBW_) : clientBW(clientBW_) {}
      uint64 operator() () { return clientBW->getImcWrites(); }
   };
   
   struct ClientIoRequestsCounter : public AbstractRawCounter
   {
      std::shared_ptr<ClientBW> clientBW;
      ClientIoRequestsCounter(std::shared_ptr<ClientBW> clientBW_) : clientBW(clientBW_) {}
      uint64 operator() () { return clientBW->getIoRequests(); }
   };

   struct MBLCounter : public AbstractRawCounter
   {
       std::shared_ptr<SafeMsrHandle> msr;
       MBLCounter(std::shared_ptr<SafeMsrHandle> msr_) : msr(msr_){}
       uint64 operator() ()
       {
           msr->lock();
           msr->write(IA32_QM_EVTSEL, 0xdead); // TODO: change 0xdead to MBL event value
           uint64 value = 0;
           msr->read(IA32_PQR_ASSOC, &value);
           msr->unlock();
           return value;
       }
   };

   struct MBRCounter : public AbstractRawCounter
   {
       SafeMsrHandle * msr;
       MBRCounter(SafeMsrHandle * msr_) : msr(msr_){}
       uint64 operator() ()
       {
           msr->lock();
           msr->write(IA32_QM_EVTSEL, 0xdead); // TODO: change 0xdead to MBR event value
           uint64 value = 0;
           msr->read(IA32_PQR_ASSOC, &value);
           msr->unlock();
           return value;
       }
   };

private:

#ifdef _MSC_VER
	HANDLE UpdateThread;
#else
    pthread_t UpdateThread;
#endif

    PCM_Util::Mutex CounterMutex;

    AbstractRawCounter * raw_counter;
    uint64 extended_value;
    uint64 last_raw_value;

    CounterWidthExtender(); // forbidden
    CounterWidthExtender(CounterWidthExtender&); // forbidden
    CounterWidthExtender & operator = (const CounterWidthExtender &); // forbidden

    uint64 internal_read()
    {
		uint64 result = 0, new_raw_value = 0;
        CounterMutex.lock();

         new_raw_value = (*raw_counter)();
	 if(new_raw_value < last_raw_value)
         {
                extended_value += ((1ULL<<32ULL)-last_raw_value) + new_raw_value;
         }
	 else
	 {
		extended_value += (new_raw_value-last_raw_value);
         }

         last_raw_value = new_raw_value;

         result = extended_value;	

         CounterMutex.unlock();
         return result;
    }

public:
    CounterWidthExtender(AbstractRawCounter * raw_counter_): raw_counter(raw_counter_) 
    {
        last_raw_value = (*raw_counter)();
        extended_value = last_raw_value;

#ifdef _MSC_VER
		UpdateThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)WatchDogProc,this,0,NULL);
#else
        pthread_create(&UpdateThread, NULL, WatchDogProc, this);
#endif
    }
    virtual ~CounterWidthExtender()
    {
#ifdef _MSC_VER
		TerminateThread(UpdateThread,0);
		CloseHandle(UpdateThread);
#else
        pthread_cancel(UpdateThread);
#endif
        if(raw_counter) delete raw_counter;
    }
    
    uint64 read() // read extended value
    {
	return internal_read();
    }
};


#endif
