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
#include "__core.h"

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
	sprintf(fnBuffer,"call $%04x",address);
	WORD16 dTop = CPURead(0x100)+CPURead(0x101)*256 + DICT_BASE_ADDRESS;
	BYTE8 link;
	do {
		WORD16 addr = ((CPURead(dTop+1) + CPURead(dTop+2) * 256) << 1) & 0xFFFF;
		addr = addr + CODE_BASE_ADDRESS;
		//printf("%x %x\n",addr,address);
		if (addr == address && (CPURead(dTop+3) & 0x80) == 0) {
			BYTE8 len = CPURead(dTop+3) & 31;
			for (BYTE8 i = 0;i < len;i++) {
				BYTE8 ch = CPURead(dTop+4+i);
				ch = (ch ^ 32) + 32;
				if (ch >= 'A' && ch <= 'Z') ch += 32;
				fnBuffer[i] = ch;
			}
			fnBuffer[len] = '\0';
		}
		link = CPURead(dTop);
		dTop = dTop - link;
	} while (link != 0); 
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

	GFXSetCharacterSize(32,25);
	frameCount++;

	CPUSTATUS *s = CPUGetStatus();													// Get the CPU Status

	const char *labels2[] = { "PC","BP","CY",NULL };
	n = 0;
	while (labels2[n] != NULL) {
		GFXString(GRID(18,n*3),labels2[n],GRIDSIZE,DBGC_ADDRESS,-1);
		n++;
	}
	GFXNumber(GRID(18,1),s->pc,16,4,GRIDSIZE,DBGC_DATA,-1);
	GFXNumber(GRID(18,4),address[3],16,4,GRIDSIZE,DBGC_DATA,-1);
	GFXNumber(GRID(18,7),s->cycles,16,4,GRIDSIZE,DBGC_DATA,-1);

	n = address[1];																	// Dump memory.
	for (int row = 17;row < 24;row++) {
		GFXNumber(GRID(2,row),n & 0xFFFF,16,4,GRIDSIZE,DBGC_ADDRESS,-1);			// Head of line
		GFXCharacter(GRID(6,row),':',GRIDSIZE,DBGC_HIGHLIGHT,-1);
		for (int col = 0;col < 8;col++) {											// Data on line
			GFXNumber(GRID(7+col*3,row),CPURead(n & 0xFFFF),16,2,GRIDSIZE,DBGC_DATA,-1);
			n++;
		}
	}
																					// Output text labels.																					// Macros to simplify dump drawing.
	GFXString(GRID(23,0),"Data",GRIDSIZE,DBGC_ADDRESS,-1);							
	GFXString(GRID(28,0),"Ret",GRIDSIZE,DBGC_ADDRESS,-1);							
	for (n = 0;n < s->dsp;n++) {
		GFXNumber(GRID(23,n+1),s->dataStack[n],16,4,GRIDSIZE,DBGC_DATA,-1);
	}
	for (n = 0;n < s->rsp;n++) {
		GFXNumber(GRID(28,n+1),s->returnStack[n],16,4,GRIDSIZE,DBGC_DATA,-1);
	}
	n = address[0] & 0xFFFE; 														// Dump code.
	int nextData = 0;	
	for (int row = 0;row < 15;row++) {
		int col = DBGC_CORE;
		int isPC = (n & 0xFFFF) == (s->pc);											// Check for breakpoint and being at PC
		int isBrk = ((n & 0xFFFF) == address[3]);
		GFXNumber(GRID(0,row),n & 0xFFFF,16,4,GRIDSIZE,isPC ? DBGC_HIGHLIGHT : DBGC_ADDRESS,isBrk ? 0xF00 : -1);
		int opcode = CPURead(n)+CPURead(n+1)*256;
		if (nextData == 0) {
			if ((opcode & 0x8000) == 0) {
				sprintf(buffer,"core %04x",opcode & 0x7FFF);
				if (opcode < KWDCOUNT) {
					strcpy(buffer,_mnemonics[opcode]);
				}
				if (opcode == KWD_SYSDOLLARBZ || opcode == KWD_SYSDOLLARLITERAL) {
					nextData = 1;
				}
			} else {
				strcpy(buffer,__DBGXGetFunctionName(((opcode << 1) & 0xFFFF)+0x1200));
				col = DBGC_DATA;
			}
		} else {
			sprintf(buffer,"%04x",opcode);	
			col = DBGC_DATA;
			nextData = 0;
		}
		n = n + 2;
		buffer[12] = '\0';
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
