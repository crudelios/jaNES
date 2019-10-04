// Memory.cpp
// Classe de memória


#define MEMORY_CPP
#include "Memory.h"
#include "Disassembler.h"
#include "Interrupts.h"
#include "Input.h"

#include "../PPU/PPU.h"
#include "../APU/APU.h"

// Prepara o mapper
void CPU::Memory::Populate()
{
	mapPages[0x00] = mapPages[0x08] = mapPages[0x10] = mapPages[0x18] = ZP;
	mapPages[0x01] = mapPages[0x09] = mapPages[0x11] = mapPages[0x19] = Stack::memory;
	mapPages[0x02] = mapPages[0x0A] = mapPages[0x12] = mapPages[0x1A] = RAM;
	mapPages[0x03] = mapPages[0x0B] = mapPages[0x13] = mapPages[0x1B] = &RAM[0x100];
	mapPages[0x04] = mapPages[0x0C] = mapPages[0x14] = mapPages[0x1C] = &RAM[0x200];
	mapPages[0x05] = mapPages[0x0D] = mapPages[0x15] = mapPages[0x1D] = &RAM[0x300];
	mapPages[0x06] = mapPages[0x0E] = mapPages[0x16] = mapPages[0x1E] = &RAM[0x400];
	mapPages[0x07] = mapPages[0x0F] = mapPages[0x17] = mapPages[0x1F] = &RAM[0x500];

	// Fill SRAM if it is battery-backed
	if(CPU::BatterySRAM)
	{
		// First, get game name
		size_t nameLen = strlen(CPU::GameName);
		char * SRAMname = new char[nameLen + 1];
		memcpy(SRAMname, CPU::GameName, nameLen - 3);
		strcpy(SRAMname + (nameLen - 3), "raw");

		FILE * SRAMFile = fopen(SRAMname, "rb");

		// If there is a file, load it to SRAM
		if(SRAMFile)
		{
			// Check file size
			fseek(SRAMFile, 0 , SEEK_END);
			unsigned int fSize = ftell(SRAMFile);
			rewind(SRAMFile);

			// Only copy if the file is the correct size!
			if(fSize == 0x2000)
				fread(SRAM, sizeof(unsigned char), 0x2000, SRAMFile);

			fclose(SRAMFile);
		}
	}

	// Map the SRAM
	for(int i = 0; i < 0x20; i += 8)
	{
		int current = i | 0x60;

		mapPages[current]     = &SRAM[i << 8];
		mapPages[current + 1] = &SRAM[(i + 1) << 8];
		mapPages[current + 2] = &SRAM[(i + 2) << 8];
		mapPages[current + 3] = &SRAM[(i + 3) << 8];
		mapPages[current + 4] = &SRAM[(i + 4) << 8];
		mapPages[current + 5] = &SRAM[(i + 5) << 8];
		mapPages[current + 6] = &SRAM[(i + 6) << 8];
		mapPages[current + 7] = &SRAM[(i + 7) << 8];
	}

	// Populate the mapper
	mapper->Populate();
}

// Escreve num register associado à memória
void CPU::Memory::RegisterWrite(unsigned short addr, unsigned char value)
{
	switch(addr & 0xE000)
	{
		// PPU I/0
		case 0x2000:
		{
			PPU::RegWrite(addr & 7, value);
			return;
		}
		// Misc. I/O
		case 0x4000:
		{
			register unsigned short reg = addr & 0x3FFF;

			// Escrita em APU Register
			if((reg < 0x14) || (reg == 0x15) || (reg == 0x17))
				return APU::RegWrite(reg, value);

			// OAM Write
			if(reg == 0x14)
			{
				doOAMWrite = true;
				return;
			}

			// Comandos
			if(reg == 0x16)
				return Input::ResetButtons(value);
			
			// Leitura inválida
			if(reg & 0xFE0)
				return;

			return;
		}
		// SRAM I/O
		case 0x6000:
		{
			SRAM[addr & 0x1FFF] = value;
		}
	}
}

// Lê de um register associado à memória
unsigned char CPU::Memory::RegisterRead(unsigned short addr)
{
	switch(addr & 0xE000)
	{
		// PPU I/0
		case 0x2000:
		{
			// Ler PPUADDR três vezes se tiver havido DMC DMA neste ciclo
			if((addr == 0x2007) && dmcDelayReg)
			{
				PPU::RegRead(7);
				PPU::RegRead(7);
			}

			return PPU::RegRead(addr & 7);
		}
		// PPU/APU I/O
		case 0x4000:
		{
			register unsigned short reg = addr & 0x3FFF;

			// APU Status
			if(reg == 0x15)
				return APU::ReadStatus() | (openBusValue & 0x20);

			// Comando 1
			if(reg == 0x16)
			{
				// Ler input do botão duas vezes se tiver havido DMC DMA neste ciclo
				if(dmcDelayReg)
					Input::ReadButtons(0);

				return Input::ReadButtons(0) | (openBusValue & 0xE6);
			}

			// Comando 2
			if(reg == 0x17)
				return Input::ReadButtons(1) | (openBusValue & 0xE6);

			// Quando o número está a ir a um reg inválido, usar o open bus
			return openBusValue;
		}
		// SRAM I/O
		case 0x6000:
		{
			//if(!UsingSRAM)
			//	return ReadOpenBus();

			return SRAM[addr & 0x1FFF];
		}
	}

	return 0;
}

// Lê em modo debug um register associado à memória
unsigned char CPU::Memory::RegisterDebugRead(unsigned short addr)
{
	switch(addr & 0xE000)
	{
		// PPU I/0
		case 0x2000:
		{
			return PPU::RegDebugRead(addr & 7);
		}
		// Misc. I/O
		case 0x4000:
		{
			register unsigned short reg = addr & 0x3FFF;

			// Quando o número está a ir a um reg inválido, usar o open bus
			if((reg == 0x14) || (reg & 0xFE0))
				return ReadOpenBus();

			// return APU::RegDebugRead(reg);
			return 0;
		}
		// SRAM I/O
		case 0x6000:
		{
			//if(!UsingSRAM)
			//	return ReadOpenBus();

			return SRAM[addr & 0x1FFF];
		}
	}

	return 0;
}

// Lê do Open Bus
unsigned char CPU::Memory::ReadOpenBus()
{
	return openBusValue;
}