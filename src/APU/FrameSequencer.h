// FrameSequencer.h
// Trata do Frame Sequencer

#ifndef FRAMESEQUENCER_H
#define FRAMESEQUENCER_H

#ifndef FRAMESEQUENCER_CPP
#define EXTERNAL extern
#else
#define EXTERNAL
#endif

namespace APU
{
	namespace FrameSequencer
	{
		// Variáveis de frame counter
		EXTERNAL int  Steps;             // Steps de funcionamento. Pode ser 4 ou 5
		EXTERNAL int  StepCycles[5];     // Ciclos dos clocks
		EXTERNAL int  LastStep;          // Posição do último clock
		EXTERNAL int  InternalIRQCycle;  // Ciclo em que se dará o frame irq
		EXTERNAL bool IRQPending;        // Indica se há IRQ pendente
		EXTERNAL bool DontEnablePending; // Indica se se deve permitir pending
		EXTERNAL int  ExtraIRQCycles;    // Adiciona ciclos ao próximo IRQ
		EXTERNAL int  CyclesToAdd;       // Indica o número de ciclos a adicionar ao sequencer
		EXTERNAL int  LastValue;         // Last value written

		// Indica o modo de funcionamento do frame sequencer
		void SetMode(unsigned char value);

		// Faz clock do frame sequencer
		void Clock();

		// Indica se há IRQ pendente
		int IRQStatus();

		// Notifica que houve IRQ
		bool NotifyInterrupt();

	} // END namespace FrameSequencer
} // END namespace APU

#undef EXTERNAL
#endif