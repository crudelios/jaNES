// Interrupts.h
// Classe que lida com os interrupts

#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "Internal.h"

#ifndef INTERRUPTS_CPP
#define EXTERNAL extern
#else
#define EXTERNAL
#endif


// ------------------------------------------------------------------------
// INTERRUPTS -------------------------------------------------------------
// ------------------------------------------------------------------------

namespace CPU
{
	namespace Interrupts
	{
		// Indica o tipo de interrupt IRQ
		enum IRQType
		{
			APU_FRAME,
			APU_DMC,
			MAPPER
		};

		// Variáveis
		EXTERNAL int     NMIEnabled;     // Reproduzir o bug de não chamar o NMI em certos casos
		EXTERNAL int     IRQEnabled;     // Ligar ou desligar IRQ
		EXTERNAL bool    Reset;          // Fazer reset
		EXTERNAL bool    IRQPending;     // Quando há um IRQ pendente
		EXTERNAL int     IRQActive;      // Quando IRQ está activo
		EXTERNAL bool    NMIThisFrame;   // Indica se já houve NMI neste frame
		EXTERNAL int     NMIPending;     // Indica se há um NMI pendente
		EXTERNAL bool    DelayNMI;       // Atrasar o NMI por uma instrução
		EXTERNAL int     InNMI;          // Indica se estamos num NMI
		EXTERNAL int     NextIRQCycle;   // Indica o ciclo em que deve decorrer o próximo interrupt
		EXTERNAL IRQType NextIRQType;    // Indica o tipo do próximo interrupt
		EXTERNAL int     IRQs[3];        // Indica os ciclos dos IRQs


		// Verifica se há NMI
		inline bool NMIHit()
		{
			return (cycles > delayedTotalCycles) & NMIEnabled;
		}

		// Processa um Reset
		void StartReset();

		// Processa um NMI
		void StartNMI();

		// Processa um IRQ
		void StartIRQ();

		// Verifica se há IRQ
		inline bool IRQHit()
		{
			return (Interrupts::NextIRQCycle <= CPU::cycles); // & IRQEnabled & (regs.P.I ^ 1);
		}

		// Processa todos os interrupts
		void Handle();
		
		// Indica o próximo interrupt
		inline void SetNextIRQ()
		{
			NextIRQType  = APU_FRAME;
			NextIRQCycle = IRQs[APU_FRAME];

			if(IRQs[APU_DMC] < NextIRQCycle)
			{
				NextIRQCycle = IRQs[APU_DMC];
				NextIRQType  = APU_DMC;
			}

			if(IRQs[MAPPER] < NextIRQCycle)
			{
				NextIRQCycle = IRQs[MAPPER];
				NextIRQType  = MAPPER;
			}
		}

		// Adiciona um interrupt ao ciclo
		inline void SetIRQ(IRQType type, int cycle)
		{
			IRQs[type] = cycle;
			SetNextIRQ();
		}

		// Desactiva um tipo de IRQ
		inline void UnsetIRQ(IRQType type)
		{
			SetIRQ(type, 200000);
		}

		// Calcula o ciclo para o próximo frame
		inline void CalculateCyclesForNewFrame()
		{
			if(IRQs[APU_FRAME] != 200000)
				IRQs[APU_FRAME] -= CPU::cyclesForFrame;

			if(IRQs[APU_DMC] != 200000)
				IRQs[APU_DMC] -= CPU::cyclesForFrame;

			if(IRQs[MAPPER] != 200000)
				IRQs[MAPPER] -= CPU::cyclesForFrame;

			if(NextIRQCycle != 200000)
				NextIRQCycle -= CPU::cyclesForFrame;

			IRQs[APU_FRAME] = (IRQs[APU_FRAME] < 0) ? 0 : IRQs[APU_FRAME];
			IRQs[APU_DMC]   = (IRQs[APU_DMC]   < 0) ? 0 : IRQs[APU_DMC];
			IRQs[MAPPER]    = (IRQs[MAPPER]    < 0) ? 0 : IRQs[MAPPER];
			NextIRQCycle    = (NextIRQCycle    < 0) ? 0 : NextIRQCycle;
		}

	} // END namespace Interrupts
} // END namespace CPU

#undef EXTERNAL
#endif