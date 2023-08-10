#include <windows.h>
#include <d3d11.h>

#include "IMGUI/imgui.h"
#include "IMGUI/imgui_impl_win32.h"
#include "IMGUI/imgui_impl_dx11.h"


#include "Menu.h"


extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND _Hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND _Hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(_Hwnd, message, wParam, lParam))
		return true;


	switch (message)
	{
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(_Hwnd, message, wParam, lParam);
}


ID3D11Device* g_pd3dDevice = NULL;
ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
IDXGISwapChain* g_pSwapChain = NULL;
ID3D11RenderTargetView* g_mainRenderTargetView = NULL;

void CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

HRESULT CreateDeviceD3D(HWND hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	//sd.BufferCount = 2;
	sd.BufferCount = 1;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//sd.BufferDesc.RefreshRate.Numerator = 60;
	//sd.BufferDesc.RefreshRate.Numerator = 144;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, 
		&sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
		return E_FAIL;

	CreateRenderTarget();

	return S_OK;
}




bool Init(HINSTANCE hInstance)
{
	WNDCLASSEXA wcex;
	ZeroMemory(&wcex, sizeof(WNDCLASSEX));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hCursor = 0;
	wcex.hbrBackground = 0;
	wcex.lpszClassName = "Medusa";
	if (!RegisterClassExA(&wcex))
	{
		return false;
	}

	HWND _Hwnd = CreateWindowA("Medusa", "Medusa", WS_OVERLAPPEDWINDOW,
		0, 0, 1672, 931,
		nullptr, nullptr, hInstance, nullptr);

	if (!_Hwnd)
	{
		int a = GetLastError();
		return false;
	}
	ShowWindow(_Hwnd, SW_SHOW);
	UpdateWindow(_Hwnd);


	if (CreateDeviceD3D(_Hwnd) < 0)
	{
		return false;
	}

	ImGui::CreateContext();
	ImGui_ImplWin32_Init(_Hwnd);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
	ImGui::StyleColorsClassic();

	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = 0;

	return true;
}










int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	Init(hInstance);


	MSG msg;
	GetMessage(&msg, nullptr, 0, 0);
	while (msg.message != WM_QUIT)//msg.message != WM_QUIT
	{
		Draw();

		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}
	}
	return (int)msg.wParam;

}