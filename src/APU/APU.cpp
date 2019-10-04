// APU.cpp
// Ficheiro principal de emulação do APU

#define APU_CPP

#include <algorithm>

#include "APU.h"
#include "Internal.h"
#include "FrameSequencer.h"
#include "Output.h"

#include "../CPU/Memory.h"

// Prepara os dados para utilização
void APU::Prepare()
{
	// Inicializar dados do APU
	InternalCycle  = 0;

	// Inicializar frame counter
	FrameSequencer::InternalIRQCycle  = 89466;
	FrameSequencer::IRQPending        = false;
	FrameSequencer::DontEnablePending = false;
	FrameSequencer::ExtraIRQCycles    = 6;
	FrameSequencer::Steps             = 4;
	FrameSequencer::LastStep          = 0;
	FrameSequencer::CyclesToAdd       = 89490;

	// Iniciar clocks internos
	FrameSequencer::StepCycles[0] = 22347;
	FrameSequencer::StepCycles[1] = 44715;
	FrameSequencer::StepCycles[2] = 67089;
	FrameSequencer::StepCycles[3] = 89463;
	FrameSequencer::StepCycles[4] = 111825;


	// --------------------
	// Inicializar channels
	// --------------------

	// Square channel 1
	SquareChannel::Data[0].lengthCounter            = 0;
	SquareChannel::Data[0].dutyCycle                = 0;
	SquareChannel::Data[0].dutyCyclePos             = 0;
	SquareChannel::Data[0].active                   = false;
	SquareChannel::Data[0].halted                   = false;
	SquareChannel::Data[0].DAC                      = 0;
	SquareChannel::Data[0].timer                    = 0;
	SquareChannel::Data[0].timerInternal            = 2;
	SquareChannel::Data[0].constant                 = 0;
	SquareChannel::Data[0].start                    = false;
	SquareChannel::Data[0].volume                   = 0;
	SquareChannel::Data[0].volCycle                 = 0xF;
	SquareChannel::Data[0].volDecay                 = 1;
	SquareChannel::Data[0].volDecayInternal         = 1;
	SquareChannel::Data[0].sweepOn                  = false;
	SquareChannel::Data[0].sweepReset               = false;
	SquareChannel::Data[0].sweepStopped             = false;
	SquareChannel::Data[0].sweepRefreshRate         = 1;
	SquareChannel::Data[0].sweepRefreshRateInternal = 1;
	SquareChannel::Data[0].sweepDecreaseWave        = false;
	SquareChannel::Data[0].sweepRightShift          = 0;

	// Square channel 2
	SquareChannel::Data[1].lengthCounter            = 0;
	SquareChannel::Data[1].dutyCycle                = 0;
	SquareChannel::Data[1].dutyCyclePos             = 0;
	SquareChannel::Data[1].active                   = false;
	SquareChannel::Data[1].halted                   = false;
	SquareChannel::Data[1].DAC                      = 0;
	SquareChannel::Data[1].timer                    = 0;
	SquareChannel::Data[1].timerInternal            = 2;
	SquareChannel::Data[1].constant                 = 0;
	SquareChannel::Data[1].start                    = false;
	SquareChannel::Data[1].volume                   = 0;
	SquareChannel::Data[1].volCycle                 = 0xF;
	SquareChannel::Data[1].volDecay                 = 1;
	SquareChannel::Data[1].volDecayInternal         = 1;
	SquareChannel::Data[1].sweepOn                  = false;
	SquareChannel::Data[1].sweepReset               = false;
	SquareChannel::Data[1].sweepStopped             = false;
	SquareChannel::Data[1].sweepRefreshRate         = 1;
	SquareChannel::Data[1].sweepRefreshRateInternal = 1;
	SquareChannel::Data[1].sweepDecreaseWave        = false;
	SquareChannel::Data[1].sweepRightShift          = 0;

	// Triangle channel
	TriangleChannel::Timer             = 0;
	TriangleChannel::TimerInternal     = 1;
	TriangleChannel::LengthCounter     = 0;
	TriangleChannel::LinearCounter     = 0;
	TriangleChannel::LinearReloadValue = 0;
	TriangleChannel::WavePos           = 0;
	TriangleChannel::LinearHalted      = false;
	TriangleChannel::Active            = false;
	TriangleChannel::Halted            = false;
	TriangleChannel::DAC               = TriangleChannel::WaveValues[TriangleChannel::WavePos];

	// Noise Channel
	NoiseChannel::ShiftRegister    = 1;
	NoiseChannel::BitMode          = 1;
	NoiseChannel::Timer            = NoiseChannel::TimerList[0];
	NoiseChannel::TimerInternal    = NoiseChannel::TimerList[0];
	NoiseChannel::LengthCounter    = 0;
	NoiseChannel::Active           = false;
	NoiseChannel::Halted           = false;
	NoiseChannel::DAC              = 0;
	NoiseChannel::Constant         = 0;
	NoiseChannel::Start            = false;
	NoiseChannel::Volume           = 0;
	NoiseChannel::VolCycle         = 0x1E;
	NoiseChannel::VolDecay         = 1;
	NoiseChannel::VolDecayInternal = 1;

	//DMC Channel
	DMCChannel::Active           = false;
	DMCChannel::IRQEnabled       = 0;
	DMCChannel::IRQPending       = 0;
	DMCChannel::InternalIRQCycle = 200000;
	DMCChannel::Timer            = DMCChannel::TimerList[0];
	DMCChannel::TimerInternal    = DMCChannel::TimerList[0];
	DMCChannel::Loop             = 0;
	DMCChannel::Buffer           = 0;
	DMCChannel::Empty            = true;
	DMCChannel::Addr             = 0xC000;
	DMCChannel::AddrInternal     = 0xC000;
	DMCChannel::Bytes            = 1;
	DMCChannel::Remaining        = 0;
	DMCChannel::Restart          = false;
	DMCChannel::Bit              = 7;
	DMCChannel::OutputSample     = 0;
	DMCChannel::Silence          = true;
	DMCChannel::Counter          = 0;

	CPU::dmcDelayCycle           = 24 * DMCChannel::Timer;

	// Preparar IRQs
	CPU::Interrupts::SetIRQ(CPU::Interrupts::APU_FRAME, FrameSequencer::InternalIRQCycle);

	// Preparar eventos
	std::fill_n(EventList, 6, 200000);
	std::fill_n(LastCycle, 6, 0);

	NextEventList[FRAMESEQUENCER] = 0;
	NextEventList[SQUARE1]        = SquareChannel::Data[0].timerInternal * 3;
	NextEventList[SQUARE2]        = SquareChannel::Data[1].timerInternal * 3;
	NextEventList[TRIANGLE]       = TriangleChannel::TimerInternal       * 3;
	NextEventList[NOISE]          = NoiseChannel::TimerInternal          * 3;
	NextEventList[DMC]            = DMCChannel::TimerInternal            * 3;

	AddEvent(FRAMESEQUENCER, FrameSequencer::StepCycles[0]);
	//AddEvent(SQUARE1,        SquareChannel::Data[0].timerInternal * 3);
	//AddEvent(SQUARE2,        SquareChannel::Data[1].timerInternal * 3);
	////AddEvent(TRIANGLE,       TriangleChannel::TimerInternal       * 3);
	////AddEvent(NOISE,          NoiseChannel::TimerInternal          * 3);
	//AddEvent(DMC,            DMCChannel::TimerInternal            * 3);

	SetNextEvent();

	// Preparar output
	Output::Initialize();
}

// Pausa o som
void APU::Pause()
{
	Output::Pause();
}

// Destrói o APU
void APU::Destroy()
{
	Output::Destroy();
}

// Resets the APU
void APU::Reset()
{
	Update();
	std::fill_n(EventList, 6, 200000);

	FrameSequencer::IRQPending = false;
	FrameSequencer::DontEnablePending = false;

	FrameSequencer::SetMode(FrameSequencer::LastValue);

	// --------------------
	// Inicializar channels
	// --------------------

	// Square channel 1
	SquareChannel::Data[0].lengthCounter            = 0;
	SquareChannel::Data[0].dutyCycle                = 0;
	SquareChannel::Data[0].dutyCyclePos             = 0;
	SquareChannel::Data[0].active                   = false;
	SquareChannel::Data[0].halted                   = false;
	SquareChannel::Data[0].DAC                      = 0;
	SquareChannel::Data[0].timer                    = 0;
	SquareChannel::Data[0].timerInternal            = 2;
	SquareChannel::Data[0].constant                 = 0;
	SquareChannel::Data[0].start                    = false;
	SquareChannel::Data[0].volume                   = 0;
	SquareChannel::Data[0].volCycle                 = 0xF;
	SquareChannel::Data[0].volDecay                 = 1;
	SquareChannel::Data[0].volDecayInternal         = 1;
	SquareChannel::Data[0].sweepOn                  = false;
	SquareChannel::Data[0].sweepReset               = false;
	SquareChannel::Data[0].sweepStopped             = false;
	SquareChannel::Data[0].sweepRefreshRate         = 1;
	SquareChannel::Data[0].sweepRefreshRateInternal = 1;
	SquareChannel::Data[0].sweepDecreaseWave        = false;
	SquareChannel::Data[0].sweepRightShift          = 0;

	// Square channel 2
	SquareChannel::Data[1].lengthCounter            = 0;
	SquareChannel::Data[1].dutyCycle                = 0;
	SquareChannel::Data[1].dutyCyclePos             = 0;
	SquareChannel::Data[1].active                   = false;
	SquareChannel::Data[1].halted                   = false;
	SquareChannel::Data[1].DAC                      = 0;
	SquareChannel::Data[1].timer                    = 0;
	SquareChannel::Data[1].timerInternal            = 2;
	SquareChannel::Data[1].constant                 = 0;
	SquareChannel::Data[1].start                    = false;
	SquareChannel::Data[1].volume                   = 0;
	SquareChannel::Data[1].volCycle                 = 0xF;
	SquareChannel::Data[1].volDecay                 = 1;
	SquareChannel::Data[1].volDecayInternal         = 1;
	SquareChannel::Data[1].sweepOn                  = false;
	SquareChannel::Data[1].sweepReset               = false;
	SquareChannel::Data[1].sweepStopped             = false;
	SquareChannel::Data[1].sweepRefreshRate         = 1;
	SquareChannel::Data[1].sweepRefreshRateInternal = 1;
	SquareChannel::Data[1].sweepDecreaseWave        = false;
	SquareChannel::Data[1].sweepRightShift          = 0;

	// Triangle channel
	//TriangleChannel::Timer             = 0;
	//TriangleChannel::TimerInternal     = 1;
	//TriangleChannel::LengthCounter     = 0;
	TriangleChannel::LinearCounter     = 0;
	TriangleChannel::LinearReloadValue = 0;
	TriangleChannel::WavePos           = 0;
	TriangleChannel::LinearHalted      = false;
	TriangleChannel::Active            = false;
	//TriangleChannel::Halted            = false;
	TriangleChannel::DAC               = TriangleChannel::WaveValues[TriangleChannel::WavePos];

	// Noise Channel
	NoiseChannel::ShiftRegister    = 1;
	NoiseChannel::BitMode          = 1;
	NoiseChannel::Timer            = NoiseChannel::TimerList[0];
	NoiseChannel::TimerInternal    = NoiseChannel::TimerList[0];
	NoiseChannel::LengthCounter    = 0;
	NoiseChannel::Active           = false;
	NoiseChannel::Halted           = false;
	NoiseChannel::DAC              = 0;
	NoiseChannel::Constant         = 0;
	NoiseChannel::Start            = false;
	NoiseChannel::Volume           = 0;
	NoiseChannel::VolCycle         = 0x1E;
	NoiseChannel::VolDecay         = 1;
	NoiseChannel::VolDecayInternal = 1;

	//DMC Channel
	DMCChannel::Active           = false;
	DMCChannel::IRQEnabled       = 0;
	DMCChannel::IRQPending       = 0;
	DMCChannel::InternalIRQCycle = 200000;
	DMCChannel::Timer            = DMCChannel::TimerList[0];
	DMCChannel::TimerInternal    = DMCChannel::TimerList[0];
	DMCChannel::Loop             = 0;
	DMCChannel::Buffer           = 0;
	DMCChannel::Empty            = true;
	DMCChannel::Addr             = 0xC000;
	DMCChannel::AddrInternal     = 0xC000;
	DMCChannel::Bytes            = 1;
	DMCChannel::Remaining        = 0;
	DMCChannel::Restart          = false;
	DMCChannel::Bit              = 7;
	DMCChannel::OutputSample     = 0;
	DMCChannel::Silence          = true;
	DMCChannel::Counter          = 0;

	CPU::dmcDelayCycle           = 24 * DMCChannel::Timer;

	std::fill_n(LastCycle, 6, 0);

	NextEventList[FRAMESEQUENCER] = 0;
	NextEventList[SQUARE1]        = SquareChannel::Data[0].timerInternal * 3;
	NextEventList[SQUARE2]        = SquareChannel::Data[1].timerInternal * 3;
	NextEventList[TRIANGLE]       = TriangleChannel::TimerInternal       * 3;
	NextEventList[NOISE]          = NoiseChannel::TimerInternal          * 3;
	NextEventList[DMC]            = DMCChannel::TimerInternal            * 3;
}



// ---------------------------------------------------------------------------------
// Actualiza o estado do APU -------------------------------------------------------
// ---------------------------------------------------------------------------------

namespace APU
{
	void UpdateOld();
}


void APU::Update()
{
	while(CPU::cycles >= NextEventCycle)
	{
		if(NextEventCycle == EventList[FRAMESEQUENCER])
		{
			FrameSequencer::Clock();
			EventList[FRAMESEQUENCER] = FrameSequencer::StepCycles[FrameSequencer::LastStep];
		}

		if(NextEventCycle == EventList[SQUARE1])
		{
			++SquareChannel::Data[0].dutyCyclePos;
			SquareChannel::Data[0].dutyCyclePos &= 7;

			LastCycle[SQUARE1] = EventList[SQUARE1];
			EventList[SQUARE1] += (((SquareChannel::Data[0].timer + 1) << 1) * 3);
			NextEventList[SQUARE1] = EventList[SQUARE1];

			if(!SquareChannel::Data[0].lengthCounter)
				EventList[SQUARE1] = 200000;

			SquareChannel::Data[0].DAC = (SquareChannel::Data[0].lengthCounter && !SquareChannel::Data[0].sweepStopped) ? (((SquareChannel::Data[0].constant) ? SquareChannel::Data[0].volume : SquareChannel::Data[0].volCycle) * SquareChannel::DutyCycle[SquareChannel::Data[0].dutyCycle][SquareChannel::Data[0].dutyCyclePos]) : 0;
		}

		if(NextEventCycle == EventList[SQUARE2])
		{
			++SquareChannel::Data[1].dutyCyclePos;
			SquareChannel::Data[1].dutyCyclePos &= 7;

			LastCycle[SQUARE2] = EventList[SQUARE2];
			EventList[SQUARE2] += (((SquareChannel::Data[1].timer + 1) << 1) * 3);
			NextEventList[SQUARE2] = EventList[SQUARE2];

			//if(!SquareChannel::Data[1].lengthCounter)
			//	EventList[SQUARE2] = 200000;

			SquareChannel::Data[1].DAC = (SquareChannel::Data[1].lengthCounter && !SquareChannel::Data[1].sweepStopped) ? (((SquareChannel::Data[1].constant) ? SquareChannel::Data[1].volume : SquareChannel::Data[1].volCycle) * SquareChannel::DutyCycle[SquareChannel::Data[1].dutyCycle][SquareChannel::Data[1].dutyCyclePos]) : 0;
		}

		if(NextEventCycle == EventList[TRIANGLE])
		{
			if(TriangleChannel::LinearCounter && TriangleChannel::LengthCounter)
			{
				++TriangleChannel::WavePos;
				TriangleChannel::WavePos &= 0x1F;

				TriangleChannel::DAC = TriangleChannel::WaveValues[TriangleChannel::WavePos];
			}
			LastCycle[TRIANGLE] = EventList[TRIANGLE];
			EventList[TRIANGLE] += ((TriangleChannel::Timer + 1) * 3);
			NextEventList[TRIANGLE] = EventList[TRIANGLE];

			if(!TriangleChannel::LengthCounter)
				EventList[TRIANGLE] = 200000;
		}

		if(NextEventCycle == EventList[NOISE])
		{
			int feedback = (NoiseChannel::ShiftRegister ^ (NoiseChannel::ShiftRegister >> NoiseChannel::BitMode)) & 1;
			NoiseChannel::ShiftRegister >>= 1;
			NoiseChannel::ShiftRegister |= feedback << 14;

			LastCycle[NOISE] = EventList[NOISE];
			EventList[NOISE] += NoiseChannel::Timer * 3;

			NoiseChannel::DAC = (NoiseChannel::LengthCounter && !(NoiseChannel::ShiftRegister & 1)) ? ((NoiseChannel::Constant) ? NoiseChannel::Volume : NoiseChannel::VolCycle) : 0;
		}

		if(NextEventCycle == EventList[DMC])
		{
			if(!DMCChannel::Silence)
			{
				int bit = (DMCChannel::OutputSample >> (7 - DMCChannel::Bit)) & 1;

				if(!bit && (DMCChannel::Counter > 1))
					DMCChannel::Counter -= 2;

				if(bit && (DMCChannel::Counter < 126))
					DMCChannel::Counter += 2;
			}

			if(!DMCChannel::Bit--)
			{
				// Load sample
				DMCChannel::Bit = 7;

				if(!DMCChannel::Empty)
				{
					DMCChannel::Silence = false;
					DMCChannel::OutputSample = DMCChannel::Buffer;
				}

				else
					DMCChannel::Silence = true;
			}
			LastCycle[DMC] = EventList[DMC];
			EventList[DMC] += DMCChannel::Timer * 3;

			NextEventList[DMC] = EventList[DMC];

			//if(DMCChannel::Silence)
			//	EventList[DMC] = 200000;
		}

		Output::AddSample(NextEventCycle);

		SetNextEvent();
	}

	// Fazer contagem dos inactivos
	if(EventList[SQUARE1] == 200000)
	{
		// Calcular último evento
		register int timer = (((SquareChannel::Data[0].timer + 1) << 1) * 3);
		int numtimes = (CPU::cycles - LastCycle[SQUARE1]) / timer;
		LastCycle[SQUARE1] += timer * numtimes;
		NextEventList[SQUARE1] = LastCycle[SQUARE1] + timer;

		SquareChannel::Data[0].dutyCyclePos += numtimes;
		SquareChannel::Data[0].dutyCyclePos &= 7;
	}

	if(EventList[SQUARE2] == 200000)
	{
		// Calcular último evento
		register int timer = (((SquareChannel::Data[1].timer + 1) << 1) * 3);
		int numtimes = (CPU::cycles - LastCycle[SQUARE2]) / timer;
		LastCycle[SQUARE2] += timer * numtimes;
		NextEventList[SQUARE2] = LastCycle[SQUARE2] + timer;

		SquareChannel::Data[1].dutyCyclePos += numtimes;
		SquareChannel::Data[1].dutyCyclePos &= 7;
	}

	if(EventList[TRIANGLE] == 200000)
	{
		// Calcular último evento
		register int timer = ((TriangleChannel::Timer + 1) * 3);
		int numtimes = (CPU::cycles - LastCycle[TRIANGLE]) / timer;
		LastCycle[TRIANGLE] += timer * numtimes;
		NextEventList[TRIANGLE] = LastCycle[TRIANGLE] + timer;
	}

	if(!NoiseChannel::Active)
	{
		EventList[NOISE] = 200000;

		// Calcular último evento
		register int timer = NoiseChannel::Timer * 3;
		int numtimes = (CPU::cycles - LastCycle[NOISE]) / timer;
		LastCycle[NOISE] += timer * numtimes;

		for(int i = 0; i < numtimes; ++ i)
		{
			int feedback = (NoiseChannel::ShiftRegister ^ (NoiseChannel::ShiftRegister >> NoiseChannel::BitMode)) & 1;
			NoiseChannel::ShiftRegister >>= 1;
			NoiseChannel::ShiftRegister |= feedback << 14;
		}
	}

	if(EventList[DMC] == 200000)
	{
		// Calcular último evento
		register int timer = (DMCChannel::Timer * 3);
		int numtimes = (CPU::cycles - LastCycle[DMC]) / timer;
		LastCycle[DMC] += timer * numtimes;
		NextEventList[DMC] = LastCycle[DMC] + timer;
	}
}

void APU::UpdateOld()
{
	if(InternalCycle == 0)
		InternalCycle = CPU::startCycle + 3;

	// Loop principal do APU
	while(InternalCycle <= CPU::cycles)
	{
		// Fazer clock ao frame sequencer, caso seja aplicável
		if(FrameSequencer::StepCycles[FrameSequencer::LastStep] == InternalCycle)
			FrameSequencer::Clock();

		// Determinar valor dos squares
		//int square = GetSample(SquareChannel::Data[0]) + GetSample(SquareChannel::Data[1]);

		//// Determinar valor do triangle
		//if(!--TriangleChannel::TimerInternal)
		//{
		//	TriangleChannel::TimerInternal = TriangleChannel::Timer + 1;
		//	if(TriangleChannel::LinearCounter && TriangleChannel::LengthCounter)
		//	{
		//		++TriangleChannel::WavePos;
		//		TriangleChannel::WavePos &= 0x1F;
		//	}
		//}

		//int tnd = TriangleChannel::WaveValues[TriangleChannel::WavePos];

		//// Noise
		//if(!--NoiseChannel::TimerInternal)
		//{
		//	NoiseChannel::TimerInternal = NoiseChannel::Timer;
		//	int feedback = (NoiseChannel::ShiftRegister ^ (NoiseChannel::ShiftRegister >> NoiseChannel::BitMode)) & 1;
		//	NoiseChannel::ShiftRegister >>= 1;
		//	NoiseChannel::ShiftRegister |= feedback << 14;
		//}

		//tnd += (NoiseChannel::LengthCounter && !(NoiseChannel::ShiftRegister & 1)) ? ((NoiseChannel::Constant) ? NoiseChannel::Volume : NoiseChannel::VolCycle) : 0;

		//if(!--DMCChannel::TimerInternal)
		//{
		//	DMCChannel::TimerInternal = DMCChannel::Timer;

		//	if(!DMCChannel::Silence)
		//	{
		//		int bit = (DMCChannel::OutputSample >> (7 - DMCChannel::Bit)) & 1;

		//		if(!bit && (DMCChannel::Counter > 1))
		//			DMCChannel::Counter -= 2;

		//		if(bit && (DMCChannel::Counter < 126))
		//			DMCChannel::Counter += 2;
		//	}

		//	if(!DMCChannel::Bit--)
		//	{
		//		// Load sample
		//		DMCChannel::Bit = 7;

		//		if(!DMCChannel::Empty)
		//		{
		//			DMCChannel::Silence = false;
		//			DMCChannel::OutputSample = DMCChannel::Buffer;
		//		}

		//		else
		//			DMCChannel::Silence = true;
		//	}
		//}

		//// DMC
		//tnd += DMCChannel::Counter;

		//// Calcular alteração da onda sonora
		//Output::AddSample(InternalCycle, square, tnd);

		InternalCycle += 3;
	}
}



// ---------------------------------------------------------------------------------
// Square Channels -----------------------------------------------------------------
// ---------------------------------------------------------------------------------

// Register 0: Sequence, length counter, envelope
void APU::SquareChannel::SetReg0(unsigned int channel, unsigned char value)
{
	// Pequeno hack para contar devidamente o halt
	CPU::cycles += 3;
	Update();
	CPU::cycles -= 3;

	Data[channel].dutyCycle = (value >> 6);
	Data[channel].halted    = (value >> 5) & 1;
	Data[channel].constant  = (value >> 4) & 1;
	Data[channel].volDecay  = (value & 0xF) + 1;
	Data[channel].volume    = value & 0xF;

	CheckSweepStopped(channel);
}

// Register 1: Sweep
void APU::SquareChannel::SetReg1(unsigned int channel, unsigned char value)
{
	// Pequeno hack para contar devidamente o halt
	CPU::cycles += 3;
	Update();
	CPU::cycles -= 3;

	Data[channel].sweepOn           = value >> 7;
	Data[channel].sweepRefreshRate  = ((value >> 4) & 7) + 1;
	Data[channel].sweepDecreaseWave = (value >> 3) & 1;
	Data[channel].sweepRightShift   = value & 7;

	// Reload!
	Data[channel].sweepReset = true;

	CheckSweepStopped(channel);
}

// Register 2: Frequency LSB
void APU::SquareChannel::SetReg2(unsigned int channel, unsigned char value)
{
	// Pequeno hack para contar devidamente o halt
	CPU::cycles += 3;
	Update();
	CPU::cycles -= 3;

	Events mychan = (channel == 0) ? SQUARE1 : SQUARE2;
	EventList[mychan] = NextEventList[mychan];
	SetNextEvent();

	Data[channel].timer &= 0xFF00;
	Data[channel].timer |= value;

	CheckSweepStopped(channel);
}

// Register 3: Length counter timer, frequency MSB
void APU::SquareChannel::SetReg3(unsigned int channel, unsigned char value)
{
	int nextclock = FrameSequencer::StepCycles[FrameSequencer::LastStep];

	// Pequeno hack para contar devidamente o halt
	CPU::cycles += 3;
	Update();

	if(Data[channel].active && !((nextclock == CPU::cycles) && Data[channel].lengthCounter))
		Data[channel].lengthCounter = LengthIndex[value >> 3];

	Events mychan = (channel == 0) ? SQUARE1 : SQUARE2;
	EventList[mychan] = NextEventList[mychan];
	SetNextEvent();

	Data[channel].timer &= 0xFF;
	Data[channel].timer |= (value & 7) << 8;

	Data[channel].dutyCyclePos = 0;
	Data[channel].start        = true;

	CPU::cycles -= 3;

	CheckSweepStopped(channel);
}

// Faz clock dos sweep units
void APU::SquareChannel::ClockSweepUnit(int channel)
{
	// Fazer clock
	if(!--Data[channel].sweepRefreshRateInternal)
	{
		// Fazer reset ao valor se tiver chegado a zero
		Data[channel].sweepRefreshRateInternal = Data[channel].sweepRefreshRate;

		int newtimer;

		// Efectuar sweep
		if(!Data[channel].sweepDecreaseWave)
			newtimer = Data[channel].timer + (Data[channel].timer >> Data[channel].sweepRightShift);

		else
			newtimer = Data[channel].timer - (Data[channel].timer >> Data[channel].sweepRightShift) - (1 - channel);

		// Silenciar canal nos casos de serem excedidos valores
		if((newtimer > 0x7FF) || (Data[channel].timer < 8))
			Data[channel].sweepStopped = true;

		else
		{
			Data[channel].sweepStopped = false;

			// Nestes três casos não se efectua sweep
			if(Data[channel].sweepOn && Data[channel].sweepRightShift && Data[channel].lengthCounter)
			{
				Events mychan = (channel == 0) ? SQUARE1 : SQUARE2;
				EventList[mychan] = NextEventList[mychan];
				SetNextEvent();

				Data[channel].timer = newtimer;
			}

		}
	}
	if(Data[channel].sweepReset)
	{
		Data[channel].sweepReset = false;

		Data[channel].sweepRefreshRateInternal = Data[channel].sweepRefreshRate;
	}
}

// Verifica se devemos silenciar o sweep
void APU::SquareChannel::CheckSweepStopped(int channel)
{
	int offset = Data[channel].timer >> Data[channel].sweepRightShift; 
	Data[channel].sweepStopped = (Data[channel].timer < 8) || (!Data[channel].sweepDecreaseWave && ((Data[channel].timer + offset) >= 0x800)); 
}

// Faz clock dos envelopes
void APU::SquareChannel::ClockEnvelope(int channel)
{
	// Reiniciar contagem
	if(Data[channel].start)
	{
		Data[channel].start = false;
		Data[channel].volDecayInternal = Data[channel].volDecay;

		Data[channel].volCycle = 0xF;

		return;
	}

	// Reduzir o decay
	if(Data[channel].volDecayInternal)
	{
		--Data[channel].volDecayInternal;
		return;
	}

	// Se decay for 0, fazer reset e reduzir ao volume
	Data[channel].volDecayInternal = Data[channel].volDecay;

	// Reduzir o volume
	if(Data[channel].volCycle)
	{
		--Data[channel].volCycle;
		return;
	}

	// Só se faz reset ao volume se permitido
	if(Data[channel].halted)
		Data[channel].volCycle = 0xF;
}



// ---------------------------------------------------------------------------------
// Triangle Channel ----------------------------------------------------------------
// ---------------------------------------------------------------------------------

// Register 0: Control flag, linear counter
void APU::TriangleChannel::SetReg0(unsigned char value)
{
	// Pequeno hack para contar devidamente o halt
	CPU::cycles += 3;
	Update();
	CPU::cycles -= 3;

	Halted = (value >> 7) & 1;
	LinearReloadValue = value & 0x7F;
}

// Register 1: frequency LSB
void APU::TriangleChannel::SetReg1(unsigned char value)
{
	// Pequeno hack para contar devidamente o halt
	CPU::cycles += 3;
	Update();
	CPU::cycles -= 3;

	EventList[TRIANGLE] = NextEventList[TRIANGLE];
	SetNextEvent();

	Timer &= 0xFF00;
	Timer |= value;
}

// Register 2: Length counter timer, frequency MSB
void APU::TriangleChannel::SetReg2(unsigned char value)
{
	int nextclock = FrameSequencer::StepCycles[FrameSequencer::LastStep];

	// Pequeno hack para contar devidamente o halt
	CPU::cycles += 3;
	Update();

	if(Active && !((nextclock == CPU::cycles) && LengthCounter))
		LengthCounter = LengthIndex[value >> 3];

	EventList[TRIANGLE] = NextEventList[TRIANGLE];
	SetNextEvent();

	Timer &= 0xFF;
	Timer |= (value & 7) << 8;

	LinearHalted = true;

	CPU::cycles -= 3;
}

// Faz clock do linear counter
void APU::TriangleChannel::ClockLinearCounter()
{
	if(LinearHalted)
		LinearCounter = LinearReloadValue;

	else if(LinearCounter)
		--LinearCounter;

	if(!Halted)
		LinearHalted = false;
}



// ---------------------------------------------------------------------------------
// Noise Channel -------------------------------------------------------------------
// ---------------------------------------------------------------------------------

// Register 0: Length counter halt, envelope
void APU::NoiseChannel::SetReg0(unsigned char value)
{
	// Pequeno hack para contar devidamente o halt
	CPU::cycles += 3;
	Update();
	CPU::cycles -= 3;

	Halted = (value >> 5) & 1;

	Constant  = (value >> 4) & 1;
	VolDecay  = value & 0xF;
	Volume    = (value & 0xF) << 1;
}

// Register 1: Mode, timer
void APU::NoiseChannel::SetReg1(unsigned char value)
{
	// Pequeno hack para contar devidamente o halt
	CPU::cycles += 3;
	Update();
	CPU::cycles -= 3;

	BitMode = (value & 0x80) ? 6 : 1;

	EventList[NOISE] = LastCycle[NOISE] + (NoiseChannel::Timer * 3);

	Timer = TimerList[value & 0xF];
}

// Register 2: Length counter timer
void APU::NoiseChannel::SetReg2(unsigned char value)
{
	int nextclock = FrameSequencer::StepCycles[FrameSequencer::LastStep];

	// Pequeno hack para contar devidamente o halt
	CPU::cycles += 3;
	Update();

	if(Active && !((nextclock == CPU::cycles) && LengthCounter))
		LengthCounter = LengthIndex[value >> 3];

	Start = true;

	CPU::cycles -= 3;
}


// Faz clock dos envelopes
void APU::NoiseChannel::ClockEnvelope()
{
	// Reiniciar contagem
	if(Start)
	{
		Start = false;
		VolDecayInternal = VolDecay;

		VolCycle = 0x1E;

		return;
	}

	// Reduzir o decay
	if(VolDecayInternal)
	{
		--VolDecayInternal;
		return;
	}

	// Se decay for 0, fazer reset e reduzir ao volume
	VolDecayInternal = VolDecay;

	// Reduzir o volume
	if(VolCycle)
	{
		VolCycle -= 2;
		return;
	}

	// Só se faz reset ao volume se permitido
	if(Halted)
		VolCycle = 0x1E;
}



// ---------------------------------------------------------------------------------
// DMC Channel ---------------------------------------------------------------------
// ---------------------------------------------------------------------------------

// Register 0: IRQ, loop, frequency
void APU::DMCChannel::SetReg0(unsigned char value)
{
	Update();

	IRQEnabled   = (value >> 7);
	Loop         = (value >> 6) & 1;
	int oldTimer = Timer * 3;
	Timer        = TimerList[value & 0xF];

	// Com a alteração dos timers deve alterar-se o ciclo do dmc
	int remTimer = CPU::dmcDelayCycle - CPU::cycles;
	int i = 0;
	while(remTimer >= oldTimer)
	{
		remTimer -= oldTimer;
		++i;
	}
	CPU::dmcDelayCycle = CPU::cycles + remTimer + (i * 3 * Timer);

	EventList[DMC] = NextEventList[DMC];
	SetNextEvent();

	// Limpar IRQ
	if(!IRQEnabled)
	{
		IRQPending = false;
		CPU::Interrupts::UnsetIRQ(CPU::Interrupts::APU_DMC);
	}
}

// Register 1: Counter
void APU::DMCChannel::SetReg1(unsigned char value)
{
	Update();

	Counter = value & 0x7F;
}

// Register 2: Address
void APU::DMCChannel::SetReg2(unsigned char value)
{
	Update();

	Addr    = 0xC000 | (value << 6);
}

// Register 3: Bytes to read
void APU::DMCChannel::SetReg3(unsigned char value)
{
	Update();

	Bytes   = (value << 4) | 1;
}

// Obtém o próximo DMC Sample
bool APU::LoadDMCSample(int cyclesToAdd)
{
	using namespace DMCChannel;

	if(!Remaining)
	{
		Empty = true;
		Update();
		CPU::dmcDelayCycle += Timer * 24;
		return false;
	}

	CPU::cycles += cyclesToAdd;

	// Actualizar som
	Update();

	// Lê...
	Buffer = *CPU::Memory::GetAbsoluteAddress(AddrInternal);

	// Incrementa...
	++AddrInternal;
	AddrInternal &= 0xFFFF;
	AddrInternal |= 0x8000;

	// Reduz!
	--Remaining;

	// Não está vazio!
	Empty = false;

	// Verifica zero
	if(Remaining)
		CPU::dmcDelayCycle += 24 * Timer;

	// Se se tiver atingido o limite, fazer loop
	else if(Loop)
	{
		CPU::dmcDelayCycle += 24 * Timer;
		Remaining = Bytes;
		AddrInternal = Addr;
	}

	// De outro modo, parar play
	else
	{
		CPU::dmcDelayCycle += 24 * Timer;
		if(IRQEnabled)
		{
			CPU::Interrupts::SetIRQ(CPU::Interrupts::APU_DMC, CPU::cycles);
			IRQPending = 1;
		}
	}

	return true;
}



// ---------------------------------------------------------------------------------
// Funções variadas ----------------------------------------------------------------
// ---------------------------------------------------------------------------------

// Faz clock dos length counters
void APU::ClockLengthCounters()
{
	// Clock square channel 1
	if(!SquareChannel::Data[0].halted && SquareChannel::Data[0].lengthCounter)
		--SquareChannel::Data[0].lengthCounter;

	// Clock square channel 2
	if(!SquareChannel::Data[1].halted && SquareChannel::Data[1].lengthCounter)
		--SquareChannel::Data[1].lengthCounter;

	// Clock triangle channel
	if(!TriangleChannel::Halted && TriangleChannel::LengthCounter)
		--TriangleChannel::LengthCounter;

	// Clock noise channel
	if(!NoiseChannel::Halted && NoiseChannel::LengthCounter)
		--NoiseChannel::LengthCounter;
}

// Indica que houve um IRQ
bool APU::NotifyInterrupt(CPU::Interrupts::IRQType type)
{
	if(type == CPU::Interrupts::APU_FRAME)
		return FrameSequencer::NotifyInterrupt();

	// DMC interrupt retorna sempre true
	return true;
}

// Indica que estamos num novo frame
void APU::NotifyNewFrame()
{
	Update();

	InternalCycle = 0;

	// Reduzir clocks internos
	FrameSequencer::StepCycles[0] -= CPU::cyclesForFrame;
	FrameSequencer::StepCycles[1] -= CPU::cyclesForFrame;
	FrameSequencer::StepCycles[2] -= CPU::cyclesForFrame;
	FrameSequencer::StepCycles[3] -= CPU::cyclesForFrame;
	FrameSequencer::StepCycles[4] -= CPU::cyclesForFrame;

	// Reduzir clocks dos eventos
	NextEventCycle            -= CPU::cyclesForFrame;
	EventList[FRAMESEQUENCER] -= CPU::cyclesForFrame;

	if(EventList[SQUARE1]  != 200000)
		EventList[SQUARE1]   -= CPU::cyclesForFrame;
	if(EventList[SQUARE2]  != 200000)
		EventList[SQUARE2]   -= CPU::cyclesForFrame;
	if(EventList[TRIANGLE] != 200000)
		EventList[TRIANGLE]  -= CPU::cyclesForFrame;
	if(EventList[NOISE]    != 200000)
		EventList[NOISE]     -= CPU::cyclesForFrame;
	if(EventList[DMC]      != 200000)
		EventList[DMC]       -= CPU::cyclesForFrame;

	LastCycle[SQUARE1]  -= CPU::cyclesForFrame;
	LastCycle[SQUARE2]  -= CPU::cyclesForFrame;
	LastCycle[TRIANGLE] -= CPU::cyclesForFrame;
	LastCycle[NOISE]    -= CPU::cyclesForFrame;
	LastCycle[DMC]      -= CPU::cyclesForFrame;

	NextEventList[SQUARE1]  -= CPU::cyclesForFrame;
	NextEventList[SQUARE2]  -= CPU::cyclesForFrame;
	NextEventList[TRIANGLE] -= CPU::cyclesForFrame;
	NextEventList[NOISE]    -= CPU::cyclesForFrame;
	NextEventList[DMC]      -= CPU::cyclesForFrame;

	// Reduzir aos ciclos para IRQ
	if(FrameSequencer::InternalIRQCycle != 200000)
		FrameSequencer::InternalIRQCycle -= CPU::cyclesForFrame;

	if(FrameSequencer::InternalIRQCycle < 0)
		FrameSequencer::InternalIRQCycle = 0;

	CPU::dmcDelayCycle -= CPU::cyclesForFrame;

	Output::EndFrame(CPU::cyclesForFrame);
}

// Lê o status do APU
unsigned char APU::ReadStatus()
{
	Update();

	return (DMCChannel::IRQPending     << 7)
			| (FrameSequencer::IRQStatus() << 6) 
		  | ((DMCChannel::Remaining)                ? 16 : 0)
		  | ((NoiseChannel::LengthCounter)          ? 8  : 0)
		  | ((TriangleChannel::LengthCounter)       ? 4  : 0)
			| ((SquareChannel::Data[1].lengthCounter) ? 2  : 0)
			| ((SquareChannel::Data[0].lengthCounter) ? 1  : 0);
}

// Escreve no status do APU
void APU::WriteStatus(unsigned char value)
{
	Update();

	// Limpar o IRQ do DMC
	DMCChannel::IRQPending = false;
	CPU::Interrupts::UnsetIRQ(CPU::Interrupts::APU_DMC);

	// Square channel 1 enable/disable
	if(!(value & 1))
	{
		SquareChannel::Data[0].active = false;
		SquareChannel::Data[0].lengthCounter = 0;
	}
	else
	{
		SquareChannel::Data[0].active = true;

		//if(EventList[SQUARE1] == 200000)
		//{
		//	EventList[SQUARE1] = NextEventList[SQUARE1];
		//	SetNextEvent();
		//}
	}

	// Square channel 2 enable/disable
	if(!(value & 2))
	{
		SquareChannel::Data[1].active = false;
		SquareChannel::Data[1].lengthCounter = 0;
	}
	else
	{
		SquareChannel::Data[1].active = true;

		//if(EventList[SQUARE2] == 200000)
		//{
		//	EventList[SQUARE2] = NextEventList[SQUARE2];
		//	SetNextEvent();
		//}
	}

	// Triangle channel enable/disable
	if(!(value & 4))
	{
		TriangleChannel::Active = false;
		TriangleChannel::LengthCounter = 0;
	}
	else
	{
		TriangleChannel::Active = true;
		//EventList[TRIANGLE] = LastCycle[TRIANGLE] + ((TriangleChannel::Timer + 1) * 3);
	}

	// Noise channel enable/disable
	if(!(value & 8))
	{
		NoiseChannel::Active = false;
		NoiseChannel::LengthCounter = 0;
	}
	else
	{
		NoiseChannel::Active = true;
		EventList[NOISE] = LastCycle[NOISE] + (NoiseChannel::Timer * 3);
	}

	// DMC channel enable/disable
	if(!(value & 16))
	{
		DMCChannel::Active    = false;
		DMCChannel::Remaining = 0;
	}
	else
	{
		DMCChannel::Active = true;

		if(EventList[DMC] == 200000)
		{
			EventList[DMC] = NextEventList[DMC];
			SetNextEvent();
		}

		if(!DMCChannel::Remaining)
		{
			DMCChannel::Remaining     = DMCChannel::Bytes;
			DMCChannel::AddrInternal  = DMCChannel::Addr;

			if(DMCChannel::Empty)
			{
				register int oldDMCCycle = CPU::dmcDelayCycle;
				LoadDMCSample(9);
				CPU::dmcDelayCycle = oldDMCCycle;
			}
		}
	}
}

// Escreve nos registers do APU
void APU::RegWrite(int reg, unsigned char value)
{
	switch(reg)
	{
		case 0x00: return SquareChannel::SetReg0(0, value);
		case 0x01: return SquareChannel::SetReg1(0, value);
		case 0x02: return SquareChannel::SetReg2(0, value);
		case 0x03: return SquareChannel::SetReg3(0, value);
		case 0x04: return SquareChannel::SetReg0(1, value);
		case 0x05: return SquareChannel::SetReg1(1, value);
		case 0x06: return SquareChannel::SetReg2(1, value);
		case 0x07: return SquareChannel::SetReg3(1, value);
		case 0x08: return TriangleChannel::SetReg0(value);
		case 0x0A: return TriangleChannel::SetReg1(value);
		case 0x0B: return TriangleChannel::SetReg2(value);
		case 0x0C: return NoiseChannel::SetReg0(value);
		case 0x0E: return NoiseChannel::SetReg1(value);
		case 0x0F: return NoiseChannel::SetReg2(value);
		case 0x10: return DMCChannel::SetReg0(value);
		case 0x11: return DMCChannel::SetReg1(value);
		case 0x12: return DMCChannel::SetReg2(value);
		case 0x13: return DMCChannel::SetReg3(value);
		case 0x15: return WriteStatus(value);
		case 0x17: return FrameSequencer::SetMode(value);
	}
}



// ---------------------------------------------------------------------------------
// Variáveis -----------------------------------------------------------------------
// ---------------------------------------------------------------------------------

// Índice de tamanhos
unsigned char APU::LengthIndex[32] =
{
	10,254, 20,  2, 40,  4, 80,  6, 160,  8, 60, 10, 14, 12, 26, 14,
	12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30
};

// Índice de duty cycle dos square channels
unsigned char APU::SquareChannel::DutyCycle[4][8] = 
{
	{ 0, 1, 0, 0, 0, 0, 0, 0 },
	{ 0, 1, 1, 0, 0, 0, 0, 0 },
	{ 0, 1, 1, 1, 1, 0, 0, 0 },
	{ 1, 0, 0, 1, 1, 1, 1, 1 }
};

// Índice de valores de onda do triangle channel
int APU::TriangleChannel::WaveValues[32] = 
{
	45, 42, 39, 36, 33, 30, 27, 24, 21, 18, 15, 12,  9,  6,  3,  0,
	 0,  3,  6,  9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45
};

// Índice de valores de timing do noise channel
int APU::NoiseChannel::TimerList[16] = 
{
	4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068
};

// Índice de valores de timing do DMC channel
int APU::DMCChannel::TimerList[16] = 
{
	0x1AC, 0x17C, 0x154, 0x140, 0x11E, 0x0FE, 0x0E2, 0x0D6,
	0x0BE, 0x0A0, 0x08E, 0x080, 0x06A, 0x054, 0x048, 0x036
};

// Índice de valores do square
double APU::SquareTable[31] =
{
	0.0000000000, 0.0116091391, 0.0229394805, 0.0340009480,
	0.0448030003, 0.0553546573, 0.0656645256, 0.0757408220,
	0.0855913948, 0.0952237450, 0.1046450445, 0.1138621546,
	0.1228816423, 0.1317097960, 0.1403526399, 0.1488159482,
	0.1571052576, 0.1652258794, 0.1731829109, 0.1809812461,
	0.1886255858, 0.1961204468, 0.2034701710, 0.2106789339,
	0.2177507522, 0.2246894915, 0.2314988733, 0.2381824815,
	0.2447437689, 0.2511860630, 0.2575125718
};

// Índice de valores dos restantes channels
double APU::TndTable[203] =
{
	0.0000000000, 0.0066998239, 0.0133450200, 0.0199362538,
	0.0264741798, 0.0329594422, 0.0393926748, 0.0457745011,
	0.0521055349, 0.0583863801, 0.0646176312, 0.0707998734,
	0.0769336824, 0.0830196253, 0.0890582601, 0.0950501364,
	0.1009957951, 0.1068957689, 0.1127505824, 0.1185607520,
	0.1243267866, 0.1300491869, 0.1357284466, 0.1413650516,
	0.1469594806, 0.1525122052, 0.1580236902, 0.1634943931,
	0.1689247650, 0.1743152502, 0.1796662865, 0.1849783056,
	0.1902517324, 0.1954869861, 0.2006844797, 0.2058446202,
	0.2109678089, 0.2160544412, 0.2211049071, 0.2261195908,
	0.2310988714, 0.2360431223, 0.2409527121, 0.2458280038,
	0.2506693557, 0.2554771209, 0.2602516477, 0.2649932796,
	0.2697023555, 0.2743792094, 0.2790241709, 0.2836375652,
	0.2882197130, 0.2927709306, 0.2972915302, 0.3017818197,
	0.3062421029, 0.3106726796, 0.3150738456, 0.3194458928,
	0.3237891091, 0.3281037789, 0.3323901827, 0.3366485972,
	0.3408792959, 0.3450825483, 0.3492586207, 0.3534077759,
	0.3575302732, 0.3616263689, 0.3656963156, 0.3697403630,
	0.3737587576, 0.3777517426, 0.3817195583, 0.3856624419,
	0.3895806277, 0.3934743470, 0.3973438282, 0.4011892971,
	0.4050109763, 0.4088090861, 0.4125838436, 0.4163354638,
	0.4200641586, 0.4237701375, 0.4274536075, 0.4311147731,
	0.4347538362, 0.4383709963, 0.4419664507, 0.4455403941,
	0.4490930189, 0.4526245154, 0.4561350715, 0.4596248729,
	0.4630941031, 0.4665429434, 0.4699715732, 0.4733801694,
	0.4767689072, 0.4801379597, 0.4834874978, 0.4868176907,
	0.4901287054, 0.4934207071, 0.4966938591, 0.4999483229,
	0.5031842581, 0.5064018224, 0.5096011718, 0.5127824606,
	0.5159458413, 0.5190914646, 0.5222194797, 0.5253300340,
	0.5284232733, 0.5314993418, 0.5345583820, 0.5376005350,
	0.5406259402, 0.5436347355, 0.5466270573, 0.5496030406,
	0.5525628188, 0.5555065240, 0.5584342866, 0.5613462359,
	0.5642424995, 0.5671232041, 0.5699884744, 0.5728384344,
	0.5756732063, 0.5784929112, 0.5812976690, 0.5840875982,
	0.5868628161, 0.5896234387, 0.5923695808, 0.5951013562,
	0.5978188772, 0.6005222551, 0.6032116001, 0.6058870211,
	0.6085486260, 0.6111965215, 0.6138308133, 0.6164516060,
	0.6190590030, 0.6216531068, 0.6242340188, 0.6268018393,
	0.6293566678, 0.6318986025, 0.6344277409, 0.6369441793,
	0.6394480131, 0.6419393368, 0.6444182439, 0.6468848271,
	0.6493391779, 0.6517813873, 0.6542115449, 0.6566297399,
	0.6590360603, 0.6614305934, 0.6638134256, 0.6661846424,
	0.6685443286, 0.6708925681, 0.6732294439, 0.6755550384,
	0.6778694330, 0.6801727085, 0.6824649448, 0.6847462210,
	0.6870166157, 0.6892762063, 0.6915250700, 0.6937632829,
	0.6959909204, 0.6982080574, 0.7004147680, 0.7026111254,
	0.7047972024, 0.7069730711, 0.7091388027, 0.7112944679,
	0.7134401367, 0.7155758786, 0.7177017621, 0.7198178556,
	0.7219242263, 0.7240209413, 0.7261080666, 0.7281856680,
	0.7302538105, 0.7323125585, 0.7343619760, 0.7364021261,
	0.7384330717, 0.7404548748, 0.7424675971
};