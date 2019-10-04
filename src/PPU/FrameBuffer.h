// FrameBuffer.h
// Classe que trata do FrameBuffer

#ifndef PPUFRAMEBUFFER_H
#define PPUFRAMEBUFFER_H

// GLext.h
#define GL_ARRAY_BUFFER_ARB 0x8892
#define GL_STATIC_DRAW_ARB 0x88E4
typedef void (APIENTRY * PFNGLBINDBUFFERARBPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRY * PFNGLDELETEBUFFERSARBPROC) (GLsizei n, const GLuint *buffers);
typedef void (APIENTRY * PFNGLGENBUFFERSARBPROC) (GLsizei n, GLuint *buffers);
typedef void (APIENTRY * PFNGLBUFFERDATAARBPROC) (GLenum target, int size, const GLvoid *data, GLenum usage);

#ifndef PPUFRAMEBUFFER_CPP
#define EXTERNAL extern
#else
#define EXTERNAL
#endif

// --------------------------------------------------------------------------
// FRAMEBUFFER --------------------------------------------------------------
// --------------------------------------------------------------------------

namespace PPU
{
	namespace FrameBuffer
	{
		EXTERNAL unsigned int   FinalBmp[61440];
		EXTERNAL unsigned short FinalBmpCount;

		// Textura
		EXTERNAL GLuint TextureID;

		// Controlo do tamanho
		EXTERNAL bool         DoResize;
        EXTERNAL sf::Vector2u ResizeSize;

		// Controlo de render
		EXTERNAL bool OwnRenderer;

		// Controlo de FPS
		EXTERNAL unsigned long NumTicks;
		EXTERNAL bool          LimitFPS;
		EXTERNAL unsigned char FrameTimePos;
		extern   double        FrameTime;
		EXTERNAL double        FrameTimeAvg;
		EXTERNAL double        FrameTimes[8];
		EXTERNAL double        ExtraMS;
		EXTERNAL double        TargetTime;
		EXTERNAL double        LastTime;
        EXTERNAL float         Scale;

		// Este código é uma extensão do OpenGL mas não tinha mais para onde ir...
		// GL_BGRA é mais rápido que GL_RGBA
		static const GLenum GL_BGRA = 0x80E1;

		void Draw();
		void Clear();
		void UpdateScreen();
		void FPSLimiter();

        inline void SetScale(const float scale) { Scale = scale; }

		// Displays a string for a specific amount of time (default: 2000 milisseconds, 0 = display forever)
		void DisplayString(const char * string, unsigned int time = 2000);

		// Recalculates the string position
		void RecalculateStringPositions();

	} // END namespace FrameBuffer
} // END namespace PPU

#undef EXTERNAL
#endif