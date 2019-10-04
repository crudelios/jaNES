// Interrupts.cpp
// Classe de interrupts

#define INTERRUPTS_CPP

#include "Interrupts.h"
#include "Memory.h"
#include "../PPU/PPU.h"
#include "../APU/APU.h"

void CPU::Interrupts::Handle()
{
	if(Reset)
	{
		Reset = false;

		// Matar os interrupts
		NMIEnabled = false;
		IRQEnabled = true;
		IRQPending = false;

		// Limpar o frame
		PPU::ClearFrame();

		// Reiniciar ciclo
		cycles = 9;

		// Fazer reset ao program counter
		Registers::PC = (Memory::Read(0xFFFD) << 8) | Memory::Read(0xFFFC);
		Registers::SP -= 3;

		Registers::P::I = 1;

		// Calar o audio
		Memory::Write(0x4015, 0);

		return;
	}

	if(IRQHit())
	{
		Stack::Push((unsigned char) (Registers::PC >> 8));
		Stack::Push((unsigned char) (Registers::PC & 0xFF));
		Stack::Push(Registers::P::Pack());

		cycles += 12;

		if(NMIHit())
		{
			Registers::PC = (Memory::Read(0xFFFB) << 8) | Memory::Read(0xFFFA);
			IRQPending = true;
			NMIThisFrame = true;

			cycles += 3;
			return;
		}

		Registers::P::I = 1;
		IRQActive = 1;
		Registers::PC = (Memory::Read(0xFFFF) << 8) | Memory::Read(0xFFFE);
		cycles += 9;

		return;
	}
}

// Dá início a um interrupt NMI
void CPU::Interrupts::StartNMI()
{
	if(NMIThisFrame)
	{
		NMIThisFrame = false;
		return;
	}

	if(dmcDelayCycle < cycles)
		APU::LoadDMCSample(12 - (cycles - dmcDelayCycle));

	NMIPending = 0;

	Stack::Push((unsigned char) (Registers::PC >> 8));
	Stack::Push((unsigned char) (Registers::PC & 0xFF));
	Stack::Push(Registers::P::Pack());

	cycles += 9;

	Registers::PC = (Memory::Read(0xFFFB) << 8);

	if(IRQHit())
	{
		Registers::P::I = 1;
		Interrupts::IRQActive = 1;
		IRQPending = true;
	}

	Registers::PC |= Memory::Read(0xFFFA);

	cycles += 6;

	if(dmcDelayCycle < cycles)
		APU::LoadDMCSample();

	InNMI = true;

	return;
}

// Dá início a um interrupt IRQ
void CPU::Interrupts::StartIRQ()
{
	if(dmcDelayCycle < cycles)
		APU::LoadDMCSample(12 - (cycles - dmcDelayCycle));

	// Guardar o tipo de IRQ actual
	IRQType type = NextIRQType;

	// Obter já os dados do IRQ seguinte
	Interrupts::SetNextIRQ();

	// Notificar o mapper do interrupt caso tenha partido de lá
	if(type == MAPPER)
	{
		// Se o mapper retornar false, não se procede a interrupt
		if(!Memory::mapper->NotifyInterrupt())
			return;
	}

	// Interrupt de APU, notificar
	else
	{
		// Se o mapper retornar false, não se procede a interrupt
		if(!APU::NotifyInterrupt(type))
			return;
	}

	if(IRQActive || !IRQEnabled)
		return;

	cycles += 6;

	if(dmcDelayCycle < cycles)
		APU::LoadDMCSample();

	// Preparar o IRQ
	Stack::Push((unsigned char) (Registers::PC >> 8));
	Stack::Push((unsigned char) (Registers::PC & 0xFF));
	Stack::Push(Registers::P::Pack());

	cycles += 9;

	if(dmcDelayCycle < cycles)
		APU::LoadDMCSample(12 - (cycles - dmcDelayCycle));

	Registers::P::I = 1;
	IRQActive = 1;

	// Surgiu NMI!
	if(NMIHit())
	{
		Registers::PC = (Memory::Read(0xFFFB) << 8) | Memory::Read(0xFFFA);
		IRQPending = true;
		NMIThisFrame = true;

		return;
	}

	Registers::PC = (Memory::Read(0xFFFF) << 8) | Memory::Read(0xFFFE);
	cycles += 6;

	if(dmcDelayCycle < cycles)
		APU::LoadDMCSample();
}
