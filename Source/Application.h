#pragma once

class FApplication
{
public:
	HWND Window;
	double FrameTime;
	float FrameDeltaTime;

	void Initialize();
	void Update();
	double GetTime();

private:
	const char* ApplicationName = "Performance Experiments";
	LARGE_INTEGER Frequency;
	LARGE_INTEGER StartCounter;

	void UpdateFrameTime();
	static LRESULT CALLBACK ProcessWindowMessage(HWND InWindow, UINT Message, WPARAM WParam, LPARAM LParam);
	void MakeWindow(uint32_t ResolutionX, uint32_t ResolutionY);
};

extern FApplication GApp;
