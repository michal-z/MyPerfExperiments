#include "Pch.h"
#include "Application.h"


FApplication GApp;

void FApplication::Initialize()
{
	QueryPerformanceCounter(&StartCounter);
	QueryPerformanceFrequency(&Frequency);
	SetProcessDPIAware();
	MakeWindow(1280, 720);
}

void FApplication::Update()
{
	UpdateFrameTime();
}

double FApplication::GetTime()
{
	LARGE_INTEGER Counter;
	QueryPerformanceCounter(&Counter);
	return (Counter.QuadPart - StartCounter.QuadPart) / (double)Frequency.QuadPart;
}

void FApplication::UpdateFrameTime()
{
	static double LastTime = -1.0;
	static double LastFpsTime = 0.0;
	static unsigned FpsFrame = 0;

	if (LastTime < 0.0)
	{
		LastTime = GetTime();
		LastFpsTime = LastTime;
	}

	FrameTime = GetTime();
	FrameDeltaTime = (float)(FrameTime - LastTime);
	LastTime = FrameTime;

	if ((FrameTime - LastFpsTime) >= 1.0)
	{
		double Fps = FpsFrame / (FrameTime - LastFpsTime);
		double AvgFrameTime = (1.0 / Fps) * 1000000.0;
		char Text[256];
		wsprintf(Text, "[%d fps  %d us] %s", (int)Fps, (int)AvgFrameTime, ApplicationName);
		SetWindowText(Window, Text);
		LastFpsTime = FrameTime;
		FpsFrame = 0;
	}
	FpsFrame++;
}

LRESULT CALLBACK FApplication::ProcessWindowMessage(HWND InWindow, UINT Message, WPARAM WParam, LPARAM LParam)
{
	switch (Message)
	{
	case WM_KEYDOWN:
		if (WParam == VK_ESCAPE)
		{
			PostQuitMessage(0);
			return 0;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(InWindow, Message, WParam, LParam);
}

void FApplication::MakeWindow(uint32_t ResolutionX, uint32_t ResolutionY)
{
	WNDCLASS Winclass = {};
	Winclass.lpfnWndProc = ProcessWindowMessage;
	Winclass.hInstance = GetModuleHandle(nullptr);
	Winclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	Winclass.lpszClassName = ApplicationName;
	if (!RegisterClass(&Winclass))
	{
		assert(0);
	}

	RECT Rect = { 0, 0, (int32_t)ResolutionX, (int32_t)ResolutionY };
	if (!AdjustWindowRect(&Rect, WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX, 0))
	{
		assert(0);
	}

	Window = CreateWindowEx(
		0, ApplicationName, ApplicationName,
		WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT,
		Rect.right - Rect.left, Rect.bottom - Rect.top,
		NULL, NULL, NULL, 0);
	assert(Window);
}
