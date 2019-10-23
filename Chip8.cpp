/*
  ==============================================================================
    chip8_oo.cpp
    Created: 2 Aug 2019 8:59:00am
    Author:  jmelton
  ==============================================================================
*/

#include "Chip8.h"

Chip8::Chip8() {
    m_ScreenData.reserve(2048);
    systemReset();
}

Chip8::~Chip8() {
    
}

void Chip8::systemReset() {
	// Initialize System
	m_ProgramCounter = 0x200;						// Set Program Counter to address of first instruction
	m_AddressI = 0;
	memset(&m_Registers, 0, sizeof(m_Registers));
	memset(&m_GameMemory, 0, sizeof(m_GameMemory));
    std::fill(m_ScreenData.begin(), m_ScreenData.end(), 0);
	memset(&keys, 0, sizeof(keys));
	m_Stack.clear();

	// Clear Screen
	opcode00E0();
}

bool Chip8::loadRom(const char* gameDir) {
	// Load rom
	FILE* inputFile;
	//errno_t errorCode = fopen_s(&inputFile, gameDir, "rb");       Works in VS but not in XC???
    inputFile = fopen(gameDir, "rb");
	fread(&m_GameMemory[0x200], 0xFFF, 1, inputFile);
	fclose(inputFile);

	return true;
}

WORD Chip8::fetchNextOpcode() {
	WORD result = 0;

	result = m_GameMemory[m_ProgramCounter];
	result <<= 8;
	result |= m_GameMemory[m_ProgramCounter + 1];

	m_ProgramCounter += 2;

	return result;
}

void Chip8::decodeOpcode(WORD opcode) {
	switch (opcode & 0xF000) {
	case 0x0000: switch (opcode & 0x00FF) {
	case 0x00E0: opcode00E0(); break;
	case 0x00EE: opcode00EE(); break;
	}

	case 0x1000: opcode1NNN(opcode); break;

	case 0x2000: opcode2NNN(opcode); break;

	case 0x3000: opcode3XNN(opcode); break;

	case 0x4000: opcode4XNN(opcode); break;

	case 0x5000: opcode5XY0(opcode); break;

	case 0x6000: opcode6XNN(opcode); break;

	case 0x7000: opcode7XNN(opcode); break;

	case 0x8000: switch (opcode & 0x000F) {
	case 0x0000: opcode8XY0(opcode); break;
	case 0x0001: opcode8XY1(opcode); break;
	case 0x0002: opcode8XY2(opcode); break;
	case 0x0003: opcode8XY3(opcode); break;
	case 0x0004: opcode8XY4(opcode); break;
	case 0x0005: opcode8XY5(opcode); break;
	case 0x0006: opcode8XY6(opcode); break;
	case 0x0007: opcode8XY7(opcode); break;
	case 0x000E: opcode8XYE(opcode); break;
	}

	case 0x9000: opcode9XY0(opcode); break;

	case 0xA000: opcodeANNN(opcode); break;

	case 0xB000: opcodeBNNN(opcode); break;

	case 0xC000: opcodeCXNN(opcode); break;

	case 0xD000: opcodeDXYN(opcode); break;

	case 0xE000: switch (opcode & 0x00FF) {
	case 0x009E: opcodeEX9E(opcode); break;
	case 0x00A1: opcodeEX9E(opcode); break;
	}

	case 0xF000: switch (opcode & 0x00FF) {
	case 0x0007: opcodeFX07(opcode); break;
	case 0x000A: opcodeFX0A(opcode); break;
	case 0x0015: opcodeFX15(opcode); break;
	case 0x0018: opcodeFX18(opcode); break;
	case 0x001E: opcodeFX1E(opcode); break;
	case 0x0029: opcodeFX29(opcode); break;
	case 0x0033: opcodeFX33(opcode); break;
	case 0x0055: opcodeFX55(opcode); break;
	case 0x0065: opcodeFX65(opcode); break;
	}

	default: std::cout << "Error: Invalid opcode." << std::endl; break; // Invalid opcode
	}
}

void Chip8::emulateCycle() {
	decodeOpcode(fetchNextOpcode());
	updateTimers();
}

void Chip8::updateTimers() {
	if (m_SoundTimer > 0) {
		m_SoundTimer--;
	}

	if (m_DelayTimer > 0) {
		m_DelayTimer--;
	}
}

const std::vector<BYTE>& Chip8::getScreenData(){
    return m_ScreenData;
}

// Opcode function Definitions =============================================================================
// =========================================================================================================

inline void Chip8::opcode0NNN(WORD opcode) {
	// Jump to machine code routine at NNN (IGNORED in modern applications)
}

inline void Chip8::opcode00E0() {
	// ***Clear the screen
    std::fill(m_ScreenData.begin(), m_ScreenData.end(), 0);
}

inline void Chip8::opcode00EE() {
	// ***Return from Subroutine
	m_ProgramCounter = m_Stack[m_Stack.size() - 1];	// Set Program Counter to address stored in stack
	m_Stack.pop_back();								// Remove the address from the stack
}

inline void Chip8::opcode1NNN(WORD opcode) {
	m_ProgramCounter = opcode & 0x0FFF;				// ***Jump to address NNN
}

inline void Chip8::opcode2NNN(WORD opcode) {
	// ***Call Subroutine at address NNN
	m_Stack.push_back(m_ProgramCounter);			// Save Program Counter
	m_ProgramCounter = opcode & 0x0FFF;				// Jump to address NNN
}

inline void Chip8::opcode3XNN(WORD opcode) {
	// ***Skip next instruction if VX==NN
	if (m_Registers[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {		// Mask out VX and shift right by 8 bits. Mask out NN and compare to VX.
		m_ProgramCounter += 2;											// Skip next opcode if equal
	}
}

inline void Chip8::opcode4XNN(WORD opcode) {
	// ***Skip next instruction if VX!=NN
	if (m_Registers[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {		// Mask out VX and shift right by 8 bits. Mask out NN and compare to VX.
		m_ProgramCounter += 2;											// Skip next opcode if not equal
	}
}

inline void Chip8::opcode5XY0(WORD opcode) {
	// ***Skip next instruction if VX==VY
	// ***Mask out VX and shift right by 8 bits
	// ***Mask out VY and shift right by 4 bits
	if (m_Registers[(opcode & 0x0F00) >> 8] == m_Registers[(opcode & 0x00F0) >> 4]) 		// Compare and skip next opcode if equal
		m_ProgramCounter += 2;
}

inline void Chip8::opcode6XNN(WORD opcode) {
	// ***Set VX to NN
	m_Registers[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;				// Mask VX and move right by 8 bits.  
}

inline void Chip8::opcode7XNN(WORD opcode) {
	// ***Add NN to VX
	m_Registers[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;				// Mask out VX.  Mask out NN and add to VX.
}

inline void Chip8::opcode8XY0(WORD opcode) {
	// ***Set value of VX to value of VY
	m_Registers[(opcode & 0x0F00) >> 8] = m_Registers[(opcode & 0x00F0) >> 4];		// Mask out VX and VY.  Set VX equal to VY
}

inline void Chip8::opcode8XY1(WORD opcode) {
	// ***Set VX to VX or VY. (Bitwise OR operation)
	int vx = m_Registers[(opcode & 0x0F00) >> 8];
	m_Registers[vx] = m_Registers[vx] | m_Registers[(opcode & 0x00F0) >> 4];
}

inline void Chip8::opcode8XY2(WORD opcode) {
	// ***Set VX to VX and VY. (Bitwise AND operation)
	int vx = m_Registers[(opcode & 0x0F00) >> 8];
	m_Registers[vx] = m_Registers[vx] & m_Registers[(opcode & 0x00F0) >> 4];
}

inline void Chip8::opcode8XY3(WORD opcode) {
	// ***Sets VX to VX xor VY (Bitwise XOR operation)
	int vx = m_Registers[(opcode & 0x0F00) >> 8];
	m_Registers[vx] = m_Registers[vx] ^ m_Registers[(opcode & 0x00F0) >> 4];
}

inline void Chip8::opcode8XY4(WORD opcode) {
	// ***Add the value of register VY to register VX
	// ***Set VF to 01 if a carry occurs
	// ***Set VF to 00 if a carry does not occur
	m_Registers[0xF] = 0;

	int vx = (opcode & 0x0F00) >> 8;
	int vy = (opcode & 0x00F0) >> 4;

	int xval = m_Registers[vx];
	int yval = m_Registers[vy];

	if (yval > (0xFF - xval))
		m_Registers[0xF] = 1;

	m_Registers[vx] += m_Registers[vy];
}

inline void Chip8::opcode8XY5(WORD opcode) {
	// ***Subtract the value of register VY from register VX
	// ***Set VF to 00 if a borrow occurs
	// ***Set VF to 01 if a borrow does not occur
	m_Registers[0xF] = 1;

	int vx = (opcode & 0x0F00) >> 8;
	int vy = (opcode & 0x00F0) >> 4;

	int xval = m_Registers[vx];
	int yval = m_Registers[vy];

	if (yval > xval)
		m_Registers[0xF] = 0;

	m_Registers[vx] = xval - yval;
}

inline void Chip8::opcode8XY6(WORD opcode) {
	// ***Store the least significant bit of VX in VF and then shifts VX to the right by 1
	int vy = m_Registers[(opcode & 0x00F0) >> 4];
	m_Registers[0xF] = m_Registers[vy] & 0b00000001;
	m_Registers[(opcode & 0x0F00) >> 8] = m_Registers[vy] >> 1;
}

inline void Chip8::opcode8XY7(WORD opcode) {
	// ***Set register VX to the value of VY minus VX
	// ***Set VF to 00 if a borrow occurs
	// ***Set VF to 01 if a borrow does not occur
	m_Registers[0xF] = 1;

	int vx = (opcode & 0x0F00) >> 8;
	int vy = (opcode & 0x00F0) >> 4;

	int xval = m_Registers[vx];
	int yval = m_Registers[vy];

	if (yval < xval)
		m_Registers[0xF] = 0;

	m_Registers[vx] = yval - xval;
}

inline void Chip8::opcode8XYE(WORD opcode) {
	// ***Store the value of register VY shifted left one bit in register VX
	// ***Set register VF to the most significant bit prior to the shift
	int vy = m_Registers[(opcode & 0x00F0) >> 4];
	m_Registers[0xF] = m_Registers[vy] & 0b10000000;
	m_Registers[(opcode & 0x0F00) >> 8] = m_Registers[vy] << 1;
}

inline void Chip8::opcode9XY0(WORD opcode) {
	// ***Skip next instruction if VX!=VY
	if (m_Registers[(opcode & 0x0F00) >> 8] != m_Registers[(opcode & 0x00F0) >> 4]) {		// Compare and skip next opcode if not equal
		m_ProgramCounter += 2;
	}
}

inline void Chip8::opcodeANNN(WORD opcode) {
	// ***Store memory address NNN in register I
	m_AddressI = opcode & 0x0FFF;
}

inline void Chip8::opcodeBNNN(WORD opcode) {
	// ***Jumps to the address NNN plus V0
	m_ProgramCounter = (opcode & 0x0FFF) + m_Registers[0];
}

inline void Chip8::opcodeCXNN(WORD opcode) {
	// ***Set VX to a random number with a mask of NN
	srand((unsigned int)time(0));												// Seed RNG
	m_Registers[(opcode & 0x0F00) >> 8] = (rand() % 256) & (opcode & 0x00FF);
}

inline void Chip8::opcodeDXYN(WORD opcode) {
	// *** Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
	int height = (opcode & 0x000F);
	int coordx = m_Registers[(opcode & 0x0F00) >> 8];
	int coordy = m_Registers[(opcode & 0x00F0) >> 4];

	m_Registers[0xF] = 0;

	// Loop for the amount of verical lines that need to be drawn
	for (int yline = 0; yline < height; yline++) {
		BYTE data = m_GameMemory[m_AddressI + yline];
		int xpixelinv = 7;
		int xpixel = 0;

		for (xpixel = 0; xpixel < 8; xpixel++, xpixel--) {
			int mask = 1 << xpixelinv;

			if (data & mask) {
				int x = coordx + xpixel;
				int y = coordy + yline;

                if(m_ScreenData.at((x * 32) + y) )
                    m_Registers[0xF] = 1;               // Collision!
                
                m_ScreenData.at( (x * 32) + y ) ^= 1;
			}
		}
	}
}

inline void Chip8::opcodeEX9E(WORD opcode) {
	// ***Skip the following instruction if the key corresponding 
	// ***to the hex value currently stored in register VX is pressed
	if (keys[m_Registers[(opcode & 0x0F00) >> 8]] != 0)
		m_ProgramCounter += 2;
}

inline void Chip8::opcodeEXA1(WORD opcode) {
	// ***Skip the following instruction if the key corresponding to 
	// ***the hex value currently stored in register VX is not pressed
	if (keys[m_Registers[(opcode & 0x0F00) >> 8]] == 0)
		m_ProgramCounter += 2;
}

inline void Chip8::opcodeFX07(WORD opcode) {
	// ***Store the current value of the delay timer in register VX
	m_Registers[(opcode & 0x0F00) >> 8] = m_DelayTimer;
}

inline void Chip8::opcodeFX0A(WORD opcode) {
	// ***Wait for a keypress and store the result in register VX
	bool keyPressed = false;
	int vx = (opcode & 0x0F00) >> 8;

	for (int i = 0; i < 16; i++) {
		if (keys[i] != 0) {
			keyPressed = true;
			m_Registers[vx] = (BYTE)i;
		}
	}

	if (!keyPressed) {
		m_ProgramCounter -= 2;
	}
}

inline void Chip8::opcodeFX15(WORD opcode) {
	// ***Set the delay timer to the value of register VX
	m_DelayTimer = m_Registers[(opcode & 0x0F00) >> 8];
}

inline void Chip8::opcodeFX18(WORD opcode) {
	// ***Set the sound timer to the value of register VX
	m_SoundTimer = m_Registers[(opcode & 0x0F00) >> 8];
}

inline void Chip8::opcodeFX1E(WORD opcode) {
	// ***Adds VX to I
	m_AddressI += m_Registers[(opcode & 0x0F00) >> 8];
}

inline void Chip8::opcodeFX29(WORD opcode) {
	// ***Set I to memory address of sprite data corresponding to the hex digit stored in register VX
	m_AddressI = m_Registers[(opcode & 0x0F00) >> 8] * 0x5;
}

inline void Chip8::opcodeFX33(WORD opcode) {
	// ***Binary-encoded decimal
	// ***Stores decimal representation of VX to memory
	// ***I = hundreds
	// ***I + 1 = tens
	// ***I + 2 = ones
	int value = m_Registers[(opcode & 0x0F00) >> 8];

	m_GameMemory[m_AddressI] = value / 100;
	m_GameMemory[m_AddressI + 1] = (value / 10) % 10;
	m_GameMemory[m_AddressI + 2] = value % 10;
}

inline void Chip8::opcodeFX55(WORD opcode) {
	// ***Stores V0 through VX (inclusive) in memory starting at address I
	// ***Set I = I + VX + 1 after operation
	int vx = (opcode & 0x0F00) >> 8;

	for (int i = 0; i <= vx; i++) {
		m_GameMemory[m_AddressI + i] = m_Registers[i];
	}

	m_AddressI = m_AddressI + vx + 1;
}

inline void Chip8::opcodeFX65(WORD opcode) {
	// ***Fill registers V0 through VX (inclusive) with the values stored in memory starting at address I
	// ***Set I = I + VX + 1 after operation
	int vx = (opcode & 0x0F00) >> 8;

	for (int i = 0; i <= vx; i++) {
		m_Registers[i] = m_GameMemory[m_AddressI + i];
	}

	m_AddressI = m_AddressI + vx + 1;
}
