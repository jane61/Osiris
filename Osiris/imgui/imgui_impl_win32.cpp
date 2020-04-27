// dear imgui: Platform Binding for Windows (standard windows API for 32 and 64 bits applications)
// This needs to be used along with a Renderer (e.g. DirectX11, OpenGL3, Vulkan..)

// Implemented features:
//  [X] Platform: Clipboard support (for Win32 this is actually part of core dear imgui)
//  [X] Platform: Mouse cursor shape and visibility. Disable with 'io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange'.
//  [X] Platform: Keyboard arrays indexed using VK_* Virtual Key Codes, e.g. ImGui::IsKeyPressed(VK_SPACE).
//  [X] Platform: Gamepad support. Enabled with 'io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad'.

#include "imgui.h"
#include "imgui_impl_win32.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <tchar.h>

// Using XInput library for gamepad (with recent Windows SDK this may leads to executables which won't run on Windows 7)
#ifndef IMGUI_IMPL_WIN32_DISABLE_GAMEPAD
#include <XInput.h>
#else
#define IMGUI_IMPL_WIN32_DISABLE_LINKING_XINPUT
#endif
#if defined(_MSC_VER) && !defined(IMGUI_IMPL_WIN32_DISABLE_LINKING_XINPUT)
#pragma comment(lib, "xinput")
//#pragma comment(lib, "Xinput9_1_0")
#endif

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2020-03-03: Inputs: Calling AddInputCharacterUTF16() to support surrogate pairs leading to codepoint >= 0x10000 (for more complete CJK inputs)
//  2020-02-17: Added ImGui_ImplWin32_EnableDpiAwareness(), ImGui_ImplWin32_GetDpiScaleForHwnd(), ImGui_ImplWin32_GetDpiScaleForMonitor() helper functions.
//  2020-01-14: Inputs: Added support for #define IMGUI_IMPL_WIN32_DISABLE_GAMEPAD/IMGUI_IMPL_WIN32_DISABLE_LINKING_XINPUT.
//  2019-12-05: Inputs: Added support for ImGuiMouseCursor_NotAllowed mouse cursor.
//  2019-05-11: Inputs: Don't filter value from WM_CHAR before calling AddInputCharacter().
//  2019-01-17: Misc: Using GetForegroundWindow()+IsChild() instead of GetActiveWindow() to be compatible with windows created in a different thread or parent.
//  2019-01-17: Inputs: Added support for mouse buttons 4 and 5 via WM_XBUTTON* messages.
//  2019-01-15: Inputs: Added support for XInput gamepads (if ImGuiConfigFlags_NavEnableGamepad is set by user application).
//  2018-11-30: Misc: Setting up io.BackendPlatformName so it can be displayed in the About Window.
//  2018-06-29: Inputs: Added support for the ImGuiMouseCursor_Hand cursor.
//  2018-06-10: Inputs: Fixed handling of mouse wheel messages to support fine position messages (typically sent by track-pads).
//  2018-06-08: Misc: Extracted imgui_impl_win32.cpp/.h away from the old combined DX9/DX10/DX11/DX12 examples.
//  2018-03-20: Misc: Setup io.BackendFlags ImGuiBackendFlags_HasMouseCursors and ImGuiBackendFlags_HasSetMousePos flags + honor ImGuiConfigFlags_NoMouseCursorChange flag.
//  2018-02-20: Inputs: Added support for mouse cursors (ImGui::GetMouseCursor() value and WM_SETCURSOR message handling).
//  2018-02-06: Inputs: Added mapping for ImGuiKey_Space.
//  2018-02-06: Inputs: Honoring the io.WantSetMousePos by repositioning the mouse (when using navigation and ImGuiConfigFlags_NavMoveMouse is set).
//  2018-02-06: Misc: Removed call to ImGui::Shutdown() which is not available from 1.60 WIP, user needs to call CreateContext/DestroyContext themselves.
//  2018-01-20: Inputs: Added Horizontal Mouse Wheel support.
//  2018-01-08: Inputs: Added mapping for ImGuiKey_Insert.
//  2018-01-05: Inputs: Added WM_LBUTTONDBLCLK double-click handlers for window classes with the CS_DBLCLKS flag.
//  2017-10-23: Inputs: Added WM_SYSKEYDOWN / WM_SYSKEYUP handlers so e.g. the VK_MENU key can be read.
//  2017-10-23: Inputs: Using Win32 ::SetCapture/::GetCapture() to retrieve mouse positions outside the client area when dragging.
//  2016-11-12: Inputs: Only call Win32 ::SetCursor(NULL) when io.MouseDrawCursor is set.

// Win32 Data
static HWND                 g_hWnd = NULL;
static INT64                g_Time = 0;
static INT64                g_TicksPerSecond = 0;
static ImGuiMouseCursor     g_LastMouseCursor = ImGuiMouseCursor_COUNT;
static bool                 g_HasGamepad = false;
static bool                 g_WantUpdateHasGamepad = true;

// Functions
bool    ImGui_ImplWin32_Init(void* hwnd)
{
    if (!::QueryPerformanceFrequency((LARGE_INTEGER*)&g_TicksPerSecond))
        return false;
    if (!::QueryPerformanceCounter((LARGE_INTEGER*)&g_Time))
        return false;

    // Setup back-end capabilities flags
    g_hWnd = (HWND)hwnd;
    ImGuiIO& io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
    io.BackendPlatformName = "imgui_impl_win32";
    io.ImeWindowHandle = hwnd;

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array that we will update during the application lifetime.
    io.KeyMap[ImGuiKey_Tab] = VK_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
    io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
    io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
    io.KeyMap[ImGuiKey_Home] = VK_HOME;
    io.KeyMap[ImGuiKey_End] = VK_END;
    io.KeyMap[ImGuiKey_Insert] = VK_INSERT;
    io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
    io.KeyMap[ImGuiKey_Space] = VK_SPACE;
    io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
    io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
    io.KeyMap[ImGuiKey_KeyPadEnter] = VK_RETURN;
    io.KeyMap[ImGuiKey_A] = 'A';
    io.KeyMap[ImGuiKey_C] = 'C';
    io.KeyMap[ImGuiKey_V] = 'V';
    io.KeyMap[ImGuiKey_X] = 'X';
    io.KeyMap[ImGuiKey_Y] = 'Y';
    io.KeyMap[ImGuiKey_Z] = 'Z';

    return true;
}

void    ImGui_ImplWin32_Shutdown()
{
    g_hWnd = (HWND)0;
}

static bool ImGui_ImplWin32_UpdateMouseCursor()
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
        return false;

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
    {
        // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
        ::SetCursor(NULL);
    } else
    {
        // Show OS mouse cursor
        LPTSTR win32_cursor = IDC_ARROW;
        switch (imgui_cursor)
        {
        case ImGuiMouseCursor_Arrow:        win32_cursor = IDC_ARROW; break;
        case ImGuiMouseCursor_TextInput:    win32_cursor = IDC_IBEAM; break;
        case ImGuiMouseCursor_ResizeAll:    win32_cursor = IDC_SIZEALL; break;
        case ImGuiMouseCursor_ResizeEW:     win32_cursor = IDC_SIZEWE; break;
        case ImGuiMouseCursor_ResizeNS:     win32_cursor = IDC_SIZENS; break;
        case ImGuiMouseCursor_ResizeNESW:   win32_cursor = IDC_SIZENESW; break;
        case ImGuiMouseCursor_ResizeNWSE:   win32_cursor = IDC_SIZENWSE; break;
        case ImGuiMouseCursor_Hand:         win32_cursor = IDC_HAND; break;
        case ImGuiMouseCursor_NotAllowed:   win32_cursor = IDC_NO; break;
        }
        ::SetCursor(::LoadCursor(NULL, win32_cursor));
    }
    return true;
}

static void ImGui_ImplWin32_UpdateMousePos()
{
    ImGuiIO& io = ImGui::GetIO();

    // Set OS mouse position if requested (rarely used, only when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
    if (io.WantSetMousePos)
    {
        POINT pos = { (int)io.MousePos.x, (int)io.MousePos.y };
        ::ClientToScreen(g_hWnd, &pos);
        ::SetCursorPos(pos.x, pos.y);
    }

    // Set mouse position
    io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    POINT pos;
    if (HWND active_window = ::GetForegroundWindow())
        if (active_window == g_hWnd || ::IsChild(active_window, g_hWnd))
            if (::GetCursorPos(&pos) && ::ScreenToClient(g_hWnd, &pos))
                io.MousePos = ImVec2((float)pos.x, (float)pos.y);
}

// Gamepad navigation mapping
static void ImGui_ImplWin32_UpdateGamepads()
{
#ifndef IMGUI_IMPL_WIN32_DISABLE_GAMEPAD
    ImGuiIO& io = ImGui::GetIO();
    memset(io.NavInputs, 0, sizeof(io.NavInputs));
    if ((io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) == 0)
        return;

    // Calling XInputGetState() every frame on disconnected gamepads is unfortunately too slow.
    // Instead we refresh gamepad availability by calling XInputGetCapabilities() _only_ after receiving WM_DEVICECHANGE.
    if (g_WantUpdateHasGamepad)
    {
        XINPUT_CAPABILITIES caps;
        g_HasGamepad = (XInputGetCapabilities(0, XINPUT_FLAG_GAMEPAD, &caps) == ERROR_SUCCESS);
        g_WantUpdateHasGamepad = false;
    }

    XINPUT_STATE xinput_state;
    io.BackendFlags &= ~ImGuiBackendFlags_HasGamepad;
    if (g_HasGamepad && XInputGetState(0, &xinput_state) == ERROR_SUCCESS)
    {
        const XINPUT_GAMEPAD& gamepad = xinput_state.Gamepad;
        io.BackendFlags |= ImGuiBackendFlags_HasGamepad;

#define MAP_BUTTON(NAV_NO, BUTTON_ENUM)     { io.NavInputs[NAV_NO] = (gamepad.wButtons & BUTTON_ENUM) ? 1.0f : 0.0f; }
#define MAP_ANALOG(NAV_NO, VALUE, V0, V1)   { float vn = (float)(VALUE - V0) / (float)(V1 - V0); if (vn > 1.0f) vn = 1.0f; if (vn > 0.0f && io.NavInputs[NAV_NO] < vn) io.NavInputs[NAV_NO] = vn; }
        MAP_BUTTON(ImGuiNavInput_Activate, XINPUT_GAMEPAD_A);              // Cross / A
        MAP_BUTTON(ImGuiNavInput_Cancel, XINPUT_GAMEPAD_B);              // Circle / B
        MAP_BUTTON(ImGuiNavInput_Menu, XINPUT_GAMEPAD_X);              // Square / X
        MAP_BUTTON(ImGuiNavInput_Input, XINPUT_GAMEPAD_Y);              // Triangle / Y
        MAP_BUTTON(ImGuiNavInput_DpadLeft, XINPUT_GAMEPAD_DPAD_LEFT);      // D-Pad Left
        MAP_BUTTON(ImGuiNavInput_DpadRight, XINPUT_GAMEPAD_DPAD_RIGHT);     // D-Pad Right
        MAP_BUTTON(ImGuiNavInput_DpadUp, XINPUT_GAMEPAD_DPAD_UP);        // D-Pad Up
        MAP_BUTTON(ImGuiNavInput_DpadDown, XINPUT_GAMEPAD_DPAD_DOWN);      // D-Pad Down
        MAP_BUTTON(ImGuiNavInput_FocusPrev, XINPUT_GAMEPAD_LEFT_SHOULDER);  // L1 / LB
        MAP_BUTTON(ImGuiNavInput_FocusNext, XINPUT_GAMEPAD_RIGHT_SHOULDER); // R1 / RB
        MAP_BUTTON(ImGuiNavInput_TweakSlow, XINPUT_GAMEPAD_LEFT_SHOULDER);  // L1 / LB
        MAP_BUTTON(ImGuiNavInput_TweakFast, XINPUT_GAMEPAD_RIGHT_SHOULDER); // R1 / RB
        MAP_ANALOG(ImGuiNavInput_LStickLeft, gamepad.sThumbLX, -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, -32768);
        MAP_ANALOG(ImGuiNavInput_LStickRight, gamepad.sThumbLX, +XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, +32767);
        MAP_ANALOG(ImGuiNavInput_LStickUp, gamepad.sThumbLY, +XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, +32767);
        MAP_ANALOG(ImGuiNavInput_LStickDown, gamepad.sThumbLY, -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, -32767);
#undef MAP_BUTTON
#undef MAP_ANALOG
    }
#endif // #ifndef IMGUI_IMPL_WIN32_DISABLE_GAMEPAD
}

void    ImGui_ImplWin32_NewFrame()
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer back-end. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

    // Setup display size (every frame to accommodate for window resizing)
    RECT rect;
    ::GetClientRect(g_hWnd, &rect);
    io.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));

    // Setup time step
    INT64 current_time;
    ::QueryPerformanceCounter((LARGE_INTEGER*)&current_time);
    io.DeltaTime = (float)(current_time - g_Time) / g_TicksPerSecond;
    g_Time = current_time;

    // Read keyboard modifiers inputs
    io.KeyCtrl = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
    io.KeyShift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;
    io.KeyAlt = (::GetKeyState(VK_MENU) & 0x8000) != 0;
    io.KeySuper = false;
    // io.KeysDown[], io.MousePos, io.MouseDown[], io.MouseWheel: filled by the WndProc handler below.

    // Update OS mouse position
    ImGui_ImplWin32_UpdateMousePos();

    // Update OS mouse cursor with the cursor requested by imgui
    ImGuiMouseCursor mouse_cursor = io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
    if (g_LastMouseCursor != mouse_cursor)
    {
        g_LastMouseCursor = mouse_cursor;
        ImGui_ImplWin32_UpdateMouseCursor();
    }

    // Update game controllers (if enabled and available)
    ImGui_ImplWin32_UpdateGamepads();
}

// Allow compilation with old Windows SDK. MinGW doesn't have default _WIN32_WINNT/WINVER versions.
#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL 0x020E
#endif
#ifndef DBT_DEVNODES_CHANGED
#define DBT_DEVNODES_CHANGED 0x0007
#endif

// Win32 message handler (process Win32 mouse/keyboard inputs, etc.)
// Call from your application's message handler.
// When implementing your own back-end, you can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if Dear ImGui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
// Generally you may always pass all inputs to Dear ImGui, and hide them from your application based on those two flags.
// PS: In this Win32 handler, we use the capture API (GetCapture/SetCapture/ReleaseCapture) to be able to read mouse coordinates when dragging mouse outside of our window bounds.
// PS: We treat DBLCLK messages as regular mouse down messages, so this code will work on windows classes that have the CS_DBLCLKS flag set. Our own example app code doesn't set this flag.
#if 0
// Copy this line into your .cpp file to forward declare the function.
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif
IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui::GetCurrentContext() == NULL)
        return 0;

    ImGuiIO& io = ImGui::GetIO();
    switch (msg)
    {
    case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
    {
        int button = 0;
        if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK) { button = 0; }
        if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONDBLCLK) { button = 1; }
        if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONDBLCLK) { button = 2; }
        if (msg == WM_XBUTTONDOWN || msg == WM_XBUTTONDBLCLK) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4; }
        if (!ImGui::IsAnyMouseDown() && ::GetCapture() == NULL)
            ::SetCapture(hwnd);
        io.MouseDown[button] = true;
        return 0;
    }
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP:
    {
        int button = 0;
        if (msg == WM_LBUTTONUP) { button = 0; }
        if (msg == WM_RBUTTONUP) { button = 1; }
        if (msg == WM_MBUTTONUP) { button = 2; }
        if (msg == WM_XBUTTONUP) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4; }
        io.MouseDown[button] = false;
        if (!ImGui::IsAnyMouseDown() && ::GetCapture() == hwnd)
            ::ReleaseCapture();
        return 0;
    }
    case WM_MOUSEWHEEL:
        io.MouseWheel += (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
        return 0;
    case WM_MOUSEHWHEEL:
        io.MouseWheelH += (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
        return 0;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (wParam < 256)
            io.KeysDown[wParam] = 1;
        return 0;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        if (wParam < 256)
            io.KeysDown[wParam] = 0;
        return 0;
    case WM_CHAR:
        // You can also use ToAscii()+GetKeyboardState() to retrieve characters.
        if (wchar_t w; MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (char*)&wParam, 1, &w, 1) == 1)
            io.AddInputCharacter((unsigned int)w);
        return 0;
    case WM_SETCURSOR:
        if (LOWORD(lParam) == HTCLIENT && ImGui_ImplWin32_UpdateMouseCursor())
            return 1;
        return 0;
    case WM_DEVICECHANGE:
        if ((UINT)wParam == DBT_DEVNODES_CHANGED)
            g_WantUpdateHasGamepad = true;
        return 0;
    case WM_ACTIVATEAPP:
        if (hwnd == g_hWnd && wParam == FALSE) {
            memset(io.MouseDown, 0, sizeof(io.MouseDown));
            memset(io.KeysDown, 0, sizeof(io.KeysDown));
        }
        return 0;
    }
    return 0;
}


//--------------------------------------------------------------------------------------------------------
// DPI-related helpers (optional)
//--------------------------------------------------------------------------------------------------------
// - Use to enable DPI awareness without having to create an application manifest.
// - Your own app may already do this via a manifest or explicit calls. This is mostly useful for our examples/ apps.
// - In theory we could call simple functions from Windows SDK such as SetProcessDPIAware(), SetProcessDpiAwareness(), etc.
//   but most of the functions provided by Microsoft require Windows 8.1/10+ SDK at compile time and Windows 8/10+ at runtime,
//   neither we want to require the user to have. So we dynamically select and load those functions to avoid dependencies.
//---------------------------------------------------------------------------------------------------------
// This is the scheme successfully used by GLFW (from which we borrowed some of the code) and other apps aiming to be highly portable.
// ImGui_ImplWin32_EnableDpiAwareness() is just a helper called by main.cpp, we don't call it automatically.
// If you are trying to implement your own back-end for your own engine, you may ignore that noise.
//---------------------------------------------------------------------------------------------------------

// Implement some of the functions and types normally declared in recent Windows SDK.
#if !defined(_versionhelpers_H_INCLUDED_) && !defined(_INC_VERSIONHELPERS)
static BOOL IsWindowsVersionOrGreater(WORD major, WORD minor, WORD sp)
{
    OSVERSIONINFOEXW osvi = { sizeof(osvi), major, minor, 0, 0, { 0 }, sp };
    DWORD mask = VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR;
    ULONGLONG cond = ::VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL);
    cond = ::VerSetConditionMask(cond, VER_MINORVERSION, VER_GREATER_EQUAL);
    cond = ::VerSetConditionMask(cond, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);
    return ::VerifyVersionInfoW(&osvi, mask, cond);
}
#define IsWindows8Point1OrGreater()  IsWindowsVersionOrGreater(HIBYTE(0x0602), LOBYTE(0x0602), 0) // _WIN32_WINNT_WINBLUE
#endif

#ifndef DPI_ENUMS_DECLARED
typedef enum { PROCESS_DPI_UNAWARE = 0, PROCESS_SYSTEM_DPI_AWARE = 1, PROCESS_PER_MONITOR_DPI_AWARE = 2 } PROCESS_DPI_AWARENESS;
typedef enum { MDT_EFFECTIVE_DPI = 0, MDT_ANGULAR_DPI = 1, MDT_RAW_DPI = 2, MDT_DEFAULT = MDT_EFFECTIVE_DPI } MONITOR_DPI_TYPE;
#endif
#ifndef _DPI_AWARENESS_CONTEXTS_
DECLARE_HANDLE(DPI_AWARENESS_CONTEXT);
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE    (DPI_AWARENESS_CONTEXT)-3
#endif
#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 (DPI_AWARENESS_CONTEXT)-4
#endif
typedef HRESULT(WINAPI* PFN_SetProcessDpiAwareness)(PROCESS_DPI_AWARENESS);                     // Shcore.lib + dll, Windows 8.1+
typedef HRESULT(WINAPI* PFN_GetDpiForMonitor)(HMONITOR, MONITOR_DPI_TYPE, UINT*, UINT*);        // Shcore.lib + dll, Windows 8.1+
typedef DPI_AWARENESS_CONTEXT(WINAPI* PFN_SetThreadDpiAwarenessContext)(DPI_AWARENESS_CONTEXT); // User32.lib + dll, Windows 10 v1607+ (Creators Update)

// Helper function to enable DPI awareness without setting up a manifest
void ImGui_ImplWin32_EnableDpiAwareness()
{
    // if (IsWindows10OrGreater()) // This needs a manifest to succeed. Instead we try to grab the function pointer!
    {
        static HINSTANCE user32_dll = ::LoadLibraryA("user32.dll"); // Reference counted per-process
        if (PFN_SetThreadDpiAwarenessContext SetThreadDpiAwarenessContextFn = (PFN_SetThreadDpiAwarenessContext)::GetProcAddress(user32_dll, "SetThreadDpiAwarenessContext"))
        {
            SetThreadDpiAwarenessContextFn(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
            return;
        }
    }
    if (IsWindows8Point1OrGreater())
    {
        static HINSTANCE shcore_dll = ::LoadLibraryA("shcore.dll"); // Reference counted per-process
        if (PFN_SetProcessDpiAwareness SetProcessDpiAwarenessFn = (PFN_SetProcessDpiAwareness)::GetProcAddress(shcore_dll, "SetProcessDpiAwareness"))
        {
            SetProcessDpiAwarenessFn(PROCESS_PER_MONITOR_DPI_AWARE);
            return;
        }
    }
    SetProcessDPIAware();
}

#if defined(_MSC_VER) && !defined(NOGDI)
#pragma comment(lib, "gdi32")   // Link with gdi32.lib for GetDeviceCaps()
#endif

float ImGui_ImplWin32_GetDpiScaleForMonitor(void* monitor)
{
    UINT xdpi = 96, ydpi = 96;
    if (IsWindows8Point1OrGreater())
    {
        static HINSTANCE shcore_dll = ::LoadLibraryA("shcore.dll"); // Reference counted per-process
        if (PFN_GetDpiForMonitor GetDpiForMonitorFn = (PFN_GetDpiForMonitor)::GetProcAddress(shcore_dll, "GetDpiForMonitor"))
            GetDpiForMonitorFn((HMONITOR)monitor, MDT_EFFECTIVE_DPI, &xdpi, &ydpi);
    }
#ifndef NOGDI
    else
    {
        const HDC dc = ::GetDC(NULL);
        xdpi = ::GetDeviceCaps(dc, LOGPIXELSX);
        ydpi = ::GetDeviceCaps(dc, LOGPIXELSY);
        ::ReleaseDC(NULL, dc);
    }
#endif
    IM_ASSERT(xdpi == ydpi); // Please contact me if you hit this assert!
    return xdpi / 96.0f;
}

float ImGui_ImplWin32_GetDpiScaleForHwnd(void* hwnd)
{
    HMONITOR monitor = ::MonitorFromWindow((HWND)hwnd, MONITOR_DEFAULTTONEAREST);
    return ImGui_ImplWin32_GetDpiScaleForMonitor(monitor);
}

//---------------------------------------------------------------------------------------------------------

// Junk Code By Peatreat & Thaisen's Gen
void jTYNmfhxOHCMYgTWwONQSomvYyxkmntuccXNaYXFiueJDialMpjjXKsugdoohwKbOIjwJGwXqH67018308() {     float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu35190797 = -693845635;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu88383709 = 64592004;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu74987698 = -24208523;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu52109408 = -304163576;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu57975026 = -927518414;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu84510092 = -491306509;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu61385998 = -242474241;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu16409030 = -999950461;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu4684293 = -372896146;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu17042088 = -17181036;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu78205791 = -735077797;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu74876472 = -903065849;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu8931050 = 71939308;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu21538019 = -241018052;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu97130494 = -94573412;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu12743407 = -248044895;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu41806201 = -736045718;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu10597316 = -853089836;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu8887138 = -468456819;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu75969525 = 61529165;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu71692486 = -29776443;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu62006645 = -155966612;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu45291663 = -351002492;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu64141853 = -253811597;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu114414 = -668821712;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu29373555 = 31229688;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu56476721 = -297842800;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu46360397 = -218912177;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu78732826 = -140116221;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu14714444 = -217806496;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu82953664 = -142648957;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu9760331 = -712197839;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu85590908 = -369078923;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu22628549 = -657930337;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu13443431 = -393191377;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu5057712 = -455402410;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu64920549 = -219379496;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu14984362 = -694509094;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu34095402 = -804953518;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu79374628 = 49431660;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu59943313 = -244082268;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu65713843 = -117373038;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu12906331 = -388996049;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu11011813 = -582488969;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu50745035 = -822070135;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu17117779 = -415311834;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu83083591 = -377117566;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu96802904 = 33901089;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu55330639 = -744993358;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu54412092 = -87201285;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu73066703 = -769859452;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu45367892 = -838496129;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu29517013 = -179241879;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu73035653 = -115371885;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu95325822 = 35071770;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu73184151 = -437879023;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu43092046 = -584405504;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu10845846 = -770396927;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu51994994 = -635341864;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu28601471 = -858748102;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu28033372 = -93463710;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu15025602 = 76437935;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu37676203 = -759834240;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu89969848 = -55089650;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu34088424 = -874532080;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu68445460 = 77120041;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu89285563 = -433986926;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu86302500 = -270130355;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu8094589 = -847826676;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu92072782 = -639171002;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu47822857 = 71334600;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu26821840 = 58463375;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu76501914 = 51863682;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu29512509 = -417888480;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu16026213 = -694388568;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu5978644 = -912403405;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu49100315 = -766970564;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu34279851 = -768513524;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu13396818 = -431741462;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu82996635 = -153509879;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu46289963 = -591652747;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu59673816 = -231743889;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu91029757 = -473918820;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu24320734 = 47085063;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu41647741 = -447947045;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu37585772 = -304152828;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu80243318 = -432955960;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu12555255 = -153707038;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu27302727 = -593002107;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu40259279 = -955312354;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu61965665 = -870996907;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu54074704 = -448982569;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu62989367 = 40832770;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu5493931 = -946205416;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu51341256 = -857104631;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu44917712 = -220520204;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu28037640 = -357538798;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu22936482 = -233906400;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu76923389 = -707956889;    float hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu82299575 = -693845635;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu35190797 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu88383709;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu88383709 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu74987698;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu74987698 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu52109408;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu52109408 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu57975026;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu57975026 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu84510092;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu84510092 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu61385998;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu61385998 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu16409030;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu16409030 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu4684293;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu4684293 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu17042088;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu17042088 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu78205791;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu78205791 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu74876472;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu74876472 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu8931050;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu8931050 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu21538019;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu21538019 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu97130494;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu97130494 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu12743407;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu12743407 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu41806201;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu41806201 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu10597316;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu10597316 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu8887138;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu8887138 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu75969525;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu75969525 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu71692486;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu71692486 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu62006645;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu62006645 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu45291663;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu45291663 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu64141853;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu64141853 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu114414;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu114414 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu29373555;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu29373555 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu56476721;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu56476721 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu46360397;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu46360397 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu78732826;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu78732826 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu14714444;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu14714444 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu82953664;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu82953664 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu9760331;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu9760331 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu85590908;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu85590908 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu22628549;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu22628549 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu13443431;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu13443431 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu5057712;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu5057712 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu64920549;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu64920549 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu14984362;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu14984362 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu34095402;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu34095402 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu79374628;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu79374628 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu59943313;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu59943313 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu65713843;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu65713843 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu12906331;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu12906331 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu11011813;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu11011813 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu50745035;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu50745035 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu17117779;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu17117779 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu83083591;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu83083591 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu96802904;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu96802904 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu55330639;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu55330639 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu54412092;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu54412092 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu73066703;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu73066703 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu45367892;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu45367892 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu29517013;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu29517013 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu73035653;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu73035653 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu95325822;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu95325822 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu73184151;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu73184151 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu43092046;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu43092046 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu10845846;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu10845846 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu51994994;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu51994994 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu28601471;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu28601471 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu28033372;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu28033372 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu15025602;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu15025602 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu37676203;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu37676203 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu89969848;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu89969848 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu34088424;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu34088424 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu68445460;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu68445460 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu89285563;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu89285563 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu86302500;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu86302500 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu8094589;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu8094589 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu92072782;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu92072782 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu47822857;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu47822857 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu26821840;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu26821840 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu76501914;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu76501914 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu29512509;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu29512509 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu16026213;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu16026213 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu5978644;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu5978644 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu49100315;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu49100315 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu34279851;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu34279851 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu13396818;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu13396818 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu82996635;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu82996635 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu46289963;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu46289963 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu59673816;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu59673816 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu91029757;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu91029757 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu24320734;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu24320734 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu41647741;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu41647741 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu37585772;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu37585772 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu80243318;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu80243318 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu12555255;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu12555255 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu27302727;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu27302727 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu40259279;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu40259279 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu61965665;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu61965665 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu54074704;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu54074704 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu62989367;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu62989367 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu5493931;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu5493931 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu51341256;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu51341256 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu44917712;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu44917712 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu28037640;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu28037640 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu22936482;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu22936482 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu76923389;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu76923389 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu82299575;     hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu82299575 = hXJdspXMYxEKWSkcgxjjXmkDuvzecGtLsQdUkDkzRVYNWElRMgZznHjvnGixpskRDsRGoPsdMkRCNXgqJiSvhu35190797;}
// Junk Finished

// Junk Code By Peatreat & Thaisen's Gen
void IndLMWjaQYWuEHIUaprUiEzqQVFUqCoXBOEQFMtxqhzzOWpIgKTJZBqEomurRPjOjysqDrOOvt76308494() {     float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz66877002 = -462090978;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz74584923 = -6973366;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz77192087 = -420324515;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz87605770 = -603587175;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz18312775 = -465978876;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz79950152 = -159365776;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz48564148 = -775051592;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz54214171 = -927949546;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz1506491 = -471268031;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz69028913 = -322735215;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz3451149 = -821623450;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz77746363 = -29876864;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz51353231 = -736346332;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz80851963 = -510663542;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz62072081 = -580363666;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz98381211 = -939225042;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz75072219 = -589947050;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz36267705 = -377568894;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz33252375 = 97844560;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz97865564 = -699874258;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz60428505 = -723129614;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz95771440 = -108608966;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz76842244 = -739006690;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz58973540 = -37266871;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz19823927 = -26248520;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz75019559 = -282390998;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz62878362 = -516317419;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz11875135 = -936920834;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz84438411 = -563968563;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz76877246 = -232765360;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz80774545 = -802350012;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz6370529 = -574994892;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz30119967 = -664373970;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz71753277 = -707321405;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz82218165 = -181645205;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz85645949 = -836437599;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz75681289 = -352660755;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz99441459 = -302886626;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz36913226 = -288918877;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz5342472 = -467557097;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz82402987 = -554276098;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz6118043 = -281034026;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz40881928 = -796232650;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz67292578 = 37341386;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz64723507 = 43862089;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz96527216 = 96812835;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz69875101 = -631563520;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz51331569 = -41080681;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz12463662 = -389816226;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz71247298 = -89797462;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz52052053 = -907199244;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz19804335 = -122665999;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz34459945 = -819065036;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz63529120 = -430705440;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz15879990 = -532798624;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz71105562 = -253482012;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz97742679 = -267966677;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz18218548 = -283057645;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz67781843 = -477338655;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz43293216 = -83587879;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz17071790 = -643048358;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz36689014 = -838130759;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz69775759 = -263980983;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz24629245 = -138502671;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz88254368 = -520385203;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz97080619 = -146628558;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz47626396 = -365502894;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz79599953 = 70975072;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz98633797 = -229018338;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz76426132 = -743926067;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz22699923 = -486564287;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz75630760 = -187060424;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz99354479 = 11349983;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz27909903 = -434598344;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz15462577 = -45598161;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz54310463 = -342095588;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz54889512 = -312376317;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz9549667 = -676348076;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz94250032 = 18871040;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz23296711 = -23061356;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz5144458 = -650827479;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz11546794 = -375236739;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz99411472 = -447104609;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz13191114 = -374171102;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz24825194 = -325566117;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz60970210 = -579684013;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz71910584 = -755929856;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz66590847 = -133668531;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz55873288 = -74522781;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz11112604 = -928163194;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz87903270 = -468470922;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz57462741 = 30396889;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz31659616 = -825547971;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz93620010 = -105330999;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz88270681 = -824508740;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz45713973 = -716145339;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz36342283 = 82946957;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz16252684 = -557729979;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz79038209 = -442273411;    float dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz67642887 = -462090978;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz66877002 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz74584923;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz74584923 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz77192087;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz77192087 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz87605770;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz87605770 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz18312775;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz18312775 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz79950152;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz79950152 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz48564148;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz48564148 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz54214171;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz54214171 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz1506491;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz1506491 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz69028913;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz69028913 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz3451149;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz3451149 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz77746363;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz77746363 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz51353231;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz51353231 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz80851963;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz80851963 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz62072081;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz62072081 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz98381211;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz98381211 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz75072219;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz75072219 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz36267705;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz36267705 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz33252375;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz33252375 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz97865564;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz97865564 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz60428505;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz60428505 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz95771440;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz95771440 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz76842244;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz76842244 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz58973540;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz58973540 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz19823927;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz19823927 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz75019559;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz75019559 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz62878362;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz62878362 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz11875135;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz11875135 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz84438411;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz84438411 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz76877246;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz76877246 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz80774545;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz80774545 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz6370529;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz6370529 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz30119967;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz30119967 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz71753277;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz71753277 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz82218165;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz82218165 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz85645949;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz85645949 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz75681289;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz75681289 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz99441459;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz99441459 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz36913226;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz36913226 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz5342472;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz5342472 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz82402987;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz82402987 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz6118043;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz6118043 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz40881928;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz40881928 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz67292578;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz67292578 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz64723507;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz64723507 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz96527216;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz96527216 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz69875101;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz69875101 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz51331569;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz51331569 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz12463662;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz12463662 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz71247298;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz71247298 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz52052053;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz52052053 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz19804335;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz19804335 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz34459945;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz34459945 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz63529120;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz63529120 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz15879990;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz15879990 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz71105562;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz71105562 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz97742679;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz97742679 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz18218548;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz18218548 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz67781843;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz67781843 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz43293216;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz43293216 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz17071790;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz17071790 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz36689014;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz36689014 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz69775759;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz69775759 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz24629245;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz24629245 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz88254368;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz88254368 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz97080619;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz97080619 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz47626396;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz47626396 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz79599953;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz79599953 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz98633797;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz98633797 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz76426132;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz76426132 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz22699923;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz22699923 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz75630760;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz75630760 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz99354479;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz99354479 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz27909903;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz27909903 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz15462577;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz15462577 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz54310463;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz54310463 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz54889512;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz54889512 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz9549667;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz9549667 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz94250032;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz94250032 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz23296711;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz23296711 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz5144458;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz5144458 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz11546794;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz11546794 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz99411472;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz99411472 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz13191114;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz13191114 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz24825194;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz24825194 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz60970210;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz60970210 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz71910584;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz71910584 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz66590847;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz66590847 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz55873288;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz55873288 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz11112604;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz11112604 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz87903270;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz87903270 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz57462741;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz57462741 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz31659616;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz31659616 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz93620010;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz93620010 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz88270681;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz88270681 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz45713973;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz45713973 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz36342283;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz36342283 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz16252684;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz16252684 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz79038209;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz79038209 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz67642887;     dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz67642887 = dBEzcugwogifyzXAbSRCgbxlQPdzFovENfXozpKAKVaDfDAtSCQwwtNCESiNQhqzkdRTcjBmIQrLAtGNPkgDXz66877002;}
// Junk Finished
