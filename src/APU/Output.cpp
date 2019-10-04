// Output.h
// Envia os streams de audio para a placa de som

#define OUTPUT_CPP

#include "Internal.h"
#include "Output.h"

#include <cstring>
#include <cstdio>

// Inicializa o output
void APU::Output::Initialize()
{
	if(HasSound)
		return;

	// Limpar vars
	PosNum        = 0;
	QueuedBuffers = 0;
	OldRes        = 0;
	OldSquare     = 0;
	OldTND        = 0;
	
	// Preparar o dispositivo de som
	AudioDevice = alcOpenDevice(NULL);

	if(AudioDevice == nullptr)
	{
		printf("An error has occured. No sound will be played. :(\n");
		HasSound = false;
		return;
	}

	// Preparar o contexto de som
	AudioContext = alcCreateContext(AudioDevice, NULL);
	alcMakeContextCurrent(AudioContext);

	if(AudioContext == nullptr)
	{
		printf("An error has occured. No sound will be played. :(\n");
		alcCloseDevice(AudioDevice);
		HasSound = false;
		return;
	}

	// Preparar o source e buffers do OpenAL
	alGenBuffers(NumBuffers, AudioBuffers);
	alGenSources(1, &AudioSource);

	if(alGetError() != AL_NO_ERROR)
	{
		printf("An error has occured. No sound will be played. :(\n");

		alcMakeContextCurrent(nullptr);
		alcDestroyContext(AudioContext);
		alcCloseDevice(AudioDevice);

		HasSound = false;
		return;
	}

	// Preparar blip buffer
	Blip = blip_new(8192);
	blip_set_rates(Blip, 1789772.727f * 3, 44100.0f);

	// Limpar o buffer
	Buffer = new short[BufSize];
	memset(Buffer, 0, sizeof(short) * BufSize);

	// Permitir som!
	HasSound = true;

	for(int i = 0; i < NumBuffers; ++ i)
		BufferQueued[i] = false;
}

// Faz queue do buffer no OpenAL
void APU::Output::QueueBuffer()
{
	// Voltar ao início do buffer
	PosNum = 0;

	// Se não houver som não vale a pena estarmos aqui...
	if(!HasSound)
		return;

  ALint result;

	// Ver se há algum buffer livre
	alGetSourcei(AudioSource, AL_BUFFERS_PROCESSED, &result);

	// Se houver, tirar da lista
	if(result > 0)
	{
		while(result)
		{
			ALuint buffer;

			// Adicionar o buffer à lista de buffers utilizáveis
			alSourceUnqueueBuffers(AudioSource, 1, &buffer);

			for(int i = 0; i < NumBuffers; ++ i)
			{
				if (AudioBuffers[i] == buffer)
				{
					BufferQueued[i] = false;
					break;
				}
			}
			--result;
		}
	}

	int mybuf = 0;

	// Procurar um buffer livre
	while(mybuf < NumBuffers)
	{
		if(!BufferQueued[mybuf])
		{
			// Buffer livre encontrado, reservar
			BufferQueued[mybuf] = true;
			break;
		}

		++mybuf;
	}

	// Se estamos aqui não há buffers dísponíveis
	if(mybuf == NumBuffers)
	{
		// Se tivermos posto em pausa, recomeçar audio
		alGetSourcei(AudioSource, AL_SOURCE_STATE, &result);
		if(result != AL_PLAYING)
			alSourcePlay(AudioSource);

		return;
	}

	// Adicionar o buffer
	alBufferData(AudioBuffers[mybuf], AudioFormat, Buffer,  BufSize * sizeof(short), AudioFrequency);
	alSourceQueueBuffers(AudioSource, 1, &AudioBuffers[mybuf]);

	// Forçar play se tiver parado
	alGetSourcei(AudioSource, AL_SOURCE_STATE, &result);
	if(result != AL_PLAYING)
	{
		static int  TotalCuts = 6;
		static int  Fill      = 1;
		static bool ItWasMe   = true;
		static int  NumCuts   = 0;
		static int  Grace     = 10000;

		if(!--Grace)
		{
			Grace = 5000;
			if(NumCuts)
				--NumCuts;
		}

		if(!ItWasMe)
			++NumCuts;

		// Aumentar ao tamanho do buffer para evitar pops
		if((NumCuts == (TotalCuts + 1)) && (BufSize < 2048))
		{
			delete [] Buffer;
			BufSize   <<= 1;
			TotalCuts <<= 1;
			Buffer = new short[BufSize];
			NumCuts = 0;
			Fill = 1;

			ItWasMe = true;

			printf("Buffer increased to %i bytes!\n", BufSize);
		}
			
		// Encher um buffer antes de colocar play (evita cuts)
		if(!Fill)
		{
			alSourcePlay(AudioSource);
			ItWasMe = false;
			Grace   = 5000;
		}

		Fill = 0;
	}
}

int  APU::Output::BufSize  = 512;
bool APU::Output::HasSound = false;