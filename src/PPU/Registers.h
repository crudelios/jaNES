// Registers.h
// Registos do PPU

#ifndef PPUREGISTERS_H
#define PPUREGISTERS_H

#include "../CPU/Internal.h"
#include "../CPU/Interrupts.h"
#include "../CPU/Memory.h"

#ifndef PPUREGISTERS_CPP
#define EXTERNAL extern
#else
#define EXTERNAL
#endif


namespace PPU
{
	// --------------------------------------------------------------------------
	// ACESSO A REGISTERS -------------------------------------------------------
	// --------------------------------------------------------------------------

	namespace Registers
	{
		EXTERNAL unsigned int increment;
		EXTERNAL unsigned int SpriteTable;
		EXTERNAL unsigned int BackgroundTable;
		EXTERNAL unsigned int spriteHeight;
		EXTERNAL unsigned int palleteNum;

		EXTERNAL unsigned char grayscale;
		EXTERNAL bool clipBackground;
		EXTERNAL bool clipSprites;
		EXTERNAL bool showBackground;
		EXTERNAL bool showSprites;

		EXTERNAL int colorMode;

		// Decay latch
		EXTERNAL unsigned char Decay;
		EXTERNAL unsigned int  DecayFrame;
		EXTERNAL int           LastReadPPUData;

		// Status register
		namespace Status
		{
			EXTERNAL int HasHitMaxSprites;
			EXTERNAL int HasHitSprite0;
			EXTERNAL int IsVBlank;
			EXTERNAL int ReadVBlankThisFrame;

			inline int CalculateVBlank()
			{
				//int vclear = (!(IsEvenFrame && Registers::showBackground) && (InternalSync > 2)) ? 0 : 1;

				// Activar vblank (reproduz o bug de nunca activar vblank se o timing for certo)
				if(CPU::cycles >= (CPU::cyclesForFrame - 3))
				{
					int ret = ReadVBlankThisFrame ^ 1;
					ReadVBlankThisFrame = 1;

					// Bug de nunca chamar o NMI se se conferir VBlank em determinada altura
					if((CPU::cycles >= (CPU::cyclesForFrame - 3)) && (CPU::cycles < CPU::cyclesForFrame))
						CPU::Interrupts::NMIThisFrame = true;

					return (CPU::cycles == (CPU::cyclesForFrame - 3)) ? 0 : ret;
				}

				int ret = (CPU::cycles < 6818) ? IsVBlank : 0;
				IsVBlank = 0;
				return ret;
			}

			// Calcula se já houve sprite zero hit
			unsigned int CalculateSprite0();

			// Calcula overflow de sprites
			unsigned int CalculateSpriteOverflow(int start = 0);

			inline unsigned char Pack()
			{
				return (CalculateVBlank() << 7) | ((HasHitSprite0) ? 0x40 : 0) | ((HasHitMaxSprites) ? 0x20 : 0) & 0xE0;
			}
		}

		// Este namespace trata do funcionamento do address latch
		namespace AddressLatch
		{
			EXTERNAL unsigned char CurrentNameTable;
			EXTERNAL unsigned char TileXScroll;
			EXTERNAL unsigned char TileYScroll;
			EXTERNAL unsigned char FineXScroll;
			EXTERNAL unsigned char FineYScroll;

			EXTERNAL unsigned char TempCurrentNameTable;
			EXTERNAL unsigned char TempTileXScroll;
			EXTERNAL unsigned char TempTileYScroll;
			EXTERNAL unsigned char TempFineYScroll;
			EXTERNAL unsigned char TempFineXScroll;

			// Obtém a posição do ram actual para leitura durante o render
			inline unsigned short GetRenderVRamLocation()
			{
				return TileXScroll | (TileYScroll << 5) | (CurrentNameTable << 10) | 0x2000;
			}

			// Obtém um pointer para um tile específico
			unsigned char * GetTilePtrFromPos(unsigned int x, unsigned int y);

			// Obtém a posição verdadeira do ram actual (mas com range 0x0000 ... 0x7FFF)
			inline unsigned short GetActualAddress()
			{
				return TileXScroll | (TileYScroll << 5) | (CurrentNameTable << 10) | (FineYScroll << 12);
			}

			// Incrementa os latches internos a partir do acesso ao register 2007
			inline void Increment2007()
			{
				// Incrementar
				TileXScroll      += increment;
				TileYScroll      += (TileXScroll & 0x20) >> 5;
				CurrentNameTable += (TileYScroll & 0x20) >> 5;
				FineYScroll      += (CurrentNameTable & 4) >> 2;

				// Trimmar
				TileXScroll      &= 0x1F;
				TileYScroll      &= 0x1F;
				CurrentNameTable &= 0x3;
				FineYScroll      &= 0x7;

				// Avisar o mapper
				CPU::Memory::mapper->NotifyPPUAddressChanged(GetActualAddress() & 0x3FFF);
			}

			// Incrementa os latches verticais durante o render e faz reset dos latches horizontais
			void IncrementVReadVert();

			// Incrementa os latches horizontais durante o render
			inline void IncrementVReadHoriz()
			{
				++TileXScroll; // += (++FineXScroll & 0x8) >> 3;
				CurrentNameTable ^= (TileXScroll & 0x20) >> 5;
				TileXScroll &= 0x1F;
				//FineXScroll &= 0x7;
			}

			// Faz reset ao x scroll
			inline void ResetXScroll()
			{
				// Fazer aqui reset ao tile X e ao nametable horizontal
				//FineXScroll      = TempFineXScroll;
				TileXScroll      = TempTileXScroll;
				CurrentNameTable = (CurrentNameTable & 2) | (TempCurrentNameTable & 1);
			}

			// Incrementa os latches horizontais durante o render para um tile inteiro
			inline void IncrementVReadHorizTile()
			{
				CurrentNameTable ^= (++TileXScroll & 0x20) >> 5;
				TileXScroll &= 0x1F;
			}

		} // END namespace AddressLatch


		// --------------------
		// Leitura de registers
		// --------------------

		// Ler PPU Status
		unsigned char ReadPPUStatus();

		// Ler OAM Data
		unsigned char ReadOAMData();

		// Ler PPU Data
		unsigned char ReadPPUData();

		// Leitura de registos write only
		unsigned char ReadDummy();

		// --------------------
		// Escrita de registers
		// --------------------

		// Escrever no PPU Controller
		void WritePPUController(unsigned char value);

		// Escrever no PPU Mask
		void WritePPUMask(unsigned char value);

		// Escrever no PPU Status - read only, não fazer nada
		void WritePPUStatus(unsigned char value);

		// Escrever endereço de OAM
		void WriteOAMAddr(unsigned char value);

		// Escrever dados no OAM
		void WriteOAMData(unsigned char value);

		// Alterar as coordenadas de scroll
		void WritePPUScroll(unsigned char value);

		// Alterar endereço do PPU
		void WritePPUAddr(unsigned char value);

		// Alterar dados do PPU
		void WritePPUData(unsigned char value);

		// Mapa de funções de leitura de registers
		extern unsigned char (*ReadFunc[8])();

		// Mapa de funções de escrita de registers
		extern void (*WriteFunc[8])(unsigned char value);

	} // END namespace Registers

} // END namespace PPU

#undef EXTERNAL
#endif