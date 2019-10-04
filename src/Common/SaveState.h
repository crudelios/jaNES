// SaveState.h
// Classe que trata do FrameBuffer

#ifndef SAVESTATE_H
#define SAVESTATE_H

#ifndef SAVESTATE_CPP
#define EXTERNAL extern
#else
#define EXTERNAL
#endif

namespace SaveState
{
	// Grava um state
	void Save(int slot = 0);

	// Carrega um state
	void Load(int slot = 0);

	// Reinicia as variáveis
	void Reset();

	// Verifica se é necessário fazer save ou load state
	void CheckForSaveOrLoad();

	// Evita o save/load constante
	EXTERNAL bool SavedWithCurrentKeypress;
	EXTERNAL bool LoadedWithCurrentKeypress;

	static const uint8_t VERSION = 1;
} // END namespace SaveState

#undef EXTERNAL
#endif