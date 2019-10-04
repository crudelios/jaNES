// CPU.h
// Emula��o do CPU

#ifndef CPU_H
#define CPU_H

namespace CPU
{
	// --------------------------------------------------------------------------
	// M�TODOS P�BLICOS ---------------------------------------------------------
	// --------------------------------------------------------------------------

	// Prepara emula��o e coloca um jogo em mem�ria
	void Prepare(char * gameName);

	// Iniciar emula��o
	bool Start();

	// Pausar emula��o
	bool Stop();

	// Reset
	void Reset();

	// Destrutor
	void Destroy();

	// Obt�m o estado da emula��o
	int GetState();

	// Indica se � poss�vel escrever na consola
	void AllowPrintf(bool allow = true);

} // END namespace CPU


#endif