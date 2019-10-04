// FrameSequencer.cpp
// Tratamento de Frame Sequencer

#include <cstdio>

#define FRAMESEQUENCER_CPP

#include "APU.h"
#include "Internal.h"
#include "FrameSequencer.h"

#include "../CPU/Internal.h"
#include "../CPU/Interrupts.h"
#include "../CPU/Registers.h"

// Define o modo de contagem de tempos do APU
void APU::FrameSequencer::SetMode(unsigned char value)
{
	Update();

	LastValue = value;

	// Reset ao clock de APU interno
	Steps = 4 + ((value & 0x80) >> 7);

	register int extraCycle = ((CPU::evenOddTotalCycles ^ (((int) ((CPU::cycles + 3 - CPU::startCycle) / 3)) & 1)) * 3);

	if(Steps == 5)
	{
		StepCycles[0] = CPU::cycles   + 9     - extraCycle;
		StepCycles[1] = StepCycles[0] + 22374;
		StepCycles[2] = StepCycles[1] + 22368;
		StepCycles[3] = StepCycles[2] + 22374;
		StepCycles[4] = StepCycles[3] + 22368;
		CyclesToAdd = 111846;
	}

	else
	{
		// Calcular clocks
		StepCycles[0] = CPU::cycles   + 22383 - extraCycle;
		StepCycles[1] = StepCycles[0] + 22368;
		StepCycles[2] = StepCycles[1] + 22374;
		StepCycles[3] = StepCycles[2] + 22374;
		StepCycles[4] = StepCycles[3] + 22362;
		CyclesToAdd = 89490;
	}

	LastStep = 0;
	AddEvent(FRAMESEQUENCER, FrameSequencer::StepCycles[0]);
	SetNextEvent();

	// Se pedido, lançar IRQ
	if(!(value & 0xC0))
	{
		InternalIRQCycle = CPU::cycles + 89502 - extraCycle;

		if(!IRQPending)
			CPU::Interrupts::SetIRQ(CPU::Interrupts::APU_FRAME, InternalIRQCycle);

		ExtraIRQCycles = 6;
	}
	else
	{
		if(value & 0x40)
			IRQPending = false;

		InternalIRQCycle = 200000;
		CPU::Interrupts::UnsetIRQ(CPU::Interrupts::APU_FRAME);
	}
}

// Faz clock do sequencer
void APU::FrameSequencer::Clock()
{
	if(Steps == 4)
	{
		switch(LastStep)
		{
			case 0: NoiseChannel::ClockEnvelope(); SquareChannel::ClockEnvelopes();  TriangleChannel::ClockLinearCounter(); break;
			case 1: NoiseChannel::ClockEnvelope(); SquareChannel::ClockSweepUnits(); ClockLengthCounters(); SquareChannel::ClockEnvelopes(); TriangleChannel::ClockLinearCounter(); break;
			case 2: NoiseChannel::ClockEnvelope(); SquareChannel::ClockEnvelopes();  TriangleChannel::ClockLinearCounter(); break;
			case 3: NoiseChannel::ClockEnvelope(); SquareChannel::ClockSweepUnits(); ClockLengthCounters(); SquareChannel::ClockEnvelopes(); TriangleChannel::ClockLinearCounter(); break;
		}
	}
	else
	{
		switch(LastStep)
		{
			case 0: NoiseChannel::ClockEnvelope(); SquareChannel::ClockSweepUnits(); ClockLengthCounters(); SquareChannel::ClockEnvelopes(); TriangleChannel::ClockLinearCounter(); break;
			case 1: NoiseChannel::ClockEnvelope(); SquareChannel::ClockEnvelopes();  TriangleChannel::ClockLinearCounter(); break;
			case 2: NoiseChannel::ClockEnvelope(); SquareChannel::ClockSweepUnits(); ClockLengthCounters(); SquareChannel::ClockEnvelopes(); TriangleChannel::ClockLinearCounter(); break;
			case 3: NoiseChannel::ClockEnvelope(); SquareChannel::ClockEnvelopes();  TriangleChannel::ClockLinearCounter(); break;
		}
	}

	StepCycles[LastStep] += CyclesToAdd;
	++LastStep;

	if(LastStep == Steps)
		LastStep = 0;
}

// Indica se há um IRQ pendente
int APU::FrameSequencer::IRQStatus()
{
	// Se estivermos a menos de dois ciclos do IRQ, retornar true e não colocar a flag a 0
	if(!IRQPending && ((InternalIRQCycle - CPU::cycles) < 9))
	{
		if((InternalIRQCycle - CPU::cycles) < 3)
		{
			if(DontEnablePending)
			{
				return 0;
			}
			else
			{
				DontEnablePending = true;
				IRQPending = false;
			}
		}

		return 1;
	}

	int ret = IRQPending;
	IRQPending = false;

	// Preparar CPU para o próximo IRQ
	if(ret)
		CPU::Interrupts::SetIRQ(CPU::Interrupts::APU_FRAME, InternalIRQCycle);

	return ret;
}

// Notifica que houve um Frame IRQ
bool APU::FrameSequencer::NotifyInterrupt()
{
	if(InternalIRQCycle <= CPU::cycles)
		InternalIRQCycle += 89490 + ExtraIRQCycles;

	// Após o primeiro hit, os IRQs seguintes têm menos seis ciclos
	ExtraIRQCycles = 0;

	// Se não há IRQ pendente, preparar o próximo IRQ
	if(!IRQPending)
	{
		if(!DontEnablePending)
			IRQPending = true;

		else
			CPU::Interrupts::SetIRQ(CPU::Interrupts::APU_FRAME, InternalIRQCycle);
		
		DontEnablePending = false;
	}

	return true;
}
