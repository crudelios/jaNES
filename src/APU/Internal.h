// Internal.h
// Dados internos da APU

#ifndef APUINTERNAL_H
#define APUINTERNAL_H

#ifndef APU_CPP
#define EXTERNAL extern
#else
#define EXTERNAL
#endif

namespace APU
{
	// Variáveis do APU
	EXTERNAL int  InternalCycle;  // Ciclo interno do APU

	// Tabelas
	extern double SquareTable[31];
	extern double TndTable[203];

	// Eventos
	enum Events
	{
		FRAMESEQUENCER,
		SQUARE1,
		SQUARE2,
		TRIANGLE,
		NOISE,
		DMC
	};

	EXTERNAL Events NextEvent;
	EXTERNAL int    NextEventCycle;
	EXTERNAL int    EventList[6];
	EXTERNAL int    LastCycle[6];
	EXTERNAL int    NextEventList[6];

	// Determina o próximo evento
	inline void SetNextEvent()
	{
		NextEvent      = FRAMESEQUENCER;
		NextEventCycle = EventList[FRAMESEQUENCER];

		if(EventList[SQUARE1] < NextEventCycle)
		{
			NextEventCycle = EventList[SQUARE1];
			NextEvent      = SQUARE1;
		}

		if(EventList[SQUARE2] < NextEventCycle)
		{
			NextEventCycle = EventList[SQUARE2];
			NextEvent      = SQUARE2;
		}

		if(EventList[TRIANGLE] < NextEventCycle)
		{
			NextEventCycle = EventList[TRIANGLE];
			NextEvent      = TRIANGLE;
		}

		if(EventList[NOISE] < NextEventCycle)
		{
			NextEventCycle = EventList[NOISE];
			NextEvent      = NOISE;
		}

		if(EventList[DMC] < NextEventCycle)
		{
			NextEventCycle = EventList[DMC];
			NextEvent      = DMC;
		}
	}

	// Adiciona um evento
	inline void AddEvent(Events type, int cycle)
	{
		EventList[type] = cycle;
	}

	// Define o estado do APU
	void WriteStatus(unsigned char value);

	// Faz clock dos length counters
	void ClockLengthCounters();

	// Variáveis
	extern unsigned char LengthIndex[32];

	// -------------------------------------------------------------------------------
	// Square Channels ---------------------------------------------------------------
	// -------------------------------------------------------------------------------

	namespace SquareChannel
	{
		struct SWData
		{
			// Vars principais
			bool active;
			bool halted;
			int  lengthCounter;
			int  timer;
			int  timerInternal;
			int  DAC;

			// Duty cycle
			int  dutyCycle;
			int  dutyCyclePos;

			// Envelope
			bool constant;
			bool start;
			int  volume;
			int  volCycle;
			int  volDecay;
			int  volDecayInternal;

			// Sweep
			int  sweepOn;
			bool sweepReset;
			bool sweepStopped;
			int  sweepRefreshRate;
			int  sweepRefreshRateInternal;
			bool sweepDecreaseWave;
			int  sweepRightShift;
		};
	
		// Duty cycles
		extern unsigned char DutyCycle[4][8];

		// Dados dos square waves
		EXTERNAL SWData Data[2];

		// Define o reg 0
		void SetReg0(unsigned int channel, unsigned char value);

		// Define o reg 1
		void SetReg1(unsigned int channel, unsigned char value);

		// Define o reg 2
		void SetReg2(unsigned int channel, unsigned char value);

		// Define o reg 3
		void SetReg3(unsigned int channel, unsigned char value);

		// Faz clock a cada um dos sweeps
		void ClockSweepUnit(int channel);

		// Faz clock aos sweeps
		inline void ClockSweepUnits()
		{
			ClockSweepUnit(0);
			ClockSweepUnit(1);
		}

		// Verifica se devemos silenciar o sweep
		void CheckSweepStopped(int channel);

		// Faz clock a cada um dos envelopes
		void ClockEnvelope(int channel);

		// Faz clock dos envelopes
		inline void ClockEnvelopes()
		{
			ClockEnvelope(0);
			ClockEnvelope(1);
		}

		// Calcula o sample do square channel
		inline int GetSample(SWData & data)
		{
			if(!--data.timerInternal)
			{
				data.timerInternal = (data.timer + 1) << 1;
				++data.dutyCyclePos;
				data.dutyCyclePos &= 7;
			}

			return (data.lengthCounter && !data.sweepStopped) ? (((data.constant) ? data.volume : data.volCycle) * SquareChannel::DutyCycle[data.dutyCycle][data.dutyCyclePos]) : 0;
		}


	} // END namespace SquareChannel


	// -------------------------------------------------------------------------------
	// Triangle Channel --------------------------------------------------------------
	// -------------------------------------------------------------------------------

	namespace TriangleChannel
	{
		EXTERNAL int  Timer;
		EXTERNAL int  TimerInternal;
		EXTERNAL int  LengthCounter;
		EXTERNAL int  LinearCounter;
		EXTERNAL int  LinearReloadValue;
		EXTERNAL bool LinearHalted;
		EXTERNAL bool Active;
		EXTERNAL bool Halted;
		EXTERNAL int  WavePos;
		EXTERNAL int  DAC;
		extern   int WaveValues[32];

		// Define o reg 0
		void SetReg0(unsigned char value);

		// Define o reg 1
		void SetReg1(unsigned char value);

		// Define o reg 2
		void SetReg2(unsigned char value);

		// Faz clock do linear counter
		void ClockLinearCounter();

	} // END namespace TriangleChannel


	// -------------------------------------------------------------------------------
	// Noise Channel -----------------------------------------------------------------
	// -------------------------------------------------------------------------------

	namespace NoiseChannel
	{
		EXTERNAL int  Timer;
		EXTERNAL int  TimerInternal;
		EXTERNAL int  LengthCounter;
		EXTERNAL int  ShiftRegister;
		EXTERNAL int  BitMode;
		EXTERNAL bool Halted;
		EXTERNAL bool Active;
		EXTERNAL int  DAC;
		extern   int  TimerList[16];

		// Envelope
		EXTERNAL int  Volume;
		EXTERNAL bool Start;
		EXTERNAL bool Constant;
		EXTERNAL	 int  VolCycle;
		EXTERNAL	 int  VolDecay;
		EXTERNAL	 int  VolDecayInternal;

		// Define o reg 0
		void SetReg0(unsigned char value);

		// Define o reg 1
		void SetReg1(unsigned char value);

		// Define o reg 2
		void SetReg2(unsigned char value);

		// Faz clock do envelope
		void ClockEnvelope();

	} // END namespace NoiseChannel



	// -------------------------------------------------------------------------------
	// DMC Channel -------------------------------------------------------------------
	// -------------------------------------------------------------------------------

	namespace DMCChannel
	{
		// Geral
		EXTERNAL bool Active;

		// IRQs
		EXTERNAL int IRQEnabled;
		EXTERNAL int IRQPending;
		EXTERNAL int InternalIRQCycle;

		// Timer
		EXTERNAL int Timer;
		EXTERNAL int TimerInternal;
		EXTERNAL int Loop;
		extern   int TimerList[16];

		// Endereço
		EXTERNAL unsigned char Buffer;
		EXTERNAL bool          Empty;
		EXTERNAL int           Addr;
		EXTERNAL int           AddrInternal;
		EXTERNAL int           Bytes;
		EXTERNAL int           Remaining;
		EXTERNAL bool          Restart;

		// Output
		EXTERNAL int           Bit;
		EXTERNAL bool          Silence;
		EXTERNAL unsigned char OutputSample;
		EXTERNAL unsigned char Counter;

		// Define o reg 0
		void SetReg0(unsigned char value);

		// Define o reg 1
		void SetReg1(unsigned char value);

		// Define o reg 2
		void SetReg2(unsigned char value);

		// Define o reg 3
		void SetReg3(unsigned char value);

		// Notifica que houve um interrupt
		bool NotifyInterrupt();



		// -------
		// Inlines
		//--------

		// Obtém o ciclo do CPU em que se vai obter o próximo byte
		inline int GetNextByteFetch(int cycle)
		{
			return cycle + (3 * Remaining * TimerInternal);
		}

	} // END namespace DMCChannel

} // END namespace APU

#undef EXTERNAL
#endif