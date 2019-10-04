// FrameBuffer.cpp
// Classe de framebuffer

#define PPUFRAMEBUFFER_CPP

#include "../CPU/Internal.h"
#include "PPU.h"
#include "Internal.h"
#include "Registers.h"
#include "FrameBuffer.h"
#include "FrameBufferString.h"
#include "../Common/Common.h"

// Desenha o frame
void PPU::FrameBuffer::Draw()
{
	// Actualizar dados para render
	PPU::Update();

	// Activar o contexto de render se estiver desactivado
	if(!OwnRenderer)
	{
		OwnRenderer = true;
		RenderWindow->setActive();
	}

	// Redimensionar a janela, se necessário
	if(DoResize)
	{
		DoResize = false;
		bool widthChanged = (ResizeSize.x != RenderWindow->getSize().x);

		RenderWindow->setSize(ResizeSize);
		glViewport(0, 0, RenderWindow->getSize().x, RenderWindow->getSize().y);

		// If resizing width, redraw strings
		if(widthChanged)
			FrameBuffer::RecalculateStringPositions();
	}

	// Limitar os FPS
	if(LimitFPS)
		FPSLimiter();

	// Actualizar o ecrã
	UpdateScreen();

	// Reset aos counters
	LastScanLine  = 0;
	LastPixel     = 0;
	FinalBmpCount = 0;
	FrameDone     = false;
	CalculatedFrameSync = false;

	memset(PixelOpacity, 0, 61440 * sizeof(unsigned char));
}

// Actualiza o ecrã com a imagem do novo frame (OpenGL output)
void PPU::FrameBuffer::UpdateScreen()
{
	// Actualizar textura
	glBindTexture(GL_TEXTURE_2D, TextureID);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 240, GL_BGRA, GL_UNSIGNED_BYTE, (unsigned char *) FinalBmp);

	// Desenhar o primitivo com a textura
	glBegin(GL_QUADS);
		glColor4ub(255, 255, 255, 255);
		glTexCoord2f(0, 0);      glVertex2f(0,   0);
    glTexCoord2f(0, 0.9375); glVertex2f(0,   240);
    glTexCoord2f(1, 0.9375); glVertex2f(256, 240);
    glTexCoord2f(1, 0);      glVertex2f(256, 0);
	glEnd();

	// Mostrar strings
	if(String::STBPrepared)
		String::Draw();

	// Actualizar ecrã
	RenderWindow->display();

	// Incrementar o contador de fps
	++NumTicks;
}

// Código que limita os FPS
void PPU::FrameBuffer::FPSLimiter()
{
	double TotalTimes = FrameTimeAvg * 8;

	TotalTimes -= FrameTimes[FrameTimePos & 0x7];
	FrameTimes[FrameTimePos & 0x7] = (Common::GetUTime() - LastTime) / 10000;
	TotalTimes += FrameTimes[FrameTimePos & 0x7];
	FrameTimeAvg = TotalTimes / 8;

	//long time = (long) floor(FrameTime - FrameTimeAvg);

	//// Se o frame tiver sido rápido, dormir
	//if(time > 0)
	//	Sleep((DWORD) time);

	// Este código permite limitar fps com elevada precisão
	double remTime = FrameTime - ((FrameTimeAvg < FrameTime) ? 0 : (FrameTimeAvg - FrameTime));
	
	timeBeginPeriod(1);

	while(((Common::GetUTime() - LastTime) / 10000) < (remTime - 5.0f))
		Sleep(1);
	
	while(((Common::GetUTime() - LastTime) / 10000) < remTime);

	timeEndPeriod(1);

	// Colocar dados de timing deste frame
	TotalTimes -= FrameTimes[FrameTimePos & 0x7];
	FrameTimes[FrameTimePos & 0x7] = (Common::GetUTime() - LastTime) / 10000;
	TotalTimes += FrameTimes[FrameTimePos & 0x7];
	FrameTimeAvg = TotalTimes / 8;

	LastTime = Common::GetUTime();
	++FrameTimePos;
}

// Clears the framebuffer
void PPU::FrameBuffer::Clear()
{
	String::Destroy();
}

// Draws a string on the output window
void PPU::FrameBuffer::DisplayString(const char * string, unsigned int time)
{
	// Start string manager
	if(!String::STBPrepared)
		String::PrepareSTB();

	// Add string to list
	String::InfoList.push_back(String::Info(string, time));
}

// Prepares the STB for usage
void PPU::FrameBuffer::String::PrepareSTB()
{
	// Clean free lines
	lastLine = 0;

	// Step one: open font file
	FILE * fontFile = fopen("C:\\Windows\\Fonts\\tahomabd.ttf", "rb");
	
	if(!fontFile)
	{
		printf("Warning: could not open font file!\n");
		return;
	}

	// Step two: get file size
	fseek(fontFile, 0L, SEEK_END);
	unsigned int fileSize = ftell(fontFile);
	rewind(fontFile);

	// Step three: set up the buffer and load the file
	unsigned char * fileData = new unsigned char[fileSize];
	fread(fileData, 1, fileSize, fontFile);

	// Step four: allocate bitmap and create it
	unsigned char * fontBitmap = new unsigned char[262144];
	if(stbtt_BakeFontBitmap(fileData, 0, 18.0, fontBitmap, 512, 512, 32, 144, STBGlyphData) < 1)
	{
		printf("Warning: error creating font bitmap!\n");
		return;
	}

	// Step five: create the texture
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &STBFontTexture);
	glBindTexture(GL_TEXTURE_2D, STBFontTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512, 512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, fontBitmap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Step six: delete the buffers
	delete [] fontBitmap;
	delete [] fileData;
	fclose(fontFile);

	printf("Font configuration loaded.\n");

	STBPrepared = true;
}

// Compares line numbers
bool PPU::FrameBuffer::String::CompareLineNumber(const Info & left, const Info & right)
{
	return (left.firstLine < right.firstLine);
}

void PPU::FrameBuffer::RecalculateStringPositions()
{
	String::UpdatePositions();
}

// Updates the string positions
void PPU::FrameBuffer::String::UpdatePositions()
{
	// No need to be here if there are no strings being displayed
	if(InfoList.empty())
		return;

	// Step one: set all lines to unused
	lastLine = 0;

	// Step two: sort the list by first line (so strings don't change their display order)
	InfoList.sort(CompareLineNumber);

	// Step three: recalculate the whole thing
	for(std::list<Info>::iterator infoIter = InfoList.begin(), end  = InfoList.end(); infoIter != end; ++infoIter)
	{
		infoIter->CreateDisplayData();
	}
}

// Draws the strings to the framebuffer
void PPU::FrameBuffer::String::Draw()
{
	// This keeps the string size the same regardless of the window size
	float xDiv = ((float) RenderWindow->getSize().x / 256) / Scale;
	float yDiv = ((float) RenderWindow->getSize().y / 240) / Scale;

	// Prepare the texture and blending
	glBindTexture(GL_TEXTURE_2D, STBFontTexture);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_QUADS);

	// Go through the whole string list
	std::list<Info>::iterator infoIter = InfoList.begin();

	while(infoIter != InfoList.end())
	{
		// If the string is no longer being shown, delete it
		if(!infoIter->alpha)
		{
			infoIter = InfoList.erase(infoIter);
			continue;
		}

		stbtt_aligned_quad * myQuad = infoIter->displayData;

		// Get current time
		double currTime = Common::GetUTime() / 10000;

		// Draw the string (with a black shadow for better contrast)
		for(unsigned int i = 0; i < infoIter->stringLength; ++ i)
		{
			// Black shadow
			glColor4ub(0, 0, 0, infoIter->alpha);

			glTexCoord2f(myQuad->s0,myQuad->t0); glVertex2f((myQuad->x0 + 1) / xDiv, (myQuad->y0 + 1) / yDiv);
			glTexCoord2f(myQuad->s1,myQuad->t0); glVertex2f((myQuad->x1 + 1) / xDiv, (myQuad->y0 + 1) / yDiv);
			glTexCoord2f(myQuad->s1,myQuad->t1); glVertex2f((myQuad->x1 + 1) / xDiv, (myQuad->y1 + 1) / yDiv);
			glTexCoord2f(myQuad->s0,myQuad->t1); glVertex2f((myQuad->x0 + 1) / xDiv, (myQuad->y1 + 1) / yDiv);

			// String
			glColor4ub(0, 255, 128, infoIter->alpha);

			glTexCoord2f(myQuad->s0,myQuad->t0); glVertex2f(myQuad->x0 / xDiv, myQuad->y0 / yDiv);
			glTexCoord2f(myQuad->s1,myQuad->t0); glVertex2f(myQuad->x1 / xDiv, myQuad->y0 / yDiv);
			glTexCoord2f(myQuad->s1,myQuad->t1); glVertex2f(myQuad->x1 / xDiv, myQuad->y1 / yDiv);
			glTexCoord2f(myQuad->s0,myQuad->t1); glVertex2f(myQuad->x0 / xDiv, myQuad->y1 / yDiv);

			++myQuad;
		}

		// If the string time has passed, reduce the alpha
		if(infoIter->totalTime && (infoIter->totalTime < currTime))
		{
			infoIter->alpha = 255 - (int) (currTime - infoIter->totalTime);
			if(infoIter->alpha < 0)
				infoIter->alpha = 0;
		}

		++infoIter;
	}

  glEnd();
}

// Destroys the string list
void PPU::FrameBuffer::String::Destroy()
{
	glDeleteTextures(1, &STBFontTexture);
	STBFontTexture = 0;
	memset(STBGlyphData, 0, sizeof(stbtt_bakedchar) * 144);
	numLines = 0;
	lastLine = 0;
	STBPrepared = false;
    InfoList.clear();
}

// Constructor
PPU::FrameBuffer::String::Info::Info(const char * string, unsigned int time) :
	string(nullptr),
	stringLength(0),
	displayData(nullptr),
	totalTime(0.0f),
	firstLine(0),
	numLines(0),
	alpha(255)
{
	stringLength = strlen(string);
	this->string = new char[stringLength + 1];
	strcpy(this->string, string);
	totalTime = (time) ? ((Common::GetUTime() / 10000) + time) : 0;

	CreateDisplayData();
}

// Move constructor
PPU::FrameBuffer::String::Info::Info(Info && former)
{
	// Copy values
	string = former.string;
	stringLength = former.stringLength;
	displayData = former.displayData;
	totalTime = former.totalTime;
	firstLine = former.firstLine;
	numLines = former.numLines;
	alpha = former.alpha;

	// Delete former values
	former.string = nullptr;
	former.displayData = nullptr;
	former.numLines = 0;
}

// Sets up the string positions on the window
void PPU::FrameBuffer::String::Info::CreateDisplayData()
{
	char * cPos = string;
	unsigned int lastSpace = 0;

	if(!displayData)
		displayData = new stbtt_aligned_quad[stringLength];

	numLines = 1;

	float x = 2.0f;
	float y = 0.0f;
    const float yPos = 16.0f * (lastLine + 1);

	// Calculate the string positions
	for(unsigned int i = 0; i < stringLength; ++ i)
	{
		if (string[i] >= 32 && string[i] < 176)
		{
			stbtt_GetBakedQuad(STBGlyphData, 512, 512, string[i] - 32, &x, &y, &displayData[i], 1);

			if(string[i] == ' ')
				lastSpace = i;

			// If we are past the window's width (plus padding), move to next line
			else if((displayData[i].x1 + 3) > RenderWindow->getSize().x)
			{
				y += 16;
				x = 2;
				++numLines;

				// If possible, move whole words
				if(lastSpace)
				{
					i = lastSpace + 1;
					lastSpace = 0;
				}

				// Reposition char
				stbtt_GetBakedQuad(STBGlyphData, 512, 512, string[i] - 32, &x, &y, &displayData[i], 1);
			}
		}

        displayData[i].y0 += yPos;
        displayData[i].y1 += yPos;
	}

	firstLine = lastLine;
	lastLine += numLines;
}

// Destroys the string
PPU::FrameBuffer::String::Info::~Info()
{
	delete [] string;
	delete [] displayData;

    if (!STBPrepared)
        return;
 
	String::numLines -= numLines;

	if(!numLines)
		return;

	// If this is the last line, update it
	if((firstLine + numLines) == lastLine)
	{
		lastLine = 0;

		// Avoid getting the destroyed string as a match
		firstLine = 0;
		numLines = 0;

		for(std::list<Info>::const_iterator iter = InfoList.begin(); iter != InfoList.end(); ++ iter)
		{
			if((iter->firstLine + iter->numLines) > lastLine)
				lastLine = iter->firstLine + iter->numLines;
		}
	}
}


double PPU::FrameBuffer::FrameTime = 0.0f;