// CPU.h
// Emulação do CPU

#ifndef CPU_H
#define CPU_H

namespace CPU
{
	// --------------------------------------------------------------------------
	// MÉTODOS PÚBLICOS ---------------------------------------------------------
	// --------------------------------------------------------------------------

	// Prepara emulação e coloca um jogo em memória
	void Prepare(char * gameName);

	// Iniciar emulação
	bool Start();

	// Pausar emulação
	bool Stop();

	// Reset
	void Reset();

	// Destrutor
	void Destroy();

	// Obtém o estado da emulação
	int GetState();

	// Indica se é possível escrever na consola
	void AllowPrintf(bool allow = true);

} // END namespace CPU


#endif