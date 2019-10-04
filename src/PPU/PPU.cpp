// PPU.cpp
// Tratamento do PPU

#define PPU_CPP

#include "PPU.h"
#include "Internal.h"
#include "Registers.h"
#include "FrameBuffer.h"
#include "../CPU/CPU.h"
#include "../CPU/Interrupts.h"
#include "../CPU/Memory.h"
#include "../Common/Common.h"

#include <cstdio>
#include <cstring>

namespace PPU
{
	inline void GetTileAttribute();
	inline void GetTilePtr();
	inline void GetTileIndex();
}

void PPU::DrawFrame()
{
	// Acrescentar frame
	IsEvenFrame ^= 1;
	Registers::LastReadPPUData = -3;

	if(ready)
	{
		ready -= CPU::cyclesForFrame;

		if(ready < 0)
			ready = 0;

		//else
			//LastScanLine  = 0;
			//LastPixel     = 0;
			//FrameBuffer::FinalBmpCount = 0;
			//CalculatedFrameSync = false;

			//GetTilePtr();

			//if(IsEvenFrame && Registers::showBackground)
			//{
			//	CPU::cyclesForFrame     = 89341;
			//	CPU::delayedTotalCycles = 89343;
			//	remVBLCycle = 1;
			//}
			//else
			//{
			//	CPU::cyclesForFrame     = 89342;
			//	CPU::delayedTotalCycles = 89344;
			//	remVBLCycle = 0;
			//}

			//return;
		
	}

	FrameBuffer::Draw();

	if(Registers::Status::HasHitSprite0)
		Registers::Status::HasHitSprite0 = 2;

	//if(Registers::Status::HasHitMaxSprites && (Registers::Status::HasHitMaxSprites < CPU::cycles))
	//	Registers::Status::HasHitMaxSprites = 94999;
	//else Registers::Status::HasHitMaxSprites = 0;

	if(Registers::Status::HasHitMaxSprites)
		Registers::Status::HasHitMaxSprites = 2;

	FrameDone = false;

	Registers::Status::IsVBlank = 1 ^ Registers::Status::ReadVBlankThisFrame;
	Registers::Status::ReadVBlankThisFrame = 0;

	if(IsEvenFrame && Registers::showBackground)
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

	++Registers::DecayFrame;
}

void PPU::ClearFrame()
{
	FrameBuffer::Clear();
}

// Faz sync dos frames
unsigned char PPU::SyncFrame()
{
	if(InternalSync > 2)
	{
		InternalSync -= 3;
		ReduceClock = 1;
		return 1;
	}
	ReduceClock = 0;
	return 0;
}

// Escreve na sprite ram!
void PPU::OAMWrite(unsigned char value)
{
	// Abrir sample
	if(CPU::dmcDelayCycle < CPU::cycles)
		APU::LoadDMCSample(9 - (CPU::cycles - CPU::dmcDelayCycle));

	//Actualizar tudo até aqui
	Update();

	bool irqpend = CPU::Interrupts::IRQHit();
	bool nmipend = CPU::Interrupts::NMIHit();


	// Copiar memória
	// Lento mas accurate
	unsigned char * memBegin = CPU::Memory::mapPages[value];
	for(unsigned int i = 0; i < 0x100; i += 8)
	{
		//unsigned char addr = SPRRAMaddr + i;

		CPU::Memory::Write(0x2004, CPU::Memory::Read(value, i, false));
		CPU::Memory::Write(0x2004, CPU::Memory::Read(value, i + 1, false));
		CPU::Memory::Write(0x2004, CPU::Memory::Read(value, i + 2, false));
		CPU::Memory::Write(0x2004, CPU::Memory::Read(value, i + 3, false));
		CPU::Memory::Write(0x2004, CPU::Memory::Read(value, i + 4, false));
		CPU::Memory::Write(0x2004, CPU::Memory::Read(value, i + 5, false));
		CPU::Memory::Write(0x2004, CPU::Memory::Read(value, i + 6, false));
		CPU::Memory::Write(0x2004, CPU::Memory::Read(value, i + 7, false));


		//SPRRAM[addr & 0xFF]       = memBegin[i];
		//SPRRAM[(addr + 1) & 0xFF] = memBegin[i + 1];
		//SPRRAM[(addr + 2) & 0xFF] = memBegin[i + 2]; 
		//SPRRAM[(addr + 3) & 0xFF] = memBegin[i + 3];
		//SPRRAM[(addr + 4) & 0xFF] = memBegin[i + 4];
		//SPRRAM[(addr + 5) & 0xFF] = memBegin[i + 5];
		//SPRRAM[(addr + 6) & 0xFF] = memBegin[i + 6];
		//SPRRAM[(addr + 7) & 0xFF] = memBegin[i + 7];
	}


	//CPU::cycles += 1539 + ((CPU::evenOddTotalCycles ^ (((int) ((CPU::cycles - CPU::startCycle) / 3)) & 1)) * 3);
	CPU::cycles += 3 + ((CPU::evenOddTotalCycles ^ (((int) ((CPU::cycles - CPU::startCycle) / 3)) & 1)) * 3);


	// Abrir sample
	if(CPU::dmcDelayCycle <= CPU::cycles)
	{
		int add = 6;
		if(CPU::dmcDelayCycle == (CPU::cycles - 9))
			add = 3;

		if(CPU::dmcDelayCycle == (CPU::cycles - 3))
			add = 9;

		APU::LoadDMCSample(add);
	}

	// Abrir sample
	if(CPU::dmcDelayCycle <= CPU::cycles)
	{
		int add = 6;
		if(CPU::dmcDelayCycle == (CPU::cycles - 9))
			add = 3;

		if(CPU::dmcDelayCycle == (CPU::cycles - 3))
			add = 9;
		APU::LoadDMCSample(add);
	}

	// Atrasar interrupts
	if(!irqpend && CPU::Interrupts::IRQHit())
		CPU::Interrupts::IRQActive = 1;

	if(!nmipend && CPU::Interrupts::NMIHit())
		CPU::Interrupts::DelayNMI = true;

	// Desactivar OAM Write
	CPU::doOAMWrite = false;
}

// Actualiza o PPU
void PPU::Update()
{
	if(CPU::cycles < 6818 || FrameDone)
		return;

	if(Registers::Status::HasHitMaxSprites == 2)
		Registers::Status::HasHitMaxSprites = 0;

	if(Registers::Status::HasHitSprite0 == 2)
		Registers::Status::HasHitSprite0 = 0;


	// Vblank, fugir
	if(CPU::cycles < 6820)
		return;


	// Preparar dados!
	unsigned int totalPixels = CPU::cycles + 3;
	totalPixels += (totalPixels >= 7160) ? remVBLCycle : 0;

	unsigned int numScanLine = totalPixels / 341;
	unsigned int numPixel = totalPixels % 341;
	unsigned int actualPixel = numPixel;
	numScanLine = (numScanLine < 20) ? 0 : (numScanLine - 20);
	numPixel = (numPixel > 255) ? 256: numPixel;

	// VBlank, preparar para fugir
	if(numScanLine > 240)
	{
		numScanLine = 240;
		numPixel = 256;
		actualPixel = 341;
		FrameDone = true;
	}

	// Rendering desligado
	if(!doRender)
	{
		if(!numScanLine)
			return;

		if(!LastScanLine)
		{
			LastScanLine = 1;
			LastPixel    = 0;
		}

		int myPixel = (LastPixel > 255) ? 256 : LastPixel;

		LastScanLine += myPixel >> 8;
		myPixel &= 0xFF;

		unsigned int pixelsToDraw = ((numScanLine << 8) + numPixel) - ((LastScanLine << 8) + myPixel);

		unsigned short addr = Registers::AddressLatch::GetActualAddress() & 0x3FFF;
		unsigned char myColor = (addr > 0x3EFF) ? (addr & 0x1F) : 0;

	  std::fill_n((FrameBuffer::FinalBmp+FrameBuffer::FinalBmpCount), pixelsToDraw, CurrPallete[Pallete[myColor]]);
	  FrameBuffer::FinalBmpCount += pixelsToDraw;

		LastScanLine = numScanLine;
		LastPixel = actualPixel;

		return;
	}



	// -------------------------------------------------------------------------------
	// --- OLD PPU UPDATE CODE -------------------------------------------------------
	// -------------------------------------------------------------------------------

	// Nametable Render Data
	static unsigned char Tile1 = 0;
	static unsigned char Tile2 = 0;
	static unsigned short LatchTile1 = 0;
	static unsigned short LatchTile2 = 0;
	static unsigned short AttrLatch  = 0;
	static unsigned char  AttrOtherLatch = 0;

	// Sprite Render Data
	static unsigned char SPRRAM2Pos = 0;
	static unsigned char mySprite   = 0;
	static int  copySprite = 0;
	static bool enableWrite = true;
	static int  numSpritesFound = 0;
	static int  totalSprites = 0;
	static int  spriteErratic = 0;
	static unsigned char * spritePtr = nullptr;

	static unsigned char spriteData1[8]    = { 0, 0, 0, 0, 0, 0, 0, 0 };
	static unsigned char spriteData2[8]    = { 0, 0, 0, 0, 0, 0, 0, 0 };
	static unsigned char spriteAttrs[8]    = { 0, 0, 0, 0, 0, 0, 0, 0 };
	static unsigned char spriteTiles[8]    = { 0, 0, 0, 0, 0, 0, 0, 0 };
	static unsigned char spriteCounters[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	static unsigned char ActiveCounters[8] = { 8, 8, 8, 8, 8, 8, 8, 8 };
	static bool sp0ThisScanLine = false;
	static bool sp0NextScanLine = false;
	static bool incPtr = false;

	// Rendering ligado
	while(LastScanLine <= numScanLine)
	{
		unsigned int pixelLimit = (LastScanLine < numScanLine) ? 256 : numPixel;

		// Escrita na TV
		while(LastPixel < pixelLimit)
		{
			if(LastPixel == 251)
			{
				Registers::AddressLatch::IncrementVReadVert();
			}

			switch(LastPixel & 7)
			{
				case 0:
				{
					GetTileIndex();

					LatchTile1 |= Tile1;
					LatchTile2 |= Tile2;
					AttrOtherLatch = TileAttribute;
					break;
				}

				case 2:
				{
					GetTileAttribute();
					break;
				}

				case 3:
				{
					Registers::AddressLatch::IncrementVReadHoriz();
					break;
				}

				case 4:
				{
					GetTilePtr();
					Tile1 = *TilePtr;
					break;
				}

				case 6:
				{
					Tile2 = *(TilePtr + 8);
					break;
				}
			}

			if(LastScanLine)
			{
				if(LastScanLine == 1)
					totalSprites = 0;

				// Reset aos counters de sprites!
				if(!LastPixel)
				{
					SPRRAM2Pos = 0;
					mySprite   = 0;
					copySprite = 0;
					enableWrite = true;
					numSpritesFound = 0;
					spriteErratic   = 0;
					sp0ThisScanLine = sp0NextScanLine;
					sp0NextScanLine = false;
					incPtr = false;
					std::fill_n(ActiveCounters, 8, 8);
				}

				// Sprites!
				if(LastPixel < 64)
				{
					SPRRAM2[LastPixel] = 0xFF;
					SPRRAMptr = &SPRRAM2[LastPixel];
				}

				// Sprite write!
				else if(LastPixel & 1)
				{
					if(enableWrite)
						SPRRAM2[SPRRAM2Pos] = *SPRRAMptr;

					else
						SPRRAMptr = SPRRAM2;
				}
				
				// Sprite read!
				else
				{
					// If we aren't copying the bytes of a matched sprite, we need to check if the current sprite is on the next scanline
					if(!copySprite)
					{
						// If not all 64 sprites have been checked, we come here
						if(!(mySprite & 0x40) && (numSpritesFound < 9))
						{
							// We have eight sprites and are checking for overflow, so the PPU behaviour is erratic
							// and does not check against the Y value of the sprite
							if(numSpritesFound & 8)
							{
								// Disable writes to secondary OAM
								enableWrite = false;

								// Fetch the sprite and byte for which to compare against the range.
								// Each sprite has four bytes, and when checking for sprite overflow,
								// the sprite comparison logic not only increments the sprite, but also
								// the internal byte. So it checks for sprite 0 byte 0, sprite 1 byte 1,
								// sprite 2 byte 2, sprite 3 byte 3, sprite 4 byte 0, and so on.
								SPRRAMptr = &SPRRAM[mySprite << 2];
								SPRRAMptr += spriteErratic;
								++spriteErratic;

								// Sprite overflow!!!
								if((*SPRRAMptr <= (LastScanLine - 1)) && (((*SPRRAMptr) + Registers::spriteHeight) > (LastScanLine - 1)))
								{
									// Set the overflow flag
									Registers::Status::HasHitMaxSprites = 1;
									++numSpritesFound;

									// Prepare (and fail) to copy the sprite to Secondary OAM
									copySprite = 3;
								}

								// No sprite overflow, go to next sprite
								else
								{
									++mySprite;
									spriteErratic &= 3;
								}
							}

							// We have less than eight sprites found, so the PPU behaves normally
							else
							{
								// This is needed to avoid problems with pointer increments
								if(incPtr)
								{
									++SPRRAM2Pos;
									incPtr = false;
								}

								// Set the pointer to the next sprite
								SPRRAMptr = &SPRRAM[mySprite << 2];

								// We have a match, so set to copy for the following 8 cycles
								if((*SPRRAMptr <= (LastScanLine - 1)) && (((*SPRRAMptr) + Registers::spriteHeight) > (LastScanLine - 1)))
								{
									++numSpritesFound;
									copySprite = 3;

									incPtr = true;

									// Sprite 0
									if(!mySprite)
										sp0NextScanLine = true;
								}

								++mySprite;
							}
						}

						// Either we went through all 64 sprites, or we have 9 sprite hits
						// Either way, we know simply cycle the sprites and do nothing else
						else
						{
							enableWrite = false;
							SPRRAMptr = &SPRRAM[((mySprite & 0x3F) << 2)];
							++mySprite;
						}
					}

					// We have a matched sprite, so we write its values to the secondary SPRRAM
					else
					{
						// If this is the last sprite, we need to read the next three bytes and
						// then properly pad to the next sprite's Y value (evaluation becomes correct)
						if(numSpritesFound > 8)
						{
							SPRRAMptr = &SPRRAM[((mySprite << 2) + spriteErratic) & 0xFF];
							++spriteErratic;


							// We do that by increasing mySprite by one when spriteErratic has overflowed,
							// but we only do it when we're done copying, otherwise we would copy from the
							// wrong sprite. We know we're done copying when copySprite is 1 (it will be
							// decremented to 0 shortly).
							if((copySprite == 1) && (spriteErratic > 4))
								++mySprite;
						}

						// If this isn't the last sprite, simply increment the pointers
						else
						{
							++SPRRAMptr;
							++SPRRAM2Pos;
						}

						--copySprite;
					}
				}
				
				if(!Registers::showBackground || (!Registers::clipBackground && LastPixel < 8))
				{
					// Adicionar pixel ao bitmap
					FrameBuffer::FinalBmp[FrameBuffer::FinalBmpCount] = CurrPallete[Pallete[0]];
				}

				else
				{
					// Obter o índice da cor
					unsigned char colorIndex = ((((LatchTile1 << Registers::AddressLatch::FineXScroll) >> 15) & 1) | ((((LatchTile2 << Registers::AddressLatch::FineXScroll) >> 15) & 1) << 1)) & 3;

					// Adicionar pixel ao bitmap
					PixelOpacity[FrameBuffer::FinalBmpCount] = (colorIndex) ? 1 : 0;
					FrameBuffer::FinalBmp[FrameBuffer::FinalBmpCount] = CurrPallete[Pallete[(colorIndex) ? ((((AttrLatch << (Registers::AddressLatch::FineXScroll << 1)) >> 12) & 0xC) | colorIndex) : 0] & Registers::grayscale];
				}

				// Desenhar sprites
				if(Registers::showSprites)
				{			
					for(int spList = 0; spList < totalSprites; ++spList)
					{
						// Reduzir ao counter
						if(spriteCounters[spList])
							--spriteCounters[spList];

						// Desenhar o sprite!
						else if(ActiveCounters[spList])
						{
							--ActiveCounters[spList];
							unsigned char colorIndex = (PixelOpacity[FrameBuffer::FinalBmpCount] & 2) ? 0 : ((spriteData1[spList] >> ActiveCounters[spList]) & 1) | (((spriteData2[spList] >> ActiveCounters[spList]) & 1) << 1);
							if(colorIndex && !(!Registers::clipSprites && (LastPixel < 8)))
							{
								// Sprite 0 hit!
								if(!spList && (PixelOpacity[FrameBuffer::FinalBmpCount] & 1) && sp0ThisScanLine && (LastPixel != 255))
									Registers::Status::HasHitSprite0 = 1;

								if(!((spriteAttrs[spList] & 0x20) && (PixelOpacity[FrameBuffer::FinalBmpCount]) & 1))
								{
									FrameBuffer::FinalBmp[FrameBuffer::FinalBmpCount] = CurrPallete[Pallete[(0x10 | ((spriteAttrs[spList] & 3) << 2)) | colorIndex]];
								}
								PixelOpacity[FrameBuffer::FinalBmpCount] |= 2;
							}
						}
					}
				}

				++FrameBuffer::FinalBmpCount;
			}

			LatchTile1 <<= 1;
			LatchTile2 <<= 1;

			AttrLatch <<= 2;
			AttrLatch |= AttrOtherLatch;

			++LastPixel;
		}

		pixelLimit = (LastScanLine < numScanLine) ? 341 : actualPixel;

		// HBLANK
		while(LastPixel < pixelLimit)
		{
			if(LastPixel < 320)
			{
				if(LastPixel == 256)
				{
					totalSprites = numSpritesFound;
					mySprite   = 0;
				}

				if(LastPixel == 257)
				{
					Registers::AddressLatch::ResetXScroll();
				}

				if(!LastScanLine && (LastPixel == 304))
				{
					Registers::AddressLatch::TileXScroll = Registers::AddressLatch::TempTileXScroll;
					Registers::AddressLatch::TileYScroll = Registers::AddressLatch::TempTileYScroll;

					Registers::AddressLatch::FineYScroll      = Registers::AddressLatch::TempFineYScroll;
					Registers::AddressLatch::CurrentNameTable = Registers::AddressLatch::TempCurrentNameTable;
				}

				// Sprite fun
				switch(LastPixel & 7)
				{
					case 0:
					{
						SPRRAMptr = &SPRRAM2[(mySprite << 2)];
						break;
					}
					case 1:
					{
						SPRRAMptr = &SPRRAM2[(mySprite << 2) + 1];
						spriteTiles[mySprite] =  *SPRRAMptr;
						break;
					}

					case 2:
					{
						SPRRAMptr = &SPRRAM2[(mySprite << 2) + 2];
						spriteAttrs[mySprite] = *SPRRAMptr;
						break;
					}

					case 3:
					{
						SPRRAMptr = &SPRRAM2[(mySprite << 2) + 3];
						spriteCounters[mySprite] = *SPRRAMptr;
						break;
					}

					case 4:
					{
						if(mySprite >= totalSprites)
							break;

						int yPos = (LastScanLine - 1 - SPRRAM2[(mySprite << 2)]);

						if(yPos >= 8)
							yPos += 8;

						if(spriteAttrs[mySprite] & 0x80)
							yPos = ((Registers::spriteHeight == 8) ? 7 : 23) - yPos;


						unsigned char spriteIndex = (Registers::spriteHeight == 8) ? spriteTiles[mySprite] : spriteTiles[mySprite] & 0xFE;

						spriteData1[mySprite] = mapper[((Registers::spriteHeight == 8) ? Registers::SpriteTable : ((spriteTiles[mySprite] & 1) << 2)) | (((spriteIndex << 4) & 0xC00) >> 10)][((spriteIndex << 4) + yPos) & 0x3FF];

						// Inverter o sprite
						if(spriteAttrs[mySprite] & 0x40)
							spriteData1[mySprite] = (unsigned char) ((spriteData1[mySprite] * 0x0202020202ULL & 0x010884422010ULL) % 1023);

						break;
					}

					case 6:
					{
						if(mySprite >= totalSprites)
							break;

						int yPos = (LastScanLine - 1 - SPRRAM2[(mySprite << 2)]);

						if(yPos >= 8)
							yPos += 8;

						if(spriteAttrs[mySprite] & 0x80)
							yPos = ((Registers::spriteHeight == 8) ? 7 : 23) - yPos;

						unsigned char spriteIndex = (Registers::spriteHeight == 8) ? spriteTiles[mySprite] : spriteTiles[mySprite] & 0xFE;

						spriteData2[mySprite] = mapper[((Registers::spriteHeight == 8) ? Registers::SpriteTable : ((spriteTiles[mySprite] & 1) << 2)) | (((spriteIndex << 4) & 0xC00) >> 10)][((spriteIndex << 4) + 8 + yPos) & 0x3FF];

						// Inverter o sprite
						if(spriteAttrs[mySprite] & 0x40)
							spriteData2[mySprite] = (unsigned char) ((spriteData2[mySprite] * 0x0202020202ULL & 0x010884422010ULL) % 1023);

						break;
					}

					case 7:
					{
						++mySprite;
						break;
					}
				}

			}

			else if((LastPixel > 319) && (LastPixel < 336))
			{
				switch(LastPixel & 7)
				{
					case 0:
					{
						// Sprite read
						SPRRAMptr = SPRRAM2;

						GetTileIndex();

						LatchTile1 |= Tile1;
						LatchTile2 |= Tile2;
						AttrOtherLatch = TileAttribute;
						break;
					}

					case 2:
					{
						GetTileAttribute();
						break;
					}

					case 3:
					{
						Registers::AddressLatch::IncrementVReadHoriz();
						break;
					}

					case 4:
					{
						GetTilePtr();
						Tile1 = *TilePtr;
						break;
					}

					case 6:
					{
						Tile2 = *(TilePtr + 8);
						break;
					}
				}
				LatchTile1 <<= 1;
				LatchTile2 <<= 1;

				AttrLatch <<= 2;
				AttrLatch |= AttrOtherLatch;
			}

			// Dummy nametable fetches
			else if(!(LastPixel & 1))
			{
				GetTileIndex();
			}
			
			// TODO mapper 2 hit should be around here

			++LastPixel;
		}

		if(LastPixel == 341)
		{
			LastPixel = 0;
			++LastScanLine;
		}

		else
			break;
	}
}


// Indica se estamos num VBlank
bool PPU::InVBlank()
{
	return (CPU::cycles < 6820);
}

// ----------------------------------------------------------------------------
// CRIAÇÃO E DESTRUIÇÃO -------------------------------------------------------
// ----------------------------------------------------------------------------

// Inicia o PPU
void PPU::Create()
{
	if(working)
		return;

	VRAM   = nullptr;
	FrameDone = false;

	registerPallete = 0;
	SPRRAMaddr      = 0;
	SPRRAMptr       = SPRRAM;
	flipFlop        = 0;
	VRAMTempBuffer  = 0;
	remVBLCycle     = 0;

	ready           = CPU::cyclesForFrame + 6795;
	doRender        = false;

	IsEvenFrame  = 0;
	InternalSync = 0;
	ReduceClock  = 0;

	memset(nameTables, 0, 0x1000 * sizeof(unsigned char));
	memset(SPRRAM,     0, 0x100  * sizeof(unsigned char));
	memset(SPRRAM2,    0, 0x40   * sizeof(unsigned char));
	std::fill_n(ReadOnly, 16, false);

	Pallete[0x00] = 0x09;
	Pallete[0x01] = 0x01;
	Pallete[0x02] = 0x00;
	Pallete[0x03] = 0x01;
	Pallete[0x04] = 0x00;
	Pallete[0x05] = 0x02;
	Pallete[0x06] = 0x02;
	Pallete[0x07] = 0x0D;
	Pallete[0x08] = 0x08;
	Pallete[0x09] = 0x10;
	Pallete[0x0A] = 0x08;
	Pallete[0x0B] = 0x24;
	Pallete[0x0C] = 0x00;
	Pallete[0x0D] = 0x00;
	Pallete[0x0E] = 0x04;
	Pallete[0x0F] = 0x2C;
	Pallete[0x10] = 0x09;
	Pallete[0x11] = 0x01;
	Pallete[0x12] = 0x34;
	Pallete[0x13] = 0x03;
	Pallete[0x14] = 0x00;
	Pallete[0x15] = 0x04;
	Pallete[0x16] = 0x00;
	Pallete[0x17] = 0x14;
	Pallete[0x18] = 0x08;
	Pallete[0x19] = 0x3A;
	Pallete[0x1A] = 0x00;
	Pallete[0x1B] = 0x02;
	Pallete[0x1C] = 0x00;
	Pallete[0x1D] = 0x20;
	Pallete[0x1E] = 0x2C;
	Pallete[0x1F] = 0x08;

	if(CPU::CHRpages)
	{
		SetReadOnly(0, 7, true);
		VRAM = CPU::Memory::CHR;
	}

	else
	{
		VRAM = new unsigned char[0x2000];
		memset(VRAM, 0, 0x2000);
	}

	// Popular os mappers
	mapper[0] = VRAM;
	mapper[1] = &VRAM[0x400];
	mapper[2] = &VRAM[0x800];
	mapper[3] = &VRAM[0xC00];
	mapper[4] = &VRAM[0x1000];
	mapper[5] = &VRAM[0x1400];
	mapper[6] = &VRAM[0x1800];
	mapper[7] = &VRAM[0x1C00];
	
	UpdateMirrors();

	// Preparar framebuffer
	LastScanLine               = 0;
	LastPixel                  = 0;
	TilePtr                    = nullptr;
	TileIndex                  = 0;
	CurrPallete                = PalleteColors[0][0];
	CalculatedFrameSync        = false;
	FrameBuffer::FinalBmpCount = 0;
	FrameBuffer::DoResize      = false;
	FrameBuffer::OwnRenderer   = true;
	FrameBuffer::NumTicks      = 0;
	FrameBuffer::LastTime      = Common::GetUTime();

	if(!FrameBuffer::FrameTime)
	{
		FrameBuffer::FrameTimeAvg = 0;
		std::fill_n(FrameBuffer::FrameTimes, 8, 0);
	}

	std::fill_n(FrameBuffer::FinalBmp, 61440, 0xFFFFFFFF);

	// Preparar latches
	Registers::AddressLatch::TempTileXScroll =
	Registers::AddressLatch::TempTileYScroll =

	Registers::AddressLatch::TempFineXScroll      =
	Registers::AddressLatch::TempFineYScroll      =
	Registers::AddressLatch::TempCurrentNameTable =

	Registers::AddressLatch::FineXScroll = 0;

	// Preparar registers
	Registers::increment       = 1;
	Registers::SpriteTable     = 4;
	Registers::BackgroundTable = 0;
	Registers::spriteHeight    = 8;
	Registers::palleteNum      = 0;
	Registers::grayscale       = 0x3F;
	Registers::clipBackground  = false;
	Registers::clipSprites     = false;
	Registers::showBackground  = false;
	Registers::showSprites     = false;

	Registers::Decay           = 0;
	Registers::DecayFrame      = 0;
	Registers::LastReadPPUData = 0;

	// Preparar status
	Registers::Status::HasHitMaxSprites = 0;
	Registers::Status::HasHitSprite0    = 0;
	Registers::Status::IsVBlank         = 1;
	Registers::Status::ReadVBlankThisFrame = 0;

	// This part should go to FrameBuffer.cpp
	RenderWindow->setActive();
	//RenderWindow->SetFramerateLimit(60);
	//RenderWindow->UseVerticalSync(false);

	// Preparar o opengl...
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 256, 240, 0, 0, 1);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.375, 0.375, 0);

	// Preparar textura!
	FrameBuffer::TextureID = 0;
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &FrameBuffer::TextureID);
	glBindTexture(GL_TEXTURE_2D, FrameBuffer::TextureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glClearColor(0, 0, 0, 255);
	glClear(GL_COLOR_BUFFER_BIT);

	// Informa o mapper que pode alterar a memória
	CPU::Memory::mapper->NotifyPPUCreated();

	// Give the tile ptr something to chew (so the PPU doesn't crash)
	TilePtr = &mapper[Registers::BackgroundTable | ((TileIndex & 0xC00) >> 10)][TileIndex & 0x3FF] + Registers::AddressLatch::FineYScroll;

	working = true;
}

void PPU::RegisterWindow(sf::Window * window)
{
	RenderWindow = window;
}

// Reset the PPU
void PPU::Reset()
{
	Update();

	Registers::WritePPUController(0);
	Registers::WritePPUMask(0);

	IsEvenFrame  = 0;
	flipFlop     = 0;
	LastScanLine = 0;
	LastPixel    = 0;

	// Prepare latches
	Registers::AddressLatch::TempTileXScroll =
	Registers::AddressLatch::TempTileYScroll =

	Registers::AddressLatch::TempFineXScroll      =
	Registers::AddressLatch::TempFineYScroll      =
	Registers::AddressLatch::TempCurrentNameTable =

	Registers::AddressLatch::FineXScroll = 0;

	FrameBuffer::FinalBmpCount = 0;

	ready = 88947;
}

// Destruidor de classe
void PPU::Destroy()
{
	if(working)
	{
		FrameBuffer::Clear();
		working = false;
		delete [] VRAM;

		// Eliminar dados e contexto do opengl
		DisableRenderer();
		glDeleteTextures(1, &FrameBuffer::TextureID);

		VRAM = nullptr;
	}
}

// Changes window size
// TODO this should really go to FrameBuffer.cpp
void PPU::ResizeRenderer(long width, long height, bool moving)
{
	// Only moving window, don't change the viewport
	if(moving)
	{
		if(!FrameBuffer::OwnRenderer && working)
			FrameBuffer::UpdateScreen();
		return;
	}

	// Resizing window!

	// PPU not working, resize only
	if(!working)
	{
		if(!RenderWindow)
			return;

		bool widthChanged = (width != RenderWindow->getSize().x);

		RenderWindow->setSize(sf::Vector2u(width, height));

		glViewport(0, 0, RenderWindow->getSize().x, RenderWindow->getSize().y);

		// If resizing width, redraw strings
		if(widthChanged)
			FrameBuffer::RecalculateStringPositions();
	}

	// PPU working and active, let it update the image next frame
	else if(FrameBuffer::OwnRenderer)
	{
		FrameBuffer::DoResize = true;
		FrameBuffer::ResizeSize = sf::Vector2u(width, height);
	}

	// PPU working and inactive, update the image now
	else
	{
		bool widthChanged = (width != RenderWindow->getSize().x);

        RenderWindow->setSize(sf::Vector2u(width, height));

        glViewport(0, 0, RenderWindow->getSize().x, RenderWindow->getSize().y);

		// If resizing width, redraw strings
		if(widthChanged)
			FrameBuffer::RecalculateStringPositions();

		FrameBuffer::UpdateScreen();
	}
}

// Desactiva o renderer
// Apenas deve ser chamado da thread de emulação
void PPU::DisableRenderer()
{
	RenderWindow->setActive(false);
	FrameBuffer::OwnRenderer = false;
}

// Define o tipo de mirror
void PPU::SetMirrorMode(MirrorMode mode)
{
	Mirror = mode;
	UpdateMirrors();
}

// Altera os mirrors
void PPU::UpdateMirrors()
{
	switch(Mirror)
	{
		case HORIZONTAL:
		{
			mapper[12] = mapper[8]  = nameTables[0];
			mapper[13] = mapper[9]  = nameTables[0];
			mapper[14] = mapper[10] = nameTables[1];
			mapper[15] = mapper[11] = nameTables[1];

			break;
		}

		case VERTICAL:
		{
			mapper[12] = mapper[8]  = nameTables[0];
			mapper[13] = mapper[9]  = nameTables[1];
			mapper[14] = mapper[10] = nameTables[0];
			mapper[15] = mapper[11] = nameTables[1];

			break;
		}

		case FOURSCREEN:
		{
			mapper[12] = mapper[8]  = nameTables[0];
			mapper[13] = mapper[9]  = nameTables[1];
			mapper[14] = mapper[10] = nameTables[2];
			mapper[15] = mapper[11] = nameTables[3];

			break;
		}

		case ONESCREEN:
		{
			mapper[12] = mapper[8]  = nameTables[0];
			mapper[13] = mapper[9]  = nameTables[0];
			mapper[14] = mapper[10] = nameTables[0];
			mapper[15] = mapper[11] = nameTables[0];

			break;
		}
	}
}

// Indica o número de frames desde a última vez que se chamou este método
unsigned long PPU::GetTicks()
{
	unsigned long ticks = FrameBuffer::NumTicks;
	FrameBuffer::NumTicks = 0;
	return ticks;
}

// Limita os FPS
void PPU::LimitFPS(unsigned long limit)
{
	if(!limit)
	{
		FrameBuffer::LimitFPS = false;
		return;
	}
	FrameBuffer::LimitFPS = true;
	FrameBuffer::FrameTime = 1000.0f / limit;

	// Colocar médias se até agora não tiver havido limite
	if(!FrameBuffer::FrameTimeAvg)
	{
		FrameBuffer::FrameTimeAvg = FrameBuffer::FrameTime;
		std::fill_n(FrameBuffer::FrameTimes, 8, FrameBuffer::FrameTime);
	}
}


		// Ver o attribute table e obter índice de cor
		// Explicação
		// Em primeiro lugar temos que ir buscar o endereço do atributo.
		// Baseado no nosso mapper, a primeira array corresponde ao NameTable actual.
		// A array interna procura o byte concreto do atributo do tile. Para isso faz a conta a partir de 0x3C0.

		// O byte é determinado através da fórmula:

		// ((Registers::AddressLatch::TileYScroll << 1) & 0x38) | (Registers::AddressLatch::TileXScroll >> 2)

		// A primeira parte aumenta de 8 em 8 bytes para cada 4 números de TileYScroll.
		// A segunda parte aumenta de byte em byte para cada 4 números de TileXScroll.
		// Uma vez obtido o byte, é necessário fazer o respectivo bitshift para obter os 2 bits correspondentes ao tile.

		// O bit é obtido pela fórmula:

		// ((Registers::AddressLatch::TileYScroll & 2) << 1) | (Registers::AddressLatch::TileXScroll & 2)

		// Os bits são aumentados em 4 para cada 2 tileyscroll.
		// Os bits são aumentados em 2 para cada 2 tilexscroll.

		// Por último, limpam-se os bytes que estiverem a mais fazendo AND com 0x3.
		// Finalmente, shifta-se à esquerda por 2 para depois se colocar o color index específico do pixel.
		inline void PPU::GetTileAttribute()
		{
			TileAttribute =
				(
					(
						mapper[Registers::AddressLatch::CurrentNameTable + 8]
									[0x3C0
										| (((Registers::AddressLatch::TileYScroll << 1) & 0x38)
										| (Registers::AddressLatch::TileXScroll >> 2))
									]
						>> 
							(
									(((Registers::AddressLatch::TileYScroll & 2) << 1)
									| (Registers::AddressLatch::TileXScroll & 2))
							)
						)
					& 0x3
				)
				//<< 2
			;
		}

		// Obtém o index do tile
		inline void PPU::GetTileIndex()
		{
			TileIndex = mapper[Registers::AddressLatch::CurrentNameTable + 8][Registers::AddressLatch::TileXScroll | (Registers::AddressLatch::TileYScroll << 5)] << 4;
		}


    		// Obtém o pointer para o tile actual
		inline void PPU::GetTilePtr()
		{
			TilePtr = &mapper[Registers::BackgroundTable | ((TileIndex & 0xC00) >> 10)][TileIndex & 0x3FF] + Registers::AddressLatch::FineYScroll;
		}

bool PPU::working = false;

unsigned int PPU::PalleteColors[2][8][64] =
{
	{
		{
			0xFF525252, 0xFF000080, 0xFF08008A, 0xFF2C007E, 0xFF4A004E, 0xFF500006, 0xFF440000, 0xFF260800,
			0xFF0A2000, 0xFF002E00, 0xFF003200, 0xFF00260A, 0xFF001C48, 0xFF000000, 0xFF000000, 0xFF000000,
			0xFFA4A4A4, 0xFF0038CE, 0xFF3416EC, 0xFF5E04DC, 0xFF8C00B0, 0xFF9A004C, 0xFF901800, 0xFF703600,
			0xFF4C5400, 0xFF0E6C00, 0xFF007400, 0xFF006C2C, 0xFF005E84, 0xFF000000, 0xFF000000, 0xFF000000,
			0xFFFFFFFF, 0xFF4C9CFF, 0xFF7C78FF, 0xFFA664FF, 0xFFDA5AFF, 0xFFF054C0, 0xFFF06A56, 0xFFD68610,
			0xFFBAA400, 0xFF76C000, 0xFF46CC1A, 0xFF2EC866, 0xFF34C2BE, 0xFF3A3A3A, 0xFF000000, 0xFF000000,
			0xFFFFFFFF, 0xFFB6DAFF, 0xFFC8CAFF, 0xFFDAC2FF, 0xFFF0BEFF, 0xFFFCBCEE, 0xFFFAC2C0, 0xFFF2CCA2,
			0xFFE6DA92, 0xFFCCE68E, 0xFFB8EEA2, 0xFFAEEABE, 0xFFAEE8E2, 0xFFB0B0B0, 0xFF000000, 0xFF000000
			//0xFF7C7C7C,	0xFF0000FC,	0xFF0000BC,	0xFF4428BC,	0xFF940084,	0xFFA80020,	0xFFA81000,	0xFF881400,
			//0xFF503000,	0xFF007800,	0xFF006800,	0xFF005800,	0xFF004058,	0xFF000000,	0xFF000000,	0xFF000000,
			//0xFFBCBCBC,	0xFF0078F8,	0xFF0058F8,	0xFF6844FC,	0xFFD800CC,	0xFFE40058,	0xFFF83800,	0xFFE45C10,
			//0xFFAC7C00,	0xFF00B800,	0xFF00A800,	0xFF00A844,	0xFF008888,	0xFF000000,	0xFF000000,	0xFF000000,
			//0xFFF8F8F8,	0xFF3CBCFC,	0xFF6888FC,	0xFF9878F8,	0xFFF878F8,	0xFFF85898,	0xFFF87858,	0xFFFCA044,
			//0xFFF8B800,	0xFFB8F818,	0xFF58D854,	0xFF58F898,	0xFF00E8D8,	0xFF787878,	0xFF000000,	0xFF000000,
			//0xFFFCFCFC,	0xFFA4E4FC,	0xFFB8B8F8,	0xFFD8B8F8,	0xFFF8B8F8,	0xFFF8A4C0,	0xFFF0D0B0,	0xFFFCE0A8,
			//0xFFF8D878,	0xFFD8F878,	0xFFB8F8B8,	0xFFB8F8D8,	0xFF00FCFC,	0xFFF8D8F8,	0xFF000000,	0xFF000000


			//0xFF757575,	0xFF8F1B27,	0xFFAB0000,	0xFF9F0047,	0xFF77008F,	0xFF1300AB,	0xFF0000A7,	0xFF000B7F,
			//0xFF002F43,	0xFF004700,	0xFF005100,	0xFF173F00,	0xFF5F3F1B,	0xFF000000,	0xFF000000,	0xFF000000,
			//0xFFBCBCBC,	0xFFEF7300,	0xFFEF3B23,	0xFFF30083,	0xFFBF00BF,	0xFF5B00E7,	0xFF002BDB,	0xFF0F4FCB,
			//0xFF00738B,	0xFF009700,	0xFF00AB00,	0xFF3B9300,	0xFF8B8300,	0xFF000000,	0xFF000000,	0xFF000000,
			//0xFFFFFFFF,	0xFFFFBF3F,	0xFFFF975F,	0xFFFD8BA7,	0xFFFF7BF7,	0xFFB777FF,	0xFF6377FF,	0xFF3B9BFF,
			//0xFF3FBFF3,	0xFF13D383,	0xFF4BDF4F,	0xFF98F858,	0xFFDBEB00,	0xFF000000,	0xFF000000,	0xFF000000,
			//0xFFFFFFFF,	0xFFFFE7AB,	0xFFFFD7C7,	0xFFFFCBD7,	0xFFFFC7FF,	0xFFDBC7FF,	0xFFB3BFFF,	0xFFABDBFF,
			//0xFFA3E7FF,	0xFFA3FFE3, 0xFFBFF3AB,	0xFFCFFFB3,	0xFFF3FF9F,	0xFF000000,	0xFF000000, 0xFF000000
		},
		{
			0xFF755D5E, 0xFF271573, 0xFF00008A, 0xFF470080, 0xFF8F0060, 0xFFAB000F, 0xFFA70000, 0xFF7F0800,
			0xFF432500, 0xFF003800, 0xFF004000, 0xFF003212, 0xFF1B324C, 0xFF000000, 0xFF000000, 0xFF000000,
			0xFFBC9698, 0xFF005CC1, 0xFF232FC1, 0xFF8300C4, 0xFFBF009A, 0xFFE70049, 0xFFDB2200, 0xFFCB3F0C,
			0xFF8B5C00, 0xFF007800, 0xFF008800, 0xFF00752F, 0xFF006870, 0xFF000000, 0xFF000000, 0xFF000000,
			0xFFFFCCCE, 0xFF3F98CE, 0xFF5F78CE, 0xFFA76FCC, 0xFFF762CE, 0xFFFF5F94, 0xFFFF5F50, 0xFFFF7C2F,
			0xFFF39833, 0xFF83A80F, 0xFF4FB23C, 0xFF58C67B, 0xFF00BCB1, 0xFF000000, 0xFF000000, 0xFF000000,
			0xFFFFCCCE, 0xFFABB8CE, 0xFFC7ACCE, 0xFFD7A2CE, 0xFFFF9FCE, 0xFFFF9FB1, 0xFFFF9890, 0xFFFFAF8A,
			0xFFFFB884, 0xFFE3CC84, 0xFFABC29A, 0xFFB3CCA7, 0xFF9FCCC4, 0xFF000000, 0xFF000000, 0xFF000000
		},
		{
			0xFF5B6D4D, 0xFF1E195E, 0xFF000070, 0xFF370068, 0xFF6F004E, 0xFF85000C, 0xFF820000, 0xFF630A00,
			0xFF342C00, 0xFF004200, 0xFF004C00, 0xFF003B0F, 0xFF153B3E, 0xFF000000, 0xFF000000, 0xFF000000,
			0xFF92B07C, 0xFF006C9D, 0xFF1B379D, 0xFF6600A0, 0xFF94007E, 0xFFB4003C, 0xFFAA2800, 0xFF9E4A09,
			0xFF6C6C00, 0xFF008D00, 0xFF00A000, 0xFF008A26, 0xFF007B5B, 0xFF000000, 0xFF000000, 0xFF000000,
			0xFFC6EFA8, 0xFF31B3A8, 0xFF4A8DA8, 0xFF8282A6, 0xFFC073A8, 0xFFC66F78, 0xFFC66F41, 0xFFC69126,
			0xFFBDB329, 0xFF66C60C, 0xFF3DD131, 0xFF44E964, 0xFF00DC90, 0xFF000000, 0xFF000000, 0xFF000000,
			0xFFC6EFA8, 0xFF85D9A8, 0xFF9BCAA8, 0xFFA7BEA8, 0xFFC6BBA8, 0xFFC6BB90, 0xFFC6B376, 0xFFC6CD70,
			0xFFC6D96B, 0xFFB1EF6B, 0xFF85E47E, 0xFF8BEF88, 0xFF7CEFA0, 0xFF000000, 0xFF000000, 0xFF000000
		},
		{
			0xFF5C5A49, 0xFF1E145A, 0xFF00006B, 0xFF380064, 0xFF70004A, 0xFF87000B, 0xFF830000, 0xFF640800,
			0xFF342400, 0xFF003600, 0xFF003E00, 0xFF00300E, 0xFF15303B, 0xFF000000, 0xFF000000, 0xFF000000,
			0xFF949076, 0xFF005896, 0xFF1B2D96, 0xFF670099, 0xFF960078, 0xFFB60039, 0xFFAD2100, 0xFFA03C09,
			0xFF6D5800, 0xFF007400, 0xFF008300, 0xFF007125, 0xFF006457, 0xFF000000, 0xFF000000, 0xFF000000,
			0xFFC9C4A0, 0xFF3193A0, 0xFF4B74A0, 0xFF836B9F, 0xFFC35EA0, 0xFFC95B73, 0xFFC95B3E, 0xFFC97725,
			0xFFBF9327, 0xFF67A20B, 0xFF3EAB2F, 0xFF45BE5F, 0xFF00B489, 0xFF000000, 0xFF000000, 0xFF000000,
			0xFFC9C4A0, 0xFF87B1A0, 0xFF9DA5A0, 0xFFA99CA0, 0xFFC999A0, 0xFFC99989, 0xFFC99370, 0xFFC9A86B,
			0xFFC9B166, 0xFFB3C466, 0xFF87BB78, 0xFF8DC482, 0xFF7DC499, 0xFF000000, 0xFF000000, 0xFF000000
		},
		{
			0xFF5F6183, 0xFF1F16A0, 0xFF0000BF, 0xFF3A00B2, 0xFF750085, 0xFF8C0015, 0xFF880000, 0xFF680900,
			0xFF362700, 0xFF003A00, 0xFF004300, 0xFF003419, 0xFF16346A, 0xFF000000, 0xFF000000, 0xFF000000,
			0xFF9A9CD2, 0xFF005FFF, 0xFF1C30FF, 0xFF6B00FF, 0xFF9C00D5, 0xFFBD0065, 0xFFB32300, 0xFFA64110,
			0xFF715F00, 0xFF007D00, 0xFF008D00, 0xFF007A42, 0xFF006C9B, 0xFF000000, 0xFF000000, 0xFF000000,
			0xFFD1D3FF, 0xFF339EFF, 0xFF4D7DFF, 0xFF8873FF, 0xFFCA66FF, 0xFFD162CC, 0xFFD1626E, 0xFFD18042,
			0xFFC79E46, 0xFF6BAF15, 0xFF40B954, 0xFF48CDAA, 0xFF00C3F5, 0xFF000000, 0xFF000000, 0xFF000000,
			0xFFD1D3FF, 0xFF8CBFFF, 0xFFA3B2FF, 0xFFB0A8FF, 0xFFD1A5FF, 0xFFD1A5F5, 0xFFD19EC8, 0xFFD1B5BF,
			0xFFD1BFB6, 0xFFBAD3B6, 0xFF8CC9D5, 0xFF92D3E7, 0xFF82D3FF, 0xFF000000, 0xFF000000, 0xFF000000
		},
		{
			0xFF5E5365, 0xFF1F137C, 0xFF000094, 0xFF39008A, 0xFF730067, 0xFF8A0010, 0xFF870000, 0xFF660700,
			0xFF362100, 0xFF003200, 0xFF003900, 0xFF002C14, 0xFF152C52, 0xFF000000, 0xFF000000, 0xFF000000,
			0xFF9885A3, 0xFF0051CF, 0xFF1C29CF, 0xFF6A00D3, 0xFF9A00A6, 0xFFBB004F, 0xFFB11E00, 0xFFA4380D,
			0xFF705100, 0xFF006B00, 0xFF007900, 0xFF006833, 0xFF005D78, 0xFF000000, 0xFF000000, 0xFF000000,
			0xFFCEB5DD, 0xFF3387DD, 0xFF4C6BDD, 0xFF8762DC, 0xFFC857DD, 0xFFCE549F, 0xFFCE5456, 0xFFCE6E33,
			0xFFC48736, 0xFF6A9510, 0xFF3F9E41, 0xFF47B084, 0xFF00A6BE, 0xFF000000, 0xFF000000, 0xFF000000,
			0xFFCEB5DD, 0xFF8AA4DD, 0xFFA198DD, 0xFFAE90DD, 0xFFCE8DDD, 0xFFCE8DBE, 0xFFCE879B, 0xFFCE9B94,
			0xFFCEA48D, 0xFFB7B58D, 0xFF8AACA6, 0xFF90B5B4, 0xFF80B5D3, 0xFF000000, 0xFF000000, 0xFF000000
		},
		{
			0xFF4F5C5C, 0xFF1A1570, 0xFF000087, 0xFF30007D, 0xFF61005E, 0xFF74000F, 0xFF710000, 0xFF560800,
			0xFF2D2500, 0xFF003800, 0xFF003F00, 0xFF003112, 0xFF12314B, 0xFF000000, 0xFF000000, 0xFF000000,
			0xFF7F9494, 0xFF005ABC, 0xFF172EBC, 0xFF5900BF, 0xFF810096, 0xFF9D0047, 0xFF942100, 0xFF8A3E0B,
			0xFF5E5A00, 0xFF007700, 0xFF008700, 0xFF00742E, 0xFF00676D, 0xFF000000, 0xFF000000, 0xFF000000,
			0xFFADC9C9, 0xFF2A96C9, 0xFF4077C9, 0xFF716DC7, 0xFFA761C9, 0xFFAD5E90, 0xFFAD5E4E, 0xFFAD7A2E,
			0xFFA59631, 0xFF59A60F, 0xFF35B03B, 0xFF3BC378, 0xFF00B9AD, 0xFF000000, 0xFF000000, 0xFF000000,
			0xFFADC9C9, 0xFF74B6C9, 0xFF87A9C9, 0xFF92A0C9, 0xFFAD9DC9, 0xFFAD9DAD, 0xFFAD968D, 0xFFADAD87,
			0xFFADB680, 0xFF9AC980, 0xFF74BF96, 0xFF79C9A3, 0xFF6CC9BF, 0xFF000000, 0xFF000000, 0xFF000000
		},
		{
			0xFF515151, 0xFF1B1264, 0xFF000077, 0xFF31006F, 0xFF640053, 0xFF77000D, 0xFF740000, 0xFF580700,
			0xFF2E2000, 0xFF003100, 0xFF003800, 0xFF002C10, 0xFF122C42, 0xFF000000, 0xFF000000, 0xFF000000,
			0xFF838383, 0xFF0050A7, 0xFF1829A7, 0xFF5B00AA, 0xFF850085, 0xFFA1003F, 0xFF991E00, 0xFF8E370A,
			0xFF615000, 0xFF006900, 0xFF007700, 0xFF006629, 0xFF005B61, 0xFF000000, 0xFF000000, 0xFF000000,
			0xFFB2B2B2, 0xFF2C85B2, 0xFF4269B2, 0xFF7461B1, 0xFFAC56B2, 0xFFB25380, 0xFFB25345, 0xFFB26C29,
			0xFFAA852C, 0xFF5B930D, 0xFF379C34, 0xFF3DAD6A, 0xFF00A499, 0xFF000000, 0xFF000000, 0xFF000000,
			0xFFB2B2B2, 0xFF77A1B2, 0xFF8B96B2, 0xFF968EB2, 0xFFB28BB2, 0xFFB28B99, 0xFFB2857D, 0xFFB29977,
			0xFFB2A172, 0xFF9EB272, 0xFF77AA85, 0xFF7DB290, 0xFF6FB2AA, 0xFF000000, 0xFF000000, 0xFF000000
		}
	}
};