// *******************************************************************************************************************************
// *******************************************************************************************************************************
//
//		Name:		processor.h
//		Purpose:	Processor Emulation (header)
//		Created:	26th October 2015
//		Author:		Paul Robson (paul@robsons.org.uk)
//
// *******************************************************************************************************************************
// *******************************************************************************************************************************

#ifndef _SYS_PROCESSOR_H
#define _SYS_PROCESSOR_H	

// *******************************************************************************************************************************
//															Timing
// *******************************************************************************************************************************
	
#define CYCLES_PER_SECOND	(500000)												// Clock at 500Khz
#define FRAME_RATE			(60)													// Frame rate
#define CYCLES_PER_FRAME	(CYCLES_PER_SECOND/FRAME_RATE)							// Instructions/Second

typedef unsigned short WORD16;														// 8 and 16 bit types.
typedef unsigned char  BYTE8;
typedef unsigned int   LONG32;

void CPUReset(void);																// CPU methods
BYTE8 CPUExecuteInstruction(void);													// Execute one instruction (multi phases)
void CPUInterruptRequest(void);														// Request Interrupt.

#ifdef INCLUDE_DEBUGGING_SUPPORT													// Only required for debugging

typedef struct _CPUSTATUS {
	WORD16 *dataStack;
	WORD16 *returnStack;
	WORD16 pc;
	WORD16 cycles;
	BYTE8 dsp;
	BYTE8 rsp;
} CPUSTATUS;

CPUSTATUS *CPUGetStatus(void);														// Access CPU State
void CPULoadBinary(char *fileName);													// Load Binary in.
BYTE8 CPURead(WORD16 address);														// Access RAM
BYTE8 CPUExecute(WORD16 break1,WORD16 break2);										// Run to break point(s)
WORD16 CPUGetStepOverBreakpoint(void);												// Get step over breakpoint

#endif
#endif