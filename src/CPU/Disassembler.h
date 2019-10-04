// Disassembler.h
// Classe que faz disassembly

#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#ifndef DISASSEMBLER_CPP
#define EXTERNAL extern
#else
#define EXTERNAL
#endif

#include <cstdio>

namespace CPU
{
	enum debugAddrMode
	{
		IMPLIED,
		IMMEDIATE,
		ZEROPAGE,
		ZEROPAGEX,
		ZEROPAGEY,
		ADDRESS,
		ABSOLUTE,
		ABSOLUTEX,
		ABSOLUTEY,
		RELATIVE,
		INDIRECT,
		INDIRECTX,
		INDIRECTY
	};

	struct Disassembler
	{
		static char * GetInfo(unsigned char byte);

		// Membros dinâmicos
		char opcode[5];
		debugAddrMode addressingMode;
		static const Disassembler DBGOpcodeList[256];
		static const int opSize[13];

		private:

			// Membros estáticos
			static char msg[256];
			static char * cursor;
			static unsigned char * last;
			static const Disassembler * op;
			static void (*MemAccessString[13])();

			inline static void	MemAccessImplied()
			{
				cursor += sprintf(cursor, "                            ");
			}

			inline static void	MemAccessImmediate()
			{
				cursor += sprintf(cursor, "#$%.2X                        ", opcodeData);
			}

			inline static void	MemAccessZeroPage()
			{
				cursor += sprintf(cursor, "$%.2X = %.2X                    ", opcodeData, Memory::ZP[opcodeData]);
			}

			inline static void	MemAccessZeroPageX()
			{
				cursor += sprintf(cursor, "$%.2X,X @ %.2X = %.2X             ", opcodeData, (Registers::X + opcodeData) & 0xFF, Memory::ZP[((opcodeData) + Registers::X) & 0xFF]);
			}

			inline static void	MemAccessZeroPageY()
			{
				cursor += sprintf(cursor, "$%.2X,Y @ %.2X = %.2X             ", opcodeData, (Registers::Y + opcodeData) & 0xFF, Memory::ZP[((opcodeData) + Registers::Y) & 0xFF]);
			}

			inline static void MemAccessAddress()
			{
				cursor += sprintf(cursor, "$%.2X%.2X                       ", *(last+2), opcodeData);
			}

			inline static void	MemAccessAbsolute()
			{
				cursor += sprintf(cursor, "$%.2X%.2X = %.2X                  ", *(last+2), opcodeData, Memory::DebugRead((*(last+2) << 8) | opcodeData));
			}

			inline static void	MemAccessAbsoluteX()
			{
				cursor += sprintf(cursor, "$%.2X%.2X,X @ %.4X = %.2X         ", *(last+2), opcodeData, (((*(last+2) << 8) | opcodeData) + Registers::X) & 0xFFFF, Memory::DebugRead(((*(last+2) << 8) | opcodeData) + Registers::X));
			}

			inline static void	MemAccessAbsoluteY()
			{
				cursor += sprintf(cursor, "$%.2X%.2X,Y @ %.4X = %.2X         ", *(last+2), opcodeData, (((*(last+2) << 8) | opcodeData) + Registers::Y) & 0xFFFF, Memory::DebugRead(((*(last+2) << 8) | opcodeData) + Registers::Y));
			}

			inline static void	MemAccessRelative()
			{
				cursor += sprintf(cursor, "$%.4X                       ", Registers::PC + 2 + ((char) opcodeData));
			}

			inline static void	MemAccessIndirect()
			{
				cursor += sprintf(cursor, "($%.2X%.2X) = %.4X              ", *(last+2), opcodeData, Memory::DebugRead((*(last+2) << 8) | opcodeData) | (Memory::DebugRead((*(last+2) << 8) | (opcodeData + 1) & 0xFF) << 8));
			}

			inline static void	MemAccessIndirectX()
			{
				unsigned char addr = opcodeData + Registers::X;
				cursor += sprintf(cursor, "($%.2X,X) @ %.2X = %.2X = %.2X     ", opcodeData, addr, (Memory::ZP[(addr + 1) & 0xFF] << 8) | Memory::ZP[addr], Memory::DebugRead((Memory::ZP[(addr + 1) & 0xFF] << 8) | Memory::ZP[addr]));
			}

			inline static void	MemAccessIndirectY()
			{
				register int low = Registers::Y + Memory::ZP[opcodeData];
				register int high = Memory::ZP[(opcodeData + 1) & 0xFF] << 8;

				cursor += sprintf(cursor, "($%.2X),Y = %.4X @ %.4X = %.2X  ", opcodeData, high | (low - Registers::Y), (high + low) & 0xFFFF, Memory::DebugRead((high + low) & 0xFFFF));

				return;
			}
	};
} // END namespace CPU

#undef EXTERNAL
#endif