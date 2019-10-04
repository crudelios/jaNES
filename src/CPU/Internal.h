// Internal.h
// Dados internos do CPU

#ifndef INTERNAL_H
#define INTERNAL_H

#ifndef CPU_CPP
#define EXTERNAL extern
#else
#define EXTERNAL
#endif

namespace CPU
{
	// --------------------------------------------------------------------------
	// VARI�VEIS ----------------------------------------------------------------
	// --------------------------------------------------------------------------

	// Coisas do "hardware"
	EXTERNAL unsigned char opcode;     // Opcode actual
	EXTERNAL unsigned char opcodeData; // PC + 1

	// Vars �teis
	EXTERNAL int  state;               // Estado da emula��o. Se for != 0 � porque algo est� errado
	EXTERNAL int  cycles;              // Contador de ciclos do processador
	EXTERNAL int  subCycle;            // Sub-ciclo do processador
	EXTERNAL bool subCycleThisFrame;   // Indica se j� se procedeu � contagem de sub-ciclo nesta frame
	EXTERNAL int  addCycle;            // N�mero de ciclos a adicionar ao contador
	EXTERNAL int  cyclesForFrame;      // Ciclos a processar na frame
	EXTERNAL int  evenOddTotalCycles;  // Indica se o total de ciclos percorridos � par ou impar
	EXTERNAL int  delayedTotalCycles;  // Ciclos para o frame com o delay de 2 cycles
	EXTERNAL int  startCycle;          // Ciclo em que come�ou o frame actual
	EXTERNAL bool doOAMWrite;          // Indica se deve ser realizado o sprite dma
	EXTERNAL bool stopped;             // Interrompe a emula��o
	extern   bool working;             // Indica se o CPU est� em funcionamento
	EXTERNAL void * ThreadID;          // ID da thread actual
	EXTERNAL bool enablePrintf;        // Indica se � poss�vel escrever na consola
	EXTERNAL int  dmcDelayCycle;       // Indica o ciclo em que se deve realizar o delay do CPU por 4 ciclos
	EXTERNAL bool dmcDelayReg;         // Indica se h� DMC delay no ciclo da leitura de register

	// Dados do jogo actual
	EXTERNAL int gameMapper;           // Mapper do jogo
	EXTERNAL int PRGpages;             // P�ginas ROM do jogo
	EXTERNAL int CHRpages;             // P�ginas sprite do jogo
	EXTERNAL unsigned int romSize;     // Tamanho do rom
	EXTERNAL unsigned int chrSize;     // Tamanho do CHR
	EXTERNAL bool SRAMEnabled;         // Indicar se o jogo tem SRAM
	EXTERNAL bool SRAMWrite;           // Indica se se pode escrever no SRAM
	EXTERNAL bool BatterySRAM;         // O jogo tem mem�ria de bateria
	EXTERNAL char * GameName;          // O nome do jogo
	EXTERNAL unsigned int CRC;         // CRC do jogo

	// ------------------------------------------------------------------------
	// CONTROLO DE PROCESSAMENTO ----------------------------------------------
	// ------------------------------------------------------------------------

	// Executa o processador
	void Work();

	// Executa o CPU e o PPU durante um frame
	void WorkUntilNextFrame();

	// Executa um determinado n�mero de ciclos
	int ChugAlong();

	// Tabela de OPCodes
	extern void (*OpcodeTable[256])();

} // END namespace CPU

#undef EXTERNAL
#endif