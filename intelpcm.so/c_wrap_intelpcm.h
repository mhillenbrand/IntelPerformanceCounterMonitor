// wrap_intelpcm.h
//
// definitions for C wrapper around Intel's (C++-only) PCM library
// Copyright 2015 Marius Hillenbrand, Karlsruhe Institute of Technology

#include <stdint.h>

typedef void * pcm_handle_t;

#ifdef __cplusplus
extern "C" {
#endif

struct LLCCounters {
    // only getMaxNumOfCBoxes() values will be set
    uint64_t lookups[18];
    uint64_t requests[18];
};

enum CBoxOpc
    {
	// PCIe read events (PCI devices reading from memory -
	// application writes to disk/network/PCIe device)
	PCIeRdCur = 0x19E, // PCIe read current (full cache line)
	PCIeNSRd = 0x1E4,  // PCIe non-snoop read (full cache line)
	// PCIe write events (PCI devices writing to memory -
	// application reads from disk/network/PCIe device)
	PCIeWiLF = 0x194,  // PCIe Write (non-allocating) (full cache line)
	PCIeItoM = 0x19C,  // PCIe Write (allocating) (full cache line)
	PCIeNSWr = 0x1E5,  // PCIe Non-snoop write (partial cache line)
	PCIeNSWrF = 0x1E6, // PCIe Non-snoop write (full cache line)
	// events shared by CPU and IO
	RFO = 0x180,       // Demand Data RFO; share the same code for CPU, use tid to filter PCIe only traffic
	CRd = 0x181,       // Demand Code Read
	DRd = 0x182,       // Demand Data Read
	PRd = 0x187,       // Partial Reads (UC) (MMIO Read)
	WCiLF = 0x18C,     // Full Streaming Store - write invalidate full cache line
	WCiL  = 0x18D,     // Partial Streaming Store - write invalidate for partial cache line
	WiL = 0x18F,       // Write Invalidate Line - partial (MMIO write), PL: Not documented in HSX/IVT
	WbMtoI = 0x1C4,    // Request writeback and invalidation of modified line
	WbMtoE = 0x1C5,    // Request writeback of modified line, set to exclusive
	ItoM = 0x1C8,      // Request Invalidate Line; share the same code for CPU, use tid to filter PCIe only traffic
	WB, // pseudo-opcode for actual writebacks
	AnyOp, // pseudo-opcode for do not filter
    };

pcm_handle_t getInstance();

int getNumSockets(pcm_handle_t instance);
int getMaxNumOfCBoxes(pcm_handle_t instance);

// Last Level Cache events
void programLLCCounters(pcm_handle_t instance);

struct LLCCounters getLLCCounterState(pcm_handle_t instance, int socket);

// PCIe related events
// program and reset counters
//	track misses (miss=1) or hits (miss=0)
void programPCIeCounters(pcm_handle_t instance, enum CBoxOpc opc, uint32_t tid, uint32_t miss);

// gets current reading of counters, aggregates over all CBoxes
uint64_t getPCIeCounters(pcm_handle_t instance, int socket);

#ifdef __cplusplus
}
#endif

