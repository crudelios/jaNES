// Registers.h
// Classe dos registers

#ifndef REGISTERS_H
#define REGISTERS_H

#ifndef CPU_CPP
#define EXTERNAL extern
#else
#define EXTERNAL
#endif

namespace CPU
{
	namespace Registers
	{
		EXTERNAL int A;             // Accumulator
		EXTERNAL int X;             // X Reg
		EXTERNAL int Y;             // Y Reg
		EXTERNAL unsigned short PC; // Program counter
		EXTERNAL unsigned char SP;  // Stack pointer

		namespace P
		{
			EXTERNAL unsigned char N; // Negative flag
			EXTERNAL unsigned char V; // Overflow flag
														  	// Unused flag
		//	EXTERNAL unsigned char B; // Break flag
			EXTERNAL unsigned char D; // Decimal flag (not used)
			EXTERNAL unsigned char I; // Interrupt flag
			EXTERNAL unsigned char Z; // Zero flag
			EXTERNAL unsigned char C; // Carry flag

			inline unsigned char Pack()
			{
				return (N << 7) | (V << 6) | (1 << 5) | (0 << 4) | (D << 3) | (I << 2) | (Z << 1) | C;
			}

			inline void Unpack(unsigned char byte)
			{
				N = byte >> 7;
				V = (byte >> 6) & 1;
				//B = (byte >> 4) & 1;
				D = (byte >> 3) & 1;
				I = (byte >> 2) & 1;
				Z = (byte >> 1) & 1;
				C = byte & 1;
			}
		} // END namespace P
	} // END namespace Registers
} // END namespace CPU

#undef EXTERNAL
#endif