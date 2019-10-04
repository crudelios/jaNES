// Disassembler.cpp
// Classe que faz disassembly

#define DISASSEMBLER_CPP

#include "Internal.h"
#include "Registers.h"
#include "Memory.h"
#include "Disassembler.h"

namespace CPU
{
	char * CPU::Disassembler::GetInfo(unsigned char byte)
	{
		last   = Memory::GetAbsoluteAddress(Registers::PC);
		cursor = msg;
			
		op = &DBGOpcodeList[byte];

		cursor += sprintf(cursor, "%.4X ", Registers::PC);

		cursor += sprintf(cursor, "%.2X ", byte);
		cursor += (opSize[op->addressingMode] > 1) ? sprintf(cursor, "%.2X ", *(last+1)) : sprintf(cursor, "   ");
		cursor += (opSize[op->addressingMode] > 2) ? sprintf(cursor, "%.2X ", *(last+2)) : sprintf(cursor, "   ");
			
		cursor += sprintf(cursor, "%s ", op->opcode);

		MemAccessString[op->addressingMode]();

		cursor += sprintf(cursor, "A:%.2X X:%.2X Y:%.2X P:%.2X SP:%.2X CYC:%5i", Registers::A, Registers::X, Registers::Y, Registers::P::Pack(), Registers::SP, cycles);

		return msg;
	}

	unsigned char * CPU::Disassembler::last = nullptr;
	char CPU::Disassembler::msg[256] = { 0 };
	char * CPU::Disassembler::cursor = msg;
	const CPU::Disassembler * CPU::Disassembler::op = nullptr;

	const int CPU::Disassembler::opSize[13] = { 1, 2, 2, 2, 2, 3, 3, 3, 3, 2, 3, 2, 2 };

	void (*CPU::Disassembler::MemAccessString[13])() = 
	{
		MemAccessImplied,   MemAccessImmediate, MemAccessZeroPage, MemAccessZeroPageX,
		MemAccessZeroPageY, MemAccessAddress,   MemAccessAbsolute, MemAccessAbsoluteX,
		MemAccessAbsoluteY, MemAccessRelative,  MemAccessIndirect, MemAccessIndirectX, MemAccessIndirectY
	};

	const CPU::Disassembler CPU::Disassembler::DBGOpcodeList[256] = {
		{ " BRK", IMPLIED   }, // 0x00
		{ " ORA", INDIRECTX }, // 0x01
		{ "*KIL", IMPLIED   }, // 0x02
		{ "*SLO", INDIRECTX }, // 0x03
		{ "*NOP", ZEROPAGE  }, // 0x04
		{ " ORA", ZEROPAGE  }, // 0x05
		{ " ASL", ZEROPAGE  }, // 0x06
		{ "*SLO", ZEROPAGE  }, // 0x07
		{ " PHP", IMPLIED   }, // 0x08
		{ " ORA", IMMEDIATE }, // 0x09
		{ " ASL", IMPLIED   }, // 0x0A
		{ "*KIL", IMPLIED   }, // 0x0B
		{ "*NOP", ABSOLUTE  }, // 0x0C
		{ " ORA", ABSOLUTE  }, // 0x0D
		{ " ASL", ABSOLUTE  }, // 0x0E
		{ "*SLO", ABSOLUTE  }, // 0x0F

		{ " BPL", RELATIVE  }, // 0x10
		{ " ORA", INDIRECTY }, // 0x11
		{ "*KIL", IMPLIED   }, // 0x12
		{ "*SLO", INDIRECTY }, // 0x13
		{ "*NOP", ZEROPAGEX }, // 0x14
		{ " ORA", ZEROPAGEX }, // 0x15
		{ " ASL", ZEROPAGEX }, // 0x16
		{ "*SLO", ZEROPAGEX }, // 0x17
		{ " CLC", IMPLIED   }, // 0x18
		{ " ORA", ABSOLUTEY }, // 0x19
		{ "*NOP", IMPLIED   }, // 0x1A
		{ "*SLO", ABSOLUTEY }, // 0x1B
		{ "*NOP", ABSOLUTEX }, // 0x1C
		{ " ORA", ABSOLUTEX }, // 0x1D
		{ " ASL", ABSOLUTEX }, // 0x1E
		{ "*SLO", ABSOLUTEX }, // 0x1F

		{ " JSR", ADDRESS   }, // 0x20
		{ " AND", INDIRECTX }, // 0x21
		{ "*KIL", IMPLIED   }, // 0x22
		{ "*RLA", INDIRECTX }, // 0x23
		{ " BIT", ZEROPAGE  }, // 0x24
		{ " AND", ZEROPAGE  }, // 0x25
		{ " ROL", ZEROPAGE  }, // 0x26
		{ "*RLA", ZEROPAGE  }, // 0x27
		{ " PLP", IMPLIED   }, // 0x28
		{ " AND", IMMEDIATE }, // 0x29
		{ " ROL", IMPLIED   }, // 0x2A
		{ "*KIL", IMPLIED   }, // 0x2B
		{ " BIT", ABSOLUTE  }, // 0x2C
		{ " AND", ABSOLUTE  }, // 0x2D
		{ " ROL", ABSOLUTE  }, // 0x2E
		{ "*RLA", ABSOLUTE  }, // 0x2F

		{ " BMI", RELATIVE  }, // 0x30
		{ " AND", INDIRECTY }, // 0x31
		{ "*KIL", IMPLIED   }, // 0x32
		{ "*RLA", INDIRECTY }, // 0x33
		{ "*NOP", ZEROPAGEX }, // 0x34
		{ " AND", ZEROPAGEX }, // 0x35
		{ " ROL", ZEROPAGEX }, // 0x36
		{ "*RLA", ZEROPAGEX }, // 0x37
		{ " SEC", IMPLIED   }, // 0x38
		{ " AND", ABSOLUTEY }, // 0x39
		{ "*NOP", IMPLIED   }, // 0x3A
		{ "*RLA", ABSOLUTEY }, // 0x3B
		{ "*NOP", ABSOLUTEX }, // 0x3C
		{ " AND", ABSOLUTEX }, // 0x3D
		{ " ROL", ABSOLUTEX }, // 0x3E
		{ "*RLA", ABSOLUTEX }, // 0x3F

		{ " RTI", IMPLIED   }, // 0x40
		{ " EOR", INDIRECTX }, // 0x41
		{ "*KIL", IMPLIED   }, // 0x42
		{ "*KIL", IMPLIED   }, // 0x43
		{ "*NOP", ZEROPAGE  }, // 0x44
		{ " EOR", ZEROPAGE  }, // 0x45
		{ " LSR", ZEROPAGE  }, // 0x46
		{ "*KIL", IMPLIED   }, // 0x47
		{ " PHA", IMPLIED   }, // 0x48
		{ " EOR", IMMEDIATE }, // 0x49
		{ " LSR", IMPLIED   }, // 0x4A
		{ "*KIL", IMPLIED   }, // 0x4B
		{ " JMP", ADDRESS   }, // 0x4C
		{ " EOR", ABSOLUTE  }, // 0x4D
		{ " LSR", ABSOLUTE  }, // 0x4E
		{ "*KIL", IMPLIED   }, // 0x4F

		{ " BVC", RELATIVE  }, // 0x50
		{ " EOR", INDIRECTY }, // 0x51
		{ "*KIL", IMPLIED   }, // 0x52
		{ "*KIL", IMPLIED   }, // 0x53
		{ "*NOP", ZEROPAGEX }, // 0x54
		{ " EOR", ZEROPAGEX }, // 0x55
		{ " LSR", ZEROPAGEX }, // 0x56
		{ "*KIL", IMPLIED   }, // 0x57
		{ " CLI", IMPLIED   }, // 0x58
		{ " EOR", ABSOLUTEY }, // 0x59
		{ "*NOP", IMPLIED   }, // 0x5A
		{ "*KIL", IMPLIED   }, // 0x5B
		{ "*NOP", ABSOLUTEX }, // 0x5C
		{ " EOR", ABSOLUTEX }, // 0x5D
		{ " LSR", ABSOLUTEX }, // 0x5E
		{ "*KIL", IMPLIED   }, // 0x5F

		{ " RTS", IMPLIED   }, // 0x60
		{ " ADC", INDIRECTX }, // 0x61
		{ "*KIL", IMPLIED   }, // 0x62
		{ "*KIL", IMPLIED   }, // 0x63
		{ "*NOP", ZEROPAGE  }, // 0x64
		{ " ADC", ZEROPAGE  }, // 0x65
		{ " ROR", ZEROPAGE  }, // 0x66
		{ "*KIL", IMPLIED   }, // 0x67
		{ " PLA", IMPLIED   }, // 0x68
		{ " ADC", IMMEDIATE }, // 0x69
		{ " ROR", IMPLIED   }, // 0x6A
		{ "*KIL", IMPLIED   }, // 0x6B
		{ " JMP", INDIRECT  }, // 0x6C
		{ " ADC", ABSOLUTE  }, // 0x6D
		{ " ROR", ABSOLUTE  }, // 0x6E
		{ "*KIL", IMPLIED   }, // 0x6F

		{ " BVS", RELATIVE  }, // 0x70
		{ " ADC", INDIRECTY }, // 0x71
		{ "*KIL", IMPLIED   }, // 0x72
		{ "*KIL", IMPLIED   }, // 0x73
		{ "*NOP", ZEROPAGEX }, // 0x74
		{ " ADC", ZEROPAGEX }, // 0x75
		{ " ROR", ZEROPAGEX }, // 0x76
		{ "*KIL", IMPLIED   }, // 0x77
		{ " SEI", IMPLIED   }, // 0x78
		{ " ADC", ABSOLUTEY }, // 0x79
		{ "*NOP", IMPLIED   }, // 0x7A
		{ "*KIL", IMPLIED   }, // 0x7B
		{ "*NOP", ABSOLUTEX }, // 0x7C
		{ " ADC", ABSOLUTEX }, // 0x7D
		{ " ROR", ABSOLUTEX }, // 0x7E
		{ "*KIL", IMPLIED   }, // 0x7F

		{ "*NOP", IMMEDIATE }, // 0x80
		{ " STA", INDIRECTX }, // 0x81
		{ "*NOP", IMMEDIATE }, // 0x82
		{ "*KIL", IMPLIED   }, // 0x83
		{ " STY", ZEROPAGE  }, // 0x84
		{ " STA", ZEROPAGE  }, // 0x85
		{ " STX", ZEROPAGE  }, // 0x86
		{ "*KIL", IMPLIED   }, // 0x87
		{ " DEY", IMPLIED   }, // 0x88
		{ "*NOP", IMMEDIATE }, // 0x89
		{ " TXA", IMPLIED   }, // 0x8A
		{ "*KIL", IMPLIED   }, // 0x8B
		{ " STY", ABSOLUTE  }, // 0x8C
		{ " STA", ABSOLUTE  }, // 0x8D
		{ " STX", ABSOLUTE  }, // 0x8E
		{ "*KIL", IMPLIED   }, // 0x8F

		{ " BCC", RELATIVE  }, // 0x90
		{ " STA", INDIRECTY }, // 0x91
		{ "*KIL", IMPLIED   }, // 0x92
		{ "*KIL", IMPLIED   }, // 0x93
		{ " STY", ZEROPAGEX }, // 0x94
		{ " STA", ZEROPAGEX }, // 0x95
		{ " STX", ZEROPAGEY }, // 0x96
		{ "*KIL", IMPLIED   }, // 0x97
		{ " TYA", IMPLIED   }, // 0x98
		{ " STA", ABSOLUTEY }, // 0x99
		{ " TXS", IMPLIED   }, // 0x9A
		{ "*KIL", IMPLIED   }, // 0x9B
		{ "*KIL", IMPLIED   }, // 0x9C
		{ " STA", ABSOLUTEX }, // 0x9D
		{ "*KIL", IMPLIED   }, // 0x9E
		{ "*KIL", IMPLIED   }, // 0x9F

		{ " LDY", IMMEDIATE }, // 0xA0
		{ " LDA", INDIRECTX }, // 0xA1
		{ " LDX", IMMEDIATE }, // 0xA2
		{ "*KIL", IMPLIED   }, // 0xA3
		{ " LDY", ZEROPAGE  }, // 0xA4
		{ " LDA", ZEROPAGE  }, // 0xA5
		{ " LDX", ZEROPAGE  }, // 0xA6
		{ "*KIL", IMPLIED   }, // 0xA7
		{ " TAY", IMPLIED   }, // 0xA8
		{ " LDA", IMMEDIATE }, // 0xA9
		{ " TAX", IMPLIED   }, // 0xAA
		{ "*KIL", IMPLIED   }, // 0xAB
		{ " LDY", ABSOLUTE  }, // 0xAC
		{ " LDA", ABSOLUTE  }, // 0xAD
		{ " LDX", ABSOLUTE  }, // 0xAE
		{ "*KIL", IMPLIED   }, // 0xAF

		{ " BCS", RELATIVE  }, // 0xB0
		{ " LDA", INDIRECTY }, // 0xB1
		{ "*KIL", IMPLIED   }, // 0xB2
		{ "*KIL", IMPLIED   }, // 0xB3
		{ " LDY", ZEROPAGEX }, // 0xB4
		{ " LDA", ZEROPAGEX }, // 0xB5
		{ " LDX", ZEROPAGEY }, // 0xB6
		{ "*KIL", IMPLIED   }, // 0xB7
		{ " CLV", IMPLIED   }, // 0xB8
		{ " LDA", ABSOLUTEY }, // 0xB9
		{ " TSX", IMPLIED   }, // 0xBA
		{ "*KIL", IMPLIED   }, // 0xBB
		{ " LDY", ABSOLUTEX }, // 0xBC
		{ " LDA", ABSOLUTEX }, // 0xBD
		{ " LDX", ABSOLUTEY }, // 0xBE
		{ "*KIL", IMPLIED   }, // 0xBF

		{ " CPY", IMMEDIATE }, // 0xC0
		{ " CMP", INDIRECTX }, // 0xC1
		{ "*NOP", IMMEDIATE }, // 0xC2
		{ "*KIL", IMPLIED   }, // 0xC3
		{ " CPY", ZEROPAGE  }, // 0xC4
		{ " CMP", ZEROPAGE  }, // 0xC5
		{ " DEC", ZEROPAGE  }, // 0xC6
		{ "*KIL", IMPLIED   }, // 0xC7
		{ " INY", IMPLIED   }, // 0xC8
		{ " CMP", IMMEDIATE }, // 0xC9
		{ " DEX", IMPLIED   }, // 0xCA
		{ "*KIL", IMPLIED   }, // 0xCB
		{ " CPY", ABSOLUTE  }, // 0xCC
		{ " CMP", ABSOLUTE  }, // 0xCD
		{ " DEC", ABSOLUTE  }, // 0xCE
		{ "*KIL", IMPLIED   }, // 0xCF

		{ " BNE", RELATIVE  }, // 0xD0
		{ " CMP", INDIRECTY }, // 0xD1
		{ "*KIL", IMPLIED   }, // 0xD2
		{ "*KIL", IMPLIED   }, // 0xD3
		{ "*NOP", ZEROPAGEX }, // 0xD4
		{ " CMP", ZEROPAGEX }, // 0xD5
		{ " DEC", ZEROPAGEX }, // 0xD6
		{ "*KIL", IMPLIED   }, // 0xD7
		{ " CLD", IMPLIED   }, // 0xD8
		{ " CMP", ABSOLUTEY }, // 0xD9
		{ "*NOP", IMPLIED   }, // 0xDA
		{ "*KIL", IMPLIED   }, // 0xDB
		{ "*NOP", ABSOLUTEX }, // 0xDC
		{ " CMP", ABSOLUTEX }, // 0xDD
		{ " DEC", ABSOLUTEX }, // 0xDE
		{ "*KIL", IMPLIED   }, // 0xDF

		{ " CPX", IMMEDIATE }, // 0xE0
		{ " SBC", INDIRECTX }, // 0xE1
		{ "*NOP", IMMEDIATE }, // 0xE2
		{ "*KIL", IMPLIED   }, // 0xE3
		{ " CPX", ZEROPAGE  }, // 0xE4
		{ " SBC", ZEROPAGE  }, // 0xE5
		{ " INC", ZEROPAGE  }, // 0xE6
		{ "*KIL", IMPLIED   }, // 0xE7
		{ " INX", IMPLIED   }, // 0xE8
		{ " SBC", IMMEDIATE }, // 0xE9
		{ " NOP", IMPLIED   }, // 0xEA
		{ "*KIL", IMPLIED   }, // 0xEB
		{ " CPX", ABSOLUTE  }, // 0xEC
		{ " SBC", ABSOLUTE  }, // 0xED
		{ " INC", ABSOLUTE  }, // 0xEE
		{ "*KIL", IMPLIED   }, // 0xEF

		{ " BEQ", RELATIVE  }, // 0xF0
		{ " SBC", INDIRECTY }, // 0xF1
		{ "*KIL", IMPLIED   }, // 0xF2
		{ "*KIL", IMPLIED   }, // 0xF3
		{ "*NOP", ZEROPAGEX }, // 0xF4
		{ " SBC", ZEROPAGEX }, // 0xF5
		{ " INC", ZEROPAGEX }, // 0xF6
		{ "*KIL", IMPLIED   }, // 0xF7
		{ " SED", IMPLIED   }, // 0xF8
		{ " SBC", ABSOLUTEY }, // 0xF9
		{ "*NOP", IMPLIED   }, // 0xFA
		{ "*KIL", IMPLIED   }, // 0xFB
		{ "*NOP", ABSOLUTEX }, // 0xFC
		{ " SBC", ABSOLUTEX }, // 0xFD
		{ " INC", ABSOLUTEX }, // 0xFE
		{ "*ISB", ABSOLUTEX }  // 0xFF
	};
} // END namespace CPU