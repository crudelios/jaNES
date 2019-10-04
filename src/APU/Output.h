// Output.h
// Envia os streams de audio para a placa de som

#ifndef OUTPUT_H
#define OUTPUT_H

#include <AL/al.h>
#include <AL/alc.h>
#include <blip_buf.h>

#ifndef OUTPUT_CPP
#define EXTERNAL extern
#else
#define EXTERNAL
#endif

namespace APU
{
	namespace Output
	{
		// Inicializa o processamento do output
		void Initialize();

		// Faz queue do buffer no OpenAL
		void QueueBuffer();

		// Membros constantes
		extern int BufSize;
		const  int NumBuffers     = 6;
		const  int AudioFrequency = 44100;
		const  int AudioFormat    = AL_FORMAT_MONO16;

		// Membros de output
		EXTERNAL short * Buffer;
		EXTERNAL int     PosNum;
		EXTERNAL int     QueuedBuffers;
		extern   bool    HasSound;
		EXTERNAL short   OldRes;
		EXTERNAL int     OldSquare;
		EXTERNAL int     OldTND;

		// Coisas específicas do OpenAL
		EXTERNAL ALCdevice  * AudioDevice;
		EXTERNAL ALCcontext * AudioContext;
		EXTERNAL ALuint       AudioSource;
		EXTERNAL ALuint       AudioBuffers[NumBuffers];
		EXTERNAL bool         BufferQueued[NumBuffers];

		// Blip Buffer
		EXTERNAL blip_t * Blip;

		// Inlines

		// Termina o processamento do output
		inline void Destroy()
		{
			// Destruir blip buffer
			blip_delete(Blip);

			if(HasSound)
			{
				alSourceStop(AudioSource);
				alDeleteSources(1, &AudioSource);
				alDeleteBuffers(NumBuffers, AudioBuffers);
				alcMakeContextCurrent(nullptr);
				alcDestroyContext(AudioContext);
				alcCloseDevice(AudioDevice);

				HasSound = false;

				delete [] Buffer;
			}
		}

		// Pausa o output
		inline void Pause()
		{
			alSourcePause(AudioSource);
		}

		// Obtém a posição do buffer actual
		inline short * GetBufferBlock()
		{
			return &Buffer[PosNum];
		}

		// Indica que foram adicionados 512 bytes ao buffer
		inline void AddBufferBlock()
		{
			PosNum += 512;

			// Se atingimos um buffer, podemos adicioná-lo aos buffers do OpenAL
			if(PosNum == BufSize)
				QueueBuffer();
		}

		// Adiciona um sample do APU
		inline void AddSample(int cycle, int square, int tnd)
		{
			// Apenas enviar pedido de onda sonora se a mesma tiver sido alterada
			if((OldTND != tnd) || (OldSquare != square))
			{
				short result = (short) (((SquareTable[square] + TndTable[tnd]) - 0.512990f) * 60000);
				blip_add_delta(Blip, cycle, result - OldRes);

				OldRes    = result;
				OldSquare = square;
				OldTND    = tnd;
			}
		}

		// Adiciona um sample para ser reproduzido
		inline void AddSample(int cycle)
		{
			AddSample(cycle, SquareChannel::Data[0].DAC + SquareChannel::Data[1].DAC, TriangleChannel::DAC + NoiseChannel::DAC + DMCChannel::Counter);
		}

		// Adiciona os dados de APU deste frame
		inline void EndFrame(int cycles)
		{
			// Actualizar buffer
			blip_end_frame(Blip, cycles);

			while(blip_samples_avail(Blip) >= 512)
			{
				blip_read_samples(Blip, &Buffer[PosNum], 512, false);
				AddBufferBlock();
			}
		}

		// Indica se há som
		inline bool IsActive()
		{
			return HasSound;
		}

	} // END namespace OUTPUT

} // END namespace APU

#undef EXTERNAL
#endif