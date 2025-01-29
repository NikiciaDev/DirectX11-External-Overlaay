#pragma once
#include <windows.h>
#include <dwmapi.h>
#include <d3d11.h>
#include <string>

#include "../external/imgui/imgui.h"
#include "../external/imgui/imgui_impl_dx11.h"
#include "../external/imgui/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class Overlay {
private:
    static HINSTANCE instance;
    static INT cmdShow;
    static HWND window;
    static ID3D11Device* device;
    static ID3D11DeviceContext* deviceContext;
    static IDXGISwapChain* swapChain;
    static ID3D11RenderTargetView* renderTargetView;
    static std::wstring windowName;

public:
    static void init(HINSTANCE& instanceIn, INT& cmdShowIn, std::string name);
    static void release();

    static void run();

private:
    static LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
    static void getDesktopResolution(int& width, int& height);
};
