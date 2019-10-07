// Common.h
// Coisas úteis

#ifndef COMMON_H
#define COMMON_H

#include <list>
#include <cstdint>

namespace Common
{
	// Gets time with microsecond precison
	double GetUTime();

	// Calculates the CRC32 of a string
	unsigned int CRC32(const unsigned char * data, unsigned int size, unsigned int CRC = 0);

    // Gets a file name from a full path
    // Memory deletion is done by the user
    const char* GetFileNameFromFullPath(const char* fullPath);

	// Serialization manager
	class Serializer
	{
		// Lists the buffers
		static const unsigned int BufSize = 524288;
		std::list<uint8_t *> List;
		uint8_t * Current;
		uint8_t * Cursor;

		inline void Assign()
		{
			Current = new uint8_t[BufSize];
			Cursor = Current;
			List.push_back(Current);
		}

		// No default constructors
		inline Serializer(const Serializer &) {}
		inline Serializer(Serializer &&) {}

	public:

		// Prepares the buffer for first usage
		inline Serializer()
		{
			Assign();
		}

		// Deletes all buffers
		inline void Clear()
		{
			for(std::list<uint8_t *>::iterator iter = List.begin(); iter != List.end(); ++ iter)
				delete [] *iter;

			List.clear();

			Current = nullptr;
			Cursor  = nullptr;
		}

		// Gets current buffer size
		inline size_t CurrentSize()
		{
			return (Cursor - Current);
		}

		// Copies bytes to the buffer
		void Copy(const void * data, unsigned int size);

		// Bytecopy method
		template <class T>
		inline void Copy(T data)
		{
			Copy(&data, sizeof(T));
		}

		// Generates a file from the buffer
		void GenerateFile(const char * fileName);

		// Destroys the buffers
		inline ~Serializer()
		{
			Clear();
		}
	};

	// Load manager
	class Unserializer
	{
		// The different var suzes in the save state
		unsigned int CharSize;
		unsigned int IntSize;
		unsigned int ShortSize;
		unsigned int LongSize;
		unsigned int BoolSize;

		// Buffer information
		unsigned char * Buffer;
		unsigned char * Cursor;
		uint32_t BufSize;

		// No default constructors
		inline Unserializer(const Unserializer &) {}
		inline Unserializer(Unserializer &&) {}

	public:

		// Basic constructor
		inline Unserializer() :
			CharSize(0),
			IntSize(0),
			ShortSize(0),
			LongSize(0),
			BoolSize(0),
			Buffer(nullptr),
			Cursor(nullptr),
			BufSize(0)
		{
		}

		// Constructor. Gets data from file to buffer
		inline Unserializer(const char * fileName) :
			CharSize(0),
			IntSize(0),
			ShortSize(0),
			LongSize(0),
			BoolSize(0),
			Buffer(nullptr),
			Cursor(nullptr),
			BufSize(0)
		{
			LoadFromFile(fileName);
		}

		// Loads data from a file
		void LoadFromFile(const char * fileName);

		// Sets data to pointer
		inline void Set(void * data, unsigned int count)
		{
			memcpy(data, Cursor, count);
			Cursor += count;
		}

		// Disable a pesky and unavoidable warning...
		#pragma warning(push)
		#pragma warning(disable:4800)

		// Sets data to variables
		template<class T>
		void Set(T & data)
		{
			uint64_t result;

			switch(GetSize(data))
			{
				case 1:
				{
					uint8_t temp;
					memcpy(&temp, Cursor, 1);
					result = temp;
					break;
				}
				case 2:
				{
					uint16_t temp;
					memcpy(&temp, Cursor, 2);
					result = temp;
					break;
				}
				case 4:
				{
					uint32_t temp;
					memcpy(&temp, Cursor, 4);
					result = temp;
					break;
				}
				case 8:
				{
					uint64_t temp;
					memcpy(&temp, Cursor, 8);
					result = temp;
					break;
				}
			}

			data = (T) result;

			Cursor += GetSize(data);
		}
		#pragma warning(pop)

		// Gets the size of the various data types
		inline unsigned int GetSize(bool)           { return BoolSize;  }
		inline unsigned int GetSize(unsigned char)  { return CharSize;  }
		inline unsigned int GetSize(char)           { return CharSize;  }
		inline unsigned int GetSize(unsigned short) { return ShortSize; }
		inline unsigned int GetSize(short)          { return ShortSize; }
		inline unsigned int GetSize(unsigned long)  { return LongSize;  }
		inline unsigned int GetSize(long)           { return LongSize;  }
		inline unsigned int GetSize(unsigned int)   { return IntSize;   }
		inline unsigned int GetSize(int)            { return IntSize;   }

		// Compares the data
		inline int Compare(const void * data, unsigned int size)
		{
			int ret = memcmp(Cursor, data, size);
			Cursor += size;
			return ret;
		}

		// Sets the sizes
		inline void SetSizes()
		{
			// For now, skip version information
			++Cursor;

			// Get the various sizes
			CharSize  = *Cursor;
			IntSize   = *(Cursor + 1);
			ShortSize = *(Cursor + 2);
			LongSize  = *(Cursor + 3);
			BoolSize  = *(Cursor + 4);

			Cursor += 5;
		}

		// Deletes the buffers
		inline void Clear()
		{
			delete [] Buffer;
			Buffer    = nullptr;
			Cursor    = nullptr;
			CharSize  = 0;
			IntSize   = 0;
			ShortSize = 0;
			LongSize  = 0;
			BoolSize  = 0;
			BufSize   = 0;
		}

		// Destructor
		inline ~Unserializer()
		{
			Clear();
		}
	};

} // END namespace Common

#endif



// Converte as cores de acordo com o enfase
//	int values[] = {
//0xFF, 0x75, 0x75, 0x75,
//0xFF, 0x8F, 0x1B, 0x27,
//0xFF, 0xAB, 0x00, 0x00,
//0xFF, 0x9F, 0x00, 0x47,
//0xFF, 0x77, 0x00, 0x8F,
//0xFF, 0x13, 0x00, 0xAB,
//0xFF, 0x00, 0x00, 0xA7,
//0xFF, 0x00, 0x0B, 0x7F,
//0xFF, 0x00, 0x2F, 0x43,
//0xFF, 0x00, 0x47, 0x00,
//0xFF, 0x00, 0x51, 0x00,
//0xFF, 0x17, 0x3F, 0x00,
//0xFF, 0x5F, 0x3F, 0x1B,
//0xFF, 0x00, 0x00, 0x00,
//0xFF, 0x00, 0x00, 0x00,
//0xFF, 0x00, 0x00, 0x00,
//0xFF, 0xBC, 0xBC, 0xBC,
//0xFF, 0xEF, 0x73, 0x00,
//0xFF, 0xEF, 0x3B, 0x23,
//0xFF, 0xF3, 0x00, 0x83,
//0xFF, 0xBF, 0x00, 0xBF,
//0xFF, 0x5B, 0x00, 0xE7,
//0xFF, 0x00, 0x2B, 0xDB,
//0xFF, 0x0F, 0x4F, 0xCB,
//0xFF, 0x00, 0x73, 0x8B,
//0xFF, 0x00, 0x97, 0x00,
//0xFF, 0x00, 0xAB, 0x00,
//0xFF, 0x3B, 0x93, 0x00,
//0xFF, 0x8B, 0x83, 0x00,
//0xFF, 0x00, 0x00, 0x00,
//0xFF, 0x00, 0x00, 0x00,
//0xFF, 0x00, 0x00, 0x00,
//0xFF, 0xFF, 0xFF, 0xFF,
//0xFF, 0xFF, 0xBF, 0x3F,
//0xFF, 0xFF, 0x97, 0x5F,
//0xFF, 0xFD, 0x8B, 0xA7,
//0xFF, 0xFF, 0x7B, 0xF7,
//0xFF, 0xB7, 0x77, 0xFF,
//0xFF, 0x63, 0x77, 0xFF,
//0xFF, 0x3B, 0x9B, 0xFF,
//0xFF, 0x3F, 0xBF, 0xF3,
//0xFF, 0x13, 0xD3, 0x83,
//0xFF, 0x4B, 0xDF, 0x4F,
//0xFF, 0x98, 0xF8, 0x58,
//0xFF, 0xDB, 0xEB, 0x00,
//0xFF, 0x00, 0x00, 0x00,
//0xFF, 0x00, 0x00, 0x00,
//0xFF, 0x00, 0x00, 0x00,
//0xFF, 0xFF, 0xFF, 0xFF,
//0xFF, 0xFF, 0xE7, 0xAB,
//0xFF, 0xFF, 0xD7, 0xC7,
//0xFF, 0xFF, 0xCB, 0xD7,
//0xFF, 0xFF, 0xC7, 0xFF,
//0xFF, 0xDB, 0xC7, 0xFF,
//0xFF, 0xB3, 0xBF, 0xFF,
//0xFF, 0xAB, 0xDB, 0xFF,
//0xFF, 0xA3, 0xE7, 0xFF,
//0xFF, 0xA3, 0xFF, 0xE3,
//0xFF, 0xBF, 0xF3, 0xAB,
//0xFF, 0xCF, 0xFF, 0xB3,
//0xFF, 0xF3, 0xFF, 0x9F,
//0xFF, 0x00, 0x00, 0x00,
//0xFF, 0x00, 0x00, 0x00,
//0xFF, 0x00, 0x00, 0x00
//};
//
//int calcvalues[256];
//
//for(int i = 0; i < 256; i += 4)
//{
//	calcvalues[i] = values[i];
//	calcvalues[i + 1] = values[i + 1] * 0.70;
//	calcvalues[i + 2] = values[i + 2] * 0.70;
//	calcvalues[i + 3] = values[i + 3] * 0.70;
//}
//
//for(int i = 0; i < 256; i += 4)
//{
//	if(!(i & 0x1F))
//		printf("\n");
//	printf("0x%02X%02X%02X%02X, ", calcvalues[i], calcvalues[i + 1], calcvalues[i + 2], calcvalues[i + 3]);
//}
//	
//const float emphasis_factor[8][3]={ 
//    {1.00, 1.00, 1.00}, 
//    {1.00, 0.80, 0.81}, 
//    {0.78, 0.94, 0.66}, 
//    {0.79, 0.77, 0.63}, 
//    {0.82, 0.83, 1.12}, 
//    {0.81, 0.71, 0.87}, 
//    {0.68, 0.79, 0.79}, 
//    {0.70, 0.70, 0.70} 
// };

