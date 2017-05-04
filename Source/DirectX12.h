#pragma once

#define VHR(hr) if (FAILED(hr)) { assert(0); }
#define SAFE_RELEASE(obj) if ((obj)) { (obj)->Release(); (obj) = nullptr; }
#define DX12_ENABLE_DEBUG_LAYER 0

class FDirectX12
{
public:
	uint32_t BackBufferResolution[2];
	uint32_t BackBufferIndex;
	uint32_t FrameIndex;

	uint32_t CbvSrvUavSize;
	uint32_t RtvSize;
	D3D12_CPU_DESCRIPTOR_HANDLE RtvHeapStart;
	D3D12_CPU_DESCRIPTOR_HANDLE DsvHeapStart;

	ID3D12Device* Device;
	ID3D12CommandQueue* CmdQueue;
	ID3D12CommandAllocator* CmdAlloc[2];
	ID3D12GraphicsCommandList* CmdList;
	ID3D12Resource* SwapBuffers[4];
	ID3D12Resource*	DepthStencilImage;

	bool Initialize(HWND Window);
	void Shutdown();
	void Present();

private:
	IDXGIFactory4* GiFactory;
	IDXGISwapChain3* SwapChain;
	ID3D12DescriptorHeap* RtvHeap;
	ID3D12DescriptorHeap* DsvHeap;
	uint64_t CpuCompletedFrames;
	ID3D12Fence* FrameFence;
	HANDLE FrameFenceEvent;
};

extern FDirectX12 GDx12;
