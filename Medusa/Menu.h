#pragma once
#include <windows.h>
#include <d3d11.h>

#include "IMGUI/imgui.h"
#include "IMGUI/imgui_impl_win32.h"
#include "IMGUI/imgui_impl_dx11.h"


extern ID3D11RenderTargetView* g_mainRenderTargetView;
extern ID3D11DeviceContext* g_pd3dDeviceContext;
extern IDXGISwapChain* g_pSwapChain;



void Draw();