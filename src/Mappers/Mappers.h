// Mappers.h
// Classe base de mappers

#ifndef MAPPERS_H
#define MAPPERS_H

#include <algorithm>
#include "../Common/Common.h"

#ifndef MAPPERS_CPP
#define EXTERNAL extern
#else
#define EXTERNAL
#endif

namespace CPU
{
	namespace Memory
	{
		class Mapper
		{
			public:

				// Cria um mapper
				static void Create();

				// Popula um mapper
				virtual void Populate();

				// Escreve no mapper
				virtual void Write(unsigned short addr, unsigned char value);

				// Indica que o PPU foi criado
				virtual void NotifyPPUCreated();

				// Indica que estamos num novo frame
				virtual void NotifyNewFrame();

				// Indica que ocorreu um interrupt que teve origem no mapper
				virtual bool NotifyInterrupt();

				// Indica que o endereço do PPU foi alterado
				virtual void NotifyPPUAddressChanged(unsigned short addr);

				// Indica que o render foi activado/desactivado
				virtual void NotifyToggleRender();

				// Destrói um mapper
				static void Destroy();

				// Save state methods
				// Creates save state info
				virtual void SerializeTo(Common::Serializer &) {}

				// Loads save state into memory
				virtual void UnserializeFrom(Common::Unserializer &) {}

			protected:
				Mapper() {}
				virtual ~Mapper() {}
		};

		class Mapper1 : public Mapper
		{
			friend class Mapper;

			public:

				// Popula um mapper
				virtual void Populate();

				// Indica que estamos num novo frame
				virtual void NotifyNewFrame();

				// Escreve no mapper
				virtual void Write(unsigned short addr, unsigned char value);

				// Save state methods
				// Creates save state info
				virtual void SerializeTo(Common::Serializer & buffer);

				// Loads save state into memory
				virtual void UnserializeFrom(Common::Unserializer & loader);

			private:

				// Latch de escrita no registo
				unsigned char mapperData; // Contém os dados a escrever no registo do mapper
				unsigned char dataLatch;  // Contém a posição actual do registo
		
				// Indica o ciclo em que foi a última escrita
				int LastWrite;

				// Indica o último endereço escrito
				unsigned short LastAddr;

				// Dados de registo 0 actuais
				unsigned char CurrentPPUMirror;
				unsigned char BankSize;
				unsigned char SwapPRGPos;
				unsigned char SwapCHRSize;
				unsigned long HighPRG;

				// Current regs 1 and 2 data
				unsigned char CurrentCHRBank1;
				unsigned char CurrentCHRBank2;

				// Dados de registo 3 actuais
				unsigned char CurrentPRGBank;
				bool Reg3Written;

				// Altera os registers
				void DoSwapPRGCtrl();
				void ChangeReg0();
				void ChangeReg1();
				void ChangeReg2();
				void ChangeReg3();

				// Tratamento de banks prg
				void ResetLowPRGBank();
				void ResetHighPRGBank();
				void SwapPRGBanks(unsigned int bankNum);

				// Inicializa o mapper
				Mapper1() : Mapper(), mapperData(0), dataLatch(0), LastWrite(-10), LastAddr(0x8000), CurrentPPUMirror(255), BankSize(0x40), SwapPRGPos(0x80), SwapCHRSize(4), HighPRG(0), CurrentCHRBank1(0), CurrentCHRBank2(0), CurrentPRGBank(0), Reg3Written(false) {}
		};

		class Mapper3 : public Mapper
		{
			friend class Mapper;

				unsigned char BankNum;

				// Initializes the mapper
				Mapper3() : BankNum(0) {}

			public:

				// Escreve no mapper
				virtual void Write(unsigned short addr, unsigned char value);

				// Save state methods
				// Creates save state info
				virtual void SerializeTo(Common::Serializer & buffer);

				// Loads save state into memory
				virtual void UnserializeFrom(Common::Unserializer & loader);
		};

		class Mapper4 : public Mapper
		{
			friend class Mapper;

			public:

				// Popula um mapper
				virtual void Populate();

				// Escreve na memória
				virtual void Write(unsigned short addr, unsigned char value);

			private:

				// Comando a realizar
				unsigned char Command;

				// Posições de swap de banking
				unsigned char SwapPRGPos;
				unsigned char SwapCHRPos;

				// PRG Banks
				unsigned char PRGBank1;
				unsigned char PRGBank2;

				// CHR banks
				unsigned char CHRBanks[6];

				// Acções de registers
				void ChangeControlReg(unsigned char value);
				void SwapBanks(unsigned char value);

				// IRQ Scanlines
				unsigned char CurrentScanlineCounter;
				unsigned char RequestedScanlineCounter;
				unsigned short PPUAddr;
				int LastCounterTickCycle;
				int InternalIRQCycle;
				bool IRQEnabled;
				bool IRQPending;
				bool HasReloadedCounter;
				int CounterCycles[256];
				unsigned char CounterCyclePos;

				// Indica que ocorreu um interrupt que teve origem no mapper
				virtual bool NotifyInterrupt();

				// Indica que o endereço do PPU foi alterado
				virtual void NotifyPPUAddressChanged(unsigned short addr);

				// Indica que estamos num novo frame
				virtual void NotifyNewFrame();

				// Indica que o render foi activado/desactivado
				virtual void NotifyToggleRender();

				// Altera os banks fixos
				void SwapFixedBanks();

				// Calcula o ciclo do CPU em que deverá suceder o próximo IRQ
				void CalculateIRQCycle();

				// Actualiza a posição do contador de IRQ
				void UpdateIRQCounter();

				// Save state methods
				// Creates save state info
				virtual void SerializeTo(Common::Serializer & buffer);

				// Loads save state into memory
				virtual void UnserializeFrom(Common::Unserializer & loader);

				// Inicializa o mapper
				Mapper4() :
					Command(0), SwapPRGPos(0), SwapCHRPos(0), PRGBank1(0), PRGBank2(1), CurrentScanlineCounter(0), RequestedScanlineCounter(0), PPUAddr(0), LastCounterTickCycle(-20), InternalIRQCycle(200000), IRQEnabled(false), IRQPending(false), HasReloadedCounter(false), CounterCyclePos(0)
				{
					CHRBanks[0] = 0;
					CHRBanks[1] = 2;
					CHRBanks[2] = 4;
					CHRBanks[3] = 5;
					CHRBanks[4] = 6;
					CHRBanks[5] = 7;

					std::fill_n(CounterCycles, 256, 200000);
				}
		};

		class Mapper7 : public Mapper
		{
			friend class Mapper;

			unsigned char NameTable;
			unsigned int  BankNum;

			Mapper7() : NameTable(0), BankNum(0) {}

			public:

				// Escreve na memória
				virtual void Write(unsigned short addr, unsigned char value);

				// Indica que o PPU foi criado		
				virtual void NotifyPPUCreated();

				// Save state methods
				// Creates save state info
				virtual void SerializeTo(Common::Serializer & buffer);

				// Loads save state into memory
				virtual void UnserializeFrom(Common::Unserializer & loader);				
		};
	}
}

#undef EXTERNAL
#endif