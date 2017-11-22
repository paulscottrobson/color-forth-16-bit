#ifndef __VMI
#define __VMI
#define SYS_ADDR_TEMP (0x1000)
#define SYS_ADDR_CODEBASE (0x1200)
#define SYS_ADDR_DICTBASE (0x0100)
#define KW_AT (0)
#define KW_CAT (1)
#define KW_PLING (2)
#define KW_CPLING (3)
#define KW_GREATERR (4)
#define KW_RGREATER (5)
#define KW_SEMICOLON (6)
#define KW_LSQBLITERALRSQB (7)
#define KW_LSQBBZERORSQB (8)
#define KW_LSQBHALTRSQB (9)
#define KW_LSQBNOPRSQB (10)
#define KW_PLUS (11)
#define KW_NAND (12)
#define KW_2SLASH (13)
#define KW_0EQUALS (14)
#define KW_LSQBTEMPRSQB (15)
#define KW_LSQBCODEBASERSQB (16)
#define KW_LSQBDICTIONARYRSQB (17)
#define KW_CURSORPLING (18)
#define KW_SCREENPLING (19)
#define KW_KEYBOARDAT (20)
#define KW_BLOCKREADAT (21)
#define KW_LSQBSTACKRESETRSQB (22)
#define KW_BLOCKWRITEPLING (23)
#define KW_COUNT (24)

#ifdef MNEMONICS
static const char *_mnemonics[] = { 
"@","c@","!","c!",">r","r>",";","[literal]","[bzero]","[halt]","[nop]","+","nand","2/","0=","[temp]","[codebase]","[dictionary]","cursor!","screen!","keyboard@","blockread@","[stackreset]","blockwrite!"
};
#endif

#endif
#endif
