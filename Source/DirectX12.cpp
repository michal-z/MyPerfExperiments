#include "Pch.h"
#include "DirectX12.h"


FDirectX12 GDx12;

bool FDirectX12::Initialize(HWND Window)
{
	assert(Device == nullptr);
	assert(Window != nullptr);

	VHR(CreateDXGIFactory1(IID_PPV_ARGS(&GiFactory)));

#if DX12_ENABLE_DEBUG_LAYER == 1
	ID3D12Debug* Dbg = nullptr;
	D3D12GetDebugInterface(IID_PPV_ARGS(&Dbg));
	if (Dbg)
	{
		Dbg->EnableDebugLayer();
		Dbg->Release();
	}
#endif

	if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&Device))))
	{
		return false;
	}

	D3D12_COMMAND_QUEUE_DESC CmdQueueDesc = {};
	CmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	CmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	CmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	VHR(Device->CreateCommandQueue(&CmdQueueDesc, IID_PPV_ARGS(&CmdQueue)));

	DXGI_SWAP_CHAIN_DESC SwapChainDesc = {};
	SwapChainDesc.BufferCount = 4;
	SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.OutputWindow = Window;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	SwapChainDesc.Windowed = 1;

	IDXGISwapChain* TempSwapChain;
	VHR(GiFactory->CreateSwapChain(CmdQueue, &SwapChainDesc, &TempSwapChain));
	VHR(TempSwapChain->QueryInterface(IID_PPV_ARGS(&SwapChain)));
	TempSwapChain->Release();

	for (uint32_t Idx = 0; Idx < 2; ++Idx)
	{
		VHR(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CmdAlloc[Idx])));
	}

	RECT Rect;
	GetClientRect(Window, &Rect);
	BackBufferResolution[0] = (uint32_t)(Rect.right - Rect.left);
	BackBufferResolution[1] = (uint32_t)(Rect.bottom - Rect.top);

	CbvSrvUavSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	RtvSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	{
		D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
		HeapDesc.NumDescriptors = 4;
		HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		VHR(Device->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&RtvHeap)));
		RtvHeapStart = RtvHeap->GetCPUDescriptorHandleForHeapStart();

		D3D12_CPU_DESCRIPTOR_HANDLE Handle = RtvHeapStart;

		for (uint32_t Idx = 0; Idx < 4; ++Idx)
		{
			VHR(SwapChain->GetBuffer(Idx, IID_PPV_ARGS(&SwapBuffers[Idx])));

			Device->CreateRenderTargetView(SwapBuffers[Idx], nullptr, Handle);
			Handle.ptr += RtvSize;
		}
	}
	{
		D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
		HeapDesc.NumDescriptors = 1;
		HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		VHR(Device->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&DsvHeap)));
		DsvHeapStart = DsvHeap->GetCPUDescriptorHandleForHeapStart();

		D3D12_CLEAR_VALUE ClearValue = {};
		ClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		ClearValue.DepthStencil.Depth = 1.0f;
		ClearValue.DepthStencil.Stencil = 0;

		D3D12_HEAP_PROPERTIES HeapProps = {};
		HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

		D3D12_RESOURCE_DESC ImageDesc = {};
		ImageDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		ImageDesc.Width = BackBufferResolution[0];
		ImageDesc.Height = BackBufferResolution[1];
		ImageDesc.DepthOrArraySize = 1;
		ImageDesc.MipLevels = 1;
		ImageDesc.Format = DXGI_FORMAT_D32_FLOAT;
		ImageDesc.SampleDesc.Count = 1;
		ImageDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		ImageDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		VHR(Device->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &ImageDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE, &ClearValue, IID_PPV_ARGS(&DepthStencilImage)));

		D3D12_DEPTH_STENCIL_VIEW_DESC ViewDesc = {};
		ViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
		ViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		ViewDesc.Flags = D3D12_DSV_FLAG_NONE;
		Device->CreateDepthStencilView(DepthStencilImage, &ViewDesc, DsvHeapStart);
	}
	VHR(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&FrameFence)));

	FrameFenceEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
	assert(FrameFenceEvent);

	VHR(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CmdAlloc[0], nullptr,
		IID_PPV_ARGS(&CmdList)));

	return true;
}

void FDirectX12::Shutdown()
{
}

void FDirectX12::Present()
{
	SwapChain->Present(0, 0);
	CmdQueue->Signal(FrameFence, ++CpuCompletedFrames);

	const uint64_t GpuCompletedFrames = FrameFence->GetCompletedValue();

	if ((CpuCompletedFrames - GpuCompletedFrames) >= 2)
	{
		FrameFence->SetEventOnCompletion(GpuCompletedFrames + 1, FrameFenceEvent);
		WaitForSingleObject(FrameFenceEvent, INFINITE);
	}

	BackBufferIndex = SwapChain->GetCurrentBackBufferIndex();
	FrameIndex = !FrameIndex;
}
