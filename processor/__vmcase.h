case KW_AT:
     W1 = PULLD();W1 = memory[W1]+memory[(W1+1) & 0xFFFF] * 256;PUSHD(W1);;break;
case KW_CAT:
     W1 = PULLD();PUSHD(memory[W1]);;break;
case KW_PLING:
     W1 = PULLD();W2 = PULLD(); memory[W1] = W2 & 0xFF;memory[(W1+1) & 0xFFFF] = (W2 >> 8) & 0xFF;break;
case KW_CPLING:
     W1 = PULLD();W2 = PULLD(); memory[W1] = W2 & 0xFF;break;
case KW_GREATERR:
     W1 = PULLD();PUSHR(W1);;break;
case KW_RGREATER:
     W1 = PULLR();PUSHD(W1);;break;
case KW_SEMICOLON:
     pc = PULLR();;break;
case KW_LSQBLITERALRSQB:
     W1 = memory[pc]+memory[pc+1] * 256;pc = pc + 2;PUSHD(W1);;break;
case KW_LSQBBZERORSQB:
     W1 = memory[pc]+memory[pc+1] * 256;pc = pc + 2; W2 = PULLD();if (W2 == 0) pc = (pc + W1) & 0xFFFF;;break;
case KW_LSQBHALTRSQB:
     pc = pc - 2;;break;
case KW_LSQBNOPRSQB:
     ;;break;
case KW_PLUS:
     W1 = PULLD();W2 = PULLD(); PUSHD(W1+W2);;break;
case KW_NAND:
     W1 = PULLD();W2 = PULLD(); PUSHD((W1 & W2) ^ 0xFFFF);;break;
case KW_2SLASH:
     W1 = PULLD(); W1 = (W1 >> 1) | (W1 & 0x8000); PUSHD(W1);;break;
case KW_0EQUALS:
     W1 = PULLD(); W1 = (W1 == 0) ? 0xFFFF:0x0000; PUSHD(W1);;break;
case KW_LSQBTEMPRSQB:
     PUSHD(SYS_ADDR_TEMP);;break;
case KW_LSQBCODEBASERSQB:
     PUSHD(SYS_ADDR_CODEBASE);;break;
case KW_LSQBDICTIONARYRSQB:
     PUSHD(SYS_ADDR_DICTBASE);;break;
case KW_CURSORPLING:
     HWISetCursor(PULLD());;break;
case KW_SCREENPLING:
     W3 = PULLD();W2 = PULLD();W1 = PULLD(); while ((W3 & 0xFF) != 0) { if (W1 >= 0 && W1 < 240) { memory[0x1100+W1] = memory[W2 & 0xFFFF]; W1++; W2++; } W3--; };break;
case KW_KEYBOARDAT:
     W1 = HWIGetKey(); PUSHD(W1);;break;
case KW_BLOCKREADAT:
     ;;break;
case KW_LSQBSTACKRESETRSQB:
     rSP = dSP = 0;;break;
case KW_BLOCKWRITEPLING:
     ;;break;
