case KW_AT:
     W1 = PULLD();W1 = memory[W1]+memory[(W1+1) & 0xFFFF] * 256;PUSHD(W1);;break;
case KW_CAT:
     W1 = PULLD();PUSHD(memory[W1]);;break;
case KW_PLING:
     W1 = PULLD();W2 = PULLD(); memory[W1] = W2 & 0xFF;memory[(W1+1) & 0xFFFF] = (W2 >> 1) & 0xFF;break;
case KW_CPLING:
     W1 = PULLD();W2 = PULLD(); memory[W1] = W2 & 0xFF;break;
case KW_GREATERR:
     W1 = PULLD();PUSHR(W1);;break;
case KW_RGREATER:
     W1 = PULLR();PUSHD(W1);;break;
case KW_SEMICOLON:
     PC = PULLR();;break;
case KW_LSQBLITERALRSQB:
     W1 = memory[PC]+memory[PC+1] * 256;PC = PC + 2;PUSHD(W1);;break;
case KW_LSQBBZERORSQB:
     W1 = memory[PC]+memory[PC+1] * 256;PC = PC + 2 W2 = PULLD();if (W2 == 0) PC = (PC + W1) & 0xFFFF;;break;
case KW_LSQBHALTRSQB:
     PC = PC - 2;;break;
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
     w3 = PULLD();w2 = PULLD();w1 = PULLD(); while ((w3 & 0xFF) != 0) { if (w1 >= 0 && w1 < 240) { memory[0x1100+w1] = memory[w2 & 0xFFFF]; w1++; w2++; } w3--; };break;
case KW_KEYBOARDAT:
     w1 = HWIGetKey(); PUSHD(w1);;break;
case KW_BLOCKREADAT:
     ;;break;
case KW_LSQBSTACKRESETRSQB:
     RSP = DSP = 0;;break;
case KW_BLOCKWRITEPLING:
     ;;break;
