#include "Pch.h"
#include "Experiments.h"
#include "DirectX12.h"
#include "Library.h"
#include "Application.h"

//#define USE_PIX 1
#include "pix3.h"
#if defined(USE_PIX)
#pragma comment(lib, "WinPixEventRuntime.lib")
#endif


void* operator new[](size_t Size, const char* /*Name*/, int /*Flags*/, unsigned /*DebugFlags*/, const char* /*File*/, int /*Line*/)
{
	return malloc(Size);
}

void* operator new[](size_t Size, size_t Alignment, size_t AlignmentOffset, const char* /*Name*/, int /*Flags*/, unsigned /*DebugFlags*/, const char* /*File*/, int /*Line*/)
{
	return _aligned_offset_malloc(Size, Alignment, AlignmentOffset);
}

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	GApp.Initialize();

	if (!GDx12.Initialize(GApp.Window))
	{
		// #TODO: Add MessageBox
		return 1;
	}

	IExperiment Ex = Experiment0();
	if (!Ex.Initialize())
	{
		Ex.Shutdown();
		GDx12.Shutdown();
		return 2;
	}

	MSG Message = {};
	for (;;)
	{
		if (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
			if (Message.message == WM_QUIT)
			{
				break;
			}
		}
		else
		{
			GApp.Update();
			Ex.Update();
			GDx12.Present();
		}
	}

	Ex.Shutdown();
	GDx12.Shutdown();
	return 0;
}
