#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>
#include <dwmapi.h>
#include <d3d11.h>
#include <thread>
#include <chrono>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#include "overlay.h"

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK window_procedure(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
    if (ImGui_ImplWin32_WndProcHandler(window, message, w_param, l_param))
        return 0L;

    if (message == WM_DESTROY)
    {
        PostQuitMessage(0);
        return 0L;
    }

    return DefWindowProc(window, message, w_param, l_param);
}

int RunOverlay(HINSTANCE instance, int cmd_show)
{
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = window_procedure;
    wc.hInstance = instance;
    wc.lpszClassName = L"External Overlay Class";

    if (!RegisterClassExW(&wc))
    {
        std::cerr << "RegisterClassExW failed. GetLastError=" << GetLastError() << "\n";
        return 1;
    }

    const HWND window = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
        wc.lpszClassName,
        L"External Overlay",
        WS_POPUP,
        0,
        0,
        1920, // TODO: replace with GetSystemMetrics(SM_CXSCREEN)
        1080, // TODO: replace with GetSystemMetrics(SM_CYSCREEN)
        nullptr,
        nullptr,
        wc.hInstance,
        nullptr
    );

    if (!window)
    {
        std::cerr << "CreateWindowExW failed. GetLastError=" << GetLastError() << "\n";
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Fully transparent background
    SetLayeredWindowAttributes(window, RGB(0, 0, 0), BYTE(255), LWA_ALPHA);

    {
        RECT client_area{};
        GetClientRect(window, &client_area);

        RECT window_area{};
        GetWindowRect(window, &window_area);

        POINT diff{};
        ClientToScreen(window, &diff);

        const MARGINS margins{
            window_area.left + (diff.x - window_area.left),
            window_area.top + (diff.y - window_area.top),
            client_area.right,
            client_area.bottom
        };

        DwmExtendFrameIntoClientArea(window, &margins);
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

    constexpr D3D_FEATURE_LEVEL levels[2] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0
    };

    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* device_context = nullptr;
    IDXGISwapChain* swap_chain = nullptr;
    ID3D11RenderTargetView* render_target_view = nullptr;
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
        &swap_chain,
        &device,
        &level,
        &device_context
    );

    if (FAILED(hr) || !swap_chain || !device || !device_context)
    {
        std::cerr << "[!] D3D11CreateDeviceAndSwapChain failed. HRESULT=0x" << std::hex << (unsigned)hr << std::dec << "\n";
        if (swap_chain) swap_chain->Release();
        if (device_context) device_context->Release();
        if (device) device->Release();
        DestroyWindow(window);
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ID3D11Texture2D* back_buffer = nullptr;
    hr = swap_chain->GetBuffer(0U, IID_PPV_ARGS(&back_buffer));
    if (FAILED(hr) || !back_buffer)
    {
        std::cerr << "[!] swap_chain->GetBuffer failed. HRESULT=0x" << std::hex << (unsigned)hr << std::dec << "\n";
        swap_chain->Release();
        device_context->Release();
        device->Release();
        DestroyWindow(window);
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    hr = device->CreateRenderTargetView(back_buffer, nullptr, &render_target_view);
    back_buffer->Release();

    if (FAILED(hr) || !render_target_view)
    {
        std::cerr << "[!] CreateRenderTargetView failed. HRESULT=0x" << std::hex << (unsigned)hr << std::dec << "\n";
        swap_chain->Release();
        device_context->Release();
        device->Release();
        DestroyWindow(window);
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ShowWindow(window, cmd_show);
    UpdateWindow(window);

    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    if (!ImGui_ImplWin32_Init(window))
    {
        std::cerr << "[!] ImGui_ImplWin32_Init failed\n";
        ImGui::DestroyContext();
        render_target_view->Release();
        swap_chain->Release();
        device_context->Release();
        device->Release();
        DestroyWindow(window);
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    if (!ImGui_ImplDX11_Init(device, device_context))
    {
        std::cerr << "[!] ImGui_ImplDX11_Init failed\n";
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        render_target_view->Release();
        swap_chain->Release();
        device_context->Release();
        device->Release();
        DestroyWindow(window);
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    std::cout << "\n[+] External Overlay created successfully\n";

    bool running = true;
    while (running)
    {
        MSG msg{};
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
                running = false;
        }

        if (!running)
            break;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Example draw
        ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(800.0f, 800.0f), 50.0f, ImColor(1.f, 0.f, 0.f));

        ImGui::Render();

        constexpr float color[4]{ 0.f, 0.f, 0.f, 0.f };
        device_context->OMSetRenderTargets(1U, &render_target_view, nullptr);
        device_context->ClearRenderTargetView(render_target_view, color);

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        swap_chain->Present(1U, 0U);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    render_target_view->Release();
    swap_chain->Release();
    device_context->Release();
    device->Release();

    DestroyWindow(window);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}