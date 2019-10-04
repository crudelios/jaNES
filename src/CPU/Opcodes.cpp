// Opcodes.cpp
// Opcodes do CPU

#include "Memory.h"
#include "Interrupts.h"
#include "Instructions.h"

using namespace CPU;

struct Opcodes
{
	// BRK
	static void op0x00()
	{
		Registers::PC += 2;
		cycles += 9;

		Stack::Push((unsigned char) (Registers::PC >> 8));
		Stack::Push((unsigned char) (Registers::PC & 0xFF));

		Stack::Push(Registers::P::Pack() | 0x10);

		Registers::PC -= 2;

		Registers::P::I = 1;

		if(Interrupts::NMIHit())
		{
			Registers::PC = (Memory::Read(0xFFFB) << 8) | Memory::Read(0xFFFA);
			Interrupts::NMIThisFrame = true;
		}

		else
		{
			if(Interrupts::IRQHit() /*&& (Registers::P::I == 0)*/)
			{
				Interrupts::IRQPending = true;
			}

			Registers::PC = (Memory::Read(0xFFFF) << 8) | Memory::Read(0xFFFE);
		}


		// Se o NMI surgir durante a obtenção de vectores, executar uma instrução antes de chamar o NMI
		if(!Interrupts::NMIThisFrame && Interrupts::NMIHit())
			Interrupts::DelayNMI = true;
	}

	// ORA - (Indirect, X)
	static void op0x01()
	{
		Instructions::ORA(Memory::IndirectX());

		Registers::PC += 2;
	}

	// ORA - Zero Page
	static void op0x05()
	{
		cycles += 3;

		Instructions::ORA(Memory::ZeroPage());

		Registers::PC += 2;
	}

	// ASL - Zero Page
	static void op0x06()
	{
		cycles += 3;
		Instructions::ASL(Memory::ZeroPage());

		Registers::PC += 2;
		cycles += 6;
	}

	// PHP
	static void op0x08()
	{
		Stack::Push(Registers::P::Pack() | 0x10);

		++Registers::PC;
		cycles += 3;
	}

	// ORA - Immediate
	static void op0x09()
	{
		Instructions::ORA(Memory::Immediate());

		Registers::PC += 2;
	}

	// ASL - Accumulator
	static void op0x0A()
	{
		Instructions::ASL(Memory::Accumulator());

		++Registers::PC;
	}

	// ORA - Absolute
	static void op0x0D()
	{
		Instructions::ORA(Memory::Absolute());

		Registers::PC += 3;
	}

	// ASL - Absolute
	static void op0x0E()
	{
		Instructions::ASL(Memory::Absolute());

		Registers::PC += 3;
	}

	// BPL
	static void op0x10()
	{
		Instructions::Branch(Registers::P::N == 0);
	}

	// ORA - (Indirect), Y
	static void op0x11()
	{
		Instructions::ORA(Memory::IndirectY());

		Registers::PC += 2;
	}

	// ORA - Zero Page, X
	static void op0x15()
	{
		cycles += 6;

		Instructions::ORA(Memory::ZeroPageX());

		Registers::PC += 2;
	}

	// ASL - Zero Page, X
	static void op0x16()
	{
		cycles += 6;

		Instructions::ASL(Memory::ZeroPageX());

		Registers::PC += 2;
		cycles += 6;
	}

	// CLC
	static void op0x18()
	{
		Registers::P::C = 0;

		++Registers::PC;
	}

	// ORA - Absolute, Y
	static void op0x19()
	{
		Instructions::ORA(Memory::AbsoluteY());

		Registers::PC += 3;
	}

	// ORA - Absolute, X
	static void op0x1D()
	{
		Instructions::ORA(Memory::AbsoluteX());

		Registers::PC += 3;
	}

	// ASL - Absolute, X
	static void op0x1E()
	{
		Instructions::ASL(Memory::AbsoluteX(true));

		Registers::PC += 3;
	}

	// JSR
	static void op0x20()
	{
		// Só se aumenta o PC por 2 porque é suposto decrementar por 1
		Stack::Push((unsigned char) ((Registers::PC + 2) >> 8));
		Stack::Push((unsigned char) ((Registers::PC + 2) & 0xFF));

		cycles += 3;

		if(dmcDelayCycle <= cycles)
				APU::LoadDMCSample();

		cycles += 6;
		
		Registers::PC = Memory::Absolute();
	}

	// AND - (Indirect, X)
	static void op0x21()
	{
		Instructions::AND(Memory::IndirectX());

		Registers::PC += 2;
	}

	// BIT - Zero Page
	static void op0x24()
	{
		cycles += 3;

		Instructions::BIT(Memory::ZeroPage());

		Registers::PC += 2;
	}

	// AND - Zero Page
	static void op0x25()
	{
		cycles += 3;

		Instructions::AND(Memory::ZeroPage());

		Registers::PC += 2;
	}

	// ROL - Zero Page
	static void op0x26()
	{
		cycles += 3;

		Instructions::ROL(Memory::ZeroPage());

		Registers::PC += 2;
		cycles += 6;
	}

	// PLP
	static void op0x28()
	{
		Registers::P::Unpack(Stack::Pull() & 0xEF);

		++Registers::PC;
		cycles += 6;

		if(dmcDelayCycle < cycles)
			APU::LoadDMCSample();
	}

	// AND - Immediate
	static void op0x29()
	{
		Instructions::AND(Memory::Immediate());

		Registers::PC += 2;
	}

	// ROL - Accumulator
	static void op0x2A()
	{
		Instructions::ROL(Memory::Accumulator());

		++Registers::PC;
	}

	// BIT - Absolute
	static void op0x2C()
	{
		Instructions::BIT(Memory::Absolute());

		Registers::PC += 3;
	}

	// AND - Absolute
	static void op0x2D()
	{
		Instructions::AND(Memory::Absolute());

		Registers::PC += 3;
	}

	// ROL - Absolute
	static void op0x2E()
	{
		Instructions::ROL(Memory::Absolute());

		Registers::PC += 3;
	}

	// BMI
	static void op0x30()
	{
		Instructions::Branch(Registers::P::N == 1);
	}

	// AND - (Indirect), Y
	static void op0x31()
	{
		Instructions::AND(Memory::IndirectY());

		Registers::PC += 2;
	}

	// AND - Zero Page, X
	static void op0x35()
	{
		cycles += 6;

		Instructions::AND(Memory::ZeroPageX());

		Registers::PC += 2;
	}

	// ROL - ZeroPage, X
	static void op0x36()
	{
		cycles += 6;

		Instructions::ROL(Memory::ZeroPageX());

		Registers::PC += 2;
		cycles += 6;
	}

	// SEC
	static void op0x38()
	{
		Registers::P::C = 1;

		++Registers::PC;
	}

	// AND - Absolute, Y
	static void op0x39()
	{
		Instructions::AND(Memory::AbsoluteY());

		Registers::PC += 3;
	}

	// AND - Absolute, X
	static void op0x3D()
	{
		Instructions::AND(Memory::AbsoluteX());

		Registers::PC += 3;
	}

	// ROL - Absolute, X
	static void op0x3E()
	{
		Instructions::ROL(Memory::AbsoluteX(true));

		Registers::PC += 3;
	}

	// RTI
	static void op0x40()
	{
		Registers::P::Unpack(Stack::Pull() & 0xEF);
		Interrupts::IRQActive = Registers::P::I;

		Registers::PC  =  Stack::Pull();
		Registers::PC |= (Stack::Pull() << 8);

		cycles += 12;

		if(dmcDelayCycle < cycles)
			APU::LoadDMCSample();
	}

	// EOR - (Indirect, X)
	static void op0x41()
	{
		Instructions::EOR(Memory::IndirectX());

		Registers::PC += 2;
	}

	// EOR - Zero Page
	static void op0x45()
	{
		cycles += 3;

		Instructions::EOR(Memory::ZeroPage());

		Registers::PC += 2;
	}

	// LSR - Zero Page
	static void op0x46()
	{
		cycles += 3;

		Instructions::LSR(Memory::ZeroPage());

		Registers::PC += 2;
		cycles += 6;
	}

	// PHA
	static void op0x48()
	{
		Stack::Push((unsigned char) Registers::A);

		++Registers::PC;
		cycles += 3;
	}

	// EOR - Immediate
	static void op0x49()
	{
		Instructions::EOR(Memory::Immediate());

		Registers::PC += 2;
	}

	// LSR - Accumulator
	static void op0x4A()
	{
		Instructions::LSR(Memory::Accumulator());

		++Registers::PC;
	}

	// JMP - Absolute
	static void op0x4C()
	{
		Registers::PC = Memory::Absolute();
	}

	// EOR - Absolute
	static void op0x4D()
	{
		Instructions::EOR(Memory::Absolute());

		Registers::PC += 3;
	}

	// LSR - Absolute
	static void op0x4E()
	{
		Instructions::LSR(Memory::Absolute());

		Registers::PC += 3;
	}

	// BVC
	static void op0x50()
	{
		Instructions::Branch(Registers::P::V == 0);
	}

	// EOR - (Indirect), Y
	static void op0x51()
	{
		Instructions::EOR(Memory::IndirectY());

		Registers::PC += 2;
	}

	// EOR - Zero Page, X
	static void op0x55()
	{
		cycles += 6;

		Instructions::EOR(Memory::ZeroPageX());

		Registers::PC += 2;
	}

	// LSR - Zero Page, X
	static void op0x56()
	{
		cycles += 6;

		Instructions::LSR(Memory::ZeroPageX());

		Registers::PC += 2;
		cycles += 6;
	}

	// CLI
	static void op0x58()
	{
		Registers::P::I = 0;

		++Registers::PC;
	}

	// EOR - Absolute, Y
	static void op0x59()
	{
		Instructions::EOR(Memory::AbsoluteY());

		Registers::PC += 3;
	}

	// EOR - Absolute, X
	static void op0x5D()
	{
		Instructions::EOR(Memory::AbsoluteX());

		Registers::PC += 3;
	}

	// LSR - Absolute, X
	static void op0x5E()
	{
		Instructions::LSR(Memory::AbsoluteX(true));

		Registers::PC += 3;
	}

	// RTS
	static void op0x60()
	{
		Registers::PC  = Stack::Pull();
		Registers::PC |= Stack::Pull() << 8;
		++Registers::PC;

		cycles += 12;

		if(dmcDelayCycle < cycles)
			APU::LoadDMCSample();
	}

	// ADC - (Indirect, X)
	static void op0x61()
	{
		Instructions::ADC(Memory::IndirectX());

		Registers::PC += 2;
	}

	// ADC - Zero Page
	static void op0x65()
	{
		cycles += 3;

		Instructions::ADC(Memory::ZeroPage());

		Registers::PC += 2;
	}

	// ROR - Zero Page
	static void op0x66()
	{
		cycles += 3;
		Instructions::ROR(Memory::ZeroPage());

		Registers::PC += 2;
		cycles += 6;
	}

	// PLA
	static void op0x68()
	{
		Instructions::PLA();

		++Registers::PC;
		cycles += 6;

		if(dmcDelayCycle < cycles)
			APU::LoadDMCSample();
	}

	// ADC - Immediate
	static void op0x69()
	{
		Instructions::ADC(Memory::Immediate());

		Registers::PC += 2;
	}

	// ROR - Accumulator
	static void op0x6A()
	{
		Instructions::ROR(Memory::Accumulator());

		++Registers::PC;
	}

	// JMP - Indirect
	static void op0x6C()
	{
		unsigned char high = Memory::Read(Registers::PC + 2);

		// Reproduzir o bug do JMP indirect
		Registers::PC = Memory::Read(high, opcodeData) | (Memory::Read(high, (opcodeData + 1) & 0xFF) << 8);
	}

	// ADC - Absolute
	static void op0x6D()
	{
		Instructions::ADC(Memory::Absolute());

		Registers::PC += 3;
	}

	// ROR - Absolute
	static void op0x6E()
	{
		Instructions::ROR(Memory::Absolute());

		Registers::PC += 3;
	}

	// BVS
	static void op0x70()
	{
		Instructions::Branch(Registers::P::V == 1);
	}

	// ADC - Indirect, Y
	static void op0x71()
	{
		Instructions::ADC(Memory::IndirectY());

		Registers::PC += 2;
	}

	// ADC - Zero Page, X
	static void op0x75()
	{
		cycles += 6;

		Instructions::ADC(Memory::ZeroPageX());

		Registers::PC += 2;
	}

	// ROR - Zero Page, X
	static void op0x76()
	{
		cycles += 6;

		Instructions::ROR(Memory::ZeroPageX());

		Registers::PC += 2;
		cycles += 6;
	}

	// SEI
	static void op0x78()
	{
		Registers::P::I = 1;

		++Registers::PC;
	}

	// ADC - Absolute, Y
	static void op0x79()
	{
		Instructions::ADC(Memory::AbsoluteY());

		Registers::PC += 3;
	}

	// ADC - Absolute, X
	static void op0x7D()
	{
		Instructions::ADC(Memory::AbsoluteX());

		Registers::PC += 3;
	}

	// ROR - Absolute, X
	static void op0x7E()
	{
		Instructions::ROR(Memory::AbsoluteX(true));

		Registers::PC += 3;
	}

	// STA - (Indirect, X)
	static void op0x81()
	{
		Instructions::STA(Memory::IndirectX());

		Registers::PC += 2;
	}

	// STY - Zero Page
	static void op0x84()
	{
		Instructions::STY(Memory::ZeroPage());

		Registers::PC += 2;
		cycles += 3;
	}

	// STA - Zero Page
	static void op0x85()
	{
		Instructions::STA(Memory::ZeroPage());

		Registers::PC += 2;
		cycles += 3;
	}

	// STX - Zero Page
	static void op0x86()
	{
		Instructions::STX(Memory::ZeroPage());

		Registers::PC += 2;
		cycles += 3;
	}

	// DEY
	static void op0x88()
	{
		Instructions::DEY();

		++Registers::PC;
	}

	// TXA
	static void op0x8A()
	{
		Instructions::TXA();

		++Registers::PC;
	}

	// STY - Absolute
	static void op0x8C()
	{
		Instructions::STY(Memory::Absolute());

		Registers::PC += 3;
	}

	// STA - Absolute
	static void op0x8D()
	{
		Instructions::STA(Memory::Absolute());

		Registers::PC += 3;
	}

	// STX - Absolute
	static void op0x8E()
	{
		Instructions::STX(Memory::Absolute());

		Registers::PC += 3;
	}

	// BCC
	static void op0x90()
	{
		Instructions::Branch(Registers::P::C == 0);
	}

	// STA - (Indirect), Y
	static void op0x91()
	{
		Instructions::STA(Memory::IndirectY(true));

		Registers::PC += 2;
	}

	// STY - Zero Page, X
	static void op0x94()
	{
		cycles += 3;

		Instructions::STY(Memory::ZeroPageX());

		Registers::PC += 2;
		cycles += 3;
	}

	// STA - Zero Page, X
	static void op0x95()
	{
		cycles += 3;

		Instructions::STA(Memory::ZeroPageX());

		Registers::PC += 2;
		cycles += 3;
	}

	// STX - Zero Page, Y
	static void op0x96()
	{
		cycles += 3;

		Instructions::STX(Memory::ZeroPageY());

		Registers::PC += 2;
		cycles += 3;
	}

	// TYA
	static void op0x98()
	{
		Instructions::TYA();

		++Registers::PC;
	}

	// STA - Absolute, Y
	static void op0x99()
	{
		Instructions::STA(Memory::AbsoluteY(true));

		Registers::PC += 3;
	}

	// TXS
	static void op0x9A()
	{
		Registers::SP = (unsigned char) Registers::X;

		++Registers::PC;
	}

	// STA - Absolute, X
	static void op0x9D()
	{
		Instructions::STA(Memory::AbsoluteX(true));

		Registers::PC += 3;
	}

	// LDY - Immediate
	static void op0xA0()
	{
		Instructions::LDY(Memory::Immediate());

		Registers::PC += 2;
	}

	// LDA - (Indirect, X)
	static void op0xA1()
	{
		Instructions::LDA(Memory::IndirectX());

		Registers::PC += 2;
	}

	// LDX - Immediate
	static void op0xA2()
	{
		Instructions::LDX(Memory::Immediate());

		Registers::PC += 2;
	}

	// LDY - Zero Page
	static void op0xA4()
	{
		cycles += 3;
		Instructions::LDY(Memory::ZeroPage());

		Registers::PC += 2;
	}

	// LDA - Zero Page
	static void op0xA5()
	{
		cycles += 3;
		Instructions::LDA(Memory::ZeroPage());

		Registers::PC += 2;
	}

	// LDX - Zero Page
	static void op0xA6()
	{
		cycles += 3;
		Instructions::LDX(Memory::ZeroPage());

		Registers::PC += 2;
	}

	// TAY
	static void op0xA8()
	{
		Instructions::TAY();

		++Registers::PC;
	}

	// LDA - Immediate
	static void op0xA9()
	{
		Instructions::LDA(Memory::Immediate());

		Registers::PC += 2;
	}

	// TAX
	static void op0xAA()
	{
		Instructions::TAX();

		++Registers::PC;
	}

	// LDY - Absolute
	static void op0xAC()
	{
		Instructions::LDY(Memory::Absolute());

		Registers::PC += 3;
	}

	// LDA - Absolute
	static void op0xAD()
	{
		Instructions::LDA(Memory::Absolute());

		Registers::PC += 3;
	}

	// LDX - Absolute
	static void op0xAE()
	{
		Instructions::LDX(Memory::Absolute());

		Registers::PC += 3;
	}

	// BCS
	static void op0xB0()
	{
		Instructions::Branch(Registers::P::C == 1);
	}

	// LDA - (Indirect), Y
	static void op0xB1()
	{
		Instructions::LDA(Memory::IndirectY());

		Registers::PC += 2;
	}

	// LDY - Zero Page, X
	static void op0xB4()
	{
		cycles += 6;

		Instructions::LDY(Memory::ZeroPageX());

		Registers::PC += 2;
	}

	// LDA - Zero Page, X
	static void op0xB5()
	{
		cycles += 6;

		Instructions::LDA(Memory::ZeroPageX());

		Registers::PC += 2;
	}

	// LDX - Zero Page, Y
	static void op0xB6()
	{
		cycles += 6;

		Instructions::LDX(Memory::ZeroPageY());

		Registers::PC += 2;
	}

	// CLV
	static void op0xB8()
	{
		Registers::P::V = 0;

		++Registers::PC;
	}

	// LDA - Absolute, Y
	static void op0xB9()
	{
		Instructions::LDA(Memory::AbsoluteY());

		Registers::PC += 3;
	}

	// TSX
	static void op0xBA()
	{
		Instructions::TSX();

		++Registers::PC;
	}

	// LDY - Absolute, X
	static void op0xBC()
	{
		Instructions::LDY(Memory::AbsoluteX());

		Registers::PC += 3;
	}

	// LDA - Absolute, X
	static void op0xBD()
	{
		Instructions::LDA(Memory::AbsoluteX());

		Registers::PC += 3;
	}

	// LDX - Absolute, Y
	static void op0xBE()
	{
		Instructions::LDX(Memory::AbsoluteY());

		Registers::PC += 3;
	}

	// CPY - Immediate
	static void op0xC0()
	{
		Instructions::CPY(Memory::Immediate());

		Registers::PC += 2;
	}

	// CMP - (Indirect, X)
	static void op0xC1()
	{
		Instructions::CMP(Memory::IndirectX());

		Registers::PC += 2;
	}

	// CPY - Zero Page
	static void op0xC4()
	{
		cycles += 3;

		Instructions::CPY(Memory::ZeroPage());

		Registers::PC += 2;
	}

	// CMP - Zero Page
	static void op0xC5()
	{
		cycles += 3;

		Instructions::CMP(Memory::ZeroPage());

		Registers::PC += 2;
	}

	// DEC - Zero Page
	static void op0xC6()
	{
		cycles += 3;

		Instructions::DEC(Memory::ZeroPage());

		Registers::PC += 2;
		cycles += 6;
	}

	// INY
	static void op0xC8()
	{
		Instructions::INY();

		++Registers::PC;
	}

	// CMP - Immediate
	static void op0xC9()
	{
		Instructions::CMP(Memory::Immediate());

		Registers::PC += 2;
	}

	// DEX
	static void op0xCA()
	{
		Instructions::DEX();

		++Registers::PC;
	}

	// CPY - Absolute
	static void op0xCC()
	{
		Instructions::CPY(Memory::Absolute());

		Registers::PC += 3;
	}

	// CMP - Absolute
	static void op0xCD()
	{
		Instructions::CMP(Memory::Absolute());

		Registers::PC += 3;
	}

	// DEC - Absolute
	static void op0xCE()
	{
		Instructions::DEC(Memory::Absolute());

		Registers::PC += 3;
	}

	// BNE
	static void op0xD0()
	{
		Instructions::Branch(Registers::P::Z == 0);
	}

	// CMP - (Indirect), Y
	static void op0xD1()
	{
		Instructions::CMP(Memory::IndirectY());

		Registers::PC += 2;
	}

	// CMP - Zero Page, X
	static void op0xD5()
	{
		cycles += 6;

		Instructions::CMP(Memory::ZeroPageX());

		Registers::PC += 2;
	}

	// DEC - Zero Page, X
	static void op0xD6()
	{
		cycles += 6;

		Instructions::DEC(Memory::ZeroPageX());

		Registers::PC += 2;
		cycles += 6;
	}

	// CLD
	static void op0xD8()
	{
		Registers::P::D = 0;

		++Registers::PC;
	}

	// CMP - Absolute, Y
	static void op0xD9()
	{
		Instructions::CMP(Memory::AbsoluteY());

		Registers::PC += 3;
	}

	// CMP - Absolute, X
	static void op0xDD()
	{
		Instructions::CMP(Memory::AbsoluteX());

		Registers::PC += 3;
	}

	// DEC - Absolute, X
	static void op0xDE()
	{
		Instructions::DEC(Memory::AbsoluteX(true));

		Registers::PC += 3;
	}

	// CPX - Immediate
	static void op0xE0()
	{
		Instructions::CPX(Memory::Immediate());

		Registers::PC += 2;
	}

	// SBC - (Indirect, X)
	static void op0xE1()
	{
		Instructions::SBC(Memory::IndirectX());

		Registers::PC += 2;
	}

	// CPX - Zero Page
	static void op0xE4()
	{
		cycles += 3;

		Instructions::CPX(Memory::ZeroPage());

		Registers::PC += 2;
	}

	// SBC - Zero Page
	static void op0xE5()
	{
		cycles += 3;

		Instructions::SBC(Memory::ZeroPage());

		Registers::PC += 2;
	}

	// INC - Zero Page
	static void op0xE6()
	{
		cycles += 3;

		Instructions::INC(Memory::ZeroPage());

		Registers::PC += 2;
		cycles += 6;
	}

	// INX
	static void op0xE8()
	{
		Instructions::INX();

		++Registers::PC;
	}

	// SBC - Immediate
	static void op0xE9()
	{
		Instructions::SBC(Memory::Immediate());

		Registers::PC += 2;
	}

	// CPX - Absolute
	static void op0xEC()
	{
		Instructions::CPX(Memory::Absolute());

		Registers::PC += 3;
	}

	// SBC - Absolute
	static void op0xED()
	{
		Instructions::SBC(Memory::Absolute());

		Registers::PC += 3;
	}

	// INC - Absolute
	static void op0xEE()
	{
		Instructions::INC(Memory::Absolute());

		Registers::PC += 3;
	}

	// BEQ
	static void op0xF0()
	{
		Instructions::Branch(Registers::P::Z == 1);
	}

	// SBC - (Indirect), Y
	static void op0xF1()
	{
		Instructions::SBC(Memory::IndirectY());

		Registers::PC += 2;
	}

	// SBC - Zero Page, X
	static void op0xF5()
	{
		cycles += 6;

		Instructions::SBC(Memory::ZeroPageX());

		Registers::PC += 2;
	}

	// INC - Zero Page, X
	static void op0xF6()
	{
		cycles += 6;

		Instructions::INC(Memory::ZeroPageX());

		Registers::PC += 2;
		cycles += 6;
	}

	// SED
	static void op0xF8()
	{
		Registers::P::D = 1;

		++Registers::PC;
	}

	// SBC - Absolute, Y
	static void op0xF9()
	{
		Instructions::SBC(Memory::AbsoluteY());

		Registers::PC += 3;
	}

	// SBC - Absolute, X
	static void op0xFD()
	{
		Instructions::SBC(Memory::AbsoluteX());

		Registers::PC += 3;
	}

	// INC - Absolute, X
	static void op0xFE()
	{
		Instructions::INC(Memory::AbsoluteX(true));

		Registers::PC += 3;
	}


	// ------------------------------------------------------------------------
	// OPCODES NÃO OFICIAIS ---------------------------------------------------
	// ------------------------------------------------------------------------

	// SLO - (Indirect, X)
	static void Uop0x03()
	{
		Instructions::SLO(Memory::IndirectX());

		Registers::PC += 2;
	}

	// SLO - Zero Page
	static void Uop0x07()
	{
		cycles += 3;

		Instructions::SLO(Memory::ZeroPage());

		Registers::PC += 2;
		cycles += 6;
	}

	// ANC - Immediate
	static void Uop0x0B()
	{
		Instructions::ANC(Memory::Immediate());

		Registers::PC += 2;
	}

	// SLO - Absolute
	static void Uop0x0F()
	{
		Instructions::SLO(Memory::Absolute());

		Registers::PC += 3;
	}

	// SLO - (Indirect), Y
	static void Uop0x13()
	{
		Instructions::SLO(Memory::IndirectY(true));

		Registers::PC += 2;
	}

	// SLO - Zero Page, X
	static void Uop0x17()
	{
		cycles += 6;

		Instructions::SLO(Memory::ZeroPageX());

		Registers::PC += 2;
		cycles += 6;
	}

	// SLO - Absolute, Y
	static void Uop0x1B()
	{
		Instructions::SLO(Memory::AbsoluteY(true));

		Registers::PC += 3;
	}

	// SLO - Absolute, X
	static void Uop0x1F()
	{
		Instructions::SLO(Memory::AbsoluteX(true));

		Registers::PC += 3;
	}

	// RLA - (Indirect, X)
	static void Uop0x23()
	{
		Instructions::RLA(Memory::IndirectX());

		Registers::PC += 2;
	}

	// RLA - Zero Page
	static void Uop0x27()
	{
		cycles += 3;

		Instructions::RLA(Memory::ZeroPage());

		Registers::PC += 2;
		cycles += 6;
	}

	// ANC - Immediate
	static void Uop0x2B()
	{
		Instructions::ANC(Memory::Immediate());

		Registers::PC += 2;
	}

	// RLA - Absolute
	static void Uop0x2F()
	{
		Instructions::RLA(Memory::Absolute());

		Registers::PC += 3;
	}

	// RLA - (Indirect), Y
	static void Uop0x33()
	{
		Instructions::RLA(Memory::IndirectY(true));

		Registers::PC += 2;
	}

	// RLA - Zero Page, X
	static void Uop0x37()
	{
		cycles += 6;

		Instructions::RLA(Memory::ZeroPageX());

		Registers::PC += 2;
		cycles += 6;
	}

	// RLA - Absolute, Y
	static void Uop0x3B()
	{
		Instructions::RLA(Memory::AbsoluteY(true));

		Registers::PC += 3;
	}

	// RLA - Absolute, X
	static void Uop0x3F()
	{
		Instructions::RLA(Memory::AbsoluteX(true));

		Registers::PC += 3;
	}

	// SRE - (Indirect, X)
	static void Uop0x43()
	{
		Instructions::SRE(Memory::IndirectX());

		Registers::PC += 2;
	}

	// SRE - Zero Page
	static void Uop0x47()
	{
		cycles += 3;

		Instructions::SRE(Memory::ZeroPage());

		Registers::PC += 2;
		cycles += 6;
	}

	// ASR - Immediate
	static void Uop0x4B()
	{
		Instructions::ASR(Memory::Immediate());

		Registers::PC += 2;
	}

	// SRE - Absolute
	static void Uop0x4F()
	{
		Instructions::SRE(Memory::Absolute());

		Registers::PC += 3;
	}

	// SRE - (Indirect), Y
	static void Uop0x53()
	{
		Instructions::SRE(Memory::IndirectY(true));

		Registers::PC += 2;
	}

	// SRE - Zero Page, X
	static void Uop0x57()
	{
		cycles += 6;

		Instructions::SRE(Memory::ZeroPageX());

		Registers::PC += 2;
		cycles += 6;
	}

	// SRE - Absolute, Y
	static void Uop0x5B()
	{
		Instructions::SRE(Memory::AbsoluteY(true));

		Registers::PC += 3;
	}

	// SRE - Absolute, X
	static void Uop0x5F()
	{
		Instructions::SRE(Memory::AbsoluteX(true));

		Registers::PC += 3;
	}

	// RRA - (Indirect, X)
	static void Uop0x63()
	{
		Instructions::RRA(Memory::IndirectX());

		Registers::PC += 2;
	}

	// RRA - Zero Page
	static void Uop0x67()
	{
		cycles += 3;

		Instructions::RRA(Memory::ZeroPage());

		Registers::PC += 2;
		cycles += 6;
	}

	// ARR - Immediate
	static void Uop0x6B()
	{
		Instructions::ARR(Memory::Immediate());

		Registers::PC += 2;
	}

	// RRA - Absolute
	static void Uop0x6F()
	{
		Instructions::RRA(Memory::Absolute());

		Registers::PC += 3;
	}

	// RRA - (Indirect), Y
	static void Uop0x73()
	{
		Instructions::RRA(Memory::IndirectY(true));

		Registers::PC += 2;
	}

	// RRA - Zero Page, X
	static void Uop0x77()
	{
		cycles += 6;

		Instructions::RRA(Memory::ZeroPageX());

		Registers::PC += 2;
		cycles += 6;
	}

	// RRA - Absolute, Y
	static void Uop0x7B()
	{
		Instructions::RRA(Memory::AbsoluteY(true));

		Registers::PC += 3;
	}

	// RRA - Absolute, X
	static void Uop0x7F()
	{
		Instructions::RRA(Memory::AbsoluteX(true));

		Registers::PC += 3;
	}

	// SAX - (Indirect, X)
	static void Uop0x83()
	{
		Instructions::SAX(Memory::IndirectX());

		Registers::PC += 2;
	}

	// SAX - Zero Page
	static void Uop0x87()
	{
		Instructions::SAX(Memory::ZeroPage());

		Registers::PC += 2;
		cycles += 3;
	}

	// XAA - Immediate
	static void Uop0x8B()
	{
		Instructions::XAA(Memory::Immediate());

		Registers::PC += 2;
	}


	// SAX - Absolute
	static void Uop0x8F()
	{
		Instructions::SAX(Memory::Absolute());

		Registers::PC += 3;
	}

	// SHA - (Indirect), Y
	static void Uop0x93()
	{
		Instructions::SHA(Memory::IndirectY(true));

		Registers::PC += 2;
	}

	// SAX - Zero Page, Y
	static void Uop0x97()
	{
		cycles += 3;

		Instructions::SAX(Memory::ZeroPageY());

		Registers::PC += 2;
		cycles += 3;
	}

	// SHS - Absolute, Y
	static void Uop0x9B()
	{
		register int low = Registers::Y + opcodeData;
		register int high = Memory::Read(Registers::PC + 2);

		Memory::Read((high << 8)| (low & 0xFF));

		register unsigned char result = Registers::A & Registers::X;

		Registers::SP = result;

		result &= high + 1;

		if(low & 0x100)
			Memory::Write((result << 8) + (low & 0xFF), result);

		else
			Memory::Write((high << 8) + (low & 0xFF), result);

		Registers::PC += 3;
	}


	// SHY - Absolute, X
	static void Uop0x9C()
	{
		register int low = Registers::X + opcodeData;
		register int high = Memory::Read(Registers::PC + 2);

		Memory::Read((high << 8)| (low & 0xFF));

		register unsigned char result = (Registers::Y & (high + 1));

		if(low & 0x100)
			Memory::Write((result << 8) + (low & 0xFF), result);

		else
			Memory::Write((high << 8) + (low & 0xFF), result);

		Registers::PC += 3;
	}

	// SHX - Absolute, Y
	static void Uop0x9E()
	{
		register int low = Registers::Y + opcodeData;
		register int high = Memory::Read(Registers::PC + 2);

		Memory::Read((high << 8)| (low & 0xFF));

		register unsigned char result = (Registers::X & (high + 1));

		if(low & 0x100)
			Memory::Write((result << 8) + (low & 0xFF), result);

		else
			Memory::Write((high << 8) + (low & 0xFF), result);

		Registers::PC += 3;
	}

	// SHA - Absolute, Y
	static void Uop0x9F()
	{
		Instructions::SHA(Memory::AbsoluteY(true));

		Registers::PC += 3;
	}


	// LAX - (Indirect, X)
	static void Uop0xA3()
	{
		Instructions::LAX(Memory::IndirectX());

		Registers::PC += 2;
	}

	// LAX - Zero Page
	static void Uop0xA7()
	{
		cycles += 3;

		Instructions::LAX(Memory::ZeroPage());

		Registers::PC += 2;
	}

	// LXA - Immediate
	static void Uop0xAB()
	{
		Instructions::LXA(Memory::Immediate());

		Registers::PC += 2;
	}

	// LAX - Absolute
	static void Uop0xAF()
	{
		Instructions::LAX(Memory::Absolute());

		Registers::PC += 3;
	}

	// LAX - (Indirect), Y
	static void Uop0xB3()
	{
		Instructions::LAX(Memory::IndirectY());

		Registers::PC += 2;
	}

	// LAX - Zero Page, Y
	static void Uop0xB7()
	{
		cycles += 6;

		Instructions::LAX(Memory::ZeroPageY());

		Registers::PC += 2;
	}

	// LAS - Absolute, Y
	static void Uop0xBB()
	{
		Instructions::LAS(Memory::AbsoluteY());

		Registers::PC += 3;
	}

	// LAX - Absolute, Y
	static void Uop0xBF()
	{
		Instructions::LAX(Memory::AbsoluteY());

		Registers::PC += 3;
	}

	// DCP - (Indirect, X)
	static void Uop0xC3()
	{
		Instructions::DCP(Memory::IndirectX());

		Registers::PC += 2;
	}

	// DCP - Zero Page
	static void Uop0xC7()
	{
		cycles += 3;

		Instructions::DCP(Memory::ZeroPage());

		Registers::PC += 2;
		cycles += 6;
	}

	// SBX - Immediate
	static void Uop0xCB()
	{
		Instructions::SBX(Memory::Immediate());

		Registers::PC += 2;
	}


	// DCP - Absolute
	static void Uop0xCF()
	{
		Instructions::DCP(Memory::Absolute());

		Registers::PC += 3;
	}

	// DCP - (Indirect), Y
	static void Uop0xD3()
	{
		Instructions::DCP(Memory::IndirectY(true));

		Registers::PC += 2;
	}

	// DCP - Zero Page, X
	static void Uop0xD7()
	{
		cycles += 6;

		Instructions::DCP(Memory::ZeroPageX());

		Registers::PC += 2;
		cycles += 6;
	}

	// DCP - Absolute, Y
	static void Uop0xDB()
	{
		Instructions::DCP(Memory::AbsoluteY(true));

		Registers::PC += 3;
	}

	// DCP - Absolute, X
	static void Uop0xDF()
	{
		Instructions::DCP(Memory::AbsoluteX(true));

		Registers::PC += 3;
	}

	// ISB - (Indirect, X)
	static void Uop0xE3()
	{
		Instructions::ISB(Memory::IndirectX());

		Registers::PC += 2;
	}

	// ISB - Zero Page
	static void Uop0xE7()
	{
		cycles += 3;

		Instructions::ISB(Memory::ZeroPage());

		Registers::PC += 2;
		cycles += 6;
	}

	// ISB - Absolute
	static void Uop0xEF()
	{
		Instructions::ISB(Memory::Absolute());

		Registers::PC += 3;
	}

	// ISB - (Indirect), Y
	static void Uop0xF3()
	{
		Instructions::ISB(Memory::IndirectY(true));

		Registers::PC += 2;
	}

	// ISB - Zero Page, X
	static void Uop0xF7()
	{
		cycles += 6;

		Instructions::ISB(Memory::ZeroPageX());

		Registers::PC += 2;
		cycles += 6;
	}

	// ISB - Absolute, Y
	static void Uop0xFB()
	{
		Instructions::ISB(Memory::AbsoluteY(true));

		Registers::PC += 3;
	}

	// ISB - Absolute, X
	static void Uop0xFF()
	{
		Instructions::ISB(Memory::AbsoluteX(true));

		Registers::PC += 3;
	}

	// ------------------------------------------------------------------------
	// NOPS -------------------------------------------------------------------
	// ------------------------------------------------------------------------
		
	// NOP
	static void NOP()
	{
		++Registers::PC;
	}

	// NOP - Immediate
	static void NOPIMM()
	{
		Registers::PC += 2;
	}


	// NOP - Zero Page
	static void NOPZP()
	{
		Registers::PC += 2;
		cycles += 3;

		if(dmcDelayCycle < cycles)
			APU::LoadDMCSample();
	}

	// NOP - Zero Page, X
	static void NOPZPX()
	{
		Registers::PC += 2;
		cycles += 6;

		if(dmcDelayCycle < cycles)
			APU::LoadDMCSample();
	}

	// NOP - Absolute
	static void NOPABS()
	{
		// Useless read...
		Memory::Read(Memory::Absolute());

		Registers::PC += 3;
	}

	// NOP - Ansolute, X
	static void NOPABSX()
	{
		// Useless read...
		Memory::Read(Memory::AbsoluteX());

		Registers::PC += 3;
	}

	// Opcode inválido. Fazer hang da emulação (Instrução KIL)
	static void Invalid()
	{
		Interrupts::NMIEnabled = false;
		Interrupts::IRQEnabled = false;
	}

}; 
// -----------------------------------------------------------------------------
// TABELA DE OPCDES ------------------------------------------------------------
// -----------------------------------------------------------------------------

void (*CPU::OpcodeTable[256])() =
{
	Opcodes::op0x00,  Opcodes::op0x01, Opcodes::Invalid, Opcodes::Uop0x03,
	Opcodes::NOPZP,   Opcodes::op0x05, Opcodes::op0x06,  Opcodes::Uop0x07,
	Opcodes::op0x08,  Opcodes::op0x09, Opcodes::op0x0A,  Opcodes::Uop0x0B,
	Opcodes::NOPABS,  Opcodes::op0x0D, Opcodes::op0x0E,  Opcodes::Uop0x0F,

	Opcodes::op0x10,  Opcodes::op0x11, Opcodes::Invalid, Opcodes::Uop0x13,
	Opcodes::NOPZPX,  Opcodes::op0x15, Opcodes::op0x16,  Opcodes::Uop0x17,
	Opcodes::op0x18,  Opcodes::op0x19, Opcodes::NOP,     Opcodes::Uop0x1B,
	Opcodes::NOPABSX, Opcodes::op0x1D, Opcodes::op0x1E,  Opcodes::Uop0x1F,

	Opcodes::op0x20,  Opcodes::op0x21, Opcodes::Invalid, Opcodes::Uop0x23,
	Opcodes::op0x24,  Opcodes::op0x25, Opcodes::op0x26,  Opcodes::Uop0x27,
	Opcodes::op0x28,  Opcodes::op0x29, Opcodes::op0x2A,  Opcodes::Uop0x2B,
	Opcodes::op0x2C,  Opcodes::op0x2D, Opcodes::op0x2E,  Opcodes::Uop0x2F,

	Opcodes::op0x30,  Opcodes::op0x31, Opcodes::Invalid, Opcodes::Uop0x33,
	Opcodes::NOPZPX,  Opcodes::op0x35, Opcodes::op0x36,  Opcodes::Uop0x37,
	Opcodes::op0x38,  Opcodes::op0x39, Opcodes::NOP,     Opcodes::Uop0x3B,
	Opcodes::NOPABSX, Opcodes::op0x3D, Opcodes::op0x3E,  Opcodes::Uop0x3F,

	Opcodes::op0x40,  Opcodes::op0x41, Opcodes::Invalid, Opcodes::Uop0x43,
	Opcodes::NOPZP,   Opcodes::op0x45, Opcodes::op0x46,  Opcodes::Uop0x47,
	Opcodes::op0x48,  Opcodes::op0x49, Opcodes::op0x4A,  Opcodes::Uop0x4B,
	Opcodes::op0x4C,  Opcodes::op0x4D, Opcodes::op0x4E,  Opcodes::Uop0x4F,

	Opcodes::op0x50,  Opcodes::op0x51, Opcodes::Invalid, Opcodes::Uop0x53,
	Opcodes::NOPZPX,  Opcodes::op0x55, Opcodes::op0x56,  Opcodes::Uop0x57,
	Opcodes::op0x58,  Opcodes::op0x59, Opcodes::NOP,     Opcodes::Uop0x5B,
	Opcodes::NOPABSX, Opcodes::op0x5D, Opcodes::op0x5E,  Opcodes::Uop0x5F,

	Opcodes::op0x60,  Opcodes::op0x61, Opcodes::Invalid, Opcodes::Uop0x63,
	Opcodes::NOPZP,   Opcodes::op0x65, Opcodes::op0x66,  Opcodes::Uop0x67,
	Opcodes::op0x68,  Opcodes::op0x69, Opcodes::op0x6A,  Opcodes::Uop0x6B,
	Opcodes::op0x6C,  Opcodes::op0x6D, Opcodes::op0x6E,  Opcodes::Uop0x6F,

	Opcodes::op0x70,  Opcodes::op0x71, Opcodes::Invalid, Opcodes::Uop0x73,
	Opcodes::NOPZPX,  Opcodes::op0x75, Opcodes::op0x76,  Opcodes::Uop0x77,
	Opcodes::op0x78,  Opcodes::op0x79, Opcodes::NOP,     Opcodes::Uop0x7B,
	Opcodes::NOPABSX, Opcodes::op0x7D, Opcodes::op0x7E,  Opcodes::Uop0x7F,

	Opcodes::NOPIMM,  Opcodes::op0x81, Opcodes::NOPIMM,  Opcodes::Uop0x83,
	Opcodes::op0x84,  Opcodes::op0x85, Opcodes::op0x86,  Opcodes::Uop0x87,
	Opcodes::op0x88,  Opcodes::NOPIMM, Opcodes::op0x8A,  Opcodes::Uop0x8B,
	Opcodes::op0x8C,  Opcodes::op0x8D, Opcodes::op0x8E,  Opcodes::Uop0x8F,

	Opcodes::op0x90,  Opcodes::op0x91, Opcodes::Invalid, Opcodes::Uop0x93,
	Opcodes::op0x94,  Opcodes::op0x95, Opcodes::op0x96,  Opcodes::Uop0x97,
	Opcodes::op0x98,  Opcodes::op0x99, Opcodes::op0x9A,  Opcodes::Uop0x9B,
	Opcodes::Uop0x9C, Opcodes::op0x9D, Opcodes::Uop0x9E, Opcodes::Uop0x9F,

	Opcodes::op0xA0,  Opcodes::op0xA1, Opcodes::op0xA2,  Opcodes::Uop0xA3,
	Opcodes::op0xA4,  Opcodes::op0xA5, Opcodes::op0xA6,  Opcodes::Uop0xA7,
	Opcodes::op0xA8,  Opcodes::op0xA9, Opcodes::op0xAA,  Opcodes::Uop0xAB,
	Opcodes::op0xAC,  Opcodes::op0xAD, Opcodes::op0xAE,  Opcodes::Uop0xAF,

	Opcodes::op0xB0,  Opcodes::op0xB1, Opcodes::Invalid, Opcodes::Uop0xB3,
	Opcodes::op0xB4,  Opcodes::op0xB5, Opcodes::op0xB6,  Opcodes::Uop0xB7,
	Opcodes::op0xB8,  Opcodes::op0xB9, Opcodes::op0xBA,  Opcodes::Uop0xBB,
	Opcodes::op0xBC,  Opcodes::op0xBD, Opcodes::op0xBE,  Opcodes::Uop0xBF,

	Opcodes::op0xC0,  Opcodes::op0xC1, Opcodes::NOPIMM,  Opcodes::Uop0xC3,
	Opcodes::op0xC4,  Opcodes::op0xC5, Opcodes::op0xC6,  Opcodes::Uop0xC7,
	Opcodes::op0xC8,  Opcodes::op0xC9, Opcodes::op0xCA,  Opcodes::Uop0xCB,
	Opcodes::op0xCC,  Opcodes::op0xCD, Opcodes::op0xCE,  Opcodes::Uop0xCF,

	Opcodes::op0xD0,  Opcodes::op0xD1, Opcodes::Invalid, Opcodes::Uop0xD3,
	Opcodes::NOPZPX,  Opcodes::op0xD5, Opcodes::op0xD6,  Opcodes::Uop0xD7,
	Opcodes::op0xD8,  Opcodes::op0xD9, Opcodes::NOP,     Opcodes::Uop0xDB,
	Opcodes::NOPABSX, Opcodes::op0xDD, Opcodes::op0xDE,  Opcodes::Uop0xDF,

	Opcodes::op0xE0,  Opcodes::op0xE1, Opcodes::NOPIMM,  Opcodes::Uop0xE3,
	Opcodes::op0xE4,  Opcodes::op0xE5, Opcodes::op0xE6,  Opcodes::Uop0xE7,
	Opcodes::op0xE8,  Opcodes::op0xE9, Opcodes::NOP,     Opcodes::op0xE9,
	Opcodes::op0xEC,  Opcodes::op0xED, Opcodes::op0xEE,  Opcodes::Uop0xEF,

	Opcodes::op0xF0,  Opcodes::op0xF1, Opcodes::Invalid, Opcodes::Uop0xF3,
	Opcodes::NOPZPX,  Opcodes::op0xF5, Opcodes::op0xF6,  Opcodes::Uop0xF7,
	Opcodes::op0xF8,  Opcodes::op0xF9, Opcodes::NOP,     Opcodes::Uop0xFB,
	Opcodes::NOPABSX, Opcodes::op0xFD, Opcodes::op0xFE,  Opcodes::Uop0xFF
};