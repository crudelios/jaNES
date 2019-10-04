// Registers.cpp
// Registers do PPU

#define PPUREGISTERS_CPP

#include "PPU.h"
#include "Internal.h"
#include "Registers.h"
#include "FrameBuffer.h"
#include "../CPU/Interrupts.h"
#include "../CPU/Memory.h"

// --------------------
// Leitura de registers
// --------------------

// Ler PPU Status
unsigned char PPU::Registers::ReadPPUStatus()
{
	if(ready > CPU::cycles)
		return 0;

	ready = 0;

	flipFlop = 0;

	Update();

	// Os 5 bits não utilizados são lidos do Open Bus
	return (Registers::Status::Pack() | ((DecayFrame > 40) ? 0 : (Decay & 0x1F)));
}

// Ler OAM Data
unsigned char PPU::Registers::ReadOAMData()
{
	Update();

	if(((SPRRAMaddr & 3) == 2) && (SPRRAMptr == &SPRRAM[SPRRAMaddr]))
		DecayFrame = *SPRRAMptr;

	return *SPRRAMptr;
}

// Ler PPU Data
unsigned char PPU::Registers::ReadPPUData()
{
	if(ready > CPU::cycles)
		return 0;

	ready = 0;

	if(LastReadPPUData == (CPU::cycles - 3))
		return Decay;

	LastReadPPUData = CPU::cycles;

	// Só admite leitura/escrita se estivermos no VBlank ou não estivermos a renderizar
    if (doRender && !PPU::InVBlank())
    {
        AddressLatch::IncrementVReadHoriz();
        AddressLatch::IncrementVReadVert();
        return CPU::Memory::openBusValue;
    }

	Update();

	register unsigned short addr = AddressLatch::GetActualAddress() & 0x3FFF;

	// Aumentar o latch (e os valores internos)
	AddressLatch::Increment2007();

	// Pallete
	if(addr > 0x3EFF)
	{
		VRAMTempBuffer = mapper[addr >> 10][addr & 0x3FF];

		return (Pallete[addr & 0x1F] & grayscale) | (Decay & 0xC0);
	}

	// Resto da memória tem buffer, primeiro read retorna o valor temporário
	unsigned char ret = VRAMTempBuffer;

	VRAMTempBuffer = mapper[addr >> 10][addr & 0x3FF];

	Decay = ret;
	DecayFrame = 0;

	return ret;
}

// Leitura de registos write only
unsigned char PPU::Registers::ReadDummy()
{
	return (DecayFrame > 40) ? 0 : Decay;
}

// --------------------
// Escrita de registers
// --------------------

// Escrever no PPU Controller
void PPU::Registers::WritePPUController(unsigned char value)
{
	Update();

	Decay = value;
	DecayFrame = 0;

	// Separar dados
	AddressLatch::TempCurrentNameTable =  value & 0x3;
	increment       = (value & 0x4)  ?  32 : 1;
	SpriteTable     = (value & 0x8)  >> 1;
	BackgroundTable = (value & 0x10) >> 2;
	spriteHeight    = (value & 0x20) ?  16 : 8;

	if(CPU::cycles > 6816)
		Status::IsVBlank = 0;

	CPU::Interrupts::NMIPending = (Status::IsVBlank & (CPU::Interrupts::NMIEnabled ^ (value >> 7)) & (value >> 7)) << 1;

	CPU::Interrupts::NMIThisFrame = (CPU::cycles >= CPU::cyclesForFrame) && !CPU::Interrupts::NMIEnabled && (value & 0x80);

	CPU::Interrupts::NMIPending |= ((CPU::cycles >= CPU::cyclesForFrame) && CPU::Interrupts::NMIEnabled && !(value & 0x80)) ? 1 : 0;

	// Interrupts
	CPU::Interrupts::NMIEnabled = (value & 0x80) >> 7;
}

// Escrever no PPU Mask
void PPU::Registers::WritePPUMask(unsigned char value)
{
	Update();

	Decay = value;
	DecayFrame = 0;

	// Separar dados
	grayscale      = (value       & 1) ? 0x30 : 0x3F;
	clipBackground = (value >> 1) & 1;
	clipSprites    = (value >> 2) & 1;
	showBackground = (value >> 3) & 1;
	showSprites    = (value >> 4) & 1;
	palleteNum     = (value >> 5);

	CurrPallete = PalleteColors[0][palleteNum];

	// Alterar estado do render
	if((CPU::cycles < 7155) && IsEvenFrame)
	{
		if(showBackground)
		{
			CPU::cyclesForFrame     = 89341;
			CPU::delayedTotalCycles = 89343;
			remVBLCycle = 1;
		}
		else
		{
			CPU::cyclesForFrame     = 89342;
			CPU::delayedTotalCycles = 89344;
			remVBLCycle = 0;
		}
	}

	bool oldRender = doRender;

	// Não fazer render quando tudo tiver desligado
	if(!showBackground && !showSprites)
		doRender = false;

	else
		doRender = true;

	// Notificar o mapper de que o render foi activado/desactivado
	if(oldRender != doRender)
	{
		CPU::Memory::mapper->NotifyToggleRender();
	}

	// Recalcular sprite hit consoante o render
	//if(CPU::cycles > 6816)
	//{
	//	if(doRender)
	//	{
	//		//Status::CalculateSprite0(CPU::cycles);
	//		Status::CalculateSpriteOverflow(CPU::cycles);
	//	}
	//	else
	//	{
	//		//Status::HasHitSprite0    = (Status::HasHitSprite0    && (Status::HasHitSprite0    <= CPU::cycles)) ? Status::HasHitSprite0    : 95000;
	//		Status::HasHitMaxSprites = (Status::HasHitMaxSprites && (Status::HasHitMaxSprites <= CPU::cycles)) ? Status::HasHitMaxSprites : 95000;
	//	}
	//}
}

// Escrever no PPU Status - read only, não fazer nada
void PPU::Registers::WritePPUStatus(unsigned char value)
{
	Decay = value;
	DecayFrame = 0;

	return;
}

// Escrever endereço de OAM
void PPU::Registers::WriteOAMAddr(unsigned char value)
{
	Update();

	Decay = value;
	DecayFrame = 0;

	SPRRAMaddr = value;
	SPRRAMptr  = &SPRRAM[value];
}

// Escrever dados no OAM
void PPU::Registers::WriteOAMData(unsigned char value)
{
	Update();

	Decay = value;
	DecayFrame = 0;

	if((SPRRAMaddr & 3) == 2)
		value &= 0xE3;

	SPRRAM[SPRRAMaddr++] = value;

	SPRRAMptr = &SPRRAM[SPRRAMaddr];
}

// Alterar as coordenadas de scroll
void PPU::Registers::WritePPUScroll(unsigned char value)
{
	Update();

	Decay = value;
	DecayFrame = 0;

	switch(flipFlop)
	{
		case 0:
			AddressLatch::TempTileXScroll = value >> 3;
			AddressLatch::TempFineXScroll = value & 0x7;
			AddressLatch::FineXScroll = value & 0x7;
			break;

		case 1:
			AddressLatch::TempTileYScroll = value >> 3;
			AddressLatch::TempFineYScroll = value & 0x7;
	}

	flipFlop ^= 1;
}

// Alterar endereço do PPU
void PPU::Registers::WritePPUAddr(unsigned char value)
{
	Update();

	Decay = value;
	DecayFrame = 0;

	switch(flipFlop)
	{
		case 0:
			AddressLatch::TempTileYScroll = ((value & 0x3) << 3); // | (AddressLatch::TempTileYScroll & 0x7);
			AddressLatch::TempCurrentNameTable = (value >> 2) & 0x3;
			AddressLatch::TempFineYScroll = ((value >> 4) & 0x3); // | (AddressLatch::TempFineYScroll & 0x4); 
			break;

		case 1:
			AddressLatch::TempTileXScroll = value & 0x1F;
			AddressLatch::TempTileYScroll = (value >> 5) | (AddressLatch::TempTileYScroll & 0x18);

			// Copiar valores temporários para definitivos
			AddressLatch::TileXScroll = AddressLatch::TempTileXScroll;
			AddressLatch::TileYScroll = AddressLatch::TempTileYScroll;
			AddressLatch::FineYScroll = AddressLatch::TempFineYScroll;
			AddressLatch::CurrentNameTable = AddressLatch::TempCurrentNameTable;

			CPU::Memory::mapper->NotifyPPUAddressChanged(AddressLatch::GetActualAddress() & 0x3FFF);
	}

	flipFlop ^= 1;
}

// Alterar dados do PPU
void PPU::Registers::WritePPUData(unsigned char value)
{
	Update();

	Decay = value;
	DecayFrame = 0;

	unsigned short addr = AddressLatch::GetActualAddress() & 0x3FFF;

	// Pallete
	if(addr > 0x3EFF)
	{
		addr &= 0x1F;

		if(addr & 3)
			Pallete[addr] = value & 0x3F;
		else
			Pallete[addr & 0xF] = Pallete[(addr & 0xF) | 0x10] = value & 0x3F;
	}

	else if(!ReadOnly[addr >> 10])
		mapper[addr >> 10][addr & 0x3FF] = value;

    if (doRender && !PPU::InVBlank())
    {
        AddressLatch::IncrementVReadHoriz();
        AddressLatch::IncrementVReadVert();
    }
    else
	    AddressLatch::Increment2007();
}


// ----------------------------------------------------------------------------
// MEMBROS DE LEITURA E ESCRITA DE REGISTO ------------------------------------
// ----------------------------------------------------------------------------

// Escreve um valor no registo
void PPU::RegWrite(int reg, unsigned char value)
{
	Registers::WriteFunc[reg](value);
}

// Obtém um valor do registo sem alterar dados
unsigned char PPU::RegDebugRead(int reg)
{
	// Unimplemented
	dbgRead = true;
	//unsigned char val = Registers::ReadFunc[reg]();
	dbgRead = false;

	return 0;
}

// Obtém um valor do registo
unsigned char PPU::RegRead(int reg)
{
	return Registers::ReadFunc[reg]();
}

unsigned char (*PPU::Registers::ReadFunc[8])() = 
{
	PPU::Registers::ReadDummy,     PPU::Registers::ReadDummy,
	PPU::Registers::ReadPPUStatus, PPU::Registers::ReadDummy,
	PPU::Registers::ReadOAMData,   PPU::Registers::ReadDummy,
	PPU::Registers::ReadDummy,     PPU::Registers::ReadPPUData
};

void (*PPU::Registers::WriteFunc[8])(unsigned char value) = 
{
	PPU::Registers::WritePPUController, PPU::Registers::WritePPUMask,
	PPU::Registers::WritePPUStatus,     PPU::Registers::WriteOAMAddr,
	PPU::Registers::WriteOAMData,       PPU::Registers::WritePPUScroll,
	PPU::Registers::WritePPUAddr,       PPU::Registers::WritePPUData
};



// ----------------------------------------------------------------------------
// TRATAMENTO DE ADDRESS LATCH ------------------------------------------------
// ----------------------------------------------------------------------------

// Incrementa os latches verticais durante o render e faz reset dos latches horizontais
void PPU::Registers::AddressLatch::IncrementVReadVert()
{
	// Incrementar os latches
	TileYScroll += (++FineYScroll & 0x8) >> 3;
	FineYScroll &= 0x7;

	if(TileYScroll < 30)
		return;

	// Se o tile y scroll tiver chegado a 32, coloca-se a zero mas não se incrementa mais nada
	if(TileYScroll & 0x20)
	{
		TileYScroll = 0;
		return;
	}

	// Só se incrementa se o TileYScroll tiver chegado a 30
	if(TileYScroll != 30)
		return;

	TileYScroll = 0;
	CurrentNameTable ^= 2;
}

// Obtém um pointer para um tile específico
unsigned char * PPU::Registers::AddressLatch::GetTilePtrFromPos(unsigned int x, unsigned int y)
{
	// Overflow!
	if(x > 255 || y > 239)
		return nullptr;

	x += TempFineXScroll;
	int horizTile  = TempTileXScroll + (x >> 3);
	int nameTable  = TempCurrentNameTable ^ ((horizTile & 0x20) >> 5);
	horizTile     &= 0x1F;

	y += TempFineYScroll;
	int vertTile  = TempTileYScroll + (y >> 3);
	nameTable    |= TempCurrentNameTable ^ ((vertTile & 0x20) >> 4);
	vertTile      &= 0x1F;

	unsigned int tileIndex = mapper[nameTable + 8][horizTile | (vertTile << 5)] << 4;

	return &mapper[BackgroundTable | ((tileIndex & 0xC00) >> 10)][tileIndex & 0x3FF];
}

// Calcula se houve um sprite 0 hit
unsigned int PPU::Registers::Status::CalculateSprite0()
{
	if((CPU::cycles < 6818))
		return (HasHitSprite0) ? 1 : 0;

	if(HasHitSprite0 == 94999)
		HasHitSprite0 = 0;

	if(HasHitSprite0)
		return 1;

	if(!(showBackground && showSprites))
			return (HasHitSprite0) ? 1 : 0;

	if(!(clipBackground && clipSprites))
	{
		if(!PPU::SPRRAM[3])
			return 0;
	}

	unsigned int SP0Y = SPRRAM[0] + 1;

	// Sprite está fora do alcance
	if((SP0Y > 0xEF) || (SPRRAM[3] >= 0xFF))
		return 0;

	unsigned int totalPixels = CPU::cycles + remVBLCycle;

	unsigned int numScanLine = totalPixels / 341;
	unsigned int numPixel = totalPixels % 341;
	numScanLine = (numScanLine < 21) ? 0 : (numScanLine - 21);
	numPixel = (numPixel > 255) ? 256: numPixel;

	if(numScanLine > 239)
	{
		numScanLine = 239;
		numPixel = 256;
	}

	// O sprite 0 está fora de alcance...
	if(SP0Y > numScanLine)
		return 0;

	unsigned char spriteIndex = (Registers::spriteHeight == 8) ? SPRRAM[1] : SPRRAM[1] & 0xFE;

	unsigned char * spritePtr = &mapper[((Registers::spriteHeight & 16) ? 0 : Registers::SpriteTable) | (((spriteIndex << 4) & 0xC00) >> 10)][(spriteIndex << 4) & 0x3FF];
	
	unsigned char * sptr2 = &mapper[((Registers::spriteHeight == 8) ? Registers::SpriteTable : 0) | (((spriteIndex << 4) & 0xC00) >> 10)][(spriteIndex << 4) & 0x3FF];

	// Obter localização do sprite 0
	//unsigned char * spritePtr = &mapper[((Registers::spriteHeight & 16) ? 0 : Registers::SpriteTable) | (((SPRRAM[1] << 4) & 0xC00) >> 10)][(SPRRAM[1] << 4) & 0x3FF];
	
	int x = SPRRAM[3];

	//unsigned int ylimit = ((SP0Y + Registers::spriteHeight) >= 240) ? 240 : (SP0Y + Registers::spriteHeight);
	unsigned int ylimit = ((SP0Y + Registers::spriteHeight) > numScanLine) ? numScanLine : (SP0Y + Registers::spriteHeight - 1);
	//printf("%i %i\n", SP0Y, ylimit);

	unsigned int xlimit = ((x + 8) >= 255) ? 255 : (x + 8);

	int xOffset =  (!(clipBackground && clipSprites) && (x < 8)) ? (8 - x) : 0;

	// Vars úteis
	int invertX =  SPRRAM[2] & 0x40;
	int startY  = (SPRRAM[2] & 0x80) ? ((Registers::spriteHeight == 8) ? 7 : 23) : 0;
	int stepY   = (SPRRAM[2] & 0x80) ? -1 : 1;

	while(SP0Y <= ylimit) 
	{
		if(startY & 8)
			startY += stepY << 3;

		// Obter sprite e inverter bits, se for caso disso
		// O código criptico na segunda linha inverte a ordem dos bits (não Joni, não fui eu que o inventei)
		register unsigned char sprite = spritePtr[startY] | spritePtr[startY + 8];
		sprite = (invertX) ? (unsigned char) ((sprite * 0x0202020202ULL & 0x010884422010ULL) % 1023) : sprite;
	
		int bufferPos = (SP0Y << 8) + x + xOffset;

		int startX = x + xOffset;
		int endX = xlimit; 
		//if(SP0Y == ylimit)
		//	endX = (xlimit > numPixel) ? (numPixel + 1) : xlimit;

		//printf("%i ", a++);

		while(startX < endX)
		{
			//if(((sprite >> (7 - (startX - x))) & 1))
			//	printf("X");
			//else printf("-");
			if(((sprite >> (7 - (startX - x))) & 1) & PixelOpacity[bufferPos])
			{
				HasHitSprite0 = 10000;
				return 1;
			}
			++bufferPos;
			++startX;
		}

		startY += stepY;
		++SP0Y;

		//printf("\n");

	}

	//printf("\n");

	return 0;
}

struct ScanLineSpriteInfo
{
	unsigned char sprites;
	unsigned char spriteNum[9];
	ScanLineSpriteInfo() : sprites(0) {}
};

// Calcula se houve mais de 8 sprites num scanline
unsigned int PPU::Registers::Status::CalculateSpriteOverflow(int start)
{
	//if((CPU::cycles < 6818))
		return (HasHitMaxSprites) ? 1 : 0;

	if(HasHitMaxSprites == 94999)
		HasHitMaxSprites = 0;

	if(!doRender)
	{
		HasHitMaxSprites = 95000;
		return 0;
	}

	if(HasHitMaxSprites)
		return (HasHitMaxSprites > CPU::cycles) ? 0 : 1;

	ScanLineSpriteInfo lineData[240];
	unsigned char firstScanLine = 255;
	unsigned char filledScanLines[128] = {0};
	unsigned char filledScanLinesCount = 0;

	static int count = 0;
	count++;

	// Calcular max sprites
	for(unsigned int i = 0; i < 256; i += 4)
	{
		if(SPRRAM[i] > 239)
			continue;

		for(unsigned int j = 0; j < spriteHeight; ++ j)
		{
			register unsigned char myScanLine = SPRRAM[i] + j;

			if(myScanLine > 239)
				break;

			if(lineData[myScanLine].sprites > 7)
				continue;

			lineData[myScanLine].spriteNum[lineData[myScanLine].sprites++] = i;

			if(lineData[myScanLine].sprites == 8)
				filledScanLines[filledScanLinesCount++] = myScanLine;
		}
	}

	// Pathological behaviour
	for(unsigned int i = 0; i < filledScanLinesCount; ++ i)
	{
		unsigned char pathology = 0;

		for(unsigned int sprite = lineData[filledScanLines[i]].spriteNum[7] + 4; sprite < 256; sprite += 4)
		{
			unsigned char pos = filledScanLines[i] - SPRRAM[sprite + (pathology & 3)];

			// Found a hit!
			if(pos < spriteHeight)
			{
				lineData[filledScanLines[i]].spriteNum[8] = sprite;
				firstScanLine = (filledScanLines[i] < firstScanLine) ? filledScanLines[i] : firstScanLine;
				break;
			}
			
			++pathology;
		}
	}

	// Não há max sprites
	if(firstScanLine == 255)
	{
		HasHitMaxSprites = 95000;
		return 0;
	}

	HasHitMaxSprites = 7273 - remVBLCycle + (firstScanLine * 341) + (lineData[firstScanLine].spriteNum[8] >> 1);
	return (HasHitMaxSprites <= CPU::cycles);
}
