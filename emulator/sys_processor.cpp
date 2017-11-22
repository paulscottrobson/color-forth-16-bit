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
#include "__core.h"

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
	pc = CODE_BASE_ADDRESS + 2;
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

BYTE8 CPUExecuteInstruction(void) {
	WORD16 w1,w2,w3;
	cycles++;
	WORD16 opcode = memory[pc] + (memory[pc+1] << 8);
	pc = (pc + 2) & 0xFFFF;
	if ((opcode & 0x8000) == 0) {											// Bit 15 clear core
		switch(opcode) {
			case KWD_HALT:			/* halt */
				pc = (pc - 2) & 0xFFFF;cycles = CYCLES_PER_FRAME;
				break;
			case KWD_PLING:			/* ! */
				w1 = PULLD();w2 = PULLD();
				memory[w1] = (w2 & 0xFF);
				memory[w1+1] = ((w2 >> 8) & 0xFF);
				break;
			case KWD_PLUS:			/* + */
				w1 = PULLD();w2 = PULLD();PUSHD((w1 + w2) & 0xFFFF);
				break;
			case KWD_0EQUAL:		/* 0= */
				w1 = PULLD();PUSHD(w1 == 0 ? 0xFFFF: 0x0000);
				break;
			case KWD_2SLASH:		/* 2/ */
				w1 = PULLD();w1 = (w1 >> 1) | (w1 & 0x8000);PUSHD(w1);
				break;
			case KWD_SEMICOLON:		/* ; */
				pc = returnStack[--rSP];
				break;
			case KWD_GREATERR:		/* >R */
				w1 = PULLD();returnStack[rSP++] = w1;
				break;
			case KWD_AT:			/* @ */
				w1 = PULLD();w2 = memory[w1] + memory[w1+1] * 256;PUSHD(w2);
				break;
			case KWD_ATCODE:		/* @code */
				PUSHD(CODE_BASE_ADDRESS);
				break;
			case KWD_ATDICT:		/* @dict */
				PUSHD(DICT_BASE_ADDRESS);
				break;
			case KWD_ATTEMP:		/* @temp */
				PUSHD(0x1000);
				break;
			case KWD_CPLING: 		/* c! */
				w1 = PULLD();w2 = PULLD();
				memory[w1] = (w2 & 0xFF);
				break;
			case KWD_CAT:			/* c@ */
				w1 = PULLD();w2 = memory[w1];PUSHD(w2);
				break;
			case KWD_NAND:			/* nand */
				w1 = PULLD();w2 = PULLD();
				w1 = (w1 & w2) ^ 0xFFFF;
				PUSHD(w1);
				break;
			case KWD_RGREATER:		/* R> */
				w1 = returnStack[--rSP];PUSHD(w1);
				break;
			case KWD_SYSDOLLARBZ:	/* sys$bz */
				w1 = memory[pc] + (memory[pc+1] << 8);
				pc = (pc + 2) & 0xFFFF;
				w2 = PULLD();if (w2 == 0) pc = (pc + w1) & 0xFFFF;
				break;
			case KWD_SYSDOLLARCIN:	/* sys$cin */
				w1 = HWIGetKey();
				PUSHD(w1);
				break;
			case KWD_SYSDOLLARCOUT:	/* sys$cout */
				w3 = PULLD();w2 = PULLD();w1 = PULLD();
				while ((w3 & 0xFF) != 0) {
					if (w1 >= 0 && w1 < 240) {
						memory[0x1100+w1] = memory[w2 & 0xFFFF];
						w1++;
						w2++;
					}
					w3--;
				}
				break;
			case KWD_SYSDOLLARCURSOR:	/* sys$cursor */
				HWISetCursor(PULLD());
				break;
				
			case KWD_SYSDOLLARLITERAL: /* sys$literal */
				w1 = memory[pc] + (memory[pc+1] << 8);
				pc = (pc + 2) & 0xFFFF;
				PUSHD(w1);
				break;
			case KWD_SYSDOLLARREAD:	/* sys$read */
				break;
			case KWD_SYSDOLLARRESET: /* sys$reset */
				rSP = dSP = 0;
				break;

			case KWD_SYSDOLLARWRITE: /* sys$write */
				break;
		}
	} else {																// Bit 15 set threaded.
		returnStack[rSP++] = pc;											// Push on rstack.
		pc = ((opcode << 1) & 0xFFFE) + CODE_BASE_ADDRESS;
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
