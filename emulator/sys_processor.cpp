// *******************************************************************************************************************************
// *******************************************************************************************************************************
//
//		Name:		sys_processor.cpp
//		Purpose:	Processor Emulation.
//		Created:	26th October 2015
//		Author:		Paul Robson (paul@robsons.org.uk)
//
// *******************************************************************************************************************************
// *******************************************************************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include "sys_processor.h"
#include "hardware.h"
#include "__vminclude.h"

// *******************************************************************************************************************************
//												Main Memory/CPU
// *******************************************************************************************************************************

#define RAMSIZE 	(65536)

//	0100		address of top of dictionary
//	0102-0FFF 	Dictionary
//	1000		_temp
//	1100-11FF	20x12 display memory.
//	1200 		next free code address
// 	1202		<code starts>

static BYTE8 memory[RAMSIZE];													// RAM memory
static BYTE8 rSP,dSP;															// SP (next free +ve)
static WORD16 returnStack[32],dataStack[32];									// Stacks
static WORD16 pc;																// Program Counter
static WORD16 cycles;

// *******************************************************************************************************************************
//												Processor Reset
// *******************************************************************************************************************************

#define W(n) 	memory[pc++] = (n) & 255;memory[pc++] = (n) >> 8	 

void CPUReset(void) {

	for (int i = 0x1100;i < 0x1200;i++) memory[i] = rand();
	rSP = dSP = 0;
	pc = SYS_ADDR_CODEBASE + 2;
	cycles = 0;
	HWIReset();
}

// *******************************************************************************************************************************
//													 Execute a single phase.
// *******************************************************************************************************************************

#include <stdlib.h>
#include <stdio.h>

//	1100-11FF	20x12 display memory.

#define PULLD() dataStack[--dSP]
#define PUSHD(n) dataStack[dSP++] = (n)
#define PULLR() returnStack[--rSP]
#define PUSHR(n) returnStack[rSP++] = (n)

BYTE8 CPUExecuteInstruction(void) {
	WORD16 W1,W2,W3;
	cycles++;
	WORD16 opcode = memory[pc];
	pc = (pc + 1) & 0xFFFF;
	
	if ((opcode & 0x80) == 0) {
		switch(opcode) {
			#include "__vmcase.h"
		}
	} else {
		opcode = (opcode << 8) | memory[pc];
		pc = (pc + 1) & 0xFFFF;
		PUSHR(pc);
		pc = ((opcode << 1) & 0xFFFF) + SYS_ADDR_CODEBASE;
	}
	if (cycles < CYCLES_PER_FRAME) return 0;								// Frame in progress, return 0.
	cycles -= CYCLES_PER_FRAME;												// Adjust cycle counter
	HWIEndFrame();															// Hardware stuff.
	return FRAME_RATE;														// Return the frame rate for sync speed.
}

#ifdef INCLUDE_DEBUGGING_SUPPORT

// *******************************************************************************************************************************
//										 Get the step over breakpoint value
// *******************************************************************************************************************************

WORD16 CPUGetStepOverBreakpoint(void) {
	WORD16 opcode = memory[pc] + (memory[pc+1] << 8);
	if ((opcode & 0x8000) != 0)	 return pc + 2;
	return 0;
}

// *******************************************************************************************************************************
//										Run continuously till breakpoints / halt.
// *******************************************************************************************************************************

BYTE8 CPUExecute(WORD16 break1,WORD16 break2) {
	BYTE8 rate = 0;
	while(1) {
		rate = CPUExecuteInstruction();												// Execute one instruction phase.
		if (rate != 0) {															// If end of frame, return rate.
			return rate;													
		}
		if (pc == break1 || pc == break2) return 0;
	} 																				// Until hit a breakpoint or HLT.
}

// *******************************************************************************************************************************
//												Load a binary file into RAM
// *******************************************************************************************************************************

void CPULoadBinary(char *fileName) {
	FILE *f = fopen(fileName,"rb");
	fread(memory,RAMSIZE,1,f);
	fclose(f);
}

// *******************************************************************************************************************************
//												Gets a pointer to RAM memory
// *******************************************************************************************************************************
	
BYTE8 CPURead(WORD16 address) {
	return memory[address];
}

// *******************************************************************************************************************************
//											Retrieve a snapshot of the processor
// *******************************************************************************************************************************

static CPUSTATUS s;																	// Status area

CPUSTATUS *CPUGetStatus(void) {
	s.dataStack = dataStack;
	s.returnStack = returnStack;
	s.dsp = dSP;
	s.rsp = rSP;
	s.pc = pc;
	s.cycles = cycles;
	return &s;
}
#endif
