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
		// Vari�veis de frame counter
		EXTERNAL int  Steps;             // Steps de funcionamento. Pode ser 4 ou 5
		EXTERNAL int  StepCycles[5];     // Ciclos dos clocks
		EXTERNAL int  LastStep;          // Posi��o do �ltimo clock
		EXTERNAL int  InternalIRQCycle;  // Ciclo em que se dar� o frame irq
		EXTERNAL bool IRQPending;        // Indica se h� IRQ pendente
		EXTERNAL bool DontEnablePending; // Indica se se deve permitir pending
		EXTERNAL int  ExtraIRQCycles;    // Adiciona ciclos ao pr�ximo IRQ
		EXTERNAL int  CyclesToAdd;       // Indica o n�mero de ciclos a adicionar ao sequencer
		EXTERNAL int  LastValue;         // Last value written

		// Indica o modo de funcionamento do frame sequencer
		void SetMode(unsigned char value);

		// Faz clock do frame sequencer
		void Clock();

		// Indica se h� IRQ pendente
		int IRQStatus();

		// Notifica que houve IRQ
		bool NotifyInterrupt();

	} // END namespace FrameSequencer
} // END namespace APU

#undef EXTERNAL
#endif