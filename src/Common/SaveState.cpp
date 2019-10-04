// SaveState.cpp
// Gestor de SaveState

#define SAVESTATE_CPP

#include <cstdio>
#include <cstdint>
#include <list>

#include "Common.h"
#include "SaveState.h"

// We need to include pretty much everything...
// CPU
#include "../CPU/CPU.h"
#include "../CPU/Internal.h"
#include "../CPU/Interrupts.h"
#include "../CPU/Input.h"
#include "../CPU/Memory.h"
#include "../CPU/Registers.h"

// PPU
#include "../PPU/PPU.h"
#include "../PPU/Internal.h"
#include "../PPU/FrameBuffer.h"
#include "../PPU/Registers.h"

// APU
#include "../APU/APU.h"
#include "../APU/Internal.h"
#include "../APU/FrameSequencer.h"

// Saves a state
void SaveState::Save(int slot)
{
	Common::Serializer buffer;

	// ------------------
	// HEADER INFORMATION
	// ------------------

	// Save state version
	buffer.Copy((uint8_t) VERSION);

	// First, get variable sizes...
	buffer.Copy((uint8_t) sizeof(uint8_t));
	buffer.Copy((uint8_t) sizeof(int));
	buffer.Copy((uint8_t) sizeof(unsigned short));
	buffer.Copy((uint8_t) sizeof(unsigned long));
	buffer.Copy((uint8_t) sizeof(bool));

	// Copy the game name (and its length)
	buffer.Copy((uint32_t) strlen(CPU::GameName));
	buffer.Copy(CPU::GameName, (uint32_t) strlen(CPU::GameName));

	// Copy the game's CRC
	buffer.Copy(CPU::CRC);

	// ---
	// CPU
	// ---

	// Copy CPU::Internal
	buffer.Copy(CPU::opcode);
	buffer.Copy(CPU::opcodeData);
	buffer.Copy(CPU::cycles);
	buffer.Copy(CPU::cyclesForFrame);
	buffer.Copy(CPU::evenOddTotalCycles);
	buffer.Copy(CPU::delayedTotalCycles);
	buffer.Copy(CPU::startCycle);
	buffer.Copy(CPU::doOAMWrite);
	buffer.Copy(CPU::dmcDelayCycle);
	buffer.Copy(CPU::dmcDelayReg);
	buffer.Copy(CPU::gameMapper);
	buffer.Copy(CPU::PRGpages);
	buffer.Copy(CPU::CHRpages);
	buffer.Copy(CPU::romSize);
	buffer.Copy(CPU::chrSize);
	buffer.Copy(CPU::SRAMEnabled);
	buffer.Copy(CPU::SRAMWrite);
	buffer.Copy(CPU::BatterySRAM);

	// Copy CPU::Interrupts
	buffer.Copy(CPU::Interrupts::NMIEnabled);
	buffer.Copy(CPU::Interrupts::IRQEnabled);
	buffer.Copy(CPU::Interrupts::Reset);
	buffer.Copy(CPU::Interrupts::IRQPending);
	buffer.Copy(CPU::Interrupts::IRQActive);
	buffer.Copy(CPU::Interrupts::NMIThisFrame);
	buffer.Copy(CPU::Interrupts::NMIPending);
	buffer.Copy(CPU::Interrupts::DelayNMI);
	buffer.Copy(CPU::Interrupts::InNMI);
	buffer.Copy(CPU::Interrupts::NextIRQCycle);
	buffer.Copy((int) CPU::Interrupts::NextIRQType);
	buffer.Copy(CPU::Interrupts::IRQs, 3 * sizeof(int));

	// Copy CPU::Memory
	buffer.Copy(CPU::Memory::ZP,  0x100);
	buffer.Copy(CPU::Memory::RAM, 0x600);
	buffer.Copy(CPU::Memory::SRAM, 0x2000);
	buffer.Copy(CPU::Memory::openBusValue);
	buffer.Copy(CPU::Stack::memory, 0x100);

	// Copy CPU::Registers
	buffer.Copy(CPU::Registers::A);
	buffer.Copy(CPU::Registers::X);
	buffer.Copy(CPU::Registers::Y);
	buffer.Copy(CPU::Registers::PC);
	buffer.Copy(CPU::Registers::SP);
	buffer.Copy(CPU::Registers::P::N);
	buffer.Copy(CPU::Registers::P::V);
	buffer.Copy(CPU::Registers::P::D);
	buffer.Copy(CPU::Registers::P::I);
	buffer.Copy(CPU::Registers::P::Z);
	buffer.Copy(CPU::Registers::P::C);

	// ---
	// PPU
	// ---

	// Copy PPU::Internal
	if(!CPU::CHRpages)
		buffer.Copy(PPU::VRAM, 0x2000);
	buffer.Copy(PPU::nameTables, 0x1000);
	buffer.Copy(PPU::SPRRAM, 0x100);
	buffer.Copy(PPU::Pallete, 0x20);
	buffer.Copy(PPU::registerPallete);
	buffer.Copy(PPU::SPRRAMaddr);
	buffer.Copy(PPU::flipFlop);
	buffer.Copy(PPU::VRAMTempBuffer);
	buffer.Copy(PPU::remVBLCycle);
	buffer.Copy((int) PPU::Mirror);
	buffer.Copy(PPU::doRender);
	buffer.Copy(PPU::IsEvenFrame);
	buffer.Copy(PPU::InternalSync);
	buffer.Copy(PPU::CalculatedFrameSync);
	buffer.Copy(PPU::ReduceClock);

	// Copy PPU::Registers
	buffer.Copy(PPU::Registers::increment);
	buffer.Copy(PPU::Registers::SpriteTable);
	buffer.Copy(PPU::Registers::BackgroundTable);
	buffer.Copy(PPU::Registers::spriteHeight);
	buffer.Copy(PPU::Registers::palleteNum);
	buffer.Copy(PPU::Registers::grayscale);
	buffer.Copy(PPU::Registers::clipBackground);
	buffer.Copy(PPU::Registers::clipSprites);
	buffer.Copy(PPU::Registers::showBackground);
	buffer.Copy(PPU::Registers::showSprites);
	buffer.Copy(PPU::Registers::colorMode);
	buffer.Copy(PPU::Registers::Decay);
	buffer.Copy(PPU::Registers::DecayFrame);
	buffer.Copy(PPU::Registers::LastReadPPUData);
	buffer.Copy(PPU::Registers::Status::HasHitMaxSprites);
	buffer.Copy(PPU::Registers::Status::HasHitSprite0);
	buffer.Copy(PPU::Registers::Status::IsVBlank);
	buffer.Copy(PPU::Registers::Status::ReadVBlankThisFrame);
	buffer.Copy(PPU::Registers::AddressLatch::CurrentNameTable);
	buffer.Copy(PPU::Registers::AddressLatch::TileXScroll);
	buffer.Copy(PPU::Registers::AddressLatch::TileYScroll);
	buffer.Copy(PPU::Registers::AddressLatch::FineXScroll);
	buffer.Copy(PPU::Registers::AddressLatch::FineYScroll);
	buffer.Copy(PPU::Registers::AddressLatch::TempCurrentNameTable);
	buffer.Copy(PPU::Registers::AddressLatch::TempTileXScroll);
	buffer.Copy(PPU::Registers::AddressLatch::TempTileYScroll);
	buffer.Copy(PPU::Registers::AddressLatch::TempFineYScroll);
	buffer.Copy(PPU::Registers::AddressLatch::TempFineXScroll);

	// ---
	// APU
	// ---

	// Copy APU::Internal
	buffer.Copy(APU::InternalCycle);
	buffer.Copy((int) APU::NextEvent);
	buffer.Copy(APU::NextEventCycle);
	buffer.Copy(APU::EventList, 6 * sizeof(int));
	buffer.Copy(APU::LastCycle, 6 * sizeof(int));
	buffer.Copy(APU::NextEventList, 6 * sizeof(int));
	buffer.Copy(APU::SquareChannel::Data, sizeof(APU::SquareChannel::SWData) << 1);
	buffer.Copy(APU::TriangleChannel::Timer);
	buffer.Copy(APU::TriangleChannel::TimerInternal);
	buffer.Copy(APU::TriangleChannel::LengthCounter);
	buffer.Copy(APU::TriangleChannel::LinearCounter);
	buffer.Copy(APU::TriangleChannel::LinearReloadValue);
	buffer.Copy(APU::TriangleChannel::LinearHalted);
	buffer.Copy(APU::TriangleChannel::Active);
	buffer.Copy(APU::TriangleChannel::Halted);
	buffer.Copy(APU::TriangleChannel::WavePos);
	buffer.Copy(APU::TriangleChannel::DAC);
	buffer.Copy(APU::NoiseChannel::Timer);
	buffer.Copy(APU::NoiseChannel::TimerInternal);
	buffer.Copy(APU::NoiseChannel::LengthCounter);
	buffer.Copy(APU::NoiseChannel::ShiftRegister);
	buffer.Copy(APU::NoiseChannel::BitMode);
	buffer.Copy(APU::NoiseChannel::Halted);
	buffer.Copy(APU::NoiseChannel::Active);
	buffer.Copy(APU::NoiseChannel::DAC);
	buffer.Copy(APU::NoiseChannel::Volume);
	buffer.Copy(APU::NoiseChannel::Start);
	buffer.Copy(APU::NoiseChannel::Constant);
	buffer.Copy(APU::NoiseChannel::VolCycle);
	buffer.Copy(APU::NoiseChannel::VolDecay);
	buffer.Copy(APU::NoiseChannel::VolDecayInternal);
	buffer.Copy(APU::DMCChannel::Active);
	buffer.Copy(APU::DMCChannel::IRQEnabled);
	buffer.Copy(APU::DMCChannel::IRQPending);
	buffer.Copy(APU::DMCChannel::InternalIRQCycle);
	buffer.Copy(APU::DMCChannel::Timer);
	buffer.Copy(APU::DMCChannel::TimerInternal);
	buffer.Copy(APU::DMCChannel::Loop);
	buffer.Copy(APU::DMCChannel::TimerList[16]);
	buffer.Copy(APU::DMCChannel::Buffer);
	buffer.Copy(APU::DMCChannel::Empty);
	buffer.Copy(APU::DMCChannel::Addr);
	buffer.Copy(APU::DMCChannel::AddrInternal);
	buffer.Copy(APU::DMCChannel::Bytes);
	buffer.Copy(APU::DMCChannel::Remaining);
	buffer.Copy(APU::DMCChannel::Restart);
	buffer.Copy(APU::DMCChannel::Bit);
	buffer.Copy(APU::DMCChannel::Silence);
	buffer.Copy(APU::DMCChannel::OutputSample);
	buffer.Copy(APU::DMCChannel::Counter);

	// Copy APU::FrameSequencer
	buffer.Copy(APU::FrameSequencer::Steps);
	buffer.Copy(APU::FrameSequencer::StepCycles, 5 * sizeof(int));
	buffer.Copy(APU::FrameSequencer::LastStep);
	buffer.Copy(APU::FrameSequencer::InternalIRQCycle);
	buffer.Copy(APU::FrameSequencer::IRQPending);
	buffer.Copy(APU::FrameSequencer::DontEnablePending);
	buffer.Copy(APU::FrameSequencer::ExtraIRQCycles);
	buffer.Copy(APU::FrameSequencer::CyclesToAdd);

	// ------
	// MAPPER
	// ------

	// Get the mapper size and save it to the file
	CPU::Memory::mapper->SerializeTo(buffer);
	
	// ---------------
	// FILE GENERATION
	// ---------------

	// Generate the file name
	char * stateFileName = new char[strlen(CPU::GameName) + 5];

	memcpy(stateFileName, ".\\", 2);
	memcpy(stateFileName + 2, CPU::GameName, strlen(CPU::GameName) - 3);
	sprintf(stateFileName + strlen(CPU::GameName) - 1, "%d.sst", slot);

	try
	{
		buffer.GenerateFile(stateFileName);

		delete [] stateFileName;

		PPU::FrameBuffer::DisplayString("State saved");
	}

	// Caught an error, destroy memory
	catch(...)
	{
		PPU::FrameBuffer::DisplayString("Error saving state");
			
		delete [] stateFileName;
	}
}

// Loads a state
void SaveState::Load(int slot)
{
	// ------------
	// FILE LOADING
	// ------------

	// Generate the file name
	char * stateFileName = new char[strlen(CPU::GameName) + 5];

	memcpy(stateFileName, ".\\", 2);
	memcpy(stateFileName + 2, CPU::GameName, strlen(CPU::GameName) - 3);
	sprintf(stateFileName + strlen(CPU::GameName) - 1, "%d.sst", slot);

	Common::Unserializer loader;

	try
	{
		loader.LoadFromFile(stateFileName);

		// Set the sizes
		loader.SetSizes();

		// Load the game name
		uint32_t nameLen;
		loader.Set(nameLen);

		// Check if the save state is for another game
		if(nameLen != strlen(CPU::GameName))
			throw 3;

		// Compare by game name
		if(loader.Compare(CPU::GameName, nameLen))
			throw 3;

		unsigned int CRC;
		loader.Set(CRC);

		// Compare by CRC
		if(CRC != CPU::CRC)
			throw 3;
	}

	catch(int error)
	{
		delete [] stateFileName;

		switch(error)
		{
			case 1: PPU::FrameBuffer::DisplayString("No state file in this slot");     break;
			case 2: PPU::FrameBuffer::DisplayString("Invalid or corrupt save state");  break;
			case 3: PPU::FrameBuffer::DisplayString("Save state is for another game"); break;
		}

		return;
	}

	// This variable stores enum data
	int enumTemp = 0;

	// ---
	// CPU
	// ---

	// Copy CPU::Internal
	loader.Set(CPU::opcode);
	loader.Set(CPU::opcodeData);
	loader.Set(CPU::cycles);
	loader.Set(CPU::cyclesForFrame);
	loader.Set(CPU::evenOddTotalCycles);
	loader.Set(CPU::delayedTotalCycles);
	loader.Set(CPU::startCycle);
	loader.Set(CPU::doOAMWrite);
	loader.Set(CPU::dmcDelayCycle);
	loader.Set(CPU::dmcDelayReg);
	loader.Set(CPU::gameMapper);
	loader.Set(CPU::PRGpages);
	loader.Set(CPU::CHRpages);
	loader.Set(CPU::romSize);
	loader.Set(CPU::chrSize);
	loader.Set(CPU::SRAMEnabled);
	loader.Set(CPU::SRAMWrite);
	loader.Set(CPU::BatterySRAM);

	// Copy CPU::Interrupts
	loader.Set(CPU::Interrupts::NMIEnabled);
	loader.Set(CPU::Interrupts::IRQEnabled);
	loader.Set(CPU::Interrupts::Reset);
	loader.Set(CPU::Interrupts::IRQPending);
	loader.Set(CPU::Interrupts::IRQActive);
	loader.Set(CPU::Interrupts::NMIThisFrame);
	loader.Set(CPU::Interrupts::NMIPending);
	loader.Set(CPU::Interrupts::DelayNMI);
	loader.Set(CPU::Interrupts::InNMI);
	loader.Set(CPU::Interrupts::NextIRQCycle);
	loader.Set(enumTemp);
	CPU::Interrupts::NextIRQType = (CPU::Interrupts::IRQType) enumTemp;
	loader.Set(CPU::Interrupts::IRQs, 3 * sizeof(int));

	// Copy CPU::Memory
	loader.Set(CPU::Memory::ZP,  0x100);
	loader.Set(CPU::Memory::RAM, 0x600);
	loader.Set(CPU::Memory::SRAM, 0x2000);
	loader.Set(CPU::Memory::openBusValue);
	loader.Set(CPU::Stack::memory, 0x100);

	// Copy CPU::Registers
	loader.Set(CPU::Registers::A);
	loader.Set(CPU::Registers::X);
	loader.Set(CPU::Registers::Y);
	loader.Set(CPU::Registers::PC);
	loader.Set(CPU::Registers::SP);
	loader.Set(CPU::Registers::P::N);
	loader.Set(CPU::Registers::P::V);
	loader.Set(CPU::Registers::P::D);
	loader.Set(CPU::Registers::P::I);
	loader.Set(CPU::Registers::P::Z);
	loader.Set(CPU::Registers::P::C);

	// ---
	// PPU
	// ---

	// Copy PPU::Internal
	if(!CPU::CHRpages)
		loader.Set(PPU::VRAM, 0x2000);
	loader.Set(PPU::nameTables, 0x1000);
	loader.Set(PPU::SPRRAM, 0x100);
	loader.Set(PPU::Pallete, 0x20);
	loader.Set(PPU::registerPallete);
	loader.Set(PPU::SPRRAMaddr);
	loader.Set(PPU::flipFlop);
	loader.Set(PPU::VRAMTempBuffer);
	loader.Set(PPU::remVBLCycle);
	loader.Set(enumTemp);
	PPU::Mirror = (PPU::MirrorMode) enumTemp;
	loader.Set(PPU::doRender);
	loader.Set(PPU::IsEvenFrame);
	loader.Set(PPU::InternalSync);
	loader.Set(PPU::CalculatedFrameSync);
	loader.Set(PPU::ReduceClock);

	// Copy PPU::Registers
	loader.Set(PPU::Registers::increment);
	loader.Set(PPU::Registers::SpriteTable);
	loader.Set(PPU::Registers::BackgroundTable);
	loader.Set(PPU::Registers::spriteHeight);
	loader.Set(PPU::Registers::palleteNum);
	loader.Set(PPU::Registers::grayscale);
	loader.Set(PPU::Registers::clipBackground);
	loader.Set(PPU::Registers::clipSprites);
	loader.Set(PPU::Registers::showBackground);
	loader.Set(PPU::Registers::showSprites);
	loader.Set(PPU::Registers::colorMode);
	loader.Set(PPU::Registers::Decay);
	loader.Set(PPU::Registers::DecayFrame);
	loader.Set(PPU::Registers::LastReadPPUData);
	loader.Set(PPU::Registers::Status::HasHitMaxSprites);
	loader.Set(PPU::Registers::Status::HasHitSprite0);
	loader.Set(PPU::Registers::Status::IsVBlank);
	loader.Set(PPU::Registers::Status::ReadVBlankThisFrame);
	loader.Set(PPU::Registers::AddressLatch::CurrentNameTable);
	loader.Set(PPU::Registers::AddressLatch::TileXScroll);
	loader.Set(PPU::Registers::AddressLatch::TileYScroll);
	loader.Set(PPU::Registers::AddressLatch::FineXScroll);
	loader.Set(PPU::Registers::AddressLatch::FineYScroll);
	loader.Set(PPU::Registers::AddressLatch::TempCurrentNameTable);
	loader.Set(PPU::Registers::AddressLatch::TempTileXScroll);
	loader.Set(PPU::Registers::AddressLatch::TempTileYScroll);
	loader.Set(PPU::Registers::AddressLatch::TempFineYScroll);
	loader.Set(PPU::Registers::AddressLatch::TempFineXScroll);

	// ---
	// APU
	// ---

	// Copy APU::Internal
	loader.Set(APU::InternalCycle);
	loader.Set(enumTemp);
	APU::NextEvent = (APU::Events) enumTemp;
	loader.Set(APU::NextEventCycle);
	loader.Set(APU::EventList, 6 * sizeof(int));
	loader.Set(APU::LastCycle, 6 * sizeof(int));
	loader.Set(APU::NextEventList, 6 * sizeof(int));
	loader.Set(APU::SquareChannel::Data, sizeof(APU::SquareChannel::SWData) << 1);
	loader.Set(APU::TriangleChannel::Timer);
	loader.Set(APU::TriangleChannel::TimerInternal);
	loader.Set(APU::TriangleChannel::LengthCounter);
	loader.Set(APU::TriangleChannel::LinearCounter);
	loader.Set(APU::TriangleChannel::LinearReloadValue);
	loader.Set(APU::TriangleChannel::LinearHalted);
	loader.Set(APU::TriangleChannel::Active);
	loader.Set(APU::TriangleChannel::Halted);
	loader.Set(APU::TriangleChannel::WavePos);
	loader.Set(APU::TriangleChannel::DAC);
	loader.Set(APU::NoiseChannel::Timer);
	loader.Set(APU::NoiseChannel::TimerInternal);
	loader.Set(APU::NoiseChannel::LengthCounter);
	loader.Set(APU::NoiseChannel::ShiftRegister);
	loader.Set(APU::NoiseChannel::BitMode);
	loader.Set(APU::NoiseChannel::Halted);
	loader.Set(APU::NoiseChannel::Active);
	loader.Set(APU::NoiseChannel::DAC);
	loader.Set(APU::NoiseChannel::Volume);
	loader.Set(APU::NoiseChannel::Start);
	loader.Set(APU::NoiseChannel::Constant);
	loader.Set(APU::NoiseChannel::VolCycle);
	loader.Set(APU::NoiseChannel::VolDecay);
	loader.Set(APU::NoiseChannel::VolDecayInternal);
	loader.Set(APU::DMCChannel::Active);
	loader.Set(APU::DMCChannel::IRQEnabled);
	loader.Set(APU::DMCChannel::IRQPending);
	loader.Set(APU::DMCChannel::InternalIRQCycle);
	loader.Set(APU::DMCChannel::Timer);
	loader.Set(APU::DMCChannel::TimerInternal);
	loader.Set(APU::DMCChannel::Loop);
	loader.Set(APU::DMCChannel::TimerList[16]);
	loader.Set(APU::DMCChannel::Buffer);
	loader.Set(APU::DMCChannel::Empty);
	loader.Set(APU::DMCChannel::Addr);
	loader.Set(APU::DMCChannel::AddrInternal);
	loader.Set(APU::DMCChannel::Bytes);
	loader.Set(APU::DMCChannel::Remaining);
	loader.Set(APU::DMCChannel::Restart);
	loader.Set(APU::DMCChannel::Bit);
	loader.Set(APU::DMCChannel::Silence);
	loader.Set(APU::DMCChannel::OutputSample);
	loader.Set(APU::DMCChannel::Counter);

	// Copy APU::FrameSequencer
	loader.Set(APU::FrameSequencer::Steps);
	loader.Set(APU::FrameSequencer::StepCycles, 5 * sizeof(int));
	loader.Set(APU::FrameSequencer::LastStep);
	loader.Set(APU::FrameSequencer::InternalIRQCycle);
	loader.Set(APU::FrameSequencer::IRQPending);
	loader.Set(APU::FrameSequencer::DontEnablePending);
	loader.Set(APU::FrameSequencer::ExtraIRQCycles);
	loader.Set(APU::FrameSequencer::CyclesToAdd);

	// ------
	// MAPPER
	// ------

	CPU::Memory::mapper->UnserializeFrom(loader);

	// All done!
	PPU::FrameBuffer::DisplayString("State loaded");
}

// Reinicia as variáveis
void SaveState::Reset()
{
	SavedWithCurrentKeypress = false;
	LoadedWithCurrentKeypress = false;
}

// Verifica se é necessário fazer save ou load state
void SaveState::CheckForSaveOrLoad()
{
	if(CPU::Input::StateSaveHotKeyPressed())
	{
		if(!SavedWithCurrentKeypress)
		{
			Save();
			SavedWithCurrentKeypress = true;
		}
	}
	else
		SavedWithCurrentKeypress = false;

	if(CPU::Input::StateLoadHotKeyPressed())
	{
		if(!LoadedWithCurrentKeypress)
		{
			Load();
			LoadedWithCurrentKeypress = true;
		}
	}
	else
		LoadedWithCurrentKeypress = false;
}