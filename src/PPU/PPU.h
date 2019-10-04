// PPU.h
// Tratamento do PPU

#ifndef PPU_H
#define PPU_H

extern "C"
{
	#include <windows.h>
}

#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>

#ifndef PPU_CPP
#define EXTERNAL extern
#else
#define EXTERNAL
#endif

namespace PPU
{
	// Mirrorring
	enum MirrorMode
	{
		HORIZONTAL,
		VERTICAL,
		FOURSCREEN,
		ONESCREEN
	};

	// Inicia o PPU
	void Create();

	// Desenha uma frame
	void DrawFrame();

	// Actualiza o PPU
	void Update();

	// Escreve um valor no registo
	void RegWrite(int reg, unsigned char value);

	// Obtém um valor do registo
	unsigned char RegRead(int reg);

	// Obtém um valor do registo sem alterar dados
	unsigned char RegDebugRead(int reg);

	// Escreve no OAM
	void OAMWrite(unsigned char value);

	// Faz sync dos frames
	unsigned char SyncFrame();

	// Define o modo de mirrorring
	void SetMirrorMode(MirrorMode mode);

	// Limpa um frame, colocando os buffers pristinos para novos dados
	void ClearFrame();

	// Esconde a janela
	void Hide(bool hide = true);

	// Muda o tamanho da janela
	void ResizeRenderer(long width, long height, bool moving);

	// Desactiva o renderer
	void DisableRenderer();

	// Regista a janela principal
	void RegisterWindow(sf::Window * window);

	// Indica o número de frames desde a última vez que se chamou este método
	unsigned long GetTicks();

	// Limita os FPS
	void LimitFPS(unsigned long limit);

	// Resets the PPU
	void Reset();

	// Destrói o PPU
	void Destroy();
} // END namespace PPU

#undef EXTERNAL
#endif