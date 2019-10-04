// Internal.h
// Dados internos do PPU

#ifndef PPUINTERNAL_H
#define PPUINTERNAL_H

#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>

#ifndef PPU_CPP
#define EXTERNAL extern
#else
#define EXTERNAL
#endif

namespace PPU
{
	// Memória (tem que ir para um namespace próprio)
	EXTERNAL unsigned char * mapper[16];  // Mapeador de memória
	EXTERNAL unsigned char * VRAM;
	EXTERNAL unsigned char nameTables[4][0x400];
	EXTERNAL unsigned char SPRRAM[0x100];
	EXTERNAL unsigned char SPRRAM2[0x40];
	EXTERNAL unsigned char Pallete[0x20];
	EXTERNAL bool ReadOnly[16];

	// Endereçamentos internos
	EXTERNAL unsigned short registerPallete;

	EXTERNAL unsigned char   SPRRAMaddr;
	EXTERNAL unsigned char * SPRRAMptr;
	EXTERNAL int flipFlop;
	EXTERNAL unsigned char VRAMTempBuffer;

	// Dados de VBlank
	EXTERNAL unsigned int remVBLCycle;

	// Mirror mode
	EXTERNAL MirrorMode Mirror;

	// Informações sobre o estado de funcionamento do PPU
	EXTERNAL int  ready;
	extern   bool working;
	EXTERNAL sf::Window * RenderWindow;
	EXTERNAL HWND MainWindow;

	EXTERNAL bool doRender;

	// PPU <-> CPU sync
	EXTERNAL unsigned char IsEvenFrame;
	EXTERNAL unsigned char InternalSync;
	EXTERNAL bool CalculatedFrameSync;
	EXTERNAL int ReduceClock;

	// Indica se deve alterar os regs do PPU quando se faz uma leitura
	EXTERNAL bool dbgRead;

	// Dados de render
	extern   unsigned int PalleteColors[2][8][64];
	EXTERNAL unsigned int * CurrPallete;
	EXTERNAL unsigned int LastScanLine;
	EXTERNAL unsigned int LastPixel;
	EXTERNAL unsigned char PixelOpacity[61440];

	EXTERNAL unsigned char * TilePtr;
	EXTERNAL unsigned char TileAttribute;
	EXTERNAL unsigned int  TileIndex;

	EXTERNAL bool FrameDone;

	void UpdateMirrors();

	// Indica se estamos em VBlank
	bool InVBlank();

	// Colocar um range de memória como read-only
	inline void SetReadOnly(unsigned int start, unsigned int end, bool readOnly)
	{
		for(unsigned int i = start; i <= end; ++ i)
			ReadOnly[i] = readOnly;
	}

	// Colocar a memória como read-only
	inline void SetReadOnly(unsigned int index, bool readOnly)
	{
		SetReadOnly(index, index, readOnly);
	}

#ifdef _PRINTDISASSEMBLY
#define IFNOTDEBUGGING() if(!dbgRead)


#else
	#define IFNOTDEBUGGING()
#endif
} // END namespace PPU

#undef EXTERNAL
#endif