#include "Cartridge.h"
#include "Common/Common.h"
#include <cstring>

Cartridge::Cartridge(const char * fileName)
{
    FILE* myfile = NULL;

    m_fullPath = new char[strlen(fileName) + 1];
    strcpy(m_fullPath, fileName);

    // Find the gamename
    const char* m_fileName = GetFileNameFromFullPath(m_fullPath);

    try
    {
        // Agora que est� tudo limpo, carregar o jogo
        if (fopen_s(&myfile, gameName, "rb") != 0)
        {
            printf("Erro! Imposs�vel ler ficheiro!\n");
            throw 1;
        }

        // Obter tamanho do ficheiro
        fseek(myfile, 0, SEEK_END);
        unsigned int fSize = ftell(myfile);
        rewind(myfile);

        // Ler o header
        int header;

        size_t lSize = fread(&header, sizeof(int), 1, myfile);
        if (lSize != 1)
        {
            printf("Erro a ler o ficheiro!\n");
            throw 2;
        }

        // Se n�o for um header v�lido, morrer
        if (header != 0x1A53454E)
        {
            printf("Erro a ler o ficheiro!\n");
            throw 2;
        }

        // Obter p�ginas
        PRGpages = fgetc(myfile);
        CHRpages = fgetc(myfile);

        // Flags v�rias
        unsigned int flags = fgetc(myfile);

        BatterySRAM = (((flags >> 1) & 1) == 1);
        bool gameHasTrainer = (((flags >> 2) & 1) == 1);

        // Mirroring
        unsigned int mirror = ((flags & 8) >> 2);
        if (!mirror)
            mirror = (flags & 1);

        PPU::MirrorMode mirrorMode;

        switch (mirror)
        {
        case 0: mirrorMode = PPU::HORIZONTAL; break;
        case 1: mirrorMode = PPU::VERTICAL;   break;
        case 2: mirrorMode = PPU::FOURSCREEN; break;
        }

        // Mapper
        gameMapper = flags >> 4;
        flags = fgetc(myfile);

        // Header inv�lido
        if ((flags & 0xF) != 0)
        {
            printf("Erro a ler o ficheiro!\n");
            throw 2;
        }

        gameMapper |= flags;

        // Preparar para obter dados (passar trainer � frente)
        fseek(myfile, 0x10 + ((gameHasTrainer) ? 512 : 0), SEEK_SET);

        // Criar mem�ria para o jogo
        romSize = PRGpages << 14; // 1 p�g = 16384 bytes
        Memory::ROM = new unsigned char[romSize];

        // Ler o jogo!
        lSize = fread(Memory::ROM, 1, romSize, myfile);

        if (lSize != romSize)
        {
            printf("Erro a ler o ficheiro!\n");
            throw 2;
        }

        // Criar mem�ria para os gr�ficos
        if (CHRpages)
        {
            chrSize = CHRpages << 13;
            Memory::CHR = new unsigned char[chrSize];

            // Ler dados dos gr�ficos
            lSize = fread(Memory::CHR, 1, chrSize, myfile);

            if (lSize != chrSize)
            {
                printf("Erro a ler o ficheiro!\n");
                throw 2;
            }
        }

        // Get the game CRC
        CRC = Common::CRC32(Memory::ROM, romSize);

        if (chrSize)
            CRC = Common::CRC32(Memory::CHR, chrSize, CRC);

        // J� deviamos estar no fim do ficheiro, se n�o
        // estivermos � porque este ficheiro n�o � valido
        //if(fSize != ftell(myfile))
        //{
        //	printf("Erro a ler o ficheiro!\n");
        //	throw 2;
        //}

        fclose(myfile);
    }
}

Cartridge* Cartridge::List::Pick(const char* fileName)
{
    for each (auto cartridge in list)
    {
        if (strcmp(fileName, cartridge->fileName) == 0)
            return cartridge;
    }

    Cartridge* cartridge = new Cartridge(fileName);

    if (!cartridge->loaded)
        return nullptr;

    list.push_back(cartridge);

    return cartridge;
}

void Cartridge::List::Clear()
{
    if (running != nullptr)
        printf("Warning: there was a running cartridge when the list was removed. The app will probably crash.\n");

    for each (auto cartridge in list)
    {
        delete cartridge;
    }

    list.clear();
}