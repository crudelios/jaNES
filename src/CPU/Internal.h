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
	// VARIÁVEIS ----------------------------------------------------------------
	// --------------------------------------------------------------------------

	// Coisas do "hardware"
	EXTERNAL unsigned char opcode;     // Opcode actual
	EXTERNAL unsigned char opcodeData; // PC + 1

	// Vars úteis
	EXTERNAL int  state;               // Estado da emulação. Se for != 0 é porque algo está errado
	EXTERNAL int  cycles;              // Contador de ciclos do processador
	EXTERNAL int  subCycle;            // Sub-ciclo do processador
	EXTERNAL bool subCycleThisFrame;   // Indica se já se procedeu à contagem de sub-ciclo nesta frame
	EXTERNAL int  addCycle;            // Número de ciclos a adicionar ao contador
	EXTERNAL int  cyclesForFrame;      // Ciclos a processar na frame
	EXTERNAL int  evenOddTotalCycles;  // Indica se o total de ciclos percorridos é par ou impar
	EXTERNAL int  delayedTotalCycles;  // Ciclos para o frame com o delay de 2 cycles
	EXTERNAL int  startCycle;          // Ciclo em que começou o frame actual
	EXTERNAL bool doOAMWrite;          // Indica se deve ser realizado o sprite dma
	EXTERNAL bool stopped;             // Interrompe a emulação
	extern   bool working;             // Indica se o CPU está em funcionamento
	EXTERNAL void * ThreadID;          // ID da thread actual
	EXTERNAL bool enablePrintf;        // Indica se é possível escrever na consola
	EXTERNAL int  dmcDelayCycle;       // Indica o ciclo em que se deve realizar o delay do CPU por 4 ciclos
	EXTERNAL bool dmcDelayReg;         // Indica se há DMC delay no ciclo da leitura de register

	// Dados do jogo actual
	EXTERNAL int gameMapper;           // Mapper do jogo
	EXTERNAL int PRGpages;             // Páginas ROM do jogo
	EXTERNAL int CHRpages;             // Páginas sprite do jogo
	EXTERNAL unsigned int romSize;     // Tamanho do rom
	EXTERNAL unsigned int chrSize;     // Tamanho do CHR
	EXTERNAL bool SRAMEnabled;         // Indicar se o jogo tem SRAM
	EXTERNAL bool SRAMWrite;           // Indica se se pode escrever no SRAM
	EXTERNAL bool BatterySRAM;         // O jogo tem memória de bateria
	EXTERNAL char * GameName;          // O nome do jogo
	EXTERNAL unsigned int CRC;         // CRC do jogo

	// ------------------------------------------------------------------------
	// CONTROLO DE PROCESSAMENTO ----------------------------------------------
	// ------------------------------------------------------------------------

	// Executa o processador
	void Work();

	// Executa o CPU e o PPU durante um frame
	void WorkUntilNextFrame();

	// Executa um determinado número de ciclos
	int ChugAlong();

	// Tabela de OPCodes
	extern void (*OpcodeTable[256])();

} // END namespace CPU

#undef EXTERNAL
#endif