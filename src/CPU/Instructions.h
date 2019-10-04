// Instructions.h
// Instruções do CPU

#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "Memory.h"
#include "Interrupts.h"

#include "../PPU/PPU.h"
#include "../APU/APU.h"

#ifndef INSTRUCTIONS_CPP
#define EXTERNAL extern
#else
#define EXTERNAL
#endif


namespace CPU
{
	namespace Instructions
	{
		// AND byte with accumulator
		inline void ANC(unsigned char * byte)
		{
			Registers::A  &= *byte;

			// Flags
			Registers::P::Z = (Registers::A == 0);
			Registers::P::N = Registers::A >> 7;
			Registers::P::C = Registers::A >> 7;
		}

		// Add with carry
		void ADC(unsigned char * byte);
		void ADC(unsigned short addr);

		// And memory with accumulator	
		inline void AND(unsigned char * byte)
		{
			Registers::A  &= *byte;

			// Flags
			Registers::P::Z = (Registers::A == 0);
			Registers::P::N = Registers::A >> 7;
		}

		// And memory with accumulator	
		inline void AND(unsigned short addr)
		{
			Registers::A  &= Memory::Read(addr);

			// Flags
			Registers::P::Z = (Registers::A == 0);
			Registers::P::N = Registers::A >> 7;
		}

		// AND byte with accumulator and do strange things
		inline void ARR(unsigned char * byte)
		{
			register unsigned char val = Registers::A & *byte;

			val = (val >> 1) | (Registers::P::C << 7);

			// Flags
			Registers::P::Z = (val == 0);
			Registers::P::N = val >> 7;
			Registers::P::C = (val >> 6) & 1;
			Registers::P::V = ((val >> 5) & 1) ^ Registers::P::C;

			Registers::A = val;
		}

		// Shift byte left
		void ASL(unsigned short addr);
		inline void ASL(unsigned char * byte)
		{
			Registers::P::C = (*byte) >> 7;

			(*byte) <<= 1;

			// Flags
			Registers::P::Z = (*byte == 0);
			Registers::P::N = (*byte) >> 7;
		}

		// Shift byte right, and with accumulator
		inline void ASR(unsigned char * byte)
		{
			Registers::A &= *byte;
			Registers::P::C = Registers::A & 1;
			Registers::A >>= 1;

			// Flags
			Registers::P::Z = (Registers::A == 0);
			Registers::P::N = Registers::A >> 7;
		}

		// Test bits
		inline void BIT(unsigned char * byte)
		{
			register int val = *byte;
			Registers::P::N = val >> 7;
			Registers::P::V = (val >> 6) & 1;
			Registers::P::Z = ((Registers::A & val) == 0);
		}

		// Test bits
		inline void BIT(unsigned short addr)
		{
			register int val = Memory::Read(addr);
			Registers::P::N = val >> 7;
			Registers::P::V = (val >> 6) & 1;
			Registers::P::Z = ((Registers::A & val) == 0);
		}

		// Compare memory and Accumulator
		inline void CMP(unsigned char * byte)
		{
			register int result = Registers::A - *byte;

			// Flags
			Registers::P::C = ((result >> 8) & 1) ^ 1;
			result &= 0xFF;
			Registers::P::N = result >> 7;
			Registers::P::Z = (result == 0);
		}

		// Compare memory and Accumulator
		inline void CMP(unsigned short addr)
		{
			register int result = Registers::A - Memory::Read(addr);

			// Flags
			Registers::P::C = ((result >> 8) & 1) ^ 1;
			result &= 0xFF;
			Registers::P::N = result >> 7;
			Registers::P::Z = (result == 0);
		}

		// Compare memory and Index X
		inline void CPX(unsigned char * byte)
		{
			register int result = Registers::X - *byte;

			// Flags
			Registers::P::C = ((result >> 8) & 1) ^ 1;
			result &= 0xFF;
			Registers::P::N = result >> 7;
			Registers::P::Z = (result == 0);
		}

		// Compare memory and Index X
		inline void CPX(unsigned short addr)
		{
			register int result = Registers::X - Memory::Read(addr);

			// Flags
			Registers::P::C = ((result >> 8) & 1) ^ 1;
			result &= 0xFF;
			Registers::P::N = result >> 7;
			Registers::P::Z = (result == 0);
		}

		// Compare memory and Index Y
		inline void CPY(unsigned char * byte)
		{
			register int result = Registers::Y - *byte;

			// Flags
			Registers::P::C = ((result >> 8) & 1) ^ 1;
			result &= 0xFF;
			Registers::P::N = result >> 7;
			Registers::P::Z = (result == 0);
		}

		// Compare memory and Index Y
		inline void CPY(unsigned short addr)
		{
			register int result = Registers::Y - Memory::Read(addr);

			// Flags
			Registers::P::C = ((result >> 8) & 1) ^ 1;
			result &= 0xFF;
			Registers::P::N = result >> 7;
			Registers::P::Z = (result == 0);
		}

		// Decrement memory by one and compare with Accumulator
		void DCP(unsigned short addr);
		inline void DCP(unsigned char * byte)
		{
			--(*byte);

			register int result = Registers::A - *byte;

			// Flags
			Registers::P::C = ((result >> 8) & 1) ^ 1;
			result &= 0xFF;
			Registers::P::N = result >> 7;
			Registers::P::Z = (result == 0);
		}

		// Decrement memory by one
		inline void DEC(unsigned char * byte)
		{
			(*byte)--;

			// Flags
			Registers::P::N = (*byte) >> 7;
			Registers::P::Z = (*byte == 0);
		}

		// Decrement by one
		inline void DEC(unsigned short addr)
		{
			register unsigned char val = Memory::Read(addr);
			Memory::Write(addr, val);

			--val;

			Memory::Write(addr, val);
			
			// Flags
			Registers::P::N = val >> 7;
			Registers::P::Z = (val == 0);

			if(doOAMWrite)
				PPU::OAMWrite(val);
		}

		// Decrement X by one
		inline void DEX()
		{
			Registers::X = (Registers::X - 1) & 0xFF;

			// Flags
			Registers::P::N = Registers::X >> 7;
			Registers::P::Z = (Registers::X == 0);
		}

		// Decrement Y by one
		inline void DEY()
		{
			Registers::Y = (Registers::Y - 1) & 0xFF;

			// Flags
			Registers::P::N = Registers::Y >> 7;
			Registers::P::Z = (Registers::Y == 0);
		}

		// EOR memory with accumulator
		inline void EOR(unsigned char * byte)
		{
			Registers::A ^= *byte;

			Registers::P::N = Registers::A >> 7;
			Registers::P::Z = (Registers::A == 0);
		}

		// EOR memory with accumulator
		inline void EOR(unsigned short addr)
		{
			Registers::A ^= Memory::Read(addr);

			Registers::P::N = Registers::A >> 7;
			Registers::P::Z = (Registers::A == 0);
		}

		// Increment by one
		inline void INC(unsigned char * byte)
		{
			++(*byte);

			// Flags
			Registers::P::N = (*byte) >> 7;
			Registers::P::Z = (*byte == 0);
		}

		// Increment by one
		inline void INC(unsigned short addr)
		{
			register unsigned char val = Memory::Read(addr);
			Memory::Write(addr, val);		
				
			++val;

			Memory::Write(addr, val);

			// Flags
			Registers::P::N = val >> 7;
			Registers::P::Z = (val == 0);

			if(doOAMWrite)
				PPU::OAMWrite(val);
		}

		// Increment X by one
		inline void INX()
		{
			Registers::X = (Registers::X + 1) & 0xFF;

			// Flags
			Registers::P::N = Registers::X >> 7;
			Registers::P::Z = (Registers::X == 0);
		}

		// Increment Y by one
		inline void INY()
		{
			Registers::Y = (Registers::Y + 1) & 0xFF;

			// Flags
			Registers::P::N = Registers::Y >> 7;
			Registers::P::Z = (Registers::Y == 0);
		}

		// Increment memory by one, then subtract Accumulator with memory
		void ISB(unsigned char * byte);
		void ISB(unsigned short addr);

		// And memory with SP, save to A, X and SP
		inline void LAS(unsigned short addr)
		{
			Registers::A = Registers::X = Registers::SP = Memory::Read(addr) & Registers::SP;

			// Flags
			Registers::P::N = Registers::A >> 7;
			Registers::P::Z = (Registers::A == 0);
		}

		// Load accumulator and X register with memory
		inline void LAX(unsigned char * byte)
		{
			Registers::A = Registers::X = *byte;
			Registers::P::N = Registers::A >> 7;
			Registers::P::Z = (Registers::A == 0);
		}

		// Load accumulator and X register with memory
		inline void LAX(unsigned short addr)
		{
			Registers::A = Registers::X = Memory::Read(addr);
			Registers::P::N = Registers::A >> 7;
			Registers::P::Z = (Registers::A == 0);
		}

		// Load accumulator with memory
		inline void LDA(unsigned char * byte)
		{
			Registers::A = *byte;
			Registers::P::N = Registers::A >> 7;
			Registers::P::Z = (Registers::A == 0);
		}

		// Load accumulator with memory
		inline void LDA(unsigned short addr)
		{
			Registers::A = Memory::Read(addr);
			Registers::P::N = Registers::A >> 7;
			Registers::P::Z = (Registers::A == 0);
		}

		// Load index X with memory
		inline void LDX(unsigned char * byte)
		{
			Registers::X = *byte;
			Registers::P::N = Registers::X >> 7;
			Registers::P::Z = (Registers::X == 0);
		}

		// Load index X with memory
		inline void LDX(unsigned short addr)
		{
			Registers::X = Memory::Read(addr);
			Registers::P::N = Registers::X >> 7;
			Registers::P::Z = (Registers::X == 0);
		}

		// Load index Y with memory
		inline void LDY(unsigned char * byte)
		{
			Registers::Y = *byte;
			Registers::P::N = Registers::Y >> 7;
			Registers::P::Z = (Registers::Y == 0);
		}

		// Load index Y with memory
		inline void LDY(unsigned short addr)
		{
			Registers::Y = Memory::Read(addr);
			Registers::P::N = Registers::Y >> 7;
			Registers::P::Z = (Registers::Y == 0);
		}

		// Shift byte right
		void LSR(unsigned short addr);
		inline void LSR(unsigned char * byte)
		{
			Registers::P::C = (*byte) & 1;

			(*byte) >>= 1;

			// Flags
			Registers::P::Z = (*byte == 0);
			Registers::P::N = 0;
		}

		// And byte with accumulator, transfer to X
		inline void LXA(unsigned char * byte)
		{
			Registers::A = *byte;
			Registers::X = Registers::A;

			// Flags
			Registers::P::Z = (Registers::A == 0);
			Registers::P::N = Registers::A >> 7;
		}

		// OR memory with accumulator
		inline void ORA(unsigned char * byte)
		{
			Registers::A |= *byte;

			Registers::P::N = Registers::A >> 7;
			Registers::P::Z = (Registers::A == 0);
		}

		// OR memory with accumulator
		inline void ORA(unsigned short addr)
		{
			Registers::A |= Memory::Read(addr);

			Registers::P::N = Registers::A >> 7;
			Registers::P::Z = (Registers::A == 0);
		}

		// Pull from Stack to Accumulator
		inline void PLA()
		{
			Registers::A = Stack::Pull();

			Registers::P::N = Registers::A >> 7;
			Registers::P::Z = (Registers::A == 0);
		}

		// Rotate byte left then and with accumulator
		void RLA(unsigned short addr);
		inline void RLA(unsigned char * byte)
		{
			register unsigned char val = (*byte << 1) | Registers::P::C;

			// Flags
			Registers::P::C = *byte >> 7;
			*byte = val;

			Registers::A  &= val;

			// Flags
			Registers::P::Z = (Registers::A == 0);
			Registers::P::N = Registers::A >> 7;
		}

		// Rotate byte left
		void ROL(unsigned short addr);
		inline void ROL(unsigned char * byte)
		{
			register unsigned char val = (*byte << 1) | Registers::P::C;

			// Flags
			Registers::P::C = *byte >> 7;
			Registers::P::Z = (val == 0);
			Registers::P::N = (val >> 7);

			*byte = val;
		}

		// Rotate byte right
		void ROR(unsigned short addr);
		inline void ROR(unsigned char * byte)
		{
			register unsigned char val = (*byte >> 1) | (Registers::P::C << 7);

			// Flags
			Registers::P::C = *byte & 1;
			Registers::P::Z = (val == 0);
			Registers::P::N = (val >> 7);

			*byte = val;
		}

		// Rotate byte right, then add result with accumulator
		void RRA(unsigned char * byte);
		void RRA(unsigned short addr);

		// And X with Accumulator
		inline void SAX(unsigned char * byte)
		{
			*byte = (Registers::A & Registers::X);
		}

		// And X with Accumulator
		inline void SAX(unsigned short addr)
		{
			Memory::Write(addr, (Registers::A & Registers::X));

			if(doOAMWrite)
				PPU::OAMWrite(Registers::A & Registers::X);
		}

		// And X with accumulator, subtract byte from result, store in X
		void SBX(unsigned char * byte);

		// Subtract with borrow
		void SBC(unsigned char * byte);
		void SBC(unsigned short addr);

		// And X with A, then with 7, save to memory
		inline void SHA(unsigned short addr)
		{
			Memory::Write(addr, Registers::A & Registers::X & 7);
		}

		// Shift left from memory, OR with Accumulator
		void SLO(unsigned short addr);
		inline void SLO(unsigned char * byte)
		{
			Registers::P::C = (*byte) >> 7;
			(*byte) <<= 1;
			
			Registers::A |= *byte;

			Registers::P::N = Registers::A >> 7;
			Registers::P::Z = (Registers::A == 0);
		}

		// Shift byte right in memory, then EOR with Accumulator
		void SRE(unsigned short addr);
		inline void SRE(unsigned char * byte)
		{
			Registers::P::C = (*byte) & 1;

			(*byte) >>= 1;

			Registers::A ^= *byte;

			Registers::P::N = Registers::A >> 7;
			Registers::P::Z = (Registers::A == 0);
		}

		// Store A in memory
		inline void STA(unsigned char * byte)
		{
			*byte = Registers::A;
		}

		// Store A in memory
		inline void STA(unsigned short addr)
		{
			Memory::Write(addr, Registers::A);

			if(doOAMWrite)
				PPU::OAMWrite(Registers::A);
		}

		// Store X in memory
		inline void STX(unsigned char * byte)
		{
			*byte = Registers::X;
		}

		// Store X in memory
		inline void STX(unsigned short addr)
		{
			Memory::Write(addr, Registers::X);

			if(doOAMWrite)
				PPU::OAMWrite(Registers::X);
		}

		// Store Y in memory
		inline void STY(unsigned char * byte)
		{
			*byte = Registers::Y;
		}

		// Store Y in memory
		inline void STY(unsigned short addr)
		{
			Memory::Write(addr, Registers::Y);

			if(doOAMWrite)
				PPU::OAMWrite(Registers::A & Registers::X);
		}

		// Transfer A to X
		inline void TAX()
		{
			Registers::X = Registers::A;
			Registers::P::N = Registers::X >> 7;
			Registers::P::Z = (Registers::X == 0);
		}

		// Transfer A to Y
		inline void TAY()
		{
			Registers::Y = Registers::A;
			Registers::P::N = Registers::Y >> 7;
			Registers::P::Z = (Registers::Y == 0);
		}

		// Transfer Stack Pointer to X
		inline void TSX()
		{
			Registers::X = Registers::SP;
			Registers::P::N = Registers::X >> 7;
			Registers::P::Z = (Registers::X == 0);
		}

		// Transfer X to A
		inline void TXA()
		{
			Registers::A = Registers::X;
			Registers::P::N = Registers::A >> 7;
			Registers::P::Z = (Registers::A == 0);
		}

		// Transfer Y to A
		inline void TYA()
		{
			Registers::A = Registers::Y;
			Registers::P::N = Registers::A >> 7;
			Registers::P::Z = (Registers::A == 0);
		}

		// Transfer X to A, and A with byte
		inline void XAA(unsigned char * byte)
		{
			Registers::A = Registers::X & *byte;

			// Flags
			Registers::P::Z = (Registers::A == 0);
			Registers::P::N = Registers::A >> 7;
		}

		// Does a branch when condition is met
		inline void Branch(bool branch)
		{
			// First, increment the program and cycle counter
			Registers::PC += 2;

			if(branch)
			{
				register int newPC = Registers::PC + ((char) opcodeData);
				register int extraCycles = ((1 & ((Registers::PC ^ newPC) >> 8)) /*? 3 : 0*/);

				if(!extraCycles)
				{
					bool irqpend = Interrupts::IRQHit();
					cycles += 3;
					if(!irqpend && Interrupts::IRQHit())
						Interrupts::IRQActive = 1;
				}

				else
					cycles += 6;

				if(dmcDelayCycle < cycles)
					APU::LoadDMCSample();

				Registers::PC = newPC;
			}
		}
	}
} // END namespace CPU

#undef EXTERNAL
#endif