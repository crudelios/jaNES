// APU.h
// Tratamento de APU

#ifndef APU_H
#define APU_H

#include "../CPU/Interrupts.h"

#ifndef APU_CPP
#define EXTERNAL extern
#else
#define EXTERNAL
#endif

namespace APU
{
	// Prepara os dados para utilização
	void Prepare();

	// Resets the APU
	void Reset();

	// Destrói a APU
	void Destroy();

	// Pausa o som
	void Pause();

	// Indica que estamos num novo frame
	void NotifyNewFrame();

	// Indica que houve um IRQ
	bool NotifyInterrupt(CPU::Interrupts::IRQType type);

	// Lê o status do APU
	unsigned char ReadStatus();

	// Escreve nos registers do APU
	void RegWrite(int reg, unsigned char value);

	// Actualiza o estado do APU
	void Update();

	// Obtém o próximo sample DMC
	bool LoadDMCSample(int cyclesToAdd = 12);

} // END namespace APU

#undef EXTERNAL
#endif