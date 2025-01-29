#pragma once
#include "overlay.h"

HINSTANCE Overlay::instance = nullptr;
INT Overlay::cmdShow = 0;
HWND Overlay::window = nullptr;
ID3D11Device* Overlay::device = nullptr;
ID3D11DeviceContext* Overlay::deviceContext = nullptr;
IDXGISwapChain* Overlay::swapChain = nullptr;
ID3D11RenderTargetView* Overlay::renderTargetView = nullptr;
std::wstring Overlay::windowName = L"";
Vfunc Overlay::renderFunction = nullptr;
int Overlay::width = -1;
int Overlay::height = -1;

void Overlay::init(HINSTANCE& instanceIn, INT& cmdShowIn, std::string windowNameIn, Vfunc renderFunctionIn) {
    instance = instanceIn;
    cmdShow = cmdShowIn;
    windowName = std::wstring(windowNameIn.begin(), windowNameIn.end());
    renderFunction = renderFunctionIn;

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = Overlay::windowProcedure;
    wc.hInstance = instance;
    wc.lpszClassName = windowName.c_str();

    if(!RegisterClassExW(&wc)) {
        MessageBox(nullptr, L"Failed to register window class!", L"Error", MB_ICONERROR);
        return;
    }

    getDesktopResolution(width, height);

    window = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
        wc.lpszClassName,
        windowName.c_str(),
        WS_POPUP,
        0,
        0,
        width,
        height,
        nullptr,
        nullptr,
        instance,
        nullptr
    );

    if(!window) {
        MessageBox(nullptr, L"Failed to create window!", L"Error", MB_ICONERROR);
        UnregisterClassW(wc.lpszClassName, instance);
        return;
    }

    if(!SetLayeredWindowAttributes(window, RGB(0, 0, 0), BYTE(255), LWA_ALPHA)) {
        MessageBox(nullptr, L"Failed to set layered window attributes!", L"Error", MB_ICONERROR);
        DestroyWindow(window);
        UnregisterClassW(wc.lpszClassName, instance);
        return;
    }

    RECT clientArea{};
    if(!GetClientRect(window, &clientArea)) {
        MessageBox(nullptr, L"Failed to get client area!", L"Error", MB_ICONERROR);
        DestroyWindow(window);
        UnregisterClassW(wc.lpszClassName, instance);
        return;
    }

    RECT windowArea{};
    if(!GetWindowRect(window, &windowArea)) {
        MessageBox(nullptr, L"Failed to get window area!", L"Error", MB_ICONERROR);
        DestroyWindow(window);
        UnregisterClassW(wc.lpszClassName, instance);
        return;
    }

    POINT diff{};
    if(!ClientToScreen(window, &diff)) {
        MessageBox(nullptr, L"Failed to convert client to screen coordinates!", L"Error", MB_ICONERROR);
        DestroyWindow(window);
        UnregisterClassW(wc.lpszClassName, instance);
        return;
    }

    const MARGINS margins{
        windowArea.left + (diff.x - windowArea.left),
        windowArea.top + (diff.y - windowArea.top),
        clientArea.right,
        clientArea.bottom
    };

    if(DwmExtendFrameIntoClientArea(window, &margins) != S_OK) {
        MessageBox(nullptr, L"Failed to extend frame into client area!", L"Error", MB_ICONERROR);
        DestroyWindow(window);
        UnregisterClassW(wc.lpszClassName, instance);
        return;
    }

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferDesc.RefreshRate.Numerator = 60U;
    sd.BufferDesc.RefreshRate.Denominator = 1U;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.SampleDesc.Count = 1U;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 2U;
    sd.OutputWindow = window;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    constexpr D3D_FEATURE_LEVEL levels[2]{
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0
    };

    D3D_FEATURE_LEVEL level{};

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0U,
        levels,
        2U,
        D3D11_SDK_VERSION,
        &sd,
        &swapChain,
        &device,
        &level,
        &deviceContext
    );

    if(FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create D3D11 device and swap chain!", L"Error", MB_ICONERROR);
        DestroyWindow(window);
        UnregisterClassW(wc.lpszClassName, instance);
        return;
    }

    ID3D11Texture2D* backBuffer{nullptr};
    hr = swapChain->GetBuffer(0U, IID_PPV_ARGS(&backBuffer));

    if(FAILED(hr) || !backBuffer) {
        MessageBox(nullptr, L"Failed to get back buffer!", L"Error", MB_ICONERROR);
        if(swapChain) swapChain->Release();
        if(deviceContext) deviceContext->Release();
        if(device) device->Release();
        DestroyWindow(window);
        UnregisterClassW(wc.lpszClassName, instance);
        return;
    }

    hr = device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);
    backBuffer->Release();

    if(FAILED(hr)) {
        MessageBox(nullptr, L"Failed to create render target view!", L"Error", MB_ICONERROR);
        if(swapChain) swapChain->Release();
        if(deviceContext) deviceContext->Release();
        if(device) device->Release();
        DestroyWindow(window);
        UnregisterClassW(wc.lpszClassName, instance);
        return;
    }

    ShowWindow(window, cmdShow);
    UpdateWindow(window);

    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(device, deviceContext);
}

void Overlay::release() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    if(swapChain) {
        swapChain->Release();
    }

    if(deviceContext) {
        deviceContext->Release();
    }

    if(device) {
        device->Release();
    }

    if(renderTargetView) {
        renderTargetView->Release();
    }

    if(window) {
        DestroyWindow(window);
        UnregisterClassW(windowName.c_str(), instance);
    }
}

void Overlay::run() {
    bool running = true;
    while(running) {
        MSG msg;
        while(PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if(WM_QUIT == msg.message) {
                running = false;
            }
        }

        if(!running) {
            break;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();

        ImGui::NewFrame();

        renderFunction();

        ImGui::Render();

        constexpr float color[4]{0.f, 0.f, 0.f, 0.f};
        deviceContext->OMSetRenderTargets(1, &renderTargetView, nullptr);
        deviceContext->ClearRenderTargetView(renderTargetView, color);

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        HRESULT hr = swapChain->Present(1U, 0U);

        if(FAILED(hr)) {
            MessageBox(nullptr, L"Failed to present swap chain!", L"Error", MB_ICONERROR);
            running = false;
        }
    }
}

LRESULT CALLBACK Overlay::windowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    if(ImGui_ImplWin32_WndProcHandler(window, message, wParam, lParam)) {
        return 0L;
    }

    if(message == WM_DESTROY) {
        PostQuitMessage(0);
        return 0L;
    }

    return DefWindowProc(window, message, wParam, lParam);
}

void Overlay::getDesktopResolution(int& width, int& height) {
    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    width = desktop.right;
    height = desktop.bottom;
}
