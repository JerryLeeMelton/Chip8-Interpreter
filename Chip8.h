/*
  ==============================================================================

    chip8_oo.h
    Created: 2 Aug 2019 8:59:00am
    Author:  jmelton

  ==============================================================================
*/
#pragma once

#include <vector>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <stdio.h>

#ifndef BYTE
#define BYTE unsigned char
#endif

typedef unsigned short int WORD;

class Chip8 {
public:
    Chip8();
    ~Chip8();
    
	void systemReset();
	bool loadRom(const char* gameDir);
	void emulateCycle();
	void updateTimers();
    
    // Public Data ==================================================================
    const std::vector<BYTE>& getScreenData();

private:
	// Data =========================================================================
	BYTE m_GameMemory[0xFFF];		// 0xFFF (or) bytes of memory
	BYTE m_Registers[16];			// 16 Registers, 1 byte each
	WORD m_AddressI;				// The 16-bit (2-byte) address register "I"
	WORD m_ProgramCounter;			// The 16-bit program counter
	BYTE m_DelayTimer;				// Timer used for event timing in games
	BYTE m_SoundTimer;				// Timer used for sound effects.  Beeps when value is non-zero
	std::vector<BYTE> m_Stack;		// The 16-bit stack
	BYTE keys[16];                  // Array representing the first player's hexadecimal input keyboard
    std::vector<BYTE> m_ScreenData; // Vector used for storing data about the display (2048 pixels)

	// Functions ====================================================================
	WORD fetchNextOpcode();
	void decodeOpcode(WORD opcode);

	// Opcodes ======================================================================
	inline void opcode0NNN(WORD opcode);       inline void opcode00E0();

	inline void opcode00EE();                  inline void opcode1NNN(WORD opcode);

	inline void opcode2NNN(WORD opcode);       inline void opcode3XNN(WORD opcode);

	inline void opcode4XNN(WORD opcode);       inline void opcode5XY0(WORD opcode);

	inline void opcode6XNN(WORD opcode);       inline void opcode7XNN(WORD opcode);

	inline void opcode8XY0(WORD opcode);       inline void opcode8XY1(WORD opcode);

	inline void opcode8XY2(WORD opcode);       inline void opcode8XY3(WORD opcode);

	inline void opcode8XY4(WORD opcode);       inline void opcode8XY5(WORD opcode);

	inline void opcode8XY6(WORD opcode);       inline void opcode8XY7(WORD opcode);

	inline void opcode8XYE(WORD opcode);       inline void opcode9XY0(WORD opcode);

	inline void opcodeANNN(WORD opcode);       inline void opcodeBNNN(WORD opcode);

	inline void opcodeCXNN(WORD opcode);       inline void opcodeDXYN(WORD opcode);

	inline void opcodeEX9E(WORD opcode);       inline void opcodeEXA1(WORD opcode);

	inline void opcodeFX07(WORD opcode);       inline void opcodeFX0A(WORD opcode);

	inline void opcodeFX15(WORD opcode);       inline void opcodeFX18(WORD opcode);

	inline void opcodeFX1E(WORD opcode);       inline void opcodeFX29(WORD opcode);

	inline void opcodeFX33(WORD opcode);       inline void opcodeFX55(WORD opcode);

	inline void opcodeFX65(WORD opcode);
};
