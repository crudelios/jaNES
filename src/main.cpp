// main.cpp
// A zona principal do código

//#include <vld.h>
#include "Console.h"
#include "CPU/CPU.h"
#include "PPU/PPU.h"
#include "PPU/FrameBuffer.h"
#include <conio.h>
#include <cstdio>

extern "C"
{
	#include <windows.h>
}

#include "Common/Common.h"
#include "res/resources.h"

// Globais necessários
bool hasConsole = false;
bool showDebug = false;
HMENU hMenu;
HWND Window = nullptr;
HWND RenderWindow = nullptr;
sf::Window *Renderer;
bool iAmPaused = false;
bool runFPSThread = true;
bool limitFPS = true;
HANDLE FPSThreadEvent;
char Filestring[MAX_PATH];
Console* console;

// Abrir um rom para emular
bool OpenNewFile()
{
	OPENFILENAME opf;
    ZeroMemory(&opf, sizeof(opf));

	opf.hwndOwner = Window;
	opf.lpstrFilter = "iNES File (*.nes)\0*.nes\0";
	opf.lpstrCustomFilter = 0;
	opf.nMaxCustFilter = 0L;
	opf.nFilterIndex = 1L;
	opf.lpstrFile = Filestring;
	opf.lpstrFile[0] = '\0';
	opf.nMaxFile = 256;
	opf.lpstrFileTitle = 0;
	opf.nMaxFileTitle=50;
	opf.lpstrInitialDir = ".";
	opf.lpstrTitle = "Open File";
	opf.nFileOffset = 0;
	opf.nFileExtension = 0;
	opf.lpstrDefExt = "*.*";
	opf.lpfnHook = NULL;
	opf.lCustData = 0;
	opf.Flags = OFN_EXPLORER & ~OFN_ALLOWMULTISELECT;
	opf.lStructSize = sizeof(OPENFILENAME);
	
	if(!GetOpenFileName(&opf))
	{
		return false;
	}

	console->InsertCartridge(opf.lpstrFile);

	if(!showDebug)
		console->DisplayDebugInfo(false);

	return true;
}

void SetWindowSize(RECT *rect, double scale)
{
    long ClientWidth = rect->right - rect->left;
    long ClientHeight = rect->bottom - rect->top;
}

// Event processing
LRESULT CALLBACK jaNESMainEventHandler(HWND Handle, UINT Message, WPARAM WParam, LPARAM LParam)
{
	switch (Message)
  {
    // Sair
    case WM_CLOSE:
    {
      PostQuitMessage(0);
      return 0;
    }

		// Este hack é necessário para o input funcionar no SFML 1.6, talvez na v2 já não seja preciso...
		/*case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		{
			SendMessage(RenderWindow, Message, WParam, LParam);
			return 0;
		}*/

		case WM_KILLFOCUS:
		{
			//CPU::Stop();
			//Renderer.SetActive();
			break;
		}

		case WM_SETFOCUS:
		{
			//if(!iAmPaused)
			//{
			//	CPU::Start();
			//	Renderer.SetActive(false);
			//}
			break;
		}

		// Definir transparência para o render aparecer quando se move a janela
		case WM_ERASEBKGND:
		case WM_PAINT:
			break;

		case WM_CTLCOLORSTATIC:
		{
			SetBkMode((HDC) WParam, TRANSPARENT);
			return (LRESULT) GetStockObject(NULL_BRUSH);
		}

		// Alterar tamanho da janela
        case WM_DPICHANGED:
        {
            unsigned int dpi = HIWORD(WParam);
            float scale = (float) dpi / USER_DEFAULT_SCREEN_DPI;
            PPU::FrameBuffer::SetScale(scale);
            RECT* const rect = (RECT*)LParam;
            SetWindowPos(Window, NULL, rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
            RECT clientRect;
            GetClientRect(Handle, &clientRect);
            SetWindowPos(RenderWindow, NULL, 0, 0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top, SWP_NOZORDER | SWP_NOACTIVATE);
            PPU::ResizeRenderer(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top, false);

            return 0;
        }
        case WM_GETDPISCALEDSIZE:
        {
            unsigned int dpi = (unsigned int) WParam;
            double scale = (double) dpi / USER_DEFAULT_SCREEN_DPI;

            RECT rect;
            rect.left = 0;
            rect.top = 0;
            rect.right = (LONG)((double) 512 * scale);
            rect.bottom = (LONG)((double) 480 * scale);

            if (!AdjustWindowRectExForDpi(&rect, WS_SYSMENU | WS_VISIBLE | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME, true, 0, dpi))
                return 0;

            SIZE* new_size = (SIZE*)LParam;
            new_size->cx = rect.right - rect.left;
            new_size->cy = rect.bottom - rect.top;

            return 1;
        }
		case WM_SIZE:
		case WM_MOVE:
		{
			bool moving = (Message == WM_MOVE) ? true : false; 
			RECT rect;
			GetClientRect(Handle, &rect);
			PPU::ResizeRenderer(rect.right - rect.left, rect.bottom - rect.top, moving);
            break;
		}

		// Processar acções do menu
		case WM_COMMAND:
		{
			switch(LOWORD(WParam))
			{
				// Abrir novo ficheiro para emulação
				case 101:
				{
					if(!OpenNewFile())
						return 0;

					Renderer->setActive(false);
					console->PowerOn();

					if(console->HasProblem())
					{
						MessageBox(Handle, "Error opening file!", "Unhelpful Error Message", MB_OK | MB_ICONEXCLAMATION);
						Renderer->setActive();
						glClearColor(0, 0, 0, 255);
						glClear(GL_COLOR_BUFFER_BIT);
						Renderer->display();
					}

					return 0;
				}

				// Sair
				case 102:
				{
					PostQuitMessage(0);
					return 0;
				}

				// Continuar a emulação
				case 201:
				{
					if(console->Resume())
					{
						Renderer->setActive(false);
						iAmPaused = false;
					}
					return 0;
				}

				// Pausar a emulação
				case 202:
				{
					if(console->Suspend())
					{
						Renderer->setActive();
						iAmPaused = true;
					}
					return 0;
				}

                // Hard reset
				case 230:
				{
					printf(Filestring);
                    console->PowerOff();
					Renderer->setActive(false);
					console->PowerOn();

					return 0;
				}

				case 231:
				{
					// Soft Reset
					console->Reset();
					return 0;
				}

				// Limit FPS
				case 204:
				{
					if(limitFPS)
					{
						CheckMenuItem(hMenu, 204, MF_BYCOMMAND | MF_UNCHECKED);
						limitFPS = false;
						console->LimitFPS(0);
					}
					else
					{
						CheckMenuItem(hMenu, 204, MF_BYCOMMAND | MF_CHECKED);
						limitFPS = true;
						console->LimitFPS(60);
					}

					break;
				}


				// Mostrar janela de debug (consola)
				case 301:
				{
					if(hasConsole)
					{
						CheckMenuItem(hMenu, 301, MF_BYCOMMAND | MF_UNCHECKED);
						console->DisplayDebugInfo(false);
						Sleep(1);

                        freopen("NUL:", "w", stdout);
                        setvbuf(stdout, NULL, _IONBF, 0);
                        FreeConsole();

						hasConsole = false;
					}
					else
					{
                        if (!AllocConsole())
                            break;

                        // Redirect unbuffered STDOUT to the console
                        if (!freopen("CONOUT$", "w", stdout))
                        {
                            FreeConsole();
                            break;
                        }

                        setvbuf(stdout, NULL, _IONBF, 0);

						CheckMenuItem(hMenu, 301, MF_BYCOMMAND | MF_CHECKED);
						SetForegroundWindow(Handle);
						if(showDebug)
							console->DisplayDebugInfo();
						hasConsole = true;
					}
					break;
				}

				// Mostrar info de debug na consola
				case 302:
				{
					if(showDebug)
					{
						CheckMenuItem(hMenu, 302, MF_BYCOMMAND | MF_UNCHECKED);
						console->DisplayDebugInfo(false);
						showDebug = false;
					}
					else
					{
						CheckMenuItem(hMenu, 302, MF_BYCOMMAND | MF_CHECKED);
						if(hasConsole)
							console->DisplayDebugInfo();
						showDebug = true;
					}
				}
			}
		}
  }
    
  return DefWindowProc(Handle, Message, WParam, LParam);
}

void ConstructMenu(HWND hWnd)
{
	hMenu	= CreateMenu();
	HMENU hsmFile, hsmEmulation, hsmView, hsmReset;
	hsmFile       = CreatePopupMenu();
	hsmEmulation  = CreatePopupMenu();
	hsmReset      = CreatePopupMenu();
	hsmView       = CreatePopupMenu();
	MENUITEMINFO mii;
	memset(&mii, 0, sizeof(MENUITEMINFO));	// or use mii.hSubMenu	= NULL;
	mii.cbSize	= sizeof(MENUITEMINFO);
	mii.fMask	= MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_SUBMENU;
	mii.fType	= MFT_STRING;

	/*** File *****************************************************************/
	mii.dwTypeData	= TEXT("Open...");
	mii.cch		= 3;
	mii.wID		= 101;
	InsertMenuItem(hsmFile, 0, TRUE, (LPCMENUITEMINFO) &mii);

	mii.dwTypeData	= TEXT("Exit");
	mii.cch		= 4;
	mii.wID		= 102;
	InsertMenuItem(hsmFile, 1, TRUE, (LPCMENUITEMINFO) &mii);

	/*** Emulation *****************************************************************/
	mii.dwTypeData	= TEXT("Hard");
	mii.cch		= 4;
	mii.wID		= 230;
	InsertMenuItem(hsmReset, 0, TRUE, (LPCMENUITEMINFO) &mii);

	mii.dwTypeData	= TEXT("Soft");
	mii.cch		= 4;
	mii.wID		= 231;
	InsertMenuItem(hsmReset, 1, TRUE, (LPCMENUITEMINFO) &mii);

	/*** Emulation *****************************************************************/
	mii.dwTypeData	= TEXT("Continue");
	mii.cch		= 4;
	mii.wID		= 201;
	InsertMenuItem(hsmEmulation, 0, TRUE, (LPCMENUITEMINFO) &mii);

	mii.dwTypeData	= TEXT("Pause");
	mii.cch		= 3;
	mii.wID		= 202;
	InsertMenuItem(hsmEmulation, 1, TRUE, (LPCMENUITEMINFO) &mii);

	mii.dwTypeData	= TEXT("Reset");
	mii.cch		= 5;
	mii.wID		= 203;
	mii.hSubMenu = hsmReset;
	InsertMenuItem(hsmEmulation, 2, TRUE, (LPCMENUITEMINFO) &mii);
	mii.hSubMenu = NULL;

	mii.dwTypeData	= TEXT("Limit FPS");
	mii.cch		= 6;
	mii.wID		= 204;
	InsertMenuItem(hsmEmulation, 3, TRUE, (LPCMENUITEMINFO) &mii);
	CheckMenuItem(hsmEmulation, 3, MF_BYPOSITION | MF_CHECKED);

	/*** View *****************************************************************/
	mii.dwTypeData	= TEXT("Console Window");
	mii.cch		= 4;
	mii.wID		= 301;
	InsertMenuItem(hsmView, 0, TRUE, (LPCMENUITEMINFO) &mii);
	CheckMenuItem(hsmView, 0, MF_BYPOSITION | MF_UNCHECKED);

	mii.dwTypeData	= TEXT("Disassembly information");
	mii.cch		= 4;
	mii.wID		= 302;
	InsertMenuItem(hsmView, 1, TRUE, (LPCMENUITEMINFO) &mii);
	CheckMenuItem(hsmView, 1, MF_BYPOSITION | MF_UNCHECKED);

	/*** MENU BAR *************************************************************/
	mii.dwTypeData	= TEXT("File");
	mii.cch		= 4;
	mii.wID		= 100;
	mii.hSubMenu	= hsmFile;
	InsertMenuItem(hMenu, 0, TRUE, (LPCMENUITEMINFO) &mii);

	mii.dwTypeData	= TEXT("Emulation");
	mii.cch		= 4;
	mii.wID		= 200;
	mii.hSubMenu	= hsmEmulation;
	InsertMenuItem(hMenu, 1, TRUE, (LPCMENUITEMINFO) &mii);

	mii.dwTypeData	= TEXT("View");
	mii.cch		= 4;
	mii.wID		= 300;
	mii.hSubMenu	= hsmView;
	InsertMenuItem(hMenu, 2, TRUE, (LPCMENUITEMINFO) &mii);

	SetMenu(hWnd, hMenu);
}

// Indica o fps
void FPSThread()
{
	FPSThreadEvent = CreateEvent(NULL, TRUE, FALSE, "FPSCounter");

    if (!FPSThreadEvent)
    {
        printf("Error creating the FPS thread. FPS will not be displayed.");
        return;
    }

	static char WinTitle[256];
	static double LastTime = Common::GetUTime();
	while(runFPSThread)
	{
		double CurrTime = Common::GetUTime();
		double ElapsedTime = (CurrTime - LastTime) / 10000;
		LastTime = CurrTime;
		sprintf(WinTitle, "jaNES 0.35 - FPS: %.2f", (double) ((PPU::GetTicks() * 1000) / ElapsedTime));
		SetWindowText(Window, WinTitle);

		// Dormir
		WaitForSingleObject(FPSThreadEvent, 1000);
	}

	CloseHandle(FPSThreadEvent);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// Criar classe da janela
	WNDCLASS WindowClass;
	WindowClass.style         = 0;
	WindowClass.lpfnWndProc   = &jaNESMainEventHandler;
	WindowClass.cbClsExtra    = 0;
	WindowClass.cbWndExtra    = 0;
	WindowClass.hInstance     = hInstance;
	WindowClass.hIcon = (HICON) (LoadImage(hInstance, MAKEINTRESOURCE(MAIN_ICON), IMAGE_ICON,	32, 32, LR_DEFAULTSIZE));
	WindowClass.hCursor       = 0;
	WindowClass.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	WindowClass.lpszMenuName  = NULL;
	WindowClass.lpszClassName = "jaNES Main";
	RegisterClass(&WindowClass);

    DWORD window_style = WS_SYSMENU | WS_VISIBLE | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME;
    unsigned int dpi = GetDpiForSystem();
    float scale = (float) dpi / USER_DEFAULT_SCREEN_DPI;

    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = (LONG)((double)512 * scale);
    rect.bottom = (LONG)((double)480 * scale);

    if (!AdjustWindowRectExForDpi(&rect, window_style, true, 0, dpi))
        return 0;

	Window = CreateWindow("jaNES Main", "jaNES v0.30", window_style, 200, 200, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);
    RenderWindow = CreateWindow("STATIC", NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 0, 0, (int)(512 * scale), (int)(480 * scale), Window, NULL, hInstance, NULL);

    sf::Window myRender(RenderWindow);

	Renderer = &myRender;
	PPU::RegisterWindow(Renderer);
	PPU::LimitFPS(60);
    PPU::FrameBuffer::SetScale(scale);

	ConstructMenu(Window);

	// Start FPS thread
	HANDLE FPSThreadID = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) &FPSThread, NULL, 0, 0);

	MSG Message;

	// Main loop
	while(GetMessage(&Message, NULL, 0, 0))
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	// Destroy FPS thread
    if (FPSThreadID)
    {
        runFPSThread = false;
        SetEvent(FPSThreadEvent);
        WaitForSingleObject(FPSThreadID, INFINITE);
    }

	// Shut down the emulated console
	console->PowerOff();

	// Destroy console window
    if (hasConsole)
    {
        freopen("NUL:", "w", stdout);
        setvbuf(stdout, NULL, _IONBF, 0);
        FreeConsole();
    }

	// Destruir o renderer
	Renderer->close();

	// Destruir a janela
	DestroyWindow(Window);

	// Desregistar a classe
	UnregisterClass("jaNES Main", hInstance);

	return 0;
} 