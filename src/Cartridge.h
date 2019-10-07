#pragma once
#include <list>
class Cartridge
{
    const char * fileName;
    int CRC32;
    const uint8_t* PRG;
    const uint8_t* CHR;
    bool loaded;
    bool powered;
    const Chip* mapper;
    unsigned short PRGpages;
    unsigned short CHRpages;
    unsigned short romSize;
    unsigned short chrSize;


    // Loads a cartridge from a file
    Cartridge(const char* fileName);

    // Removes a cartridge from memory
    ~Cartridge();

public:
    // Powers the cartridge
    void PowerOn();

    // Powers off the cartridge
    void PowerOff();

    void clock();
    inline bool isLoaded() { return loaded; }
    inline bool isPowered() { return powered; }

    class List
    {
        List();
        static Cartridge* running = nullptr;
        inline ~List() { Clear(); }
        static std::list<Cartridge*> list;
    public:
        static Cartridge* Pick(const char* fileName);
        static void Clear();
    };
};
