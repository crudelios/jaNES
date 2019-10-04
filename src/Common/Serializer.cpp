// Serializer.cpp
// Serializer and deserializer classes

#include "Common.h"

extern "C"
{
	#include <lz4.h>
}

// Copies data
void Common::Serializer::Copy(const void * data, unsigned int size)
{
	size *= sizeof(uint8_t);

	int64_t filled = 0;

	// Set up a new buffer if this one is full
	while(((Cursor - Current) + size) > BufSize)
	{
		memcpy(Cursor, (uint8_t *) data + filled, BufSize - (Cursor - Current));

		filled += BufSize - (Cursor - Current);

		Assign();
	}

	memcpy(Cursor, (uint8_t *) data + filled, size - filled);
	Cursor += size - filled;
}

// Saves the buffers to a file
void Common::Serializer::GenerateFile(const char * fileName)
{
	FILE * stateFile = nullptr;
	char * compressedBuffer = nullptr;

	try
	{
		if(!fileName)
			throw 1;

		// Save the file
		stateFile = fopen(fileName, "wb");

		if(!stateFile)
			throw 1;

		// Write the header directly to the file
		fwrite("SST", sizeof(uint8_t), 3, stateFile);

		// Grab uncompressed data size and write it to file
		uint32_t bufSize = (uint32_t) (((List.size() - 1) * BufSize) + CurrentSize());

        fwrite(&bufSize, sizeof(uint32_t), 1, stateFile);

		unsigned int worstCaseSize = LZ4_compressBound(BufSize);

		compressedBuffer = new char[worstCaseSize];

		// Compress the buffers and copy them to the file
		for(std::list<uint8_t *>::const_iterator iter = List.begin(); iter != List.end(); ++ iter)
		{
			size_t fSize = (*iter == Current) ? CurrentSize() : BufSize;

			int writeSize = LZ4_compress((const char *) *iter, compressedBuffer, (int) fSize);

			if(fwrite(compressedBuffer, sizeof(char), writeSize, stateFile) != writeSize)
				throw 2;
		}

		delete [] compressedBuffer;

		// Close the file and buffers
		fclose(stateFile);
	}

	catch(...)
	{
		if(stateFile)
			fclose(stateFile);

		delete [] compressedBuffer;

		throw;
	}
}

// Loads a file	
void Common::Unserializer::LoadFromFile(const char * fileName)
{
	// Delete old data
	Clear();

	FILE * stateFile = nullptr;
	char * compressedBuffer = nullptr;

	try
	{
		if(!fileName)
			throw 1;

		// Open the file
		if(fopen_s(&stateFile, fileName, "rb") != 0)
			throw 1;

		// Get the file size
		fseek(stateFile, 0 , SEEK_END);
		unsigned int fSize = ftell(stateFile) - sizeof(uint32_t) - 3;
		rewind(stateFile);

		// Check the header and size
		char header[3];

		fread(header, sizeof(char), 3, stateFile);

		// Wrong header!
		if(memcmp("SST", header, 3))
			throw 2;

		// Get the file size
		fread(&BufSize, sizeof(uint32_t), 1, stateFile);

		// Read the file to a buffer
		compressedBuffer = new char[fSize];
		fread(compressedBuffer, sizeof(unsigned char), fSize, stateFile);

		// Decompress the buffer
		Buffer = new unsigned char[BufSize];
		Cursor = Buffer;

		int check = LZ4_uncompress(compressedBuffer, (char *) Buffer, BufSize);

		if(check < 0)
			throw 2;

		fclose(stateFile);
	}

	catch(...)
	{
		delete [] compressedBuffer;
		Clear();

		if(stateFile)
			fclose(stateFile);

		throw;
	}
}