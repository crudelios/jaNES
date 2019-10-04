// Mappers.cpp
// Implementação dos mappers

#include "Mappers.h"
#include "../CPU/Internal.h"
#include "../CPU/Memory.h"

#include "../PPU/PPU.h"
#include "../PPU/Internal.h"
#include "../PPU/Registers.h"
#include "../PPU/FrameBuffer.h"

#include <cstdio>

// Cria um mapper
void CPU::Memory::Mapper::Create()
{
	switch(CPU::gameMapper)
	{
		case 0:    CPU::Memory::mapper = new Mapper();    break;
		case 1:    CPU::Memory::mapper = new Mapper1();   break;
		case 3:    CPU::Memory::mapper = new Mapper3();   break;
		case 4:    CPU::Memory::mapper = new Mapper4();   break;
		case 7:    CPU::Memory::mapper = new Mapper7();   break;
		default:   printf("Mapper nao implementado!\n");  throw 4;
	}
}

// Destrói um mapper
void CPU::Memory::Mapper::Destroy()
{
	delete CPU::Memory::mapper;
	CPU::Memory::mapper = nullptr;
}

// Indica que estamos num novo frame
void CPU::Memory::Mapper::NotifyPPUCreated()
{
}

// Indica que estamos num novo frame
void CPU::Memory::Mapper::NotifyNewFrame()
{
}

// Indica que ocorreu um interrupt que teve origem no mapper
bool CPU::Memory::Mapper::NotifyInterrupt()
{
	return false;
}

// Indica que o endereço do PPU foi alterado
void CPU::Memory::Mapper::NotifyPPUAddressChanged(unsigned short addr)
{
}

// Indica que o render foi activado/desactivado
void CPU::Memory::Mapper::NotifyToggleRender()
{
}

// Coloca os dados no mapper
void CPU::Memory::Mapper::Populate()
{
	// Quando só há uma página, o conteúdo de 0x8000 - 0xBFFF
	// é espelhado em 0xC000 - 0xFFFF
	int mod = (PRGpages == 1) ? 0x3F : 0xFF;

	// Popular o resto do mapper com o conteúdo do ROM
	for(int i = 0x0; i < 0x80; i += 0x8)
	{
		int j = i | 0x80;
		int k = i & mod;

		mapPages[j]     = &ROM[k << 8];
		mapPages[j + 1] = &ROM[(k + 1) << 8];
		mapPages[j + 2] = &ROM[(k + 2) << 8];
		mapPages[j + 3] = &ROM[(k + 3) << 8];
		mapPages[j + 4] = &ROM[(k + 4) << 8];
		mapPages[j + 5] = &ROM[(k + 5) << 8];
		mapPages[j + 6] = &ROM[(k + 6) << 8];
		mapPages[j + 7] = &ROM[(k + 7) << 8];
	}
}

// Escreve num mapper
void CPU::Memory::Mapper::Write(unsigned short addr, unsigned char value)
{
	if(addr < 0x8000)
		mapPages[addr >> 8][addr & 0xFF] = value;
}

// Indica que estamos num novo frame
void CPU::Memory::Mapper1::NotifyNewFrame()
{
	LastWrite = -10;
}


// Popula o mapper 1
void CPU::Memory::Mapper1::Populate()
{
	ResetLowPRGBank();
	ResetHighPRGBank();
}

// Escreve os dados por defeito do primeiro bank
void CPU::Memory::Mapper1::ResetLowPRGBank()
{
	for(int k = 0x0; k < 0x40; k += 0x8)
	{
		unsigned long j = k | 0x80 | HighPRG;

		mapPages[j]     = &ROM[k << 8];
		mapPages[j + 1] = &ROM[(k + 1) << 8];
		mapPages[j + 2] = &ROM[(k + 2) << 8];
		mapPages[j + 3] = &ROM[(k + 3) << 8];
		mapPages[j + 4] = &ROM[(k + 4) << 8];
		mapPages[j + 5] = &ROM[(k + 5) << 8];
		mapPages[j + 6] = &ROM[(k + 6) << 8];
		mapPages[j + 7] = &ROM[(k + 7) << 8];
	}
}

// Escreve os dados por defeito do último bank
void CPU::Memory::Mapper1::ResetHighPRGBank()
{
	unsigned int subtract = 1;

	if((CPU::PRGpages == 0x20) && !HighPRG)
		subtract = 0x11;

	unsigned long start = ((CPU::PRGpages - subtract) << 6);

	// Popular a última parte do mapper com o último bank
	for(unsigned long i = 0x0; i < 0x40; i += 0x8)
	{
		int j = i | 0xC0;
		unsigned long k = i + start;

		mapPages[j]     = &ROM[k << 8];
		mapPages[j + 1] = &ROM[(k + 1) << 8];
		mapPages[j + 2] = &ROM[(k + 2) << 8];
		mapPages[j + 3] = &ROM[(k + 3) << 8];
		mapPages[j + 4] = &ROM[(k + 4) << 8];
		mapPages[j + 5] = &ROM[(k + 5) << 8];
		mapPages[j + 6] = &ROM[(k + 6) << 8];
		mapPages[j + 7] = &ROM[(k + 7) << 8];
	}
}

// Altera os banks PRG
void CPU::Memory::Mapper1::SwapPRGBanks(unsigned int bankNum)
{
	// Ignora o bit menos significativo se estivermos a alterar 32KB
	unsigned long start = HighPRG | (((BankSize == 0x80) ? (bankNum & 0xFE) : bankNum) << 6);

	// Popular a última parte do mapper com o último bank
	for(unsigned long i = 0x0; i < BankSize; i += 0x8)
	{
		int j = i | SwapPRGPos;
		unsigned long k = i + start;

		mapPages[j]     = &ROM[k << 8];
		mapPages[j + 1] = &ROM[(k + 1) << 8];
		mapPages[j + 2] = &ROM[(k + 2) << 8];
		mapPages[j + 3] = &ROM[(k + 3) << 8];
		mapPages[j + 4] = &ROM[(k + 4) << 8];
		mapPages[j + 5] = &ROM[(k + 5) << 8];
		mapPages[j + 6] = &ROM[(k + 6) << 8];
		mapPages[j + 7] = &ROM[(k + 7) << 8];
	}
}


// Escreve no mapper 1
void CPU::Memory::Mapper1::Write(unsigned short addr, unsigned char value)
{
	// Bill and Ted é estúpido e ainda nem sequer experimentei o jogo!
	unsigned int interval = CPU::cycles - LastWrite;
	if(interval < 6)
		return;

	LastWrite = CPU::cycles;

	// Reset bit
	if(value & 0x80)
	{
		dataLatch = 0;
		mapperData = 0;

		if((BankSize == 0x40) && (SwapPRGPos == 0x80))
			return;

		BankSize = 0x40;
		SwapPRGPos = 0x80;

		DoSwapPRGCtrl();
		return;
	}

	// Colocar no registo
	mapperData |= (value & 1) << dataLatch;

	// Escrever no registo e efectuar as alterações necessárias
	if(dataLatch == 4)
	{
		dataLatch = 0;

		// Um quirk interessante do MMC1... Aparentemente o MMC1 escreve os quatro primeiros bytes a partir do endereço do PC
		//if((LastAddr & 0x6000) != (addr & 0x6000))
		//{
		//	unsigned char oldMapperData = mapperData;

		//	mapperData |= 0x10;

		//	// Obtém o registo a partir do PC (!!)
		//	switch((CPU::Registers::PC & 0x6000) >> 13)
		//	{
		//		// Escolhe o registo a alterar
		//		case 0: ChangeReg0(); break;
		//		case 1: ChangeReg1(); break;
		//		case 2: ChangeReg2(); break;
		//		case 3: ChangeReg3(); break;
		//	}

		//	mapperData = oldMapperData;
		//}
		// Obtém o registo a partir do endereço da memória
		switch((addr & 0x6000) >> 13)
		{
			// Escolhe o registo a alterar
			case 0: ChangeReg0(); break;
			case 1: ChangeReg1(); break;
			case 2: ChangeReg2(); break;
			case 3: ChangeReg3(); break;
		}
		
		mapperData = 0;

		return;
	}

	LastAddr = addr;

	dataLatch++;
}

// Altera as posições de banking
void CPU::Memory::Mapper1::DoSwapPRGCtrl()
{
	switch(BankSize)
	{
		case 0x40:
		{
			CurrentPRGBank &= (Reg3Written) ? 0xFE : 0xFF;

			// Altera a posição do bank
			SwapPRGBanks(CurrentPRGBank);

			Reg3Written = false;

			switch(SwapPRGPos)
			{
				case 0x80:
				{
					ResetHighPRGBank();
					break;
				}
				case 0xC0:
				{
					ResetLowPRGBank();
					break;
				}
			}
			break;
		}
		case 0x80:
		{
			SwapPRGBanks(CurrentPRGBank);
			Reg3Written = false;
			break;
		}
	}
}

// Altera o registo 0
void CPU::Memory::Mapper1::ChangeReg0()
{
	unsigned char ppuMirror = mapperData & 3;

	// Alterar o mirror mode
	if(ppuMirror != CurrentPPUMirror)
	{
		CurrentPPUMirror = ppuMirror;
		PPU::Update();

		switch(ppuMirror & 2)
		{
			// One-screen mirrorring
			case 0:
			{
				PPU::SetMirrorMode(PPU::ONESCREEN);

				// Ainda não percebi se devo ter em conta o bit0 ou não!
				if(ppuMirror & 1)
				{
					PPU::mapper[12] = PPU::mapper[8]  = PPU::nameTables[1];
					PPU::mapper[13] = PPU::mapper[9]  = PPU::nameTables[1];
					PPU::mapper[14] = PPU::mapper[10] = PPU::nameTables[1];
					PPU::mapper[15] = PPU::mapper[11] = PPU::nameTables[1];
				}
				break;
			}

			// Horiz/vert mirrorring
			case 2:
			{
				PPU::SetMirrorMode((ppuMirror & 1) ? PPU::HORIZONTAL : PPU::VERTICAL);
			}
		}
	}

	// Bank size - 0x40 ou 0x80
	unsigned char bSize = 0x80 >> ((mapperData & 8) >> 3);

	// Posição em que é a troca de bank - 0x80 ou 0xC0 - se o bSize for 0x80 fixa-se o swapPos em 0x80
	// Arranjar isto
	unsigned char swapPos = 0x80 | ((4 ^ ((mapperData | (bSize >> 5)) & 4)) << 4);

	// Mudar posições do mapper
	if((swapPos != SwapPRGPos) || (bSize != BankSize))
	{
		SwapPRGPos = swapPos;
		BankSize = bSize;

		DoSwapPRGCtrl();
	}

	// CHR swap size - 4 ou 8
	SwapCHRSize = 8 >> ((mapperData & 0x10) >> 4);
}

// Altera o registo 1
void CPU::Memory::Mapper1::ChangeReg1()
{
	PPU::Update();

	unsigned char myOr = (CPU::PRGpages == 0x20) ? 0 : 0x10;

	CurrentCHRBank1 = ((SwapCHRSize & 8) ? (mapperData & (myOr | 0x0E)) : (mapperData & (myOr | 0x0F))) << 2;

	PPU::mapper[0] = &PPU::VRAM[CurrentCHRBank1 << 10];
	PPU::mapper[1] = &PPU::VRAM[(CurrentCHRBank1 + 1) << 10];
	PPU::mapper[2] = &PPU::VRAM[(CurrentCHRBank1 + 2) << 10];
	PPU::mapper[3] = &PPU::VRAM[(CurrentCHRBank1 + 3) << 10];

	if(SwapCHRSize & 8)
	{
		PPU::mapper[4] = &PPU::VRAM[(CurrentCHRBank1 + 4) << 10];
		PPU::mapper[5] = &PPU::VRAM[(CurrentCHRBank1 + 5) << 10];
		PPU::mapper[6] = &PPU::VRAM[(CurrentCHRBank1 + 6) << 10];
		PPU::mapper[7] = &PPU::VRAM[(CurrentCHRBank1 + 7) << 10];
	}


	// SUROM é um caso especial...
	if(CPU::PRGpages == 0x20)
	{
		unsigned long myPRG = (mapperData & 0x10) << 6;

		if(myPRG != HighPRG)
		{
			HighPRG = myPRG;
			DoSwapPRGCtrl();
		}
	}
}

// Altera o registo 2
void CPU::Memory::Mapper1::ChangeReg2()
{
	if(!(SwapCHRSize & 8))
	{
		PPU::Update();

		CurrentCHRBank2 = (mapperData & ((CPU::PRGpages == 0x20) ? 0x0F : 0x1F)) << 2;

		PPU::mapper[4] = &PPU::VRAM[(CurrentCHRBank2 + 0) << 10];
		PPU::mapper[5] = &PPU::VRAM[(CurrentCHRBank2 + 1) << 10];
		PPU::mapper[6] = &PPU::VRAM[(CurrentCHRBank2 + 2) << 10];
		PPU::mapper[7] = &PPU::VRAM[(CurrentCHRBank2 + 3) << 10];
	}
}

// Altera o registo 3
void CPU::Memory::Mapper1::ChangeReg3()
{
	CurrentPRGBank = mapperData & 0xF;
	
	if(BankSize & 0x80)
		Reg3Written = true;

	// Altera a posição do bank
	SwapPRGBanks(CurrentPRGBank);
}

// Creates save state info
void CPU::Memory::Mapper1::SerializeTo(Common::Serializer & buffer)
{
	buffer.Copy(mapperData);
	buffer.Copy(dataLatch);
	buffer.Copy(LastWrite);
	buffer.Copy(LastAddr);
	buffer.Copy(CurrentPPUMirror);
	buffer.Copy(BankSize);
	buffer.Copy(SwapPRGPos);
	buffer.Copy(SwapCHRSize);
	buffer.Copy(HighPRG);
	buffer.Copy(CurrentCHRBank1);
	buffer.Copy(CurrentCHRBank2);
	buffer.Copy(CurrentPRGBank);
	buffer.Copy(Reg3Written);
}

// Loads save state into memory
void CPU::Memory::Mapper1::UnserializeFrom(Common::Unserializer & loader)
{
	loader.Set(mapperData);
	loader.Set(dataLatch);
	loader.Set(LastWrite);
	loader.Set(LastAddr);
	loader.Set(CurrentPPUMirror);
	loader.Set(BankSize);
	loader.Set(SwapPRGPos);
	loader.Set(SwapCHRSize);
	loader.Set(HighPRG);
	loader.Set(CurrentCHRBank1);
	loader.Set(CurrentCHRBank2);
	loader.Set(CurrentPRGBank);
	loader.Set(Reg3Written);

	unsigned char currBank = CurrentPRGBank;
	bool r3 = Reg3Written;

	// Swap the PRG banks
	DoSwapPRGCtrl();

	// Control variables...
	CurrentPRGBank = currBank;
	Reg3Written = r3;

	// Swap the CHR banks
	PPU::mapper[0] = &PPU::VRAM[CurrentCHRBank1 << 10];
	PPU::mapper[1] = &PPU::VRAM[(CurrentCHRBank1 + 1) << 10];
	PPU::mapper[2] = &PPU::VRAM[(CurrentCHRBank1 + 2) << 10];
	PPU::mapper[3] = &PPU::VRAM[(CurrentCHRBank1 + 3) << 10];

	if(SwapCHRSize & 8)
	{
		PPU::mapper[4] = &PPU::VRAM[(CurrentCHRBank1 + 4) << 10];
		PPU::mapper[5] = &PPU::VRAM[(CurrentCHRBank1 + 5) << 10];
		PPU::mapper[6] = &PPU::VRAM[(CurrentCHRBank1 + 6) << 10];
		PPU::mapper[7] = &PPU::VRAM[(CurrentCHRBank1 + 7) << 10];
	}

	else
	{
		PPU::mapper[4] = &PPU::VRAM[(CurrentCHRBank2 + 0) << 10];
		PPU::mapper[5] = &PPU::VRAM[(CurrentCHRBank2 + 1) << 10];
		PPU::mapper[6] = &PPU::VRAM[(CurrentCHRBank2 + 2) << 10];
		PPU::mapper[7] = &PPU::VRAM[(CurrentCHRBank2 + 3) << 10];
	}

	// Set mirror mode
	switch(CurrentPPUMirror & 2)
	{
		// One-screen mirrorring
		case 0:
		{
			PPU::SetMirrorMode(PPU::ONESCREEN);

			// Ainda não percebi se devo ter em conta o bit0 ou não!
			if(CurrentPPUMirror & 1)
			{
				PPU::mapper[12] = PPU::mapper[8]  = PPU::nameTables[1];
				PPU::mapper[13] = PPU::mapper[9]  = PPU::nameTables[1];
				PPU::mapper[14] = PPU::mapper[10] = PPU::nameTables[1];
				PPU::mapper[15] = PPU::mapper[11] = PPU::nameTables[1];
			}
			break;
		}

		// Horiz/vert mirrorring
		case 2:
		{
			PPU::SetMirrorMode((CurrentPPUMirror & 1) ? PPU::HORIZONTAL : PPU::VERTICAL);
		}
	}
}

// Escreve num mapper
void CPU::Memory::Mapper3::Write(unsigned short addr, unsigned char value)
{
	if(addr < 0x8000)
		return;

	PPU::Update();

	BankNum = (value & 0x3) << 3;

	PPU::mapper[0] = &PPU::VRAM[BankNum << 10];
	PPU::mapper[1] = &PPU::VRAM[(BankNum + 1) << 10];
	PPU::mapper[2] = &PPU::VRAM[(BankNum + 2) << 10];
	PPU::mapper[3] = &PPU::VRAM[(BankNum + 3) << 10];

	PPU::mapper[4] = &PPU::VRAM[(BankNum + 4) << 10];
	PPU::mapper[5] = &PPU::VRAM[(BankNum + 5) << 10];
	PPU::mapper[6] = &PPU::VRAM[(BankNum + 6) << 10];
	PPU::mapper[7] = &PPU::VRAM[(BankNum + 7) << 10];
}

// Serializes mapper data. This one is simple :)
void CPU::Memory::Mapper3::SerializeTo(Common::Serializer & buffer)
{
	buffer.Copy(BankNum);
}

// Unserializes mapper data
void CPU::Memory::Mapper3::UnserializeFrom(Common::Unserializer & loader)
{
	loader.Set(BankNum);

	// Swap CHR
	PPU::mapper[0] = &PPU::VRAM[BankNum << 10];
	PPU::mapper[1] = &PPU::VRAM[(BankNum + 1) << 10];
	PPU::mapper[2] = &PPU::VRAM[(BankNum + 2) << 10];
	PPU::mapper[3] = &PPU::VRAM[(BankNum + 3) << 10];

	PPU::mapper[4] = &PPU::VRAM[(BankNum + 4) << 10];
	PPU::mapper[5] = &PPU::VRAM[(BankNum + 5) << 10];
	PPU::mapper[6] = &PPU::VRAM[(BankNum + 6) << 10];
	PPU::mapper[7] = &PPU::VRAM[(BankNum + 7) << 10];
}

// Popula o mapper 4
void CPU::Memory::Mapper4::Populate()
{
	for(int k = 0x0; k < 0x40; k += 0x8)
	{
		int j = k | 0x80;

		mapPages[j]     = &ROM[k << 8];
		mapPages[j + 1] = &ROM[(k + 1) << 8];
		mapPages[j + 2] = &ROM[(k + 2) << 8];
		mapPages[j + 3] = &ROM[(k + 3) << 8];
		mapPages[j + 4] = &ROM[(k + 4) << 8];
		mapPages[j + 5] = &ROM[(k + 5) << 8];
		mapPages[j + 6] = &ROM[(k + 6) << 8];
		mapPages[j + 7] = &ROM[(k + 7) << 8];
	}

	unsigned int start = ((CPU::PRGpages - 1) << 6);

	// Popular a última parte do mapper com o último bank
	for(int i = 0x0; i < 0x40; i += 0x8)
	{
		int j = i | 0xC0;
		int k = i + start;

		mapPages[j]     = &ROM[k << 8];
		mapPages[j + 1] = &ROM[(k + 1) << 8];
		mapPages[j + 2] = &ROM[(k + 2) << 8];
		mapPages[j + 3] = &ROM[(k + 3) << 8];
		mapPages[j + 4] = &ROM[(k + 4) << 8];
		mapPages[j + 5] = &ROM[(k + 5) << 8];
		mapPages[j + 6] = &ROM[(k + 6) << 8];
		mapPages[j + 7] = &ROM[(k + 7) << 8];
	}
}

// Escreve no mapper 4
void CPU::Memory::Mapper4::Write(unsigned short addr, unsigned char value)
{
	unsigned char reg = ((addr & 0x6000) >> 12) | (addr & 1);

	switch(reg)
	{
		case 0: ChangeControlReg(value); break;
		case 1: SwapBanks(value);        break;

		// Altera o mirrorring do PPU
		case 2:
		{
			if(PPU::Mirror == PPU::FOURSCREEN)
				break;
			PPU::Update();
			PPU::SetMirrorMode((value & 1) ? PPU::HORIZONTAL : PPU::VERTICAL);
			break;
		}

		// SRAM Management
		case 3:
		{
			CPU::SRAMEnabled =  (value >> 7)      ? true  : false;
			CPU::SRAMWrite   = ((value >> 6) & 1) ? false : true;

			break;
		}

		// Altera o counter do scanline
		case 4:
		{
			RequestedScanlineCounter = value;

			UpdateIRQCounter();
			CalculateIRQCycle();

			break;
		}

		// Faz nova conta de IRQ baseado no scanline actual
		case 5:
		{
			HasReloadedCounter = false;
			CalculateIRQCycle();

			break;
		}

		// Desactiva o interrupt
		case 6:
		{
			IRQEnabled = false;
			if(IRQPending)
				Interrupts::SetIRQ(Interrupts::MAPPER, InternalIRQCycle);

			IRQPending = false;
			break;
		}

		// Activa o interrupt
		case 7:
		{
			IRQEnabled = true;
			break;
		}
	}
}

// Indica que o endereço do PPU foi alterado
void CPU::Memory::Mapper4::NotifyPPUAddressChanged(unsigned short addr)
{
	UpdateIRQCounter();

	if((CPU::cycles - LastCounterTickCycle) < 16)
		return;

	if((addr & 0x1000) && !(PPUAddr & 0x1000))
	{
		if(!HasReloadedCounter)
		{
			HasReloadedCounter = true;
			CurrentScanlineCounter = RequestedScanlineCounter;
			CounterCyclePos = 0;
		}
		else if(CurrentScanlineCounter)
			--CurrentScanlineCounter;

		if(InternalIRQCycle == 200000)
		{
			if(!CurrentScanlineCounter)
			{
				InternalIRQCycle = CPU::cycles;
				if(!IRQPending)
					Interrupts::SetIRQ(Interrupts::MAPPER, CPU::cycles);
			}
		}
		else
		{
			InternalIRQCycle = CounterCycles[(unsigned char) (CounterCyclePos + CurrentScanlineCounter)];

			if(!IRQPending)
				Interrupts::SetIRQ(Interrupts::MAPPER, InternalIRQCycle);
		}
	}
	else if(!(addr & 0x1000) && (PPUAddr & 0x1000))
	{
		LastCounterTickCycle = CPU::cycles;
	}
	
	PPUAddr = addr;
}

// Altera os banks fixos
void CPU::Memory::Mapper4::UpdateIRQCounter()
{
	if(InternalIRQCycle == 200000)
		return;

	if((CPU::cycles - LastCounterTickCycle) < 16)
		++CounterCyclePos;

	int i = 0;

	while(CounterCycles[CounterCyclePos] < CPU::cycles)
	{
		HasReloadedCounter = true;

		++CounterCyclePos;
		--CurrentScanlineCounter;

		// Avoid endless loop (TODO fix this)
		if(++i == 256)
			break;
	}

	if(CounterCyclePos)
		LastCounterTickCycle = CounterCycles[CounterCyclePos - 1];

	// TODO actualizar o PPUAddr baseado no pixel em que estivermos
}

// Calcula o ciclo do CPU em que deverá suceder o próximo IRQ
void CPU::Memory::Mapper4::CalculateIRQCycle()
{
	// Se o contador interno não funcionar, preencher tudo com 200000!
	if((PPU::Registers::SpriteTable == PPU::Registers::BackgroundTable) || !PPU::doRender)
	{
		std::fill_n(CounterCycles, 256, 200000);
		InternalIRQCycle = 200000;

		if(!IRQPending)
			Interrupts::UnsetIRQ(Interrupts::MAPPER);

		CounterCyclePos = 0;
		return;
	}

	// Fazer reload ao counter, caso seja aplicável
	if(!HasReloadedCounter)
	{
		std::fill_n(CounterCycles, 256, 200000);
		CurrentScanlineCounter = RequestedScanlineCounter;
		CounterCyclePos = 0;
	}

	// Calcular os ciclos
	int A12PixelToggle = (!PPU::Registers::BackgroundTable) ? 260 : 324;
	int currentPixel = CPU::cycles % 341;
	int startcycle;

	// Quando inicia em vblank, o primeiro toggle é no pre-render scanline
	if(CPU::cycles < 6820)
		startcycle = 6820 + A12PixelToggle;

	// De outro modo é no scanline actual ou no seguinte, dependendo do pixel em que nos encontrarmos
	else
	{
		startcycle = CPU::cycles - (CPU::cycles % 341) + A12PixelToggle;

		if(currentPixel >= A12PixelToggle)
			startcycle += 341;
	}

	// Se o ciclo actual já for no vblank, o primeiro toggle é no pre-render scanline do próximo frame
	if(startcycle > (CPU::cyclesForFrame - 341))
		startcycle = CPU::cyclesForFrame + 6820 + A12PixelToggle;

	// Vars úteis
	int mycycle = startcycle;
	CounterCycles[CounterCyclePos] = startcycle;

	int nextFrame = CPU::cyclesForFrame - 341;

	unsigned char pos = CounterCyclePos;

	// Loop que coloca na array os próximos toggles
	for(int i = CurrentScanlineCounter; i > 0; -- i)
	{
		++pos;
		int oldcycle = mycycle;
		mycycle += 341;

		if((oldcycle < nextFrame) && (mycycle >= nextFrame))
			mycycle = CPU::cyclesForFrame + 6820 + A12PixelToggle;

		if(((mycycle - CPU::cyclesForFrame) < nextFrame) && ((mycycle - CPU::cyclesForFrame) >= nextFrame))
			mycycle += 7161;

		CounterCycles[pos] = mycycle;
	}

	// Definir o IRQ
	InternalIRQCycle = mycycle;

	if(!IRQPending)
		Interrupts::SetIRQ(Interrupts::MAPPER, mycycle);
}

// Indica que o render foi activado/desactivado
void CPU::Memory::Mapper4::NotifyToggleRender()
{
	UpdateIRQCounter();
	CalculateIRQCycle();
}

// Indica que estamos num novo frame
void CPU::Memory::Mapper4::NotifyNewFrame()
{
	UpdateIRQCounter();

	LastCounterTickCycle -= CPU::cyclesForFrame;

	if(InternalIRQCycle == 200000)
		return;

	unsigned char pos = CounterCyclePos;
	int counter = CurrentScanlineCounter;

	// Este código evita que o last tick seja superior ao ciclo actual quando se chama o UpdateIRQ
	if(pos)
	{
		CounterCycles[pos - 1] -= CPU::cyclesForFrame;

		// Diminuir ao primeiro ciclo
		if(pos > 1)
			CounterCycles[0] -= CPU::cyclesForFrame;
	}


	while(counter >= 0)
	{
		CounterCycles[pos] -= CPU::cyclesForFrame;
		++pos;
		--counter;
	}

	InternalIRQCycle -= CPU::cyclesForFrame;

	if(InternalIRQCycle < 0)
		InternalIRQCycle = 0;

	if(!IRQPending)
		Interrupts::SetIRQ(Interrupts::MAPPER, InternalIRQCycle);

	else
		Interrupts::SetIRQ(Interrupts::MAPPER, 0);
}

// Indica que ocorreu um interrupt que teve origem no mapper
bool CPU::Memory::Mapper4::NotifyInterrupt()
{
	if(!IRQPending)
	{
		if(IRQEnabled)
			IRQPending = true;
	}

	// Se estiver pendente IRQ, apenas fazer reload do counter se tiver atingido o limite de scanlines pedidos
	else if(CPU::cycles < InternalIRQCycle)
		return IRQEnabled;

	HasReloadedCounter = false;
	CalculateIRQCycle();

	return IRQEnabled;
}

// Altera os comandos a executar pelo mapper
void CPU::Memory::Mapper4::ChangeControlReg(unsigned char value)
{
	Command = value & 7;

	unsigned char prgPos = (value & 0x40) >> 6;

	if(prgPos != SwapPRGPos)
	{
		SwapPRGPos = prgPos;
		SwapFixedBanks();
	}

	unsigned char chrPos = (value & 0x80) >> 5;

	if(chrPos != SwapCHRPos)
	{
		PPU::Update();

		SwapCHRPos = chrPos;

		PPU::mapper[SwapCHRPos ^ 0] = &PPU::VRAM[ CHRBanks[0]      << 10];
		PPU::mapper[SwapCHRPos ^ 1] = &PPU::VRAM[(CHRBanks[0] + 1) << 10];
		PPU::mapper[SwapCHRPos ^ 2] = &PPU::VRAM[ CHRBanks[1]      << 10];
		PPU::mapper[SwapCHRPos ^ 3] = &PPU::VRAM[(CHRBanks[1] + 1) << 10];

		PPU::mapper[SwapCHRPos ^ 4] = &PPU::VRAM[CHRBanks[2] << 10];
		PPU::mapper[SwapCHRPos ^ 5] = &PPU::VRAM[CHRBanks[3] << 10];
		PPU::mapper[SwapCHRPos ^ 6] = &PPU::VRAM[CHRBanks[4] << 10];
		PPU::mapper[SwapCHRPos ^ 7] = &PPU::VRAM[CHRBanks[5] << 10];
	}
}

// Altera os banks fixos
void CPU::Memory::Mapper4::SwapFixedBanks()
{
	unsigned char pos[2] = { 0x80, 0xC0 };

	unsigned int start = PRGBank1 << 5;

	for(unsigned int i = 0; i < 0x20; i += 8)
	{
		int j = i | pos[SwapPRGPos];
		int k = i + start;

		mapPages[j]     = &ROM[k << 8];
		mapPages[j + 1] = &ROM[(k + 1) << 8];
		mapPages[j + 2] = &ROM[(k + 2) << 8];
		mapPages[j + 3] = &ROM[(k + 3) << 8];
		mapPages[j + 4] = &ROM[(k + 4) << 8];
		mapPages[j + 5] = &ROM[(k + 5) << 8];
		mapPages[j + 6] = &ROM[(k + 6) << 8];
		mapPages[j + 7] = &ROM[(k + 7) << 8];
	}

	start = ((CPU::PRGpages - 1) << 6);

	for(unsigned int i = 0; i < 0x20; i += 8)
	{
		int j = i | pos[SwapPRGPos ^ 1];
		int k = i + start;

		mapPages[j]     = &ROM[k << 8];
		mapPages[j + 1] = &ROM[(k + 1) << 8];
		mapPages[j + 2] = &ROM[(k + 2) << 8];
		mapPages[j + 3] = &ROM[(k + 3) << 8];
		mapPages[j + 4] = &ROM[(k + 4) << 8];
		mapPages[j + 5] = &ROM[(k + 5) << 8];
		mapPages[j + 6] = &ROM[(k + 6) << 8];
		mapPages[j + 7] = &ROM[(k + 7) << 8];
	}
}


// Altera os banks
void CPU::Memory::Mapper4::SwapBanks(unsigned char value)
{
	// Alterar os prg banks
	if(Command >= 6)
	{
		if(Command == 6)
			PRGBank1 = value & ((CPU::PRGpages << 1) - 1);
		else
		{
			PRGBank2 = value & ((CPU::PRGpages << 1) - 1);
		}

		unsigned char mypos[2] = { 0x80, 0xC0 };

		unsigned int start = ((value & ((CPU::PRGpages << 1) - 1)) << 5);
		unsigned int pos = (Command == 6) ? mypos[SwapPRGPos] : 0xA0;

		for(unsigned int i = 0; i < 0x20; i += 8)
		{
			int j = i | pos;
			int k = i + start;

			mapPages[j]     = &ROM[k << 8];
			mapPages[j + 1] = &ROM[(k + 1) << 8];
			mapPages[j + 2] = &ROM[(k + 2) << 8];
			mapPages[j + 3] = &ROM[(k + 3) << 8];
			mapPages[j + 4] = &ROM[(k + 4) << 8];
			mapPages[j + 5] = &ROM[(k + 5) << 8];
			mapPages[j + 6] = &ROM[(k + 6) << 8];
			mapPages[j + 7] = &ROM[(k + 7) << 8];
		}

		return;
	}

	PPU::Update();


	// Alterar os CHR banks
	if(Command < 2)
	{
		PPU::mapper[SwapCHRPos ^ (Command << 1)]       = &PPU::VRAM[ (value & 0xFE & ((CPU::CHRpages << 3) - 1))      << 10];
		PPU::mapper[SwapCHRPos ^ ((Command << 1) | 1)] = &PPU::VRAM[((value & 0xFE & ((CPU::CHRpages << 3) - 1)) | 1) << 10];

		CHRBanks[Command] = value & 0xFE & ((CPU::CHRpages << 3) - 1);

		return;
	}

	PPU::mapper[SwapCHRPos ^ (Command + 2)] = &PPU::VRAM[(value & ((CPU::CHRpages << 3) - 1)) << 10];

	CHRBanks[Command] = value & ((CPU::CHRpages << 3) - 1);
}

// Serializes data to save state
void CPU::Memory::Mapper4::SerializeTo(Common::Serializer & buffer)
{
	buffer.Copy(Command);
	buffer.Copy(SwapPRGPos);
	buffer.Copy(SwapCHRPos);
	buffer.Copy(PRGBank1);
	buffer.Copy(PRGBank2);
	buffer.Copy(CHRBanks, 6 * sizeof(unsigned char));
	buffer.Copy(CurrentScanlineCounter);
	buffer.Copy(RequestedScanlineCounter);
	buffer.Copy(PPUAddr);
	buffer.Copy(LastCounterTickCycle);
	buffer.Copy(InternalIRQCycle);
	buffer.Copy(IRQEnabled);
	buffer.Copy(IRQPending);
	buffer.Copy(HasReloadedCounter);
	buffer.Copy(CounterCycles, sizeof(int) * 256);
	buffer.Copy(CounterCyclePos);
}

// Reads data from save state
void CPU::Memory::Mapper4::UnserializeFrom(Common::Unserializer & loader)
{
	loader.Set(Command);
	loader.Set(SwapPRGPos);
	loader.Set(SwapCHRPos);
	loader.Set(PRGBank1);
	loader.Set(PRGBank2);
	loader.Set(CHRBanks, 6 * sizeof(unsigned char));
	loader.Set(CurrentScanlineCounter);
	loader.Set(RequestedScanlineCounter);
	loader.Set(PPUAddr);
	loader.Set(LastCounterTickCycle);
	loader.Set(InternalIRQCycle);
	loader.Set(IRQEnabled);
	loader.Set(IRQPending);
	loader.Set(HasReloadedCounter);
	loader.Set(CounterCycles, sizeof(int) * 256);
	loader.Set(CounterCyclePos);

	// Swap the first PRG bank
	SwapFixedBanks();

	// Swap the second PRG bank
	unsigned int start = PRGBank2 << 5;

	for(unsigned int i = 0; i < 0x20; i += 8)
	{
		int j = i | 0xA0;
		int k = i + start;

		mapPages[j]     = &ROM[k << 8];
		mapPages[j + 1] = &ROM[(k + 1) << 8];
		mapPages[j + 2] = &ROM[(k + 2) << 8];
		mapPages[j + 3] = &ROM[(k + 3) << 8];
		mapPages[j + 4] = &ROM[(k + 4) << 8];
		mapPages[j + 5] = &ROM[(k + 5) << 8];
		mapPages[j + 6] = &ROM[(k + 6) << 8];
		mapPages[j + 7] = &ROM[(k + 7) << 8];
	}

	// Swap the CHR banks
	PPU::mapper[SwapCHRPos ^ 0] = &PPU::VRAM[ CHRBanks[0]      << 10];
	PPU::mapper[SwapCHRPos ^ 1] = &PPU::VRAM[(CHRBanks[0] + 1) << 10];
	PPU::mapper[SwapCHRPos ^ 2] = &PPU::VRAM[ CHRBanks[1]      << 10];
	PPU::mapper[SwapCHRPos ^ 3] = &PPU::VRAM[(CHRBanks[1] + 1) << 10];

	PPU::mapper[SwapCHRPos ^ 4] = &PPU::VRAM[CHRBanks[2] << 10];
	PPU::mapper[SwapCHRPos ^ 5] = &PPU::VRAM[CHRBanks[3] << 10];
	PPU::mapper[SwapCHRPos ^ 6] = &PPU::VRAM[CHRBanks[4] << 10];
	PPU::mapper[SwapCHRPos ^ 7] = &PPU::VRAM[CHRBanks[5] << 10];
}

// Indica que o PPU foi criado
void CPU::Memory::Mapper7::NotifyPPUCreated()
{
	PPU::SetMirrorMode(PPU::ONESCREEN);
}

// Escreve no mapper 7
void CPU::Memory::Mapper7::Write(unsigned short addr, unsigned char value)
{
	PPU::Update();

	if(addr & 0x8000)
	{
		// Update internal vars
		NameTable = (value >> 4) & 1;

		PPU::mapper[12] = PPU::mapper[8]  = PPU::nameTables[NameTable];
		PPU::mapper[13] = PPU::mapper[9]  = PPU::nameTables[NameTable];
		PPU::mapper[14] = PPU::mapper[10] = PPU::nameTables[NameTable];
		PPU::mapper[15] = PPU::mapper[11] = PPU::nameTables[NameTable];

		// Swap PRG pages, if applicable
		if(BankNum != ((value & 0x7) << 7))
		{
			BankNum = ((value & 0x7) << 7);

			for(int i = 0x0; i < 0x80; i += 0x8)
			{
				int j = i | 0x80;

				unsigned int k = i + BankNum;

				mapPages[j]     = &ROM[k << 8];
				mapPages[j + 1] = &ROM[(k + 1) << 8];
				mapPages[j + 2] = &ROM[(k + 2) << 8];
				mapPages[j + 3] = &ROM[(k + 3) << 8];
				mapPages[j + 4] = &ROM[(k + 4) << 8];
				mapPages[j + 5] = &ROM[(k + 5) << 8];
				mapPages[j + 6] = &ROM[(k + 6) << 8];
				mapPages[j + 7] = &ROM[(k + 7) << 8];
			}
		}
	}

	else
	{
		mapPages[addr >> 8][addr & 0xFF] = value;
	}
}

// Serializes the data. Easy enough :)
void CPU::Memory::Mapper7::SerializeTo(Common::Serializer & buffer)
{
	buffer.Copy(NameTable);
	buffer.Copy(BankNum);
}

// Unserializes the data
void CPU::Memory::Mapper7::UnserializeFrom(Common::Unserializer & loader)
{
	loader.Set(NameTable);
	loader.Set(BankNum);

	// Update mirrorring
	PPU::mapper[12] = PPU::mapper[8]  = PPU::nameTables[NameTable];
	PPU::mapper[13] = PPU::mapper[9]  = PPU::nameTables[NameTable];
	PPU::mapper[14] = PPU::mapper[10] = PPU::nameTables[NameTable];
	PPU::mapper[15] = PPU::mapper[11] = PPU::nameTables[NameTable];

	// Update PRG Bank
	for(int i = 0x0; i < 0x80; i += 0x8)
	{
		int j = i | 0x80;

		unsigned int k = i + BankNum;

		mapPages[j]     = &ROM[k << 8];
		mapPages[j + 1] = &ROM[(k + 1) << 8];
		mapPages[j + 2] = &ROM[(k + 2) << 8];
		mapPages[j + 3] = &ROM[(k + 3) << 8];
		mapPages[j + 4] = &ROM[(k + 4) << 8];
		mapPages[j + 5] = &ROM[(k + 5) << 8];
		mapPages[j + 6] = &ROM[(k + 6) << 8];
		mapPages[j + 7] = &ROM[(k + 7) << 8];
	}
}