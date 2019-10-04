// Input.cpp
// Trata do Input

#define INPUT_CPP

#include <cstring>

#include "Input.h"

#include "../PPU/PPU.h"
#include "../PPU/Internal.h"

// Inicializa o joypad
CPU::Input::Joypad::Joypad()
 : latch(0)
{
	memset(button, 0, 8 * sizeof(unsigned char));
}

// Lê botões de um joypad
// TODO resolver bug de quando se carregam em 2 botões do d-pad no mesmo frame
unsigned char CPU::Input::ReadButtons(unsigned char joypad)
{
	if(joypad)
		return 0;

	unsigned char ret = 0;

	switch(Joy[0].latch)
	{
		// A
		case 0:
			ret = sf::Keyboard::isKeyPressed(sf::Keyboard::C) ? 1 : 0;
			break;

		// B
		case 1:
			ret = sf::Keyboard::isKeyPressed(sf::Keyboard::X) ? 1 : 0;
			break;

		// SELECT
		case 2:
			ret = sf::Keyboard::isKeyPressed(sf::Keyboard::Space) ? 1 : 0;
			break;

		// START
		case 3:
			ret = sf::Keyboard::isKeyPressed(sf::Keyboard::Return) ? 1 : 0;
			break;

		// UP
		case 4:
			ret = sf::Keyboard::isKeyPressed(sf::Keyboard::Up) ? 1 : 0;
			break;

		// DOWN
		case 5:
			ret = sf::Keyboard::isKeyPressed(sf::Keyboard::Down) ? 1 : 0;
			break;

		// LEFT
		case 6:
			ret = sf::Keyboard::isKeyPressed(sf::Keyboard::Left) ? 1 : 0;
			break;

		// RIGHT
		case 7:
			ret = sf::Keyboard::isKeyPressed(sf::Keyboard::Right) ? 1 : 0;
			break;

		default:
			Joy[0].latch = 8;
			ret = 1;
	}

	if(!Reset)
		++Joy[0].latch;

	return ret;
}

// Indica se foi pressionado o hotkey de save state
bool CPU::Input::StateSaveHotKeyPressed()
{
	return sf::Keyboard::isKeyPressed(sf::Keyboard::F5);
}

// Indica se foi pressionado o hotkey de load state
bool CPU::Input::StateLoadHotKeyPressed()
{
	return sf::Keyboard::isKeyPressed(sf::Keyboard::F7);
}
