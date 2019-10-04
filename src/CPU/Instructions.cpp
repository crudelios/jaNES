// Instructions.cpp
// Definições de instruções

#define INSTRUCTIONS_CPP

#include "Instructions.h"
#include "../PPU/PPU.h"

// Add with Carry
void CPU::Instructions::ADC(unsigned char * byte)
{
	register int result = Registers::A + *byte + Registers::P::C;

	// Se o resultado for > 255, colocar carry
	Registers::P::C = result >> 8;
	result &= 0xFF;

	// Flags
	Registers::P::N = result >> 7;
	Registers::P::V = ((*byte ^ result) & (Registers::A ^ result)) >> 7;
	Registers::P::Z = (result == 0);

	Registers::A = result;
}

// Add with Carry
void CPU::Instructions::ADC(unsigned short addr)
{
	register int val = Memory::Read(addr);
	register int result = Registers::A + val + Registers::P::C;

	// Se o resultado for > 255, colocar carry
	Registers::P::C = result >> 8;
	result &= 0xFF;

	// Flags
	Registers::P::N = result >> 7;
	Registers::P::V = ((val ^ result) & (Registers::A ^ result)) >> 7;
	Registers::P::Z = (result == 0);

	Registers::A = result;
}

// Shift byte left
void CPU::Instructions::ASL(unsigned short addr)
{
	unsigned char val = Memory::Read(addr);

	// Dummy write!
	Memory::Write(addr, val);

	Registers::P::C = val >> 7;

	val <<= 1;

	// Real write!
	Memory::Write(addr, val);

	// Flags
	Registers::P::Z = (val == 0);
	Registers::P::N = val >> 7;

	if(doOAMWrite)
		PPU::OAMWrite(val);
}

// Decrement memory by one and compare with Accumulator
void CPU::Instructions::DCP(unsigned short addr)
{
	register unsigned char val = Memory::Read(addr);
	Memory::Write(addr, val);

	--val;

	Memory::Write(addr, val);

	register int result = Registers::A - val;

	// Flags
	Registers::P::C = ((result >> 8) & 1) ^ 1;
	result &= 0xFF;
	Registers::P::N = result >> 7;
	Registers::P::Z = (result == 0);

	if(doOAMWrite)
		PPU::OAMWrite(val);
}

// Increment memory by one, then subtract Accumulator with memory
void CPU::Instructions::ISB(unsigned char * byte)
{
	++(*byte);

	register int result = Registers::A - *byte - (Registers::P::C ^ 1);

	// Flags
	Registers::P::C = ((result >> 8) & 1) ^ 1;
	result &= 0xFF;
	Registers::P::N = result >> 7;
	Registers::P::V = ((*byte ^ Registers::A) & (Registers::A ^ result)) >> 7;
	Registers::P::Z = (result == 0);

	Registers::A = result;	
}

// Increment memory by one, then subtract Accumulator with memory
void CPU::Instructions::ISB(unsigned short addr)
{
	register unsigned char val = Memory::Read(addr);
	Memory::Write(addr, val);
			
	++val;
	Memory::Write(addr, val);

	register int result = Registers::A - val - (Registers::P::C ^ 1);

	// Flags
	Registers::P::C = ((result >> 8) & 1) ^ 1;
	result &= 0xFF;
	Registers::P::N = result >> 7;
	Registers::P::V = ((val ^ Registers::A) & (Registers::A ^ result)) >> 7;
	Registers::P::Z = (result == 0);

	Registers::A = result;

	if(doOAMWrite)
		PPU::OAMWrite(val);
}

// Shift byte right
void CPU::Instructions::LSR(unsigned short addr)
{
	register unsigned char val = Memory::Read(addr);
	Memory::Write(addr, val);

	Registers::P::C = val & 1;

	val >>= 1;

	Memory::Write(addr, val);

	// Flags
	Registers::P::Z = (val == 0);
	Registers::P::N = 0;

	if(doOAMWrite)
		PPU::OAMWrite(val);
}

// Rotate byte left then and with accumulator
void CPU::Instructions::RLA(unsigned short addr)
{
	register unsigned char val = Memory::Read(addr);
	Memory::Write(addr, val);

	unsigned char carry = val >> 7;

	val <<= 1;
	val |= Registers::P::C;

	Memory::Write(addr, val);

	// Flags

	Registers::A  &= val;

	// Flags
	Registers::P::C = carry;
	Registers::P::Z = (Registers::A == 0);
	Registers::P::N = Registers::A >> 7;

	if(doOAMWrite)
		PPU::OAMWrite(val);
}

// Rotate byte left
void CPU::Instructions::ROL(unsigned short addr)
{
	register unsigned char val = Memory::Read(addr);

	Memory::Write(addr, val);

	unsigned char carry = val >> 7;

	val <<= 1;
	val |= Registers::P::C;

	// Flags
	Registers::P::C = carry;
	Registers::P::Z = (val == 0);
	Registers::P::N = (val >> 7);

	Memory::Write(addr, val);

	if(doOAMWrite)
		PPU::OAMWrite(val);
}

// Rotate byte right
void CPU::Instructions::ROR(unsigned short addr)
{
	register unsigned char val = Memory::Read(addr);
	Memory::Write(addr, val);

	unsigned char carry = val & 1;

	val = (val >> 1) | (Registers::P::C << 7);

	// Flags
	Registers::P::C = carry;
	Registers::P::Z = (val == 0);
	Registers::P::N = (val >> 7);

	Memory::Write(addr, val);

	if(doOAMWrite)
		PPU::OAMWrite(val);
}

// Rotate byte right, then add result with accumulator
void CPU::Instructions::RRA(unsigned char * byte)
{
	register unsigned char val = * byte;

	int carry = val & 1;
	
	val = (val >> 1) | (Registers::P::C << 7);

	*byte = val;

	register int result = Registers::A + val + carry;

	// Se o resultado for > 255, colocar carry
	Registers::P::C = result >> 8;
	result &= 0xFF;

	// Flags
	Registers::P::N = result >> 7;
	Registers::P::V = ((val ^ result) & (Registers::A ^ result)) >> 7;
	Registers::P::Z = (result == 0);

	Registers::A = result;
}

// Rotate byte right, then add result with accumulator
void CPU::Instructions::RRA(unsigned short addr)
{
	register unsigned char val = Memory::Read(addr);
	Memory::Write(addr, val);
			
	int carry = val & 1;

	val = (val >> 1) | (Registers::P::C << 7);

	Memory::Write(addr, val);

	register int result = Registers::A + val + carry;

	// Se o resultado for > 255, colocar carry
	Registers::P::C = result >> 8;
	result &= 0xFF;

	// Flags
	Registers::P::N = result >> 7;
	Registers::P::V = ((val ^ result) & (Registers::A ^ result)) >> 7;
	Registers::P::Z = (result == 0);

	Registers::A = result;

	if(doOAMWrite)
		PPU::OAMWrite(val);
}

// Subtract with borrow
void CPU::Instructions::SBX(unsigned char * byte)
{
	register int result = (Registers::A & Registers::X) - *byte;

	// Flags
	Registers::P::C = ((result >> 8) & 1) ^ 1;
	result &= 0xFF;
	Registers::P::N = result >> 7;
	Registers::P::Z = (result == 0);

	Registers::X = result;
}

// Subtract with borrow
void CPU::Instructions::SBC(unsigned char * byte)
{
	register int result = Registers::A - *byte - (Registers::P::C ^ 1);

	// Flags
	Registers::P::C = ((result >> 8) & 1) ^ 1;
	result &= 0xFF;
	Registers::P::N = result >> 7;
	Registers::P::V = ((*byte ^ Registers::A) & (Registers::A ^ result)) >> 7;
	Registers::P::Z = (result == 0);

	Registers::A = result;
}

// Subtract with borrow
void CPU::Instructions::SBC(unsigned short addr)
{
	register int val = Memory::Read(addr);
	register int result = Registers::A - val - (Registers::P::C ^ 1);

	// Flags
	Registers::P::C = ((result >> 8) & 1) ^ 1;
	result &= 0xFF;
	Registers::P::N = result >> 7;
	Registers::P::V = ((val ^ Registers::A) & (Registers::A ^ result)) >> 7;
	Registers::P::Z = (result == 0);

	Registers::A = result;
}

// Shift left from memory, OR with Accumulator
void CPU::Instructions::SLO(unsigned short addr)
{
	register unsigned char val = Memory::Read(addr);
	Memory::Write(addr, val);

	Registers::P::C = val >> 7;
	val <<= 1;

	Memory::Write(addr, val);
			
	Registers::A |= val;

	Registers::P::N = Registers::A >> 7;
	Registers::P::Z = (Registers::A == 0);

	if(doOAMWrite)
		PPU::OAMWrite(val);
}

// Shift byte right in memory, then EOR with Accumulator
void CPU::Instructions::SRE(unsigned short addr)
{
	register unsigned char val = Memory::Read(addr);
	Memory::Write(addr, val);

	Registers::P::C = val & 1;

	val >>= 1;

	Memory::Write(addr, val);

	Registers::A ^= val;

	Registers::P::N = Registers::A >> 7;
	Registers::P::Z = (Registers::A == 0);

	if(doOAMWrite)
		PPU::OAMWrite(val);
}