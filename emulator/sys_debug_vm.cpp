// *******************************************************************************************************************************
// *******************************************************************************************************************************
//
//		Name:		sys_debug_8008.cpp
//		Purpose:	Debugger Code (System Dependent)
//		Created:	29th June 2016
//		Author:		Paul Robson (paul@robsons.org.uk)
//
// *******************************************************************************************************************************
// *******************************************************************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfx.h"
#include "sys_processor.h"
#include "sys_debug_system.h"
#include "debugger.h"

#define DBGC_ADDRESS 	(0x0F0)														// Colour scheme.
#define DBGC_DATA 		(0x0FF)														// (Background is in main.c)
#define DBGC_HIGHLIGHT 	(0xFF0)
#define DBGC_CORE 		(0xF80)

#define MNEMONICS
#include "__vminclude.h"

static BYTE8 __font[] = {
#include "__font7x9_mcmfont.h"
};

// *******************************************************************************************************************************
//												Reset the 8008
// *******************************************************************************************************************************

void DBGXReset(void) {
	CPUReset();
}

static char fnBuffer[33];

static char *__DBGXGetFunctionName(WORD16 address) {
	sprintf(fnBuffer,"call $%04x",address+SYS_ADDR_CODEBASE);
	WORD16 dPtr = SYS_ADDR_DICTBASE;
	while (CPURead(dPtr) != 0) {
		WORD16 addr = CPURead(dPtr+1) + CPURead(dPtr+2) * 256;
		if (addr == address && (CPURead(dPtr+3) & 0x80) == 0) {
			BYTE8 len = CPURead(dPtr+3) & 31;
			for (BYTE8 i = 0;i < len;i++) {
				BYTE8 ch = CPURead(dPtr+4+i);
				ch = (ch ^ 32) + 32;
				if (ch >= 'A' && ch <= 'Z') ch += 32;
				fnBuffer[i] = ch;
			}
			fnBuffer[len] = '\0';
		}
		dPtr = dPtr + CPURead(dPtr);
	} 
	return fnBuffer;
}

// *******************************************************************************************************************************
//											This renders the debug screen
// *******************************************************************************************************************************

static int _colours[4] = { 0xFF0,0x0F0,0xF00,0xFFF };
static WORD16 frameCount = 0;

void DBGXRender(int *address,int showDisplay) {
	int n;
	char buffer[32];

	GFXSetCharacterSize(40,25);
	frameCount++;

	CPUSTATUS *s = CPUGetStatus();													// Get the CPU Status

	const char *labels2[] = { "PC","BP","CY",NULL };
	n = 0;
	while (labels2[n] != NULL) {
		GFXString(GRID(26,n*3),labels2[n],GRIDSIZE,DBGC_ADDRESS,-1);
		n++;
	}
	GFXNumber(GRID(26,1),s->pc,16,4,GRIDSIZE,DBGC_DATA,-1);
	GFXNumber(GRID(26,4),address[3],16,4,GRIDSIZE,DBGC_DATA,-1);
	GFXNumber(GRID(26,7),s->cycles,16,4,GRIDSIZE,DBGC_DATA,-1);

	n = address[1];																	// Dump memory.
	for (int row = 17;row < 24;row++) {
		GFXNumber(GRID(6,row),n & 0xFFFF,16,4,GRIDSIZE,DBGC_ADDRESS,-1);			// Head of line
		GFXCharacter(GRID(10,row),':',GRIDSIZE,DBGC_HIGHLIGHT,-1);
		for (int col = 0;col < 8;col++) {											// Data on line
			GFXNumber(GRID(11+col*3,row),CPURead(n & 0xFFFF),16,2,GRIDSIZE,DBGC_DATA,-1);
			n++;
		}
	}
																					// Output text labels.																					// Macros to simplify dump drawing.
	GFXString(GRID(31,0),"Data",GRIDSIZE,DBGC_ADDRESS,-1);							
	GFXString(GRID(36,0),"Ret",GRIDSIZE,DBGC_ADDRESS,-1);							
	for (n = 0;n < s->dsp;n++) {
		GFXNumber(GRID(31,n+1),s->dataStack[n],16,4,GRIDSIZE,DBGC_DATA,-1);
	}
	for (n = 0;n < s->rsp;n++) {
		GFXNumber(GRID(36,n+1),s->returnStack[n],16,4,GRIDSIZE,DBGC_DATA,-1);
	}
	n = address[0]; 														// Dump code.
	for (int row = 0;row < 15;row++) {
		int col = DBGC_CORE;
		int isPC = (n & 0xFFFF) == (s->pc);											// Check for breakpoint and being at PC
		int isBrk = ((n & 0xFFFF) == address[3]);
		GFXNumber(GRID(0,row),n & 0xFFFF,16,4,GRIDSIZE,isPC ? DBGC_HIGHLIGHT : DBGC_ADDRESS,isBrk ? 0xF00 : -1);
		int opcode = CPURead(n);
		n++;
		if ((opcode & 0x80) == 0) {
			sprintf(buffer,"core %02x",opcode & 0x7F);
			if (opcode < KW_COUNT) {
				strcpy(buffer,_mnemonics[opcode]);
			}
			if (opcode == KW_LSQBBZERORSQB || opcode == KW_LSQBLITERALRSQB) {
				int param = CPURead(n)+CPURead(n+1)*256;
				n = n + 2;
				sprintf(buffer+strlen(buffer)," %04x",param);
			}
		} else {
			opcode = (opcode << 8) | CPURead(n);
			opcode = (opcode << 1) & 0xFFFF;
			n++;
			strcpy(buffer,__DBGXGetFunctionName(opcode));
			col = DBGC_DATA;
		}
		GFXString(GRID(5,row),buffer,GRIDSIZE,isPC ? DBGC_HIGHLIGHT : col,-1);
	}

	if (showDisplay == 0) return;
	SDL_Rect rc,rc2,rc3;
	rc.w = 5;rc.h = 3;
	rc2.w = 20 * 8 * rc.w;rc2.h = 12 * 16 * rc.h;
	rc2.x = WIN_WIDTH/2-rc2.w/2;rc2.y = WIN_HEIGHT-rc2.h-64;
	rc3 = rc2;rc3.x -= 20;rc3.y -= 20;rc3.w+=40;rc3.h += 40;
	GFXRectangle(&rc3,0x000);

	BYTE8 cursor = HWIGetCursor();

	for (int x = 0;x < 20;x++) {
		for (int y = 0;y < 12;y++) {
			int ch = CPURead(0x1100+x+y*20);
			int cl = _colours[ch >> 6];
			BYTE8 isCursor = (cursor == x+y*20) && (frameCount & 0x20) == 0;
			ch = ((ch & 0x3F) ^ 0x20) + 0x20;
			int offset = 0;
			if (ch >= 'A' && ch <= 'Z') ch += 0x20;
			if (ch == 'p' || ch == 'q' || ch == 'j' || ch == 'g'|| ch == 'y') offset = 3;
			for (int x1 = 0;x1 < 8;x1++) {
				for (int y1 = 0;y1 < 9;y1++) {
					rc.x = rc2.x + (x * 8 + x1) * rc.w;
					rc.y = rc2.y + (y * 16 + y1 + offset) * rc.h;
					int b = __font[ch*9+y1];
					if (isCursor) b = 0xFE;
					if (b & (0x80 >> x1)) GFXRectangle(&rc,cl);
				}
			}
		}
	}
}	
