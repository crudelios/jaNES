// Input.h
// Trata do input

#ifndef INPUT_H
#define INPUT_H

#ifndef INPUT_CPP
#define EXTERNAL extern
#else
#define EXTERNAL
#endif

namespace CPU
{
	namespace Input
	{
		struct Joypad
		{
			unsigned char latch;
			unsigned char button[8];

			Joypad();
		};

		EXTERNAL Joypad Joy[2];

		EXTERNAL unsigned char Reset;

		// Coloca os botões prontos para serem lidos novamente
		inline void ResetButtons(unsigned char reset)
		{
 			Reset = reset & 1;

      if(Reset)
      {
			  Joy[0].latch = 0;
			  Joy[1].latch = 0;
      }
		}

		// Lê botões de um joypad
		unsigned char ReadButtons(unsigned char joypad);

		// Indica se foi pressionado o hotkey de save state
		bool StateSaveHotKeyPressed();

		// Indica se foi pressionado o hotkey de load state
		bool StateLoadHotKeyPressed();
	}
}

#undef EXTERNAL
#endif