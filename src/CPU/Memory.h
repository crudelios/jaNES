// Memory.h
// Memória

#ifndef MEMORY_H
#define MEMORY_H

#include "Internal.h"
#include "Registers.h"
#include "../Mappers/Mappers.h"
#include "../APU/APU.h"

#ifndef MEMORY_CPP
#define EXTERNAL extern
#else
#define EXTERNAL
#endif

namespace CPU
{
	// --------------------------------------------------------------------------
	// FUNÇÕES DE MEMÓRIA -------------------------------------------------------
	// --------------------------------------------------------------------------

	namespace Memory
	{
		// Variáveis de memória
		EXTERNAL unsigned char ZP[0x100];         // Página zero
		EXTERNAL unsigned char RAM[0x600];        // RAM
		EXTERNAL unsigned char * ROM;             // Memória do cartucho
		EXTERNAL unsigned char * CHR;	            // Gráficos do cartucho
		EXTERNAL unsigned char blankPage[0x100];  // Página de memória vazia
		EXTERNAL unsigned char SRAM[0x2000];      // Memória de gravação
		EXTERNAL unsigned char * mapPages[0x100]; // Mapeador de memória
		EXTERNAL unsigned char openBusValue;      // Valor do open bus
		EXTERNAL Mapper * mapper;                 // Classe que trata do mapeamento


		// ------------------------------------------------------------------------
		// FUNÇÕES DE ACESSO À MEMÓRIA --------------------------------------------
		// ------------------------------------------------------------------------

		// -------
		// Leitura
		// -------

		// Ler num endereçamento de registo
		unsigned char RegisterRead(unsigned short addr);

		// Lê um endereço de registo em modo debug
		unsigned char RegisterDebugRead(unsigned short addr);

		// Lê do Open Bus
		unsigned char ReadOpenBus();

		// Ler memória a partir de um endereço 16 bit
		inline unsigned char Read(unsigned short addr, bool doDelay = true)
		{
			// Se for lido o DMC ao mesmo tempo que a memória do CPU, parar CPU
			if(doDelay && (dmcDelayCycle <= cycles))
			{
				bool sameCycle = (dmcDelayCycle == cycles);
				bool delay = APU::LoadDMCSample(12 - (cycles - dmcDelayCycle));

				if(sameCycle && delay)
					dmcDelayReg = true;
			}

			if((addr < 0x2000) || (addr > 0x7FFF))
			{
				cycles += 3;
				dmcDelayReg = false;
				openBusValue = mapPages[addr >> 8][addr & 0xFF];
				return openBusValue;
			}

			// SRAM Read
			if(addr > 0x5FFF)
			{
				cycles += 3;

				if(!SRAMEnabled)
					return ReadOpenBus();

				openBusValue = mapPages[addr >> 8][addr & 0xFF];
				return openBusValue;
			}

			openBusValue = RegisterRead(addr);
			dmcDelayReg = false;
			cycles += 3;
			return openBusValue;
		}

		// Lê memória em modo debug
		inline unsigned char DebugRead(unsigned short addr)
		{
			if((addr < 0x2000) || (addr > 0x5FFF))
				return mapPages[addr >> 8][addr & 0xFF];

			return RegisterDebugRead(addr);
		}

		// Ler memória a partir de 2 endereços 8 bits
		inline unsigned char Read(unsigned char high, unsigned char low, bool doDelay = true)
		{
			return Read(high << 8 | low, doDelay);
		}


		// -------
		// Escrita
		// -------

		// Escrever num endereçamento de registo
		void RegisterWrite(unsigned short addr, unsigned char value);

		// Escrever na memória
		inline void Write(unsigned short addr, unsigned char value)
		{
			openBusValue = value;

			// RAM write
			if(addr < 0x2000)
			{
				mapPages[addr >> 8][addr & 0xFF] = value;
				cycles += 3;
				return;
			}

			// Mapper write
			if(addr > 0x7FFF)
			{
				mapper->Write(addr, value);
				cycles += 3;
				return;
			}

			// SRAM Write
			if(addr > 0x5FFF)
			{
				cycles += 3;

				if(!SRAMWrite || !SRAMEnabled)
					return;

				mapPages[addr >> 8][addr & 0xFF] = value;
				return;
			}

			// Register write
			RegisterWrite(addr, value);
			cycles += 3;
		}

		// ---------
		// Apontador
		// ---------

		// Obter o apontador de memória a partir de 2 endereços 8 bits
		inline unsigned char * GetAbsoluteAddress(unsigned char high, unsigned char low)
		{
			return &(mapPages[high][low]);
		}

		// Obter o apontador de memória a partir de um endereço 16 bit
		inline unsigned char * GetAbsoluteAddress(unsigned short addr)
		{
			return GetAbsoluteAddress(addr >> 8, addr & 0xFF);
		}

		// -------------
		// Endereçamento
		// -------------

		// Accumulator
		inline unsigned char * Accumulator()
		{
			if(dmcDelayCycle < cycles)
				APU::LoadDMCSample();

			return (unsigned char *) &Registers::A;
		}

		// Immediate
		inline unsigned char * Immediate()
		{
			if(dmcDelayCycle < cycles)
				APU::LoadDMCSample();

			unsigned char * ret = GetAbsoluteAddress(Registers::PC + 1);
			openBusValue = *ret;
			return ret;
		}

		// Zero Page
		inline unsigned char * ZeroPage()
		{
			if(dmcDelayCycle < cycles)
				APU::LoadDMCSample();

			openBusValue = ZP[opcodeData];
			return &ZP[opcodeData];
		}

		// Zero Page, X
		inline unsigned char * ZeroPageX()
		{
			if(dmcDelayCycle < cycles)
				APU::LoadDMCSample();

			unsigned char * ret = &ZP[(opcodeData + Registers::X) & 0xFF];
			openBusValue = *ret;
			return ret;
		}

		// Zero Page, Y
		inline unsigned char * ZeroPageY()
		{
			if(dmcDelayCycle < cycles)
				APU::LoadDMCSample();

			unsigned char * ret = &ZP[(opcodeData + Registers::Y) & 0xFF];
			openBusValue = *ret;
			return ret;
		}


		// ------------------------------------------------
		// Casos especiais para os nossos amigos os mappers
		// ------------------------------------------------

		// Absolute - Endereço apenas
		inline unsigned short Absolute()
		{
			return (Read(Registers::PC + 2) << 8 | opcodeData);
		}

		// Absolute, X - Endereço apenas
		inline unsigned short AbsoluteX(bool writing = false)
		{
			register int low = Registers::X + opcodeData;
			register int high = Read(Registers::PC + 2) << 8;

			// Dummy read
			if(writing || (low >> 8))
				Read(high | (low & 0xFF));

			return (high + low);
		}

		// Absolute, Y - Endereço apenas
		inline unsigned short AbsoluteY(bool writing = false)
		{
			register int low = Registers::Y + opcodeData;
			register int high = Read(Registers::PC + 2) << 8;

			// Dummy read
			if(writing || (low >> 8))
				Read(high | (low & 0xFF));

			return (high + low);
		}

		// (Indirect, X) - Endereço apenas
		inline unsigned short IndirectX()
		{
			unsigned char addr = opcodeData + Registers::X;
			cycles += 9;

			if(dmcDelayCycle < cycles)
				APU::LoadDMCSample();

			return ((ZP[(addr + 1) & 0xFF] << 8) | ZP[addr]);
		}

		// (Indirect), Y - Endereço apenas
		inline unsigned short IndirectY(bool writing = false)
		{
			register int low = Registers::Y + ZP[opcodeData];
			register int high = ZP[(opcodeData + 1) & 0xFF] << 8;

			cycles += 6;

			if(dmcDelayCycle < cycles)
				APU::LoadDMCSample();

			// Dummy read
			if(writing || (low >> 8))
				Read(high | (low & 0xFF));

			return (high + low) & 0xFFFF;
		}

		// ----------
		// Mapeamento
		// ----------

		// Prepara o mapper para uso
		void Populate();

	} // END namespace Memory


	// ------------------------------------------------------------------------
	// FUNÇÕES DE STACK -------------------------------------------------------
	// ------------------------------------------------------------------------

	namespace Stack
	{
		EXTERNAL unsigned char memory[0x100];  // Stack

		// Coloca um valor no stack
		inline void Push(unsigned char data)
		{
			memory[Registers::SP--] = data;
		}

		// Retira um valor do stack
		inline unsigned char Pull()
		{
			return memory[++Registers::SP];
		}
	} // END namespace Stack
} // END namespace CPU

#undef EXTERNAL
#endif