#include "Pch.h"
#include "Experiments.h"
#include "DirectX12.h"
#include "Library.h"
#include "EASTL/string.h"


static D3D12_VIEWPORT GViewport;
static D3D12_RECT GScissorRect;
static ID3D12PipelineState* GPso;
static ID3D12RootSignature* GRs;
static HANDLE GShaderChanged;

static void CreatePso()
{
	if (GPso)
	{
		GPso->Release();
	}
	eastl::vector<uint8_t> CsoVS = LoadFile("Assets/Shaders/Experiment0/TriangleVS.cso");
	eastl::vector<uint8_t> CsoPS = LoadFile("Assets/Shaders/Experiment0/SolidPS.cso");

	D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDesc = {};
	PsoDesc.VS = { CsoVS.data(), CsoVS.size() };
	PsoDesc.PS = { CsoPS.data(), CsoPS.size() };
	PsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	PsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	PsoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	PsoDesc.SampleMask = 0xffffffff;
	PsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	PsoDesc.NumRenderTargets = 1;
	PsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	PsoDesc.SampleDesc.Count = 1;

	VHR(GDx12.Device->CreateGraphicsPipelineState(&PsoDesc, IID_PPV_ARGS(&GPso)));
	VHR(GDx12.Device->CreateRootSignature(0, CsoVS.data(), CsoVS.size(), IID_PPV_ARGS(&GRs)));
}

static void Update()
{
	if (WaitForSingleObject(GShaderChanged, 0) == WAIT_OBJECT_0)
	{
		Sleep(10);
		CreatePso();
		FindNextChangeNotification(GShaderChanged);
	}

	ID3D12CommandAllocator* CmdAlloc = GDx12.CmdAlloc[GDx12.FrameIndex];
	CmdAlloc->Reset();

	ID3D12GraphicsCommandList* CmdList = GDx12.CmdList;

	CmdList->Reset(CmdAlloc, nullptr);
	CmdList->RSSetViewports(1, &GViewport);
	CmdList->RSSetScissorRects(1, &GScissorRect);

	D3D12_RESOURCE_BARRIER Barrier = {};
	Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	Barrier.Transition.pResource = GDx12.SwapBuffers[GDx12.BackBufferIndex];
	Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	CmdList->ResourceBarrier(1, &Barrier);

	D3D12_CPU_DESCRIPTOR_HANDLE RtvHandle = GDx12.RtvHeapStart;
	RtvHandle.ptr += GDx12.BackBufferIndex * GDx12.RtvSize;

	D3D12_CPU_DESCRIPTOR_HANDLE DsvHandle = GDx12.DsvHeapStart;

	CmdList->OMSetRenderTargets(1, &RtvHandle, 0, &DsvHandle);

	const float ClearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	CmdList->ClearRenderTargetView(RtvHandle, ClearColor, 0, nullptr);
	CmdList->ClearDepthStencilView(DsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	CmdList->SetPipelineState(GPso);
	CmdList->SetGraphicsRootSignature(GRs);
	CmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	CmdList->DrawInstanced(3, 1, 0, 0);

	Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	CmdList->ResourceBarrier(1, &Barrier);

	CmdList->Close();

	GDx12.CmdQueue->ExecuteCommandLists(1, (ID3D12CommandList**)&CmdList);
}

static bool Initialize()
{
	GDx12.CmdList->Close();

	GViewport = { 0.0f, 0.0f, (float)GDx12.BackBufferResolution[0], (float)GDx12.BackBufferResolution[1], 0.0f, 1.0f };
	GScissorRect = { 0, 0, (LONG)GDx12.BackBufferResolution[0], (LONG)GDx12.BackBufferResolution[1] };

	CreatePso();

	char Path[1024];
	GetCurrentDirectory(sizeof(Path), Path);
	eastl::string FullPath = eastl::string(Path) + "\\Assets\\Shaders\\Experiment0";
	GShaderChanged = FindFirstChangeNotification(FullPath.c_str(), FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);

	return true;
}

static void Shutdown()
{
}

IExperiment Experiment0()
{
	IExperiment Ex = {};
	Ex.Update = Update;
	Ex.Initialize = Initialize;
	Ex.Shutdown = Shutdown;
	return Ex;
}
