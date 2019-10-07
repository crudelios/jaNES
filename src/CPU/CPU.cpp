// CPU.cpp
// Engine do emulador

#define CPU_CPP

#include <cstring>
#include <algorithm>

#include "CPU.h"
#include "Internal.h"
#include "Registers.h"
#include "Memory.h"
#include "Interrupts.h"
#include "Disassembler.h"
#include "Input.h"
#include "../Common/SaveState.h"

#include "../PPU/PPU.h"
#include "../PPU/FrameBuffer.h"
#include "../APU/APU.h"

#include "../Common/Common.h"

extern "C"
{
	#include <windows.h>
}

// Código relacionado com threads
HANDLE StopEvent;

// Iniciar o emulador
void CPU::Prepare(char * gameName)
{
	CPU::Destroy();

	// Primeira missão: preparar as variáveis
	state = 0;

	// Limpar outras vars
	cycles             = 0;
	subCycle           = 0;
	cyclesForFrame     = 82152;
	evenOddTotalCycles = 0;
	startCycle         = 0;
	delayedTotalCycles = 82154;
	addCycle           = 0;
	stopped            = true;
	opcodeData         = 0;
	subCycleThisFrame  = false;
	doOAMWrite         = false;
	dmcDelayCycle      = 200000;
	dmcDelayReg        = true;
	SRAMEnabled        = true;
	SRAMWrite          = true;

	// Limpar dados do jogo
	gameMapper = 0;
	PRGpages   = 0;
	CHRpages   = 0;
	romSize    = 0;
	chrSize    = 0;

	// Iniciar regs
	Registers::A  = 0;
	Registers::PC = 0;
	Registers::X  = 0;
	Registers::Y  = 0;
	Registers::SP = 0xFD;
	Registers::P::Unpack(0x34);

	// Limpar pointers
	opcode              = 0x00;
	ThreadID            = nullptr;
	Memory::ROM         = nullptr;
	Memory::CHR         = nullptr;
	Memory::mapper      = nullptr;

	// Limpar Memória
	memset(Memory::ZP,        0,    0x100  * sizeof(unsigned char));
	memset(Memory::RAM,       0xFF, 0x600  * sizeof(unsigned char));
	memset(Memory::blankPage, 0,    0x100  * sizeof(unsigned char));
	memset(Memory::SRAM,      0,    0x2000 * sizeof(unsigned char));
	memset(Stack::memory,     0,    0x100  * sizeof(unsigned char));
	std::fill_n(Memory::mapPages,   0x100, (unsigned char *) Memory::blankPage);

	// Aparentemente a ninty fixa estes valores na memória ao ligar-se...
	Memory::ZP[0x08] = 0xF7;
	Memory::ZP[0x09] = 0xEF;
	Memory::ZP[0x0A] = 0xDF;
	Memory::ZP[0x0F] = 0xBF;

	// Interrupts
	Interrupts::NMIEnabled   = 0;
	Interrupts::IRQEnabled   = 1;
	Interrupts::Reset        = false;
	Interrupts::IRQPending   = false;
	Interrupts::IRQActive    = Registers::P::I;
	Interrupts::NMIPending   = 0;
	Interrupts::DelayNMI     = true;
	Interrupts::InNMI        = 0;
	Interrupts::NextIRQCycle = 89469;
	Interrupts::NextIRQType  = Interrupts::APU_FRAME;

	Interrupts::IRQs[Interrupts::APU_FRAME] = 89469;
	Interrupts::IRQs[Interrupts::APU_DMC]   = 200000;
	Interrupts::IRQs[Interrupts::MAPPER]    = 200000;

	Interrupts::NMIThisFrame = false;

    // Reset button states
    Input::Joy[0].latch = 0;
    Input::Joy[1].latch = 0;

		// Cria o PPU
		PPU::SetMirrorMode(mirrorMode);
		
		// Preparar o mapper
		Memory::Mapper::Create();

		// Preparar a memória
		Memory::Populate();

		// Preparar coisas do thread
		StopEvent = CreateEvent(NULL, TRUE, FALSE, "StopEmulation");

		// Preparar o Program Counter
		Registers::PC = (Memory::Read(0xFFFD) << 8) | Memory::Read(0xFFFC);

		cycles = 0;

		working = true;

		printf("Loaded %s into memory!\nCRC: %X\n", GameName, CRC);
	}

	catch(int errorCode)
	{
		state = errorCode;
		if(myfile)
			fclose(myfile);

		Memory::Mapper::Destroy();
		CPU::Destroy();
	}
}

// Works the CPU
void CPU::Work()
{
	PPU::Create();
	APU::Prepare();
	SaveState::Reset();

	char * introText = new char[strlen("Loaded ") + strlen(GameName) + 1];

	sprintf(introText, "Loaded %s", GameName);

	PPU::FrameBuffer::DisplayString(introText, 5000);

	delete [] introText;

	while(!state && working)
		WorkUntilNextFrame();

	APU::Destroy();
	PPU::Destroy();
}

// Executa o processador até ao desenho do próximo frame
void CPU::WorkUntilNextFrame()
{
	int remaining = ChugAlong();

	if(!working)
		return;

	if(remaining < 0)
	{
		// Avisa todos os componentes de uma nova frame
		APU::NotifyNewFrame();
		Interrupts::CalculateCyclesForNewFrame();
		Memory::mapper->NotifyNewFrame();
		
		PPU::DrawFrame();

		evenOddTotalCycles ^= ((int) ((cycles - startCycle) / 3)) & 1;

		startCycle = cycles = 0 - remaining;
		
		// Faça uma pausa... com kit-kat!
		if(stopped)
		{
			PPU::DisableRenderer();
			APU::Pause();
			SetEvent(StopEvent);

			// Suspender a thread
			SuspendThread(ThreadID);
		}

		SaveState::CheckForSaveOrLoad();
	}
}

// Executar código
int CPU::ChugAlong()
{
	if(Interrupts::NMIEnabled)
		Interrupts::StartNMI();
	
	for(;;)
	{
		// Reset!
		if(Interrupts::Reset)
		{
			Interrupts::Reset = false;

			cycles = 0;
			startCycle = 0;

			// Reset APU and PPU
			APU::Reset();
			PPU::Reset();

			Interrupts::IRQs[Interrupts::APU_DMC]   = 200000;
			Interrupts::SetNextIRQ();

			// Reset Program Counter
			Registers::PC = (Memory::Read(0xFFFD) << 8) | Memory::Read(0xFFFC);

			cycles += 27;
			cyclesForFrame     = 82152;
			evenOddTotalCycles = 0;
			startCycle         = 0;
			delayedTotalCycles = 82154;
			addCycle           = 0;

			Interrupts::NMIPending   = 0;
			Interrupts::NMIEnabled   = 0;
			Interrupts::InNMI        = 0;
			Interrupts::IRQEnabled   = 1;
			Interrupts::IRQPending   = false;
			Interrupts::NMIThisFrame = false;

			Registers::SP -= 3;

			Registers::P::I = 1;
		}

		// Lidar com caso especial do NMI
		if(Interrupts::NMIPending)
		{
			if(Interrupts::NMIPending > 1)
				--Interrupts::NMIPending;
			else Interrupts::StartNMI();
		}

		// Lidar com IRQs
		if(Interrupts::NextIRQCycle <= CPU::cycles)
			Interrupts::StartIRQ();

		Interrupts::IRQActive = Registers::P::I;

		// Obter o opcode
		opcode     = Memory::Read(Registers::PC);
		opcodeData = Memory::Read(Registers::PC + 1);

		// Mostrar informações de disassembly
//#ifdef _PRINTDISASSEMBLY
		if(enablePrintf)
			printf("%s\n", Disassembler::GetInfo(opcode));
//#endif

		// Executar o opcode
		OpcodeTable[opcode]();

		// Quebrar o ciclo
		if(cycles > delayedTotalCycles)
		{
			if(Interrupts::DelayNMI)
				Interrupts::DelayNMI = false;
			else
				break;
		}
	}

	return cyclesForFrame - cycles;
}


// ----------------------------------------------------------------------------
// MEMBROS PÚBLICOS -----------------------------------------------------------
// ----------------------------------------------------------------------------

// Inicia a emulação
bool CPU::Start()
{
	// Só se começa a trabalhar se o CPU estiver em condições e se não estiver pausado
	if(!working || !stopped)
		return false;

	if(ThreadID)
	{
		stopped = false;
		ResumeThread(ThreadID);

		return true;
	}

	stopped = false;

	ThreadID = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) &CPU::Work, NULL, 0, 0);

	return true;
}

// Pausa a emulação
bool CPU::Stop()
{
	if(!working || stopped)
		return false;

	stopped = true;

	// Interrompe a thread
	WaitForSingleObject(StopEvent, INFINITE);
	ResetEvent(StopEvent);

	return true;
}

// Resets CPU
void CPU::Reset()
{
	Interrupts::Reset = true;
}

// Retorna o estado da emulação
int CPU::GetState()
{
	return state;
}

// Indica se é possível escrever na consola
void CPU::AllowPrintf(bool allow)
{
	enablePrintf = allow;
}

// Destruidor de classe
void CPU::Destroy()
{
	// Save SRAM (this code really should be somewhere else)
	if(working && BatterySRAM)
	{
		// First, get game name
		size_t nameLen = strlen(GameName);
		char * SRAMname = new char[nameLen + 1];
		memcpy(SRAMname, GameName, nameLen - 3);
		strcpy(SRAMname + (nameLen - 3), "raw");

		FILE * SRAMFile = fopen(SRAMname, "wb");

		// If we have a stream, save SRAM to it
		if(SRAMFile)
		{
			fwrite(Memory::SRAM, sizeof(unsigned char), 0x2000, SRAMFile);
			fclose(SRAMFile);
		}
	}

	delete [] GameName;
	GameName = nullptr;

	if(working)
	{
		working = false;

		// Se emulação estiver em pausa, terminar a frame
		if(stopped)
		{
			stopped = false;
			ResumeThread(ThreadID);
		}

		// Interromper o processamento apenas após terminar a frame actual
		WaitForSingleObject(ThreadID, INFINITE);

		stopped = true;

		Memory::Mapper::Destroy();
		delete [] Memory::ROM;

		CloseHandle(StopEvent);

		ThreadID       = nullptr;
		Memory::ROM    = nullptr;
		Memory::CHR    = nullptr;
		Memory::mapper = nullptr;
	}
}

bool CPU::working = false;