// FrameBufferString.h
// Classe que trata de mostrar Strings

#ifndef PPUFRAMEBUFFERSTRING_H
#define PPUFRAMEBUFFERSTRING_H

// FONT RENDERING
// The following code displays information string on the Rendering Window.
// Who knew drawing text to OpenGL would be so cumbersome! I certainly didn't!

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#include <list>

namespace PPU
{
	namespace FrameBuffer
	{
		namespace String
		{
			// Estrutura que guarda os dados de string a apresentar
			struct Info
			{
				char * string;
				size_t stringLength;
				stbtt_aligned_quad * displayData;
				double totalTime;
				int firstLine;
				unsigned int numLines;
				int alpha;
				Info(const char * string, unsigned int time);
				Info(Info && former);
				void CreateDisplayData();
				~Info();
			};

			// Draws the strings to the output window
			void Draw();

			// Updates screen positions when screen is resized
			void UpdatePositions();

			// Prepares STB
			void PrepareSTB();

			// Compares line numbers
			bool CompareLineNumber(const Info & left, const Info & right);

			// Frees memory and tidies things up
			void Destroy();

			static bool STBPrepared = false;          // Indicates whether STB is ready for usage
			static GLuint STBFontTexture = 0;         // Font texture ID for OpenGL
			static stbtt_bakedchar STBGlyphData[144]; // The glyph data
			static std::list<Info> InfoList;          // Stores the current active strings
			static unsigned int numLines = 0;         // Total number of lines being displayed
			static unsigned int lastLine;             // Last line in use

		} // END namespace String

	} // END namespace FrameBuffer

} // END namespace PPU

#endif