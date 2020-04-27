// dear imgui, v1.77 WIP
// (demo code)

// Help:
// - Read FAQ at http://dearimgui.org/faq
// - Newcomers, read 'Programmer guide' in imgui.cpp for notes on how to setup Dear ImGui in your codebase.
// - Call and read ImGui::ShowDemoWindow() in imgui_demo.cpp for demo code. All applications in examples/ are doing that.
// Read imgui.cpp for more details, documentation and comments.
// Get latest version at https://github.com/ocornut/imgui

// Message to the person tempted to delete this file when integrating Dear ImGui into their code base:
// Do NOT remove this file from your project! Think again! It is the most useful reference code that you and other coders
// will want to refer to and call. Have the ImGui::ShowDemoWindow() function wired in an always-available debug menu of
// your game/app! Removing this file from your project is hindering access to documentation for everyone in your team,
// likely leading you to poorer usage of the library.
// Everything in this file will be stripped out by the linker if you don't call ImGui::ShowDemoWindow().
// If you want to link core Dear ImGui in your shipped builds but want a thorough guarantee that the demo will not be linked,
// you can setup your imconfig.h with #define IMGUI_DISABLE_DEMO_WINDOWS and those functions will be empty.
// In other situation, whenever you have Dear ImGui available you probably want this to be available for reference.
// Thank you,
// -Your beloved friend, imgui_demo.cpp (which you won't delete)

// Message to beginner C/C++ programmers about the meaning of the 'static' keyword:
// In this demo code, we frequently we use 'static' variables inside functions. A static variable persist across calls, so it is
// essentially like a global variable but declared inside the scope of the function. We do this as a way to gather code and data
// in the same place, to make the demo source code faster to read, faster to write, and smaller in size.
// It also happens to be a convenient way of storing simple UI related information as long as your function doesn't need to be
// reentrant or used in multiple threads. This might be a pattern you will want to use in your code, but most of the real data
// you would be editing is likely going to be stored outside your functions.

// The Demo code in this file is designed to be easy to copy-and-paste in into your application!
// Because of this:
// - We never omit the ImGui:: namespace when calling functions, even though most of our code is already in the same namespace.
// - We try to declare static variables in the local scope, as close as possible to the code using them.
// - We never use any of the helpers/facilities used internally by Dear ImGui, unless it has been exposed in the public API (imgui.h).
// - We never use maths operators on ImVec2/ImVec4. For other of our sources files, they are provided by imgui_internal.h w/ IMGUI_DEFINE_MATH_OPERATORS.
//   For your own sources file they are optional and require you either enable those, either provide your own via IM_VEC2_CLASS_EXTRA in imconfig.h.
//   Because we don't want to assume anything about your support of maths operators, we don't use them in imgui_demo.cpp.

/*

Index of this file:

// [SECTION] Forward Declarations, Helpers
// [SECTION] Demo Window / ShowDemoWindow()
// [SECTION] About Window / ShowAboutWindow()
// [SECTION] Style Editor / ShowStyleEditor()
// [SECTION] Example App: Main Menu Bar / ShowExampleAppMainMenuBar()
// [SECTION] Example App: Debug Console / ShowExampleAppConsole()
// [SECTION] Example App: Debug Log / ShowExampleAppLog()
// [SECTION] Example App: Simple Layout / ShowExampleAppLayout()
// [SECTION] Example App: Property Editor / ShowExampleAppPropertyEditor()
// [SECTION] Example App: Long Text / ShowExampleAppLongText()
// [SECTION] Example App: Auto Resize / ShowExampleAppAutoResize()
// [SECTION] Example App: Constrained Resize / ShowExampleAppConstrainedResize()
// [SECTION] Example App: Simple Overlay / ShowExampleAppSimpleOverlay()
// [SECTION] Example App: Manipulating Window Titles / ShowExampleAppWindowTitles()
// [SECTION] Example App: Custom Rendering using ImDrawList API / ShowExampleAppCustomRendering()
// [SECTION] Example App: Documents Handling / ShowExampleAppDocuments()

*/

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "imgui.h"
#ifndef IMGUI_DISABLE

#include <ctype.h>          // toupper
#include <limits.h>         // INT_MIN, INT_MAX
#include <math.h>           // sqrtf, powf, cosf, sinf, floorf, ceilf
#include <stdio.h>          // vsnprintf, sscanf, printf
#include <stdlib.h>         // NULL, malloc, free, atoi
#if defined(_MSC_VER) && _MSC_VER <= 1500 // MSVC 2008 or earlier
#include <stddef.h>         // intptr_t
#else
#include <stdint.h>         // intptr_t
#endif

#ifdef _MSC_VER
#pragma warning (disable: 4996) // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#endif
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wold-style-cast"             // warning : use of old-style cast                              // yes, they are more terse.
#pragma clang diagnostic ignored "-Wdeprecated-declarations"    // warning : 'xx' is deprecated: The POSIX name for this item.. // for strdup used in demo code (so user can copy & paste the code)
#pragma clang diagnostic ignored "-Wint-to-void-pointer-cast"   // warning : cast to 'void *' from smaller integer type 'int'
#pragma clang diagnostic ignored "-Wformat-security"            // warning : warning: format string is not a string literal
#pragma clang diagnostic ignored "-Wexit-time-destructors"      // warning : declaration requires an exit-time destructor       // exit-time destruction order is undefined. if MemFree() leads to users code that has been disabled before exit it might cause problems. ImGui coding style welcomes static/globals.
#pragma clang diagnostic ignored "-Wunused-macros"              // warning : warning: macro is not used                         // we define snprintf/vsnprintf on Windows so they are available, but not always used.
#if __has_warning("-Wzero-as-null-pointer-constant")
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"  // warning : zero as null pointer constant                  // some standard header variations use #define NULL 0
#endif
#if __has_warning("-Wdouble-promotion")
#pragma clang diagnostic ignored "-Wdouble-promotion"           // warning: implicit conversion from 'float' to 'double' when passing argument to function  // using printf() is a misery with this as C++ va_arg ellipsis changes float to double.
#endif
#if __has_warning("-Wreserved-id-macro")
#pragma clang diagnostic ignored "-Wreserved-id-macro"          // warning : macro name is a reserved identifier                //
#endif
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wpragmas"                      // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"          // warning: cast to pointer from integer of different size
#pragma GCC diagnostic ignored "-Wformat-security"              // warning : format string is not a string literal (potentially insecure)
#pragma GCC diagnostic ignored "-Wdouble-promotion"             // warning: implicit conversion from 'float' to 'double' when passing argument to function
#pragma GCC diagnostic ignored "-Wconversion"                   // warning: conversion to 'xxxx' from 'xxxx' may alter its value
#pragma GCC diagnostic ignored "-Wmisleading-indentation"       // [__GNUC__ >= 6] warning: this 'if' clause does not guard this statement      // GCC 6.0+ only. See #883 on GitHub.
#endif

// Play it nice with Windows users (Update: since 2018-05, Notepad finally appears to support Unix-style carriage returns!)
#ifdef _WIN32
#define IM_NEWLINE  "\r\n"
#else
#define IM_NEWLINE  "\n"
#endif

#if defined(_MSC_VER) && !defined(snprintf)
#define snprintf    _snprintf
#endif
#if defined(_MSC_VER) && !defined(vsnprintf)
#define vsnprintf   _vsnprintf
#endif

//-----------------------------------------------------------------------------
// [SECTION] Forward Declarations, Helpers
//-----------------------------------------------------------------------------

#if !defined(IMGUI_DISABLE_DEMO_WINDOWS)

// Forward Declarations
static void ShowExampleAppDocuments(bool* p_open);
static void ShowExampleAppMainMenuBar();
static void ShowExampleAppConsole(bool* p_open);
static void ShowExampleAppLog(bool* p_open);
static void ShowExampleAppLayout(bool* p_open);
static void ShowExampleAppPropertyEditor(bool* p_open);
static void ShowExampleAppLongText(bool* p_open);
static void ShowExampleAppAutoResize(bool* p_open);
static void ShowExampleAppConstrainedResize(bool* p_open);
static void ShowExampleAppSimpleOverlay(bool* p_open);
static void ShowExampleAppWindowTitles(bool* p_open);
static void ShowExampleAppCustomRendering(bool* p_open);
static void ShowExampleMenuFile();

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.txt)
static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

// Helper to display basic user controls.
void ImGui::ShowUserGuide()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui::BulletText("Double-click on title bar to collapse window.");
    ImGui::BulletText("Click and drag on lower corner to resize window\n(double-click to auto fit window to its contents).");
    ImGui::BulletText("CTRL+Click on a slider or drag box to input value as text.");
    ImGui::BulletText("TAB/SHIFT+TAB to cycle through keyboard editable fields.");
    if (io.FontAllowUserScaling)
        ImGui::BulletText("CTRL+Mouse Wheel to zoom window contents.");
    ImGui::BulletText("While inputing text:\n");
    ImGui::Indent();
    ImGui::BulletText("CTRL+Left/Right to word jump.");
    ImGui::BulletText("CTRL+A or double-click to select all.");
    ImGui::BulletText("CTRL+X/C/V to use clipboard cut/copy/paste.");
    ImGui::BulletText("CTRL+Z,CTRL+Y to undo/redo.");
    ImGui::BulletText("ESCAPE to revert.");
    ImGui::BulletText("You can apply arithmetic operators +,*,/ on numerical values.\nUse +- to subtract.");
    ImGui::Unindent();
    ImGui::BulletText("With keyboard navigation enabled:");
    ImGui::Indent();
    ImGui::BulletText("Arrow keys to navigate.");
    ImGui::BulletText("Space to activate a widget.");
    ImGui::BulletText("Return to input text into a widget.");
    ImGui::BulletText("Escape to deactivate a widget, close popup, exit child window.");
    ImGui::BulletText("Alt to jump to the menu layer of a window.");
    ImGui::BulletText("CTRL+Tab to select a window.");
    ImGui::Unindent();
}

//-----------------------------------------------------------------------------
// [SECTION] Demo Window / ShowDemoWindow()
//-----------------------------------------------------------------------------
// - ShowDemoWindowWidgets()
// - ShowDemoWindowLayout()
// - ShowDemoWindowPopups()
// - ShowDemoWindowColumns()
// - ShowDemoWindowMisc()
//-----------------------------------------------------------------------------

// We split the contents of the big ShowDemoWindow() function into smaller functions (because the link time of very large functions grow non-linearly)
static void ShowDemoWindowWidgets();
static void ShowDemoWindowLayout();
static void ShowDemoWindowPopups();
static void ShowDemoWindowColumns();
static void ShowDemoWindowMisc();

// Demonstrate most Dear ImGui features (this is big function!)
// You may execute this function to experiment with the UI and understand what it does. You may then search for keywords in the code when you are interested by a specific feature.
void ImGui::ShowDemoWindow(bool* p_open)
{
    IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing dear imgui context. Refer to examples app!"); // Exceptionally add an extra assert here for people confused with initial dear imgui setup

    // Examples Apps (accessible from the "Examples" menu)
    static bool show_app_documents = false;
    static bool show_app_main_menu_bar = false;
    static bool show_app_console = false;
    static bool show_app_log = false;
    static bool show_app_layout = false;
    static bool show_app_property_editor = false;
    static bool show_app_long_text = false;
    static bool show_app_auto_resize = false;
    static bool show_app_constrained_resize = false;
    static bool show_app_simple_overlay = false;
    static bool show_app_window_titles = false;
    static bool show_app_custom_rendering = false;

    if (show_app_documents)           ShowExampleAppDocuments(&show_app_documents);
    if (show_app_main_menu_bar)       ShowExampleAppMainMenuBar();
    if (show_app_console)             ShowExampleAppConsole(&show_app_console);
    if (show_app_log)                 ShowExampleAppLog(&show_app_log);
    if (show_app_layout)              ShowExampleAppLayout(&show_app_layout);
    if (show_app_property_editor)     ShowExampleAppPropertyEditor(&show_app_property_editor);
    if (show_app_long_text)           ShowExampleAppLongText(&show_app_long_text);
    if (show_app_auto_resize)         ShowExampleAppAutoResize(&show_app_auto_resize);
    if (show_app_constrained_resize)  ShowExampleAppConstrainedResize(&show_app_constrained_resize);
    if (show_app_simple_overlay)      ShowExampleAppSimpleOverlay(&show_app_simple_overlay);
    if (show_app_window_titles)       ShowExampleAppWindowTitles(&show_app_window_titles);
    if (show_app_custom_rendering)    ShowExampleAppCustomRendering(&show_app_custom_rendering);

    // Dear ImGui Apps (accessible from the "Tools" menu)
    static bool show_app_metrics = false;
    static bool show_app_style_editor = false;
    static bool show_app_about = false;

    if (show_app_metrics) { ImGui::ShowMetricsWindow(&show_app_metrics); }
    if (show_app_style_editor) { ImGui::Begin("Style Editor", &show_app_style_editor); ImGui::ShowStyleEditor(); ImGui::End(); }
    if (show_app_about) { ImGui::ShowAboutWindow(&show_app_about); }

    // Demonstrate the various window flags. Typically you would just use the default!
    static bool no_titlebar = false;
    static bool no_scrollbar = false;
    static bool no_menu = false;
    static bool no_move = false;
    static bool no_resize = false;
    static bool no_collapse = false;
    static bool no_close = false;
    static bool no_nav = false;
    static bool no_background = false;
    static bool no_bring_to_front = false;

    ImGuiWindowFlags window_flags = 0;
    if (no_titlebar)        window_flags |= ImGuiWindowFlags_NoTitleBar;
    if (no_scrollbar)       window_flags |= ImGuiWindowFlags_NoScrollbar;
    if (!no_menu)           window_flags |= ImGuiWindowFlags_MenuBar;
    if (no_move)            window_flags |= ImGuiWindowFlags_NoMove;
    if (no_resize)          window_flags |= ImGuiWindowFlags_NoResize;
    if (no_collapse)        window_flags |= ImGuiWindowFlags_NoCollapse;
    if (no_nav)             window_flags |= ImGuiWindowFlags_NoNav;
    if (no_background)      window_flags |= ImGuiWindowFlags_NoBackground;
    if (no_bring_to_front)  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    if (no_close)           p_open = NULL; // Don't pass our bool* to Begin

    // We specify a default position/size in case there's no data in the .ini file. Typically this isn't required! We only do it to make the Demo applications a little more welcoming.
    ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);

    // Main body of the Demo window starts here.
    if (!ImGui::Begin("Dear ImGui Demo", p_open, window_flags))
    {
        // Early out if the window is collapsed, as an optimization.
        ImGui::End();
        return;
    }

    // Most "big" widgets share a common width settings by default.
    //ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.65f);    // Use 2/3 of the space for widgets and 1/3 for labels (default)
    ImGui::PushItemWidth(ImGui::GetFontSize() * -12);           // Use fixed width for labels (by passing a negative value), the rest goes to widgets. We choose a width proportional to our font size.

    // Menu Bar
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Menu"))
        {
            ShowExampleMenuFile();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Examples"))
        {
            ImGui::MenuItem("Main menu bar", NULL, &show_app_main_menu_bar);
            ImGui::MenuItem("Console", NULL, &show_app_console);
            ImGui::MenuItem("Log", NULL, &show_app_log);
            ImGui::MenuItem("Simple layout", NULL, &show_app_layout);
            ImGui::MenuItem("Property editor", NULL, &show_app_property_editor);
            ImGui::MenuItem("Long text display", NULL, &show_app_long_text);
            ImGui::MenuItem("Auto-resizing window", NULL, &show_app_auto_resize);
            ImGui::MenuItem("Constrained-resizing window", NULL, &show_app_constrained_resize);
            ImGui::MenuItem("Simple overlay", NULL, &show_app_simple_overlay);
            ImGui::MenuItem("Manipulating window titles", NULL, &show_app_window_titles);
            ImGui::MenuItem("Custom rendering", NULL, &show_app_custom_rendering);
            ImGui::MenuItem("Documents", NULL, &show_app_documents);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Tools"))
        {
            ImGui::MenuItem("Metrics", NULL, &show_app_metrics);
            ImGui::MenuItem("Style Editor", NULL, &show_app_style_editor);
            ImGui::MenuItem("About Dear ImGui", NULL, &show_app_about);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::Text("dear imgui says hello. (%s)", IMGUI_VERSION);
    ImGui::Spacing();

    if (ImGui::CollapsingHeader("Help"))
    {
        ImGui::Text("ABOUT THIS DEMO:");
        ImGui::BulletText("Sections below are demonstrating many aspects of the library.");
        ImGui::BulletText("The \"Examples\" menu above leads to more demo contents.");
        ImGui::BulletText("The \"Tools\" menu above gives access to: About Box, Style Editor,\n"
            "and Metrics (general purpose Dear ImGui debugging tool).");
        ImGui::Separator();

        ImGui::Text("PROGRAMMER GUIDE:");
        ImGui::BulletText("See the ShowDemoWindow() code in imgui_demo.cpp. <- you are here!");
        ImGui::BulletText("See comments in imgui.cpp.");
        ImGui::BulletText("See example applications in the examples/ folder.");
        ImGui::BulletText("Read the FAQ at http://www.dearimgui.org/faq/");
        ImGui::BulletText("Set 'io.ConfigFlags |= NavEnableKeyboard' for keyboard controls.");
        ImGui::BulletText("Set 'io.ConfigFlags |= NavEnableGamepad' for gamepad controls.");
        ImGui::Separator();

        ImGui::Text("USER GUIDE:");
        ImGui::ShowUserGuide();
    }

    if (ImGui::CollapsingHeader("Configuration"))
    {
        ImGuiIO& io = ImGui::GetIO();

        if (ImGui::TreeNode("Configuration##2"))
        {
            ImGui::CheckboxFlags("io.ConfigFlags: NavEnableKeyboard", (unsigned int*)&io.ConfigFlags, ImGuiConfigFlags_NavEnableKeyboard);
            ImGui::CheckboxFlags("io.ConfigFlags: NavEnableGamepad", (unsigned int*)&io.ConfigFlags, ImGuiConfigFlags_NavEnableGamepad);
            ImGui::SameLine(); HelpMarker("Required back-end to feed in gamepad inputs in io.NavInputs[] and set io.BackendFlags |= ImGuiBackendFlags_HasGamepad.\n\nRead instructions in imgui.cpp for details.");
            ImGui::CheckboxFlags("io.ConfigFlags: NavEnableSetMousePos", (unsigned int*)&io.ConfigFlags, ImGuiConfigFlags_NavEnableSetMousePos);
            ImGui::SameLine(); HelpMarker("Instruct navigation to move the mouse cursor. See comment for ImGuiConfigFlags_NavEnableSetMousePos.");
            ImGui::CheckboxFlags("io.ConfigFlags: NoMouse", (unsigned int*)&io.ConfigFlags, ImGuiConfigFlags_NoMouse);
            if (io.ConfigFlags & ImGuiConfigFlags_NoMouse) // Create a way to restore this flag otherwise we could be stuck completely!
            {
                if (fmodf((float)ImGui::GetTime(), 0.40f) < 0.20f)
                {
                    ImGui::SameLine();
                    ImGui::Text("<<PRESS SPACE TO DISABLE>>");
                }
                if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Space)))
                    io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
            }
            ImGui::CheckboxFlags("io.ConfigFlags: NoMouseCursorChange", (unsigned int*)&io.ConfigFlags, ImGuiConfigFlags_NoMouseCursorChange);
            ImGui::SameLine(); HelpMarker("Instruct back-end to not alter mouse cursor shape and visibility.");
            ImGui::Checkbox("io.ConfigInputTextCursorBlink", &io.ConfigInputTextCursorBlink);
            ImGui::SameLine(); HelpMarker("Set to false to disable blinking cursor, for users who consider it distracting");
            ImGui::Checkbox("io.ConfigWindowsResizeFromEdges", &io.ConfigWindowsResizeFromEdges);
            ImGui::SameLine(); HelpMarker("Enable resizing of windows from their edges and from the lower-left corner.\nThis requires (io.BackendFlags & ImGuiBackendFlags_HasMouseCursors) because it needs mouse cursor feedback.");
            ImGui::Checkbox("io.ConfigWindowsMoveFromTitleBarOnly", &io.ConfigWindowsMoveFromTitleBarOnly);
            ImGui::Checkbox("io.MouseDrawCursor", &io.MouseDrawCursor);
            ImGui::SameLine(); HelpMarker("Instruct Dear ImGui to render a mouse cursor for you. Note that a mouse cursor rendered via your application GPU rendering path will feel more laggy than hardware cursor, but will be more in sync with your other visuals.\n\nSome desktop applications may use both kinds of cursors (e.g. enable software cursor only when resizing/dragging something).");
            ImGui::TreePop();
            ImGui::Separator();
        }

        if (ImGui::TreeNode("Backend Flags"))
        {
            HelpMarker("Those flags are set by the back-ends (imgui_impl_xxx files) to specify their capabilities.\nHere we expose then as read-only fields to avoid breaking interactions with your back-end.");
            ImGuiBackendFlags backend_flags = io.BackendFlags; // Make a local copy to avoid modifying actual back-end flags.
            ImGui::CheckboxFlags("io.BackendFlags: HasGamepad", (unsigned int*)&backend_flags, ImGuiBackendFlags_HasGamepad);
            ImGui::CheckboxFlags("io.BackendFlags: HasMouseCursors", (unsigned int*)&backend_flags, ImGuiBackendFlags_HasMouseCursors);
            ImGui::CheckboxFlags("io.BackendFlags: HasSetMousePos", (unsigned int*)&backend_flags, ImGuiBackendFlags_HasSetMousePos);
            ImGui::CheckboxFlags("io.BackendFlags: RendererHasVtxOffset", (unsigned int*)&backend_flags, ImGuiBackendFlags_RendererHasVtxOffset);
            ImGui::TreePop();
            ImGui::Separator();
        }

        if (ImGui::TreeNode("Style"))
        {
            HelpMarker("The same contents can be accessed in 'Tools->Style Editor' or by calling the ShowStyleEditor() function.");
            ImGui::ShowStyleEditor();
            ImGui::TreePop();
            ImGui::Separator();
        }

        if (ImGui::TreeNode("Capture/Logging"))
        {
            ImGui::TextWrapped("The logging API redirects all text output so you can easily capture the content of a window or a block. Tree nodes can be automatically expanded.");
            HelpMarker("Try opening any of the contents below in this window and then click one of the \"Log To\" button.");
            ImGui::LogButtons();
            ImGui::TextWrapped("You can also call ImGui::LogText() to output directly to the log without a visual output.");
            if (ImGui::Button("Copy \"Hello, world!\" to clipboard"))
            {
                ImGui::LogToClipboard();
                ImGui::LogText("Hello, world!");
                ImGui::LogFinish();
            }
            ImGui::TreePop();
        }
    }

    if (ImGui::CollapsingHeader("Window options"))
    {
        ImGui::Checkbox("No titlebar", &no_titlebar); ImGui::SameLine(150);
        ImGui::Checkbox("No scrollbar", &no_scrollbar); ImGui::SameLine(300);
        ImGui::Checkbox("No menu", &no_menu);
        ImGui::Checkbox("No move", &no_move); ImGui::SameLine(150);
        ImGui::Checkbox("No resize", &no_resize); ImGui::SameLine(300);
        ImGui::Checkbox("No collapse", &no_collapse);
        ImGui::Checkbox("No close", &no_close); ImGui::SameLine(150);
        ImGui::Checkbox("No nav", &no_nav); ImGui::SameLine(300);
        ImGui::Checkbox("No background", &no_background);
        ImGui::Checkbox("No bring to front", &no_bring_to_front);
    }

    // All demo contents
    ShowDemoWindowWidgets();
    ShowDemoWindowLayout();
    ShowDemoWindowPopups();
    ShowDemoWindowColumns();
    ShowDemoWindowMisc();

    // End of ShowDemoWindow()
    ImGui::End();
}

static void ShowDemoWindowWidgets()
{
    if (!ImGui::CollapsingHeader("Widgets"))
        return;

    if (ImGui::TreeNode("Basic"))
    {
        static int clicked = 0;
        if (ImGui::Button("Button"))
            clicked++;
        if (clicked & 1)
        {
            ImGui::SameLine();
            ImGui::Text("Thanks for clicking me!");
        }

        static bool check = true;
        ImGui::Checkbox("checkbox", &check);

        static int e = 0;
        ImGui::RadioButton("radio a", &e, 0); ImGui::SameLine();
        ImGui::RadioButton("radio b", &e, 1); ImGui::SameLine();
        ImGui::RadioButton("radio c", &e, 2);

        // Color buttons, demonstrate using PushID() to add unique identifier in the ID stack, and changing style.
        for (int i = 0; i < 7; i++)
        {
            if (i > 0)
                ImGui::SameLine();
            ImGui::PushID(i);
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(i / 7.0f, 0.6f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(i / 7.0f, 0.7f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(i / 7.0f, 0.8f, 0.8f));
            ImGui::Button("Click");
            ImGui::PopStyleColor(3);
            ImGui::PopID();
        }

        // Use AlignTextToFramePadding() to align text baseline to the baseline of framed elements (otherwise a Text+SameLine+Button sequence will have the text a little too high by default)
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Hold to repeat:");
        ImGui::SameLine();

        // Arrow buttons with Repeater
        static int counter = 0;
        float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
        ImGui::PushButtonRepeat(true);
        if (ImGui::ArrowButton("##left", ImGuiDir_Left)) { counter--; }
        ImGui::SameLine(0.0f, spacing);
        if (ImGui::ArrowButton("##right", ImGuiDir_Right)) { counter++; }
        ImGui::PopButtonRepeat();
        ImGui::SameLine();
        ImGui::Text("%d", counter);

        ImGui::Text("Hover over me");
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("I am a tooltip");

        ImGui::SameLine();
        ImGui::Text("- or me");
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::Text("I am a fancy tooltip");
            static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
            ImGui::PlotLines("Curve", arr, IM_ARRAYSIZE(arr));
            ImGui::EndTooltip();
        }

        ImGui::Separator();

        ImGui::LabelText("label", "Value");

        {
            // Using the _simplified_ one-liner Combo() api here
            // See "Combo" section for examples of how to use the more complete BeginCombo()/EndCombo() api.
            const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK", "LLLLLLL", "MMMM", "OOOOOOO" };
            static int item_current = 0;
            ImGui::Combo("combo", &item_current, items, IM_ARRAYSIZE(items));
            ImGui::SameLine(); HelpMarker("Refer to the \"Combo\" section below for an explanation of the full BeginCombo/EndCombo API, and demonstration of various flags.\n");
        }

        {
            // To wire InputText() with std::string or any other custom string type,
            // see the "Text Input > Resize Callback" section of this demo, and the misc/cpp/imgui_stdlib.h file.
            static char str0[128] = "Hello, world!";
            ImGui::InputText("input text", str0, IM_ARRAYSIZE(str0));
            ImGui::SameLine(); HelpMarker("USER:\nHold SHIFT or use mouse to select text.\n" "CTRL+Left/Right to word jump.\n" "CTRL+A or double-click to select all.\n" "CTRL+X,CTRL+C,CTRL+V clipboard.\n" "CTRL+Z,CTRL+Y undo/redo.\n" "ESCAPE to revert.\n\nPROGRAMMER:\nYou can use the ImGuiInputTextFlags_CallbackResize facility if you need to wire InputText() to a dynamic string type. See misc/cpp/imgui_stdlib.h for an example (this is not demonstrated in imgui_demo.cpp).");

            static char str1[128] = "";
            ImGui::InputTextWithHint("input text (w/ hint)", "enter text here", str1, IM_ARRAYSIZE(str1));

            static int i0 = 123;
            ImGui::InputInt("input int", &i0);
            ImGui::SameLine(); HelpMarker("You can apply arithmetic operators +,*,/ on numerical values.\n  e.g. [ 100 ], input \'*2\', result becomes [ 200 ]\nUse +- to subtract.\n");

            static float f0 = 0.001f;
            ImGui::InputFloat("input float", &f0, 0.01f, 1.0f, "%.3f");

            static double d0 = 999999.00000001;
            ImGui::InputDouble("input double", &d0, 0.01f, 1.0f, "%.8f");

            static float f1 = 1.e10f;
            ImGui::InputFloat("input scientific", &f1, 0.0f, 0.0f, "%e");
            ImGui::SameLine(); HelpMarker("You can input value using the scientific notation,\n  e.g. \"1e+8\" becomes \"100000000\".\n");

            static float vec4a[4] = { 0.10f, 0.20f, 0.30f, 0.44f };
            ImGui::InputFloat3("input float3", vec4a);
        }

        {
            static int i1 = 50, i2 = 42;
            ImGui::DragInt("drag int", &i1, 1);
            ImGui::SameLine(); HelpMarker("Click and drag to edit value.\nHold SHIFT/ALT for faster/slower edit.\nDouble-click or CTRL+click to input value.");

            ImGui::DragInt("drag int 0..100", &i2, 1, 0, 100, "%d%%");

            static float f1 = 1.00f, f2 = 0.0067f;
            ImGui::DragFloat("drag float", &f1, 0.005f);
            ImGui::DragFloat("drag small float", &f2, 0.0001f, 0.0f, 0.0f, "%.06f ns");
        }

        {
            static int i1 = 0;
            ImGui::SliderInt("slider int", &i1, -1, 3);
            ImGui::SameLine(); HelpMarker("CTRL+click to input value.");

            static float f1 = 0.123f, f2 = 0.0f;
            ImGui::SliderFloat("slider float", &f1, 0.0f, 1.0f, "ratio = %.3f");
            ImGui::SliderFloat("slider float (curve)", &f2, -10.0f, 10.0f, "%.4f", 2.0f);

            static float angle = 0.0f;
            ImGui::SliderAngle("slider angle", &angle);

            // Using the format string to display a name instead of an integer.
            // Here we completely omit '%d' from the format string, so it'll only display a name.
            // This technique can also be used with DragInt().
            enum Element { Element_Fire, Element_Earth, Element_Air, Element_Water, Element_COUNT };
            const char* element_names[Element_COUNT] = { "Fire", "Earth", "Air", "Water" };
            static int current_element = Element_Fire;
            const char* current_element_name = (current_element >= 0 && current_element < Element_COUNT) ? element_names[current_element] : "Unknown";
            ImGui::SliderInt("slider enum", &current_element, 0, Element_COUNT - 1, current_element_name);
            ImGui::SameLine(); HelpMarker("Using the format string parameter to display a name instead of the underlying integer.");
        }

        {
            static float col1[3] = { 1.0f,0.0f,0.2f };
            static float col2[4] = { 0.4f,0.7f,0.0f,0.5f };
            ImGui::ColorEdit3("color 1", col1);
            ImGui::SameLine(); HelpMarker("Click on the colored square to open a color picker.\nClick and hold to use drag and drop.\nRight-click on the colored square to show options.\nCTRL+click on individual component to input value.\n");

            ImGui::ColorEdit4("color 2", col2);
        }

        {
            // List box
            const char* listbox_items[] = { "Apple", "Banana", "Cherry", "Kiwi", "Mango", "Orange", "Pineapple", "Strawberry", "Watermelon" };
            static int listbox_item_current = 1;
            ImGui::ListBox("listbox\n(single select)", &listbox_item_current, listbox_items, IM_ARRAYSIZE(listbox_items), 4);

            //static int listbox_item_current2 = 2;
            //ImGui::SetNextItemWidth(-1);
            //ImGui::ListBox("##listbox2", &listbox_item_current2, listbox_items, IM_ARRAYSIZE(listbox_items), 4);
        }

        ImGui::TreePop();
    }

    // Testing ImGuiOnceUponAFrame helper.
    //static ImGuiOnceUponAFrame once;
    //for (int i = 0; i < 5; i++)
    //    if (once)
    //        ImGui::Text("This will be displayed only once.");

    if (ImGui::TreeNode("Trees"))
    {
        if (ImGui::TreeNode("Basic trees"))
        {
            for (int i = 0; i < 5; i++)
            {
                // Use SetNextItemOpen() so set the default state of a node to be open.
                // We could also use TreeNodeEx() with the ImGuiTreeNodeFlags_DefaultOpen flag to achieve the same thing!
                if (i == 0)
                    ImGui::SetNextItemOpen(true, ImGuiCond_Once);

                if (ImGui::TreeNode((void*)(intptr_t)i, "Child %d", i))
                {
                    ImGui::Text("blah blah");
                    ImGui::SameLine();
                    if (ImGui::SmallButton("button")) {}
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Advanced, with Selectable nodes"))
        {
            HelpMarker("This is a more typical looking tree with selectable nodes.\nClick to select, CTRL+Click to toggle, click on arrows or double-click to open.");
            static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
            static bool align_label_with_current_x_position = false;
            static bool test_drag_and_drop = false;
            ImGui::CheckboxFlags("ImGuiTreeNodeFlags_OpenOnArrow", (unsigned int*)&base_flags, ImGuiTreeNodeFlags_OpenOnArrow);
            ImGui::CheckboxFlags("ImGuiTreeNodeFlags_OpenOnDoubleClick", (unsigned int*)&base_flags, ImGuiTreeNodeFlags_OpenOnDoubleClick);
            ImGui::CheckboxFlags("ImGuiTreeNodeFlags_SpanAvailWidth", (unsigned int*)&base_flags, ImGuiTreeNodeFlags_SpanAvailWidth); ImGui::SameLine(); HelpMarker("Extend hit area to all available width instead of allowing more items to be layed out after the node.");
            ImGui::CheckboxFlags("ImGuiTreeNodeFlags_SpanFullWidth", (unsigned int*)&base_flags, ImGuiTreeNodeFlags_SpanFullWidth);
            ImGui::Checkbox("Align label with current X position", &align_label_with_current_x_position);
            ImGui::Checkbox("Test tree node as drag source", &test_drag_and_drop);
            ImGui::Text("Hello!");
            if (align_label_with_current_x_position)
                ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());

            static int selection_mask = (1 << 2); // Dumb representation of what may be user-side selection state. You may carry selection state inside or outside your objects in whatever format you see fit.
            int node_clicked = -1;                // Temporary storage of what node we have clicked to process selection at the end of the loop. May be a pointer to your own node type, etc.
            for (int i = 0; i < 6; i++)
            {
                // Disable the default open on single-click behavior and pass in Selected flag according to our selection state.
                ImGuiTreeNodeFlags node_flags = base_flags;
                const bool is_selected = (selection_mask & (1 << i)) != 0;
                if (is_selected)
                    node_flags |= ImGuiTreeNodeFlags_Selected;
                if (i < 3)
                {
                    // Items 0..2 are Tree Node
                    bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, "Selectable Node %d", i);
                    if (ImGui::IsItemClicked())
                        node_clicked = i;
                    if (test_drag_and_drop && ImGui::BeginDragDropSource())
                    {
                        ImGui::SetDragDropPayload("_TREENODE", NULL, 0);
                        ImGui::Text("This is a drag and drop source");
                        ImGui::EndDragDropSource();
                    }
                    if (node_open)
                    {
                        ImGui::BulletText("Blah blah\nBlah Blah");
                        ImGui::TreePop();
                    }
                } else
                {
                    // Items 3..5 are Tree Leaves
                    // The only reason we use TreeNode at all is to allow selection of the leaf.
                    // Otherwise we can use BulletText() or advance the cursor by GetTreeNodeToLabelSpacing() and call Text().
                    node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
                    ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, "Selectable Leaf %d", i);
                    if (ImGui::IsItemClicked())
                        node_clicked = i;
                    if (test_drag_and_drop && ImGui::BeginDragDropSource())
                    {
                        ImGui::SetDragDropPayload("_TREENODE", NULL, 0);
                        ImGui::Text("This is a drag and drop source");
                        ImGui::EndDragDropSource();
                    }
                }
            }
            if (node_clicked != -1)
            {
                // Update selection state. Process outside of tree loop to avoid visual inconsistencies during the clicking-frame.
                if (ImGui::GetIO().KeyCtrl)
                    selection_mask ^= (1 << node_clicked);          // CTRL+click to toggle
                else //if (!(selection_mask & (1 << node_clicked))) // Depending on selection behavior you want, this commented bit preserve selection when clicking on item that is part of the selection
                    selection_mask = (1 << node_clicked);           // Click to single-select
            }
            if (align_label_with_current_x_position)
                ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Collapsing Headers"))
    {
        static bool closable_group = true;
        ImGui::Checkbox("Show 2nd header", &closable_group);
        if (ImGui::CollapsingHeader("Header", ImGuiTreeNodeFlags_None))
        {
            ImGui::Text("IsItemHovered: %d", ImGui::IsItemHovered());
            for (int i = 0; i < 5; i++)
                ImGui::Text("Some content %d", i);
        }
        if (ImGui::CollapsingHeader("Header with a close button", &closable_group))
        {
            ImGui::Text("IsItemHovered: %d", ImGui::IsItemHovered());
            for (int i = 0; i < 5; i++)
                ImGui::Text("More content %d", i);
        }
        /*
        if (ImGui::CollapsingHeader("Header with a bullet", ImGuiTreeNodeFlags_Bullet))
            ImGui::Text("IsItemHovered: %d", ImGui::IsItemHovered());
        */
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Bullets"))
    {
        ImGui::BulletText("Bullet point 1");
        ImGui::BulletText("Bullet point 2\nOn multiple lines");
        if (ImGui::TreeNode("Tree node"))
        {
            ImGui::BulletText("Another bullet point");
            ImGui::TreePop();
        }
        ImGui::Bullet(); ImGui::Text("Bullet point 3 (two calls)");
        ImGui::Bullet(); ImGui::SmallButton("Button");
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Text"))
    {
        if (ImGui::TreeNode("Colored Text"))
        {
            // Using shortcut. You can use PushStyleColor()/PopStyleColor() for more flexibility.
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "Pink");
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Yellow");
            ImGui::TextDisabled("Disabled");
            ImGui::SameLine(); HelpMarker("The TextDisabled color is stored in ImGuiStyle.");
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Word Wrapping"))
        {
            // Using shortcut. You can use PushTextWrapPos()/PopTextWrapPos() for more flexibility.
            ImGui::TextWrapped("This text should automatically wrap on the edge of the window. The current implementation for text wrapping follows simple rules suitable for English and possibly other languages.");
            ImGui::Spacing();

            static float wrap_width = 200.0f;
            ImGui::SliderFloat("Wrap width", &wrap_width, -20, 600, "%.0f");

            ImGui::Text("Test paragraph 1:");
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(pos.x + wrap_width, pos.y), ImVec2(pos.x + wrap_width + 10, pos.y + ImGui::GetTextLineHeight()), IM_COL32(255, 0, 255, 255));
            ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + wrap_width);
            ImGui::Text("The lazy dog is a good dog. This paragraph is made to fit within %.0f pixels. Testing a 1 character word. The quick brown fox jumps over the lazy dog.", wrap_width);
            ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255, 255, 0, 255));
            ImGui::PopTextWrapPos();

            ImGui::Text("Test paragraph 2:");
            pos = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(pos.x + wrap_width, pos.y), ImVec2(pos.x + wrap_width + 10, pos.y + ImGui::GetTextLineHeight()), IM_COL32(255, 0, 255, 255));
            ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + wrap_width);
            ImGui::Text("aaaaaaaa bbbbbbbb, c cccccccc,dddddddd. d eeeeeeee   ffffffff. gggggggg!hhhhhhhh");
            ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255, 255, 0, 255));
            ImGui::PopTextWrapPos();

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("UTF-8 Text"))
        {
            // UTF-8 test with Japanese characters
            // (Needs a suitable font, try Noto, or Arial Unicode, or M+ fonts. Read docs/FONTS.txt for details.)
            // - From C++11 you can use the u8"my text" syntax to encode literal strings as UTF-8
            // - For earlier compiler, you may be able to encode your sources as UTF-8 (e.g. Visual Studio save your file as 'UTF-8 without signature')
            // - FOR THIS DEMO FILE ONLY, BECAUSE WE WANT TO SUPPORT OLD COMPILERS, WE ARE *NOT* INCLUDING RAW UTF-8 CHARACTERS IN THIS SOURCE FILE.
            //   Instead we are encoding a few strings with hexadecimal constants. Don't do this in your application!
            //   Please use u8"text in any language" in your application!
            // Note that characters values are preserved even by InputText() if the font cannot be displayed, so you can safely copy & paste garbled characters into another application.
            ImGui::TextWrapped("CJK text will only appears if the font was loaded with the appropriate CJK character ranges. Call io.Font->AddFontFromFileTTF() manually to load extra character ranges. Read docs/FONTS.txt for details.");
            ImGui::Text("Hiragana: \xe3\x81\x8b\xe3\x81\x8d\xe3\x81\x8f\xe3\x81\x91\xe3\x81\x93 (kakikukeko)"); // Normally we would use u8"blah blah" with the proper characters directly in the string.
            ImGui::Text("Kanjis: \xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e (nihongo)");
            static char buf[32] = "\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e";
            //static char buf[32] = u8"NIHONGO"; // <- this is how you would write it with C++11, using real kanjis
            ImGui::InputText("UTF-8 input", buf, IM_ARRAYSIZE(buf));
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Images"))
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::TextWrapped("Below we are displaying the font texture (which is the only texture we have access to in this demo). Use the 'ImTextureID' type as storage to pass pointers or identifier to your own texture data. Hover the texture for a zoomed view!");

        // Here we are grabbing the font texture because that's the only one we have access to inside the demo code.
        // Remember that ImTextureID is just storage for whatever you want it to be, it is essentially a value that will be passed to the render function inside the ImDrawCmd structure.
        // If you use one of the default imgui_impl_XXXX.cpp renderer, they all have comments at the top of their file to specify what they expect to be stored in ImTextureID.
        // (for example, the imgui_impl_dx11.cpp renderer expect a 'ID3D11ShaderResourceView*' pointer. The imgui_impl_opengl3.cpp renderer expect a GLuint OpenGL texture identifier etc.)
        // If you decided that ImTextureID = MyEngineTexture*, then you can pass your MyEngineTexture* pointers to ImGui::Image(), and gather width/height through your own functions, etc.
        // Using ShowMetricsWindow() as a "debugger" to inspect the draw data that are being passed to your render will help you debug issues if you are confused about this.
        // Consider using the lower-level ImDrawList::AddImage() API, via ImGui::GetWindowDrawList()->AddImage().
        ImTextureID my_tex_id = io.Fonts->TexID;
        float my_tex_w = (float)io.Fonts->TexWidth;
        float my_tex_h = (float)io.Fonts->TexHeight;

        ImGui::Text("%.0fx%.0f", my_tex_w, my_tex_h);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImGui::Image(my_tex_id, ImVec2(my_tex_w, my_tex_h), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            float region_sz = 32.0f;
            float region_x = io.MousePos.x - pos.x - region_sz * 0.5f; if (region_x < 0.0f) region_x = 0.0f; else if (region_x > my_tex_w - region_sz) region_x = my_tex_w - region_sz;
            float region_y = io.MousePos.y - pos.y - region_sz * 0.5f; if (region_y < 0.0f) region_y = 0.0f; else if (region_y > my_tex_h - region_sz) region_y = my_tex_h - region_sz;
            float zoom = 4.0f;
            ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
            ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
            ImVec2 uv0 = ImVec2((region_x) / my_tex_w, (region_y) / my_tex_h);
            ImVec2 uv1 = ImVec2((region_x + region_sz) / my_tex_w, (region_y + region_sz) / my_tex_h);
            ImGui::Image(my_tex_id, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
            ImGui::EndTooltip();
        }
        ImGui::TextWrapped("And now some textured buttons..");
        static int pressed_count = 0;
        for (int i = 0; i < 8; i++)
        {
            ImGui::PushID(i);
            int frame_padding = -1 + i;     // -1 = uses default padding
            if (ImGui::ImageButton(my_tex_id, ImVec2(32, 32), ImVec2(0, 0), ImVec2(32.0f / my_tex_w, 32 / my_tex_h), frame_padding, ImVec4(0.0f, 0.0f, 0.0f, 1.0f)))
                pressed_count += 1;
            ImGui::PopID();
            ImGui::SameLine();
        }
        ImGui::NewLine();
        ImGui::Text("Pressed %d times.", pressed_count);
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Combo"))
    {
        // Expose flags as checkbox for the demo
        static ImGuiComboFlags flags = 0;
        ImGui::CheckboxFlags("ImGuiComboFlags_PopupAlignLeft", (unsigned int*)&flags, ImGuiComboFlags_PopupAlignLeft);
        ImGui::SameLine(); HelpMarker("Only makes a difference if the popup is larger than the combo");
        if (ImGui::CheckboxFlags("ImGuiComboFlags_NoArrowButton", (unsigned int*)&flags, ImGuiComboFlags_NoArrowButton))
            flags &= ~ImGuiComboFlags_NoPreview;     // Clear the other flag, as we cannot combine both
        if (ImGui::CheckboxFlags("ImGuiComboFlags_NoPreview", (unsigned int*)&flags, ImGuiComboFlags_NoPreview))
            flags &= ~ImGuiComboFlags_NoArrowButton; // Clear the other flag, as we cannot combine both

        // General BeginCombo() API, you have full control over your selection data and display type.
        // (your selection data could be an index, a pointer to the object, an id for the object, a flag stored in the object itself, etc.)
        const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK", "LLLLLLL", "MMMM", "OOOOOOO" };
        static const char* item_current = items[0];            // Here our selection is a single pointer stored outside the object.
        if (ImGui::BeginCombo("combo 1", item_current, flags)) // The second parameter is the label previewed before opening the combo.
        {
            for (int n = 0; n < IM_ARRAYSIZE(items); n++)
            {
                bool is_selected = (item_current == items[n]);
                if (ImGui::Selectable(items[n], is_selected))
                    item_current = items[n];
                if (is_selected)
                    ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
            }
            ImGui::EndCombo();
        }

        // Simplified one-liner Combo() API, using values packed in a single constant string
        static int item_current_2 = 0;
        ImGui::Combo("combo 2 (one-liner)", &item_current_2, "aaaa\0bbbb\0cccc\0dddd\0eeee\0\0");

        // Simplified one-liner Combo() using an array of const char*
        static int item_current_3 = -1; // If the selection isn't within 0..count, Combo won't display a preview
        ImGui::Combo("combo 3 (array)", &item_current_3, items, IM_ARRAYSIZE(items));

        // Simplified one-liner Combo() using an accessor function
        struct FuncHolder { static bool ItemGetter(void* data, int idx, const char** out_str) { *out_str = ((const char**)data)[idx]; return true; } };
        static int item_current_4 = 0;
        ImGui::Combo("combo 4 (function)", &item_current_4, &FuncHolder::ItemGetter, items, IM_ARRAYSIZE(items));

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Selectables"))
    {
        // Selectable() has 2 overloads:
        // - The one taking "bool selected" as a read-only selection information. When Selectable() has been clicked is returns true and you can alter selection state accordingly.
        // - The one taking "bool* p_selected" as a read-write selection information (convenient in some cases)
        // The earlier is more flexible, as in real application your selection may be stored in a different manner (in flags within objects, as an external list, etc).
        if (ImGui::TreeNode("Basic"))
        {
            static bool selection[5] = { false, true, false, false, false };
            ImGui::Selectable("1. I am selectable", &selection[0]);
            ImGui::Selectable("2. I am selectable", &selection[1]);
            ImGui::Text("3. I am not selectable");
            ImGui::Selectable("4. I am selectable", &selection[3]);
            if (ImGui::Selectable("5. I am double clickable", selection[4], ImGuiSelectableFlags_AllowDoubleClick))
                if (ImGui::IsMouseDoubleClicked(0))
                    selection[4] = !selection[4];
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Selection State: Single Selection"))
        {
            static int selected = -1;
            for (int n = 0; n < 5; n++)
            {
                char buf[32];
                sprintf(buf, "Object %d", n);
                if (ImGui::Selectable(buf, selected == n))
                    selected = n;
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Selection State: Multiple Selection"))
        {
            HelpMarker("Hold CTRL and click to select multiple items.");
            static bool selection[5] = { false, false, false, false, false };
            for (int n = 0; n < 5; n++)
            {
                char buf[32];
                sprintf(buf, "Object %d", n);
                if (ImGui::Selectable(buf, selection[n]))
                {
                    if (!ImGui::GetIO().KeyCtrl)    // Clear selection when CTRL is not held
                        memset(selection, 0, sizeof(selection));
                    selection[n] ^= 1;
                }
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Rendering more text into the same line"))
        {
            // Using the Selectable() override that takes "bool* p_selected" parameter and toggle your booleans automatically.
            static bool selected[3] = { false, false, false };
            ImGui::Selectable("main.c", &selected[0]); ImGui::SameLine(300); ImGui::Text(" 2,345 bytes");
            ImGui::Selectable("Hello.cpp", &selected[1]); ImGui::SameLine(300); ImGui::Text("12,345 bytes");
            ImGui::Selectable("Hello.h", &selected[2]); ImGui::SameLine(300); ImGui::Text(" 2,345 bytes");
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("In columns"))
        {
            ImGui::Columns(3, NULL, false);
            static bool selected[16] = {};
            for (int i = 0; i < 16; i++)
            {
                char label[32]; sprintf(label, "Item %d", i);
                if (ImGui::Selectable(label, &selected[i])) {}
                ImGui::NextColumn();
            }
            ImGui::Columns(1);
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Grid"))
        {
            static bool selected[4 * 4] = { true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true };
            for (int i = 0; i < 4 * 4; i++)
            {
                ImGui::PushID(i);
                if (ImGui::Selectable("Sailor", &selected[i], 0, ImVec2(50, 50)))
                {
                    // Note: We _unnecessarily_ test for both x/y and i here only to silence some static analyzer. The second part of each test is unnecessary.
                    int x = i % 4;
                    int y = i / 4;
                    if (x > 0) { selected[i - 1] ^= 1; }
                    if (x < 3 && i < 15) { selected[i + 1] ^= 1; }
                    if (y > 0 && i > 3) { selected[i - 4] ^= 1; }
                    if (y < 3 && i < 12) { selected[i + 4] ^= 1; }
                }
                if ((i % 4) < 3) ImGui::SameLine();
                ImGui::PopID();
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Alignment"))
        {
            HelpMarker("By default, Selectables uses style.SelectableTextAlign but it can be overriden on a per-item basis using PushStyleVar(). You'll probably want to always keep your default situation to left-align otherwise it becomes difficult to layout multiple items on a same line");
            static bool selected[3 * 3] = { true, false, true, false, true, false, true, false, true };
            for (int y = 0; y < 3; y++)
            {
                for (int x = 0; x < 3; x++)
                {
                    ImVec2 alignment = ImVec2((float)x / 2.0f, (float)y / 2.0f);
                    char name[32];
                    sprintf(name, "(%.1f,%.1f)", alignment.x, alignment.y);
                    if (x > 0) ImGui::SameLine();
                    ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, alignment);
                    ImGui::Selectable(name, &selected[3 * y + x], ImGuiSelectableFlags_None, ImVec2(80, 80));
                    ImGui::PopStyleVar();
                }
            }
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }

    // To wire InputText() with std::string or any other custom string type,
    // see the "Text Input > Resize Callback" section of this demo, and the misc/cpp/imgui_stdlib.h file.
    if (ImGui::TreeNode("Text Input"))
    {
        if (ImGui::TreeNode("Multi-line Text Input"))
        {
            // Note: we are using a fixed-sized buffer for simplicity here. See ImGuiInputTextFlags_CallbackResize
            // and the code in misc/cpp/imgui_stdlib.h for how to setup InputText() for dynamically resizing strings.
            static char text[1024 * 16] =
                "/*\n"
                " The Pentium F00F bug, shorthand for F0 0F C7 C8,\n"
                " the hexadecimal encoding of one offending instruction,\n"
                " more formally, the invalid operand with locked CMPXCHG8B\n"
                " instruction bug, is a design flaw in the majority of\n"
                " Intel Pentium, Pentium MMX, and Pentium OverDrive\n"
                " processors (all in the P5 microarchitecture).\n"
                "*/\n\n"
                "label:\n"
                "\tlock cmpxchg8b eax\n";

            static ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
            HelpMarker("You can use the ImGuiInputTextFlags_CallbackResize facility if you need to wire InputTextMultiline() to a dynamic string type. See misc/cpp/imgui_stdlib.h for an example. (This is not demonstrated in imgui_demo.cpp because we don't want to include <string> in here)");
            ImGui::CheckboxFlags("ImGuiInputTextFlags_ReadOnly", (unsigned int*)&flags, ImGuiInputTextFlags_ReadOnly);
            ImGui::CheckboxFlags("ImGuiInputTextFlags_AllowTabInput", (unsigned int*)&flags, ImGuiInputTextFlags_AllowTabInput);
            ImGui::CheckboxFlags("ImGuiInputTextFlags_CtrlEnterForNewLine", (unsigned int*)&flags, ImGuiInputTextFlags_CtrlEnterForNewLine);
            ImGui::InputTextMultiline("##source", text, IM_ARRAYSIZE(text), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), flags);
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Filtered Text Input"))
        {
            static char buf1[64] = ""; ImGui::InputText("default", buf1, 64);
            static char buf2[64] = ""; ImGui::InputText("decimal", buf2, 64, ImGuiInputTextFlags_CharsDecimal);
            static char buf3[64] = ""; ImGui::InputText("hexadecimal", buf3, 64, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
            static char buf4[64] = ""; ImGui::InputText("uppercase", buf4, 64, ImGuiInputTextFlags_CharsUppercase);
            static char buf5[64] = ""; ImGui::InputText("no blank", buf5, 64, ImGuiInputTextFlags_CharsNoBlank);
            struct TextFilters { static int FilterImGuiLetters(ImGuiInputTextCallbackData* data) { if (data->EventChar < 256 && strchr("imgui", (char)data->EventChar)) return 0; return 1; } };
            static char buf6[64] = ""; ImGui::InputText("\"imgui\" letters", buf6, 64, ImGuiInputTextFlags_CallbackCharFilter, TextFilters::FilterImGuiLetters);

            ImGui::Text("Password input");
            static char password[64] = "password123";
            ImGui::InputText("password", password, IM_ARRAYSIZE(password), ImGuiInputTextFlags_Password);
            ImGui::SameLine(); HelpMarker("Display all characters as '*'.\nDisable clipboard cut and copy.\nDisable logging.\n");
            ImGui::InputTextWithHint("password (w/ hint)", "<password>", password, IM_ARRAYSIZE(password), ImGuiInputTextFlags_Password);
            ImGui::InputText("password (clear)", password, IM_ARRAYSIZE(password));
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Resize Callback"))
        {
            // To wire InputText() with std::string or any other custom string type,
            // you can use the ImGuiInputTextFlags_CallbackResize flag + create a custom ImGui::InputText() wrapper using your prefered type.
            // See misc/cpp/imgui_stdlib.h for an implementation of this using std::string.
            HelpMarker("Demonstrate using ImGuiInputTextFlags_CallbackResize to wire your resizable string type to InputText().\n\nSee misc/cpp/imgui_stdlib.h for an implementation of this for std::string.");
            struct Funcs
            {
                static int MyResizeCallback(ImGuiInputTextCallbackData* data)
                {
                    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
                    {
                        ImVector<char>* my_str = (ImVector<char>*)data->UserData;
                        IM_ASSERT(my_str->begin() == data->Buf);
                        my_str->resize(data->BufSize);  // NB: On resizing calls, generally data->BufSize == data->BufTextLen + 1
                        data->Buf = my_str->begin();
                    }
                    return 0;
                }

                // Tip: Because ImGui:: is a namespace you would typicall add your own function into the namespace in your own source files.
                // For example, you may add a function called ImGui::InputText(const char* label, MyString* my_str).
                static bool MyInputTextMultiline(const char* label, ImVector<char>* my_str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0)
                {
                    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
                    return ImGui::InputTextMultiline(label, my_str->begin(), (size_t)my_str->size(), size, flags | ImGuiInputTextFlags_CallbackResize, Funcs::MyResizeCallback, (void*)my_str);
                }
            };

            // For this demo we are using ImVector as a string container.
            // Note that because we need to store a terminating zero character, our size/capacity are 1 more than usually reported by a typical string class.
            static ImVector<char> my_str;
            if (my_str.empty())
                my_str.push_back(0);
            Funcs::MyInputTextMultiline("##MyStr", &my_str, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16));
            ImGui::Text("Data: %p\nSize: %d\nCapacity: %d", (void*)my_str.begin(), my_str.size(), my_str.capacity());
            ImGui::TreePop();
        }

        ImGui::TreePop();
    }

    // Plot/Graph widgets are currently fairly limited.
    // Consider writing your own plotting widget, or using a third-party one (see "Wiki->Useful Widgets", or github.com/ocornut/imgui/issues/2747)
    if (ImGui::TreeNode("Plots Widgets"))
    {
        static bool animate = true;
        ImGui::Checkbox("Animate", &animate);

        static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
        ImGui::PlotLines("Frame Times", arr, IM_ARRAYSIZE(arr));

        // Create a dummy array of contiguous float values to plot
        // Tip: If your float aren't contiguous but part of a structure, you can pass a pointer to your first float and the sizeof() of your structure in the Stride parameter.
        static float values[90] = {};
        static int values_offset = 0;
        static double refresh_time = 0.0;
        if (!animate || refresh_time == 0.0)
            refresh_time = ImGui::GetTime();
        while (refresh_time < ImGui::GetTime()) // Create dummy data at fixed 60 hz rate for the demo
        {
            static float phase = 0.0f;
            values[values_offset] = cosf(phase);
            values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);
            phase += 0.10f * values_offset;
            refresh_time += 1.0f / 60.0f;
        }

        // Plots can display overlay texts
        // (in this example, we will display an average value)
        {
            float average = 0.0f;
            for (int n = 0; n < IM_ARRAYSIZE(values); n++)
                average += values[n];
            average /= (float)IM_ARRAYSIZE(values);
            char overlay[32];
            sprintf(overlay, "avg %f", average);
            ImGui::PlotLines("Lines", values, IM_ARRAYSIZE(values), values_offset, overlay, -1.0f, 1.0f, ImVec2(0, 80));
        }
        ImGui::PlotHistogram("Histogram", arr, IM_ARRAYSIZE(arr), 0, NULL, 0.0f, 1.0f, ImVec2(0, 80));

        // Use functions to generate output
        // FIXME: This is rather awkward because current plot API only pass in indices. We probably want an API passing floats and user provide sample rate/count.
        struct Funcs
        {
            static float Sin(void*, int i) { return sinf(i * 0.1f); }
            static float Saw(void*, int i) { return (i & 1) ? 1.0f : -1.0f; }
        };
        static int func_type = 0, display_count = 70;
        ImGui::Separator();
        ImGui::SetNextItemWidth(100);
        ImGui::Combo("func", &func_type, "Sin\0Saw\0");
        ImGui::SameLine();
        ImGui::SliderInt("Sample count", &display_count, 1, 400);
        float (*func)(void*, int) = (func_type == 0) ? Funcs::Sin : Funcs::Saw;
        ImGui::PlotLines("Lines", func, NULL, display_count, 0, NULL, -1.0f, 1.0f, ImVec2(0, 80));
        ImGui::PlotHistogram("Histogram", func, NULL, display_count, 0, NULL, -1.0f, 1.0f, ImVec2(0, 80));
        ImGui::Separator();

        // Animate a simple progress bar
        static float progress = 0.0f, progress_dir = 1.0f;
        if (animate)
        {
            progress += progress_dir * 0.4f * ImGui::GetIO().DeltaTime;
            if (progress >= +1.1f) { progress = +1.1f; progress_dir *= -1.0f; }
            if (progress <= -0.1f) { progress = -0.1f; progress_dir *= -1.0f; }
        }

        // Typically we would use ImVec2(-1.0f,0.0f) or ImVec2(-FLT_MIN,0.0f) to use all available width,
        // or ImVec2(width,0.0f) for a specified width. ImVec2(0.0f,0.0f) uses ItemWidth.
        ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::Text("Progress Bar");

        float progress_saturated = (progress < 0.0f) ? 0.0f : (progress > 1.0f) ? 1.0f : progress;
        char buf[32];
        sprintf(buf, "%d/%d", (int)(progress_saturated * 1753), 1753);
        ImGui::ProgressBar(progress, ImVec2(0.f, 0.f), buf);
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Color/Picker Widgets"))
    {
        static ImVec4 color = ImVec4(114.0f / 255.0f, 144.0f / 255.0f, 154.0f / 255.0f, 200.0f / 255.0f);

        static bool alpha_preview = true;
        static bool alpha_half_preview = false;
        static bool drag_and_drop = true;
        static bool options_menu = true;
        static bool hdr = false;
        ImGui::Checkbox("With Alpha Preview", &alpha_preview);
        ImGui::Checkbox("With Half Alpha Preview", &alpha_half_preview);
        ImGui::Checkbox("With Drag and Drop", &drag_and_drop);
        ImGui::Checkbox("With Options Menu", &options_menu); ImGui::SameLine(); HelpMarker("Right-click on the individual color widget to show options.");
        ImGui::Checkbox("With HDR", &hdr); ImGui::SameLine(); HelpMarker("Currently all this does is to lift the 0..1 limits on dragging widgets.");
        ImGuiColorEditFlags misc_flags = (hdr ? ImGuiColorEditFlags_HDR : 0) | (drag_and_drop ? 0 : ImGuiColorEditFlags_NoDragDrop) | (alpha_half_preview ? ImGuiColorEditFlags_AlphaPreviewHalf : (alpha_preview ? ImGuiColorEditFlags_AlphaPreview : 0)) | (options_menu ? 0 : ImGuiColorEditFlags_NoOptions);

        ImGui::Text("Color widget:");
        ImGui::SameLine(); HelpMarker("Click on the colored square to open a color picker.\nCTRL+click on individual component to input value.\n");
        ImGui::ColorEdit3("MyColor##1", (float*)&color, misc_flags);

        ImGui::Text("Color widget HSV with Alpha:");
        ImGui::ColorEdit4("MyColor##2", (float*)&color, ImGuiColorEditFlags_DisplayHSV | misc_flags);

        ImGui::Text("Color widget with Float Display:");
        ImGui::ColorEdit4("MyColor##2f", (float*)&color, ImGuiColorEditFlags_Float | misc_flags);

        ImGui::Text("Color button with Picker:");
        ImGui::SameLine(); HelpMarker("With the ImGuiColorEditFlags_NoInputs flag you can hide all the slider/text inputs.\nWith the ImGuiColorEditFlags_NoLabel flag you can pass a non-empty label which will only be used for the tooltip and picker popup.");
        ImGui::ColorEdit4("MyColor##3", (float*)&color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | misc_flags);

        ImGui::Text("Color button with Custom Picker Popup:");

        // Generate a dummy default palette. The palette will persist and can be edited.
        static bool saved_palette_init = true;
        static ImVec4 saved_palette[32] = {};
        if (saved_palette_init)
        {
            for (int n = 0; n < IM_ARRAYSIZE(saved_palette); n++)
            {
                ImGui::ColorConvertHSVtoRGB(n / 31.0f, 0.8f, 0.8f, saved_palette[n].x, saved_palette[n].y, saved_palette[n].z);
                saved_palette[n].w = 1.0f; // Alpha
            }
            saved_palette_init = false;
        }

        static ImVec4 backup_color;
        bool open_popup = ImGui::ColorButton("MyColor##3b", color, misc_flags);
        ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
        open_popup |= ImGui::Button("Palette");
        if (open_popup)
        {
            ImGui::OpenPopup("mypicker");
            backup_color = color;
        }
        if (ImGui::BeginPopup("mypicker"))
        {
            ImGui::Text("MY CUSTOM COLOR PICKER WITH AN AMAZING PALETTE!");
            ImGui::Separator();
            ImGui::ColorPicker4("##picker", (float*)&color, misc_flags | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
            ImGui::SameLine();

            ImGui::BeginGroup(); // Lock X position
            ImGui::Text("Current");
            ImGui::ColorButton("##current", color, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 40));
            ImGui::Text("Previous");
            if (ImGui::ColorButton("##previous", backup_color, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 40)))
                color = backup_color;
            ImGui::Separator();
            ImGui::Text("Palette");
            for (int n = 0; n < IM_ARRAYSIZE(saved_palette); n++)
            {
                ImGui::PushID(n);
                if ((n % 8) != 0)
                    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.y);
                if (ImGui::ColorButton("##palette", saved_palette[n], ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoTooltip, ImVec2(20, 20)))
                    color = ImVec4(saved_palette[n].x, saved_palette[n].y, saved_palette[n].z, color.w); // Preserve alpha!

                // Allow user to drop colors into each palette entry
                // (Note that ColorButton is already a drag source by default, unless using ImGuiColorEditFlags_NoDragDrop)
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F))
                        memcpy((float*)&saved_palette[n], payload->Data, sizeof(float) * 3);
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_4F))
                        memcpy((float*)&saved_palette[n], payload->Data, sizeof(float) * 4);
                    ImGui::EndDragDropTarget();
                }

                ImGui::PopID();
            }
            ImGui::EndGroup();
            ImGui::EndPopup();
        }

        ImGui::Text("Color button only:");
        static bool no_border = false;
        ImGui::Checkbox("ImGuiColorEditFlags_NoBorder", &no_border);
        ImGui::ColorButton("MyColor##3c", *(ImVec4*)&color, misc_flags | (no_border ? ImGuiColorEditFlags_NoBorder : 0), ImVec2(80, 80));

        ImGui::Text("Color picker:");
        static bool alpha = true;
        static bool alpha_bar = true;
        static bool side_preview = true;
        static bool ref_color = false;
        static ImVec4 ref_color_v(1.0f, 0.0f, 1.0f, 0.5f);
        static int display_mode = 0;
        static int picker_mode = 0;
        ImGui::Checkbox("With Alpha", &alpha);
        ImGui::Checkbox("With Alpha Bar", &alpha_bar);
        ImGui::Checkbox("With Side Preview", &side_preview);
        if (side_preview)
        {
            ImGui::SameLine();
            ImGui::Checkbox("With Ref Color", &ref_color);
            if (ref_color)
            {
                ImGui::SameLine();
                ImGui::ColorEdit4("##RefColor", &ref_color_v.x, ImGuiColorEditFlags_NoInputs | misc_flags);
            }
        }
        ImGui::Combo("Display Mode", &display_mode, "Auto/Current\0None\0RGB Only\0HSV Only\0Hex Only\0");
        ImGui::SameLine(); HelpMarker("ColorEdit defaults to displaying RGB inputs if you don't specify a display mode, but the user can change it with a right-click.\n\nColorPicker defaults to displaying RGB+HSV+Hex if you don't specify a display mode.\n\nYou can change the defaults using SetColorEditOptions().");
        ImGui::Combo("Picker Mode", &picker_mode, "Auto/Current\0Hue bar + SV rect\0Hue wheel + SV triangle\0");
        ImGui::SameLine(); HelpMarker("User can right-click the picker to change mode.");
        ImGuiColorEditFlags flags = misc_flags;
        if (!alpha)            flags |= ImGuiColorEditFlags_NoAlpha;        // This is by default if you call ColorPicker3() instead of ColorPicker4()
        if (alpha_bar)         flags |= ImGuiColorEditFlags_AlphaBar;
        if (!side_preview)     flags |= ImGuiColorEditFlags_NoSidePreview;
        if (picker_mode == 1)  flags |= ImGuiColorEditFlags_PickerHueBar;
        if (picker_mode == 2)  flags |= ImGuiColorEditFlags_PickerHueWheel;
        if (display_mode == 1) flags |= ImGuiColorEditFlags_NoInputs;       // Disable all RGB/HSV/Hex displays
        if (display_mode == 2) flags |= ImGuiColorEditFlags_DisplayRGB;     // Override display mode
        if (display_mode == 3) flags |= ImGuiColorEditFlags_DisplayHSV;
        if (display_mode == 4) flags |= ImGuiColorEditFlags_DisplayHex;
        ImGui::ColorPicker4("MyColor##4", (float*)&color, flags, ref_color ? &ref_color_v.x : NULL);

        ImGui::Text("Programmatically set defaults:");
        ImGui::SameLine(); HelpMarker("SetColorEditOptions() is designed to allow you to set boot-time default.\nWe don't have Push/Pop functions because you can force options on a per-widget basis if needed, and the user can change non-forced ones with the options menu.\nWe don't have a getter to avoid encouraging you to persistently save values that aren't forward-compatible.");
        if (ImGui::Button("Default: Uint8 + HSV + Hue Bar"))
            ImGui::SetColorEditOptions(ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_DisplayHSV | ImGuiColorEditFlags_PickerHueBar);
        if (ImGui::Button("Default: Float + HDR + Hue Wheel"))
            ImGui::SetColorEditOptions(ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_PickerHueWheel);

        // HSV encoded support (to avoid RGB<>HSV round trips and singularities when S==0 or V==0)
        static ImVec4 color_stored_as_hsv(0.23f, 1.0f, 1.0f, 1.0f);
        ImGui::Spacing();
        ImGui::Text("HSV encoded colors");
        ImGui::SameLine(); HelpMarker("By default, colors are given to ColorEdit and ColorPicker in RGB, but ImGuiColorEditFlags_InputHSV allows you to store colors as HSV and pass them to ColorEdit and ColorPicker as HSV. This comes with the added benefit that you can manipulate hue values with the picker even when saturation or value are zero.");
        ImGui::Text("Color widget with InputHSV:");
        ImGui::ColorEdit4("HSV shown as RGB##1", (float*)&color_stored_as_hsv, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputHSV | ImGuiColorEditFlags_Float);
        ImGui::ColorEdit4("HSV shown as HSV##1", (float*)&color_stored_as_hsv, ImGuiColorEditFlags_DisplayHSV | ImGuiColorEditFlags_InputHSV | ImGuiColorEditFlags_Float);
        ImGui::DragFloat4("Raw HSV values", (float*)&color_stored_as_hsv, 0.01f, 0.0f, 1.0f);

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Range Widgets"))
    {
        static float begin = 10, end = 90;
        static int begin_i = 100, end_i = 1000;
        ImGui::DragFloatRange2("range", &begin, &end, 0.25f, 0.0f, 100.0f, "Min: %.1f %%", "Max: %.1f %%");
        ImGui::DragIntRange2("range int (no bounds)", &begin_i, &end_i, 5, 0, 0, "Min: %d units", "Max: %d units");
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Data Types"))
    {
        // The DragScalar/InputScalar/SliderScalar functions allow various data types: signed/unsigned int/long long and float/double
        // To avoid polluting the public API with all possible combinations, we use the ImGuiDataType enum to pass the type,
        // and passing all arguments by address.
        // This is the reason the test code below creates local variables to hold "zero" "one" etc. for each types.
        // In practice, if you frequently use a given type that is not covered by the normal API entry points, you can wrap it
        // yourself inside a 1 line function which can take typed argument as value instead of void*, and then pass their address
        // to the generic function. For example:
        //   bool MySliderU64(const char *label, u64* value, u64 min = 0, u64 max = 0, const char* format = "%lld")
        //   {
        //      return SliderScalar(label, ImGuiDataType_U64, value, &min, &max, format);
        //   }

        // Limits (as helper variables that we can take the address of)
        // Note that the SliderScalar function has a maximum usable range of half the natural type maximum, hence the /2 below.
#ifndef LLONG_MIN
        ImS64 LLONG_MIN = -9223372036854775807LL - 1;
        ImS64 LLONG_MAX = 9223372036854775807LL;
        ImU64 ULLONG_MAX = (2ULL * 9223372036854775807LL + 1);
#endif
        const char    s8_zero = 0, s8_one = 1, s8_fifty = 50, s8_min = -128, s8_max = 127;
        const ImU8    u8_zero = 0, u8_one = 1, u8_fifty = 50, u8_min = 0, u8_max = 255;
        const short   s16_zero = 0, s16_one = 1, s16_fifty = 50, s16_min = -32768, s16_max = 32767;
        const ImU16   u16_zero = 0, u16_one = 1, u16_fifty = 50, u16_min = 0, u16_max = 65535;
        const ImS32   s32_zero = 0, s32_one = 1, s32_fifty = 50, s32_min = INT_MIN / 2, s32_max = INT_MAX / 2, s32_hi_a = INT_MAX / 2 - 100, s32_hi_b = INT_MAX / 2;
        const ImU32   u32_zero = 0, u32_one = 1, u32_fifty = 50, u32_min = 0, u32_max = UINT_MAX / 2, u32_hi_a = UINT_MAX / 2 - 100, u32_hi_b = UINT_MAX / 2;
        const ImS64   s64_zero = 0, s64_one = 1, s64_fifty = 50, s64_min = LLONG_MIN / 2, s64_max = LLONG_MAX / 2, s64_hi_a = LLONG_MAX / 2 - 100, s64_hi_b = LLONG_MAX / 2;
        const ImU64   u64_zero = 0, u64_one = 1, u64_fifty = 50, u64_min = 0, u64_max = ULLONG_MAX / 2, u64_hi_a = ULLONG_MAX / 2 - 100, u64_hi_b = ULLONG_MAX / 2;
        const float   f32_zero = 0.f, f32_one = 1.f, f32_lo_a = -10000000000.0f, f32_hi_a = +10000000000.0f;
        const double  f64_zero = 0., f64_one = 1., f64_lo_a = -1000000000000000.0, f64_hi_a = +1000000000000000.0;

        // State
        static char   s8_v = 127;
        static ImU8   u8_v = 255;
        static short  s16_v = 32767;
        static ImU16  u16_v = 65535;
        static ImS32  s32_v = -1;
        static ImU32  u32_v = (ImU32)-1;
        static ImS64  s64_v = -1;
        static ImU64  u64_v = (ImU64)-1;
        static float  f32_v = 0.123f;
        static double f64_v = 90000.01234567890123456789;

        const float drag_speed = 0.2f;
        static bool drag_clamp = false;
        ImGui::Text("Drags:");
        ImGui::Checkbox("Clamp integers to 0..50", &drag_clamp); ImGui::SameLine(); HelpMarker("As with every widgets in dear imgui, we never modify values unless there is a user interaction.\nYou can override the clamping limits by using CTRL+Click to input a value.");
        ImGui::DragScalar("drag s8", ImGuiDataType_S8, &s8_v, drag_speed, drag_clamp ? &s8_zero : NULL, drag_clamp ? &s8_fifty : NULL);
        ImGui::DragScalar("drag u8", ImGuiDataType_U8, &u8_v, drag_speed, drag_clamp ? &u8_zero : NULL, drag_clamp ? &u8_fifty : NULL, "%u ms");
        ImGui::DragScalar("drag s16", ImGuiDataType_S16, &s16_v, drag_speed, drag_clamp ? &s16_zero : NULL, drag_clamp ? &s16_fifty : NULL);
        ImGui::DragScalar("drag u16", ImGuiDataType_U16, &u16_v, drag_speed, drag_clamp ? &u16_zero : NULL, drag_clamp ? &u16_fifty : NULL, "%u ms");
        ImGui::DragScalar("drag s32", ImGuiDataType_S32, &s32_v, drag_speed, drag_clamp ? &s32_zero : NULL, drag_clamp ? &s32_fifty : NULL);
        ImGui::DragScalar("drag u32", ImGuiDataType_U32, &u32_v, drag_speed, drag_clamp ? &u32_zero : NULL, drag_clamp ? &u32_fifty : NULL, "%u ms");
        ImGui::DragScalar("drag s64", ImGuiDataType_S64, &s64_v, drag_speed, drag_clamp ? &s64_zero : NULL, drag_clamp ? &s64_fifty : NULL);
        ImGui::DragScalar("drag u64", ImGuiDataType_U64, &u64_v, drag_speed, drag_clamp ? &u64_zero : NULL, drag_clamp ? &u64_fifty : NULL);
        ImGui::DragScalar("drag float", ImGuiDataType_Float, &f32_v, 0.005f, &f32_zero, &f32_one, "%f", 1.0f);
        ImGui::DragScalar("drag float ^2", ImGuiDataType_Float, &f32_v, 0.005f, &f32_zero, &f32_one, "%f", 2.0f); ImGui::SameLine(); HelpMarker("You can use the 'power' parameter to increase tweaking precision on one side of the range.");
        ImGui::DragScalar("drag double", ImGuiDataType_Double, &f64_v, 0.0005f, &f64_zero, NULL, "%.10f grams", 1.0f);
        ImGui::DragScalar("drag double ^2", ImGuiDataType_Double, &f64_v, 0.0005f, &f64_zero, &f64_one, "0 < %.10f < 1", 2.0f);

        ImGui::Text("Sliders");
        ImGui::SliderScalar("slider s8 full", ImGuiDataType_S8, &s8_v, &s8_min, &s8_max, "%d");
        ImGui::SliderScalar("slider u8 full", ImGuiDataType_U8, &u8_v, &u8_min, &u8_max, "%u");
        ImGui::SliderScalar("slider s16 full", ImGuiDataType_S16, &s16_v, &s16_min, &s16_max, "%d");
        ImGui::SliderScalar("slider u16 full", ImGuiDataType_U16, &u16_v, &u16_min, &u16_max, "%u");
        ImGui::SliderScalar("slider s32 low", ImGuiDataType_S32, &s32_v, &s32_zero, &s32_fifty, "%d");
        ImGui::SliderScalar("slider s32 high", ImGuiDataType_S32, &s32_v, &s32_hi_a, &s32_hi_b, "%d");
        ImGui::SliderScalar("slider s32 full", ImGuiDataType_S32, &s32_v, &s32_min, &s32_max, "%d");
        ImGui::SliderScalar("slider u32 low", ImGuiDataType_U32, &u32_v, &u32_zero, &u32_fifty, "%u");
        ImGui::SliderScalar("slider u32 high", ImGuiDataType_U32, &u32_v, &u32_hi_a, &u32_hi_b, "%u");
        ImGui::SliderScalar("slider u32 full", ImGuiDataType_U32, &u32_v, &u32_min, &u32_max, "%u");
        ImGui::SliderScalar("slider s64 low", ImGuiDataType_S64, &s64_v, &s64_zero, &s64_fifty, "%I64d");
        ImGui::SliderScalar("slider s64 high", ImGuiDataType_S64, &s64_v, &s64_hi_a, &s64_hi_b, "%I64d");
        ImGui::SliderScalar("slider s64 full", ImGuiDataType_S64, &s64_v, &s64_min, &s64_max, "%I64d");
        ImGui::SliderScalar("slider u64 low", ImGuiDataType_U64, &u64_v, &u64_zero, &u64_fifty, "%I64u ms");
        ImGui::SliderScalar("slider u64 high", ImGuiDataType_U64, &u64_v, &u64_hi_a, &u64_hi_b, "%I64u ms");
        ImGui::SliderScalar("slider u64 full", ImGuiDataType_U64, &u64_v, &u64_min, &u64_max, "%I64u ms");
        ImGui::SliderScalar("slider float low", ImGuiDataType_Float, &f32_v, &f32_zero, &f32_one);
        ImGui::SliderScalar("slider float low^2", ImGuiDataType_Float, &f32_v, &f32_zero, &f32_one, "%.10f", 2.0f);
        ImGui::SliderScalar("slider float high", ImGuiDataType_Float, &f32_v, &f32_lo_a, &f32_hi_a, "%e");
        ImGui::SliderScalar("slider double low", ImGuiDataType_Double, &f64_v, &f64_zero, &f64_one, "%.10f grams", 1.0f);
        ImGui::SliderScalar("slider double low^2", ImGuiDataType_Double, &f64_v, &f64_zero, &f64_one, "%.10f", 2.0f);
        ImGui::SliderScalar("slider double high", ImGuiDataType_Double, &f64_v, &f64_lo_a, &f64_hi_a, "%e grams", 1.0f);

        static bool inputs_step = true;
        ImGui::Text("Inputs");
        ImGui::Checkbox("Show step buttons", &inputs_step);
        ImGui::InputScalar("input s8", ImGuiDataType_S8, &s8_v, inputs_step ? &s8_one : NULL, NULL, "%d");
        ImGui::InputScalar("input u8", ImGuiDataType_U8, &u8_v, inputs_step ? &u8_one : NULL, NULL, "%u");
        ImGui::InputScalar("input s16", ImGuiDataType_S16, &s16_v, inputs_step ? &s16_one : NULL, NULL, "%d");
        ImGui::InputScalar("input u16", ImGuiDataType_U16, &u16_v, inputs_step ? &u16_one : NULL, NULL, "%u");
        ImGui::InputScalar("input s32", ImGuiDataType_S32, &s32_v, inputs_step ? &s32_one : NULL, NULL, "%d");
        ImGui::InputScalar("input s32 hex", ImGuiDataType_S32, &s32_v, inputs_step ? &s32_one : NULL, NULL, "%08X", ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::InputScalar("input u32", ImGuiDataType_U32, &u32_v, inputs_step ? &u32_one : NULL, NULL, "%u");
        ImGui::InputScalar("input u32 hex", ImGuiDataType_U32, &u32_v, inputs_step ? &u32_one : NULL, NULL, "%08X", ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::InputScalar("input s64", ImGuiDataType_S64, &s64_v, inputs_step ? &s64_one : NULL);
        ImGui::InputScalar("input u64", ImGuiDataType_U64, &u64_v, inputs_step ? &u64_one : NULL);
        ImGui::InputScalar("input float", ImGuiDataType_Float, &f32_v, inputs_step ? &f32_one : NULL);
        ImGui::InputScalar("input double", ImGuiDataType_Double, &f64_v, inputs_step ? &f64_one : NULL);

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Multi-component Widgets"))
    {
        static float vec4f[4] = { 0.10f, 0.20f, 0.30f, 0.44f };
        static int vec4i[4] = { 1, 5, 100, 255 };

        ImGui::InputFloat2("input float2", vec4f);
        ImGui::DragFloat2("drag float2", vec4f, 0.01f, 0.0f, 1.0f);
        ImGui::SliderFloat2("slider float2", vec4f, 0.0f, 1.0f);
        ImGui::InputInt2("input int2", vec4i);
        ImGui::DragInt2("drag int2", vec4i, 1, 0, 255);
        ImGui::SliderInt2("slider int2", vec4i, 0, 255);
        ImGui::Spacing();

        ImGui::InputFloat3("input float3", vec4f);
        ImGui::DragFloat3("drag float3", vec4f, 0.01f, 0.0f, 1.0f);
        ImGui::SliderFloat3("slider float3", vec4f, 0.0f, 1.0f);
        ImGui::InputInt3("input int3", vec4i);
        ImGui::DragInt3("drag int3", vec4i, 1, 0, 255);
        ImGui::SliderInt3("slider int3", vec4i, 0, 255);
        ImGui::Spacing();

        ImGui::InputFloat4("input float4", vec4f);
        ImGui::DragFloat4("drag float4", vec4f, 0.01f, 0.0f, 1.0f);
        ImGui::SliderFloat4("slider float4", vec4f, 0.0f, 1.0f);
        ImGui::InputInt4("input int4", vec4i);
        ImGui::DragInt4("drag int4", vec4i, 1, 0, 255);
        ImGui::SliderInt4("slider int4", vec4i, 0, 255);

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Vertical Sliders"))
    {
        const float spacing = 4;
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(spacing, spacing));

        static int int_value = 0;
        ImGui::VSliderInt("##int", ImVec2(18, 160), &int_value, 0, 5);
        ImGui::SameLine();

        static float values[7] = { 0.0f, 0.60f, 0.35f, 0.9f, 0.70f, 0.20f, 0.0f };
        ImGui::PushID("set1");
        for (int i = 0; i < 7; i++)
        {
            if (i > 0) ImGui::SameLine();
            ImGui::PushID(i);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor::HSV(i / 7.0f, 0.5f, 0.5f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, (ImVec4)ImColor::HSV(i / 7.0f, 0.6f, 0.5f));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, (ImVec4)ImColor::HSV(i / 7.0f, 0.7f, 0.5f));
            ImGui::PushStyleColor(ImGuiCol_SliderGrab, (ImVec4)ImColor::HSV(i / 7.0f, 0.9f, 0.9f));
            ImGui::VSliderFloat("##v", ImVec2(18, 160), &values[i], 0.0f, 1.0f, "");
            if (ImGui::IsItemActive() || ImGui::IsItemHovered())
                ImGui::SetTooltip("%.3f", values[i]);
            ImGui::PopStyleColor(4);
            ImGui::PopID();
        }
        ImGui::PopID();

        ImGui::SameLine();
        ImGui::PushID("set2");
        static float values2[4] = { 0.20f, 0.80f, 0.40f, 0.25f };
        const int rows = 3;
        const ImVec2 small_slider_size(18, (float)(int)((160.0f - (rows - 1) * spacing) / rows));
        for (int nx = 0; nx < 4; nx++)
        {
            if (nx > 0) ImGui::SameLine();
            ImGui::BeginGroup();
            for (int ny = 0; ny < rows; ny++)
            {
                ImGui::PushID(nx * rows + ny);
                ImGui::VSliderFloat("##v", small_slider_size, &values2[nx], 0.0f, 1.0f, "");
                if (ImGui::IsItemActive() || ImGui::IsItemHovered())
                    ImGui::SetTooltip("%.3f", values2[nx]);
                ImGui::PopID();
            }
            ImGui::EndGroup();
        }
        ImGui::PopID();

        ImGui::SameLine();
        ImGui::PushID("set3");
        for (int i = 0; i < 4; i++)
        {
            if (i > 0) ImGui::SameLine();
            ImGui::PushID(i);
            ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 40);
            ImGui::VSliderFloat("##v", ImVec2(40, 160), &values[i], 0.0f, 1.0f, "%.2f\nsec");
            ImGui::PopStyleVar();
            ImGui::PopID();
        }
        ImGui::PopID();
        ImGui::PopStyleVar();
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Drag and Drop"))
    {
        if (ImGui::TreeNode("Drag and drop in standard widgets"))
        {
            // ColorEdit widgets automatically act as drag source and drag target.
            // They are using standardized payload strings IMGUI_PAYLOAD_TYPE_COLOR_3F and IMGUI_PAYLOAD_TYPE_COLOR_4F to allow your own widgets
            // to use colors in their drag and drop interaction. Also see the demo in Color Picker -> Palette demo.
            HelpMarker("You can drag from the colored squares.");
            static float col1[3] = { 1.0f, 0.0f, 0.2f };
            static float col2[4] = { 0.4f, 0.7f, 0.0f, 0.5f };
            ImGui::ColorEdit3("color 1", col1);
            ImGui::ColorEdit4("color 2", col2);
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Drag and drop to copy/swap items"))
        {
            enum Mode
            {
                Mode_Copy,
                Mode_Move,
                Mode_Swap
            };
            static int mode = 0;
            if (ImGui::RadioButton("Copy", mode == Mode_Copy)) { mode = Mode_Copy; } ImGui::SameLine();
            if (ImGui::RadioButton("Move", mode == Mode_Move)) { mode = Mode_Move; } ImGui::SameLine();
            if (ImGui::RadioButton("Swap", mode == Mode_Swap)) { mode = Mode_Swap; }
            static const char* names[9] = { "Bobby", "Beatrice", "Betty", "Brianna", "Barry", "Bernard", "Bibi", "Blaine", "Bryn" };
            for (int n = 0; n < IM_ARRAYSIZE(names); n++)
            {
                ImGui::PushID(n);
                if ((n % 3) != 0)
                    ImGui::SameLine();
                ImGui::Button(names[n], ImVec2(60, 60));

                // Our buttons are both drag sources and drag targets here!
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                {
                    ImGui::SetDragDropPayload("DND_DEMO_CELL", &n, sizeof(int));    // Set payload to carry the index of our item (could be anything)
                    if (mode == Mode_Copy) { ImGui::Text("Copy %s", names[n]); }    // Display preview (could be anything, e.g. when dragging an image we could decide to display the filename and a small preview of the image, etc.)
                    if (mode == Mode_Move) { ImGui::Text("Move %s", names[n]); }
                    if (mode == Mode_Swap) { ImGui::Text("Swap %s", names[n]); }
                    ImGui::EndDragDropSource();
                }
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_DEMO_CELL"))
                    {
                        IM_ASSERT(payload->DataSize == sizeof(int));
                        int payload_n = *(const int*)payload->Data;
                        if (mode == Mode_Copy)
                        {
                            names[n] = names[payload_n];
                        }
                        if (mode == Mode_Move)
                        {
                            names[n] = names[payload_n];
                            names[payload_n] = "";
                        }
                        if (mode == Mode_Swap)
                        {
                            const char* tmp = names[n];
                            names[n] = names[payload_n];
                            names[payload_n] = tmp;
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
                ImGui::PopID();
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Drag to reorder items (simple)"))
        {
            // Simple reordering
            HelpMarker("We don't use the drag and drop api at all here! Instead we query when the item is held but not hovered, and order items accordingly.");
            static const char* item_names[] = { "Item One", "Item Two", "Item Three", "Item Four", "Item Five" };
            for (int n = 0; n < IM_ARRAYSIZE(item_names); n++)
            {
                const char* item = item_names[n];
                ImGui::Selectable(item);

                if (ImGui::IsItemActive() && !ImGui::IsItemHovered())
                {
                    int n_next = n + (ImGui::GetMouseDragDelta(0).y < 0.f ? -1 : 1);
                    if (n_next >= 0 && n_next < IM_ARRAYSIZE(item_names))
                    {
                        item_names[n] = item_names[n_next];
                        item_names[n_next] = item;
                        ImGui::ResetMouseDragDelta();
                    }
                }
            }
            ImGui::TreePop();
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Querying Status (Active/Focused/Hovered etc.)"))
    {
        // Submit an item (various types available) so we can query their status in the following block.
        static int item_type = 1;
        ImGui::Combo("Item Type", &item_type, "Text\0Button\0Button (w/ repeat)\0Checkbox\0SliderFloat\0InputText\0InputFloat\0InputFloat3\0ColorEdit4\0MenuItem\0TreeNode\0TreeNode (w/ double-click)\0ListBox\0", 20);
        ImGui::SameLine();
        HelpMarker("Testing how various types of items are interacting with the IsItemXXX functions.");
        bool ret = false;
        static bool b = false;
        static float col4f[4] = { 1.0f, 0.5, 0.0f, 1.0f };
        static char str[16] = {};
        if (item_type == 0) { ImGui::Text("ITEM: Text"); }                                              // Testing text items with no identifier/interaction
        if (item_type == 1) { ret = ImGui::Button("ITEM: Button"); }                                    // Testing button
        if (item_type == 2) { ImGui::PushButtonRepeat(true); ret = ImGui::Button("ITEM: Button"); ImGui::PopButtonRepeat(); } // Testing button (with repeater)
        if (item_type == 3) { ret = ImGui::Checkbox("ITEM: Checkbox", &b); }                            // Testing checkbox
        if (item_type == 4) { ret = ImGui::SliderFloat("ITEM: SliderFloat", &col4f[0], 0.0f, 1.0f); }   // Testing basic item
        if (item_type == 5) { ret = ImGui::InputText("ITEM: InputText", &str[0], IM_ARRAYSIZE(str)); }  // Testing input text (which handles tabbing)
        if (item_type == 6) { ret = ImGui::InputFloat("ITEM: InputFloat", col4f, 1.0f); }               // Testing +/- buttons on scalar input
        if (item_type == 7) { ret = ImGui::InputFloat3("ITEM: InputFloat3", col4f); }                   // Testing multi-component items (IsItemXXX flags are reported merged)
        if (item_type == 8) { ret = ImGui::ColorEdit4("ITEM: ColorEdit4", col4f); }                     // Testing multi-component items (IsItemXXX flags are reported merged)
        if (item_type == 9) { ret = ImGui::MenuItem("ITEM: MenuItem"); }                                // Testing menu item (they use ImGuiButtonFlags_PressedOnRelease button policy)
        if (item_type == 10) { ret = ImGui::TreeNode("ITEM: TreeNode"); if (ret) ImGui::TreePop(); }     // Testing tree node
        if (item_type == 11) { ret = ImGui::TreeNodeEx("ITEM: TreeNode w/ ImGuiTreeNodeFlags_OpenOnDoubleClick", ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_NoTreePushOnOpen); } // Testing tree node with ImGuiButtonFlags_PressedOnDoubleClick button policy.
        if (item_type == 12) { const char* items[] = { "Apple", "Banana", "Cherry", "Kiwi" }; static int current = 1; ret = ImGui::ListBox("ITEM: ListBox", &current, items, IM_ARRAYSIZE(items), IM_ARRAYSIZE(items)); }

        // Display the value of IsItemHovered() and other common item state functions.
        // Note that the ImGuiHoveredFlags_XXX flags can be combined.
        // Because BulletText is an item itself and that would affect the output of IsItemXXX functions,
        // we query every state in a single call to avoid storing them and to simplify the code
        ImGui::BulletText(
            "Return value = %d\n"
            "IsItemFocused() = %d\n"
            "IsItemHovered() = %d\n"
            "IsItemHovered(_AllowWhenBlockedByPopup) = %d\n"
            "IsItemHovered(_AllowWhenBlockedByActiveItem) = %d\n"
            "IsItemHovered(_AllowWhenOverlapped) = %d\n"
            "IsItemHovered(_RectOnly) = %d\n"
            "IsItemActive() = %d\n"
            "IsItemEdited() = %d\n"
            "IsItemActivated() = %d\n"
            "IsItemDeactivated() = %d\n"
            "IsItemDeactivatedAfterEdit() = %d\n"
            "IsItemVisible() = %d\n"
            "IsItemClicked() = %d\n"
            "IsItemToggledOpen() = %d\n"
            "GetItemRectMin() = (%.1f, %.1f)\n"
            "GetItemRectMax() = (%.1f, %.1f)\n"
            "GetItemRectSize() = (%.1f, %.1f)",
            ret,
            ImGui::IsItemFocused(),
            ImGui::IsItemHovered(),
            ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup),
            ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem),
            ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenOverlapped),
            ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly),
            ImGui::IsItemActive(),
            ImGui::IsItemEdited(),
            ImGui::IsItemActivated(),
            ImGui::IsItemDeactivated(),
            ImGui::IsItemDeactivatedAfterEdit(),
            ImGui::IsItemVisible(),
            ImGui::IsItemClicked(),
            ImGui::IsItemToggledOpen(),
            ImGui::GetItemRectMin().x, ImGui::GetItemRectMin().y,
            ImGui::GetItemRectMax().x, ImGui::GetItemRectMax().y,
            ImGui::GetItemRectSize().x, ImGui::GetItemRectSize().y
        );

        static bool embed_all_inside_a_child_window = false;
        ImGui::Checkbox("Embed everything inside a child window (for additional testing)", &embed_all_inside_a_child_window);
        if (embed_all_inside_a_child_window)
            ImGui::BeginChild("outer_child", ImVec2(0, ImGui::GetFontSize() * 20), true);

        // Testing IsWindowFocused() function with its various flags.
        // Note that the ImGuiFocusedFlags_XXX flags can be combined.
        ImGui::BulletText(
            "IsWindowFocused() = %d\n"
            "IsWindowFocused(_ChildWindows) = %d\n"
            "IsWindowFocused(_ChildWindows|_RootWindow) = %d\n"
            "IsWindowFocused(_RootWindow) = %d\n"
            "IsWindowFocused(_AnyWindow) = %d\n",
            ImGui::IsWindowFocused(),
            ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows),
            ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows | ImGuiFocusedFlags_RootWindow),
            ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow),
            ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow));

        // Testing IsWindowHovered() function with its various flags.
        // Note that the ImGuiHoveredFlags_XXX flags can be combined.
        ImGui::BulletText(
            "IsWindowHovered() = %d\n"
            "IsWindowHovered(_AllowWhenBlockedByPopup) = %d\n"
            "IsWindowHovered(_AllowWhenBlockedByActiveItem) = %d\n"
            "IsWindowHovered(_ChildWindows) = %d\n"
            "IsWindowHovered(_ChildWindows|_RootWindow) = %d\n"
            "IsWindowHovered(_ChildWindows|_AllowWhenBlockedByPopup) = %d\n"
            "IsWindowHovered(_RootWindow) = %d\n"
            "IsWindowHovered(_AnyWindow) = %d\n",
            ImGui::IsWindowHovered(),
            ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup),
            ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem),
            ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows),
            ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_RootWindow),
            ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByPopup),
            ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow),
            ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow));

        ImGui::BeginChild("child", ImVec2(0, 50), true);
        ImGui::Text("This is another child window for testing the _ChildWindows flag.");
        ImGui::EndChild();
        if (embed_all_inside_a_child_window)
            ImGui::EndChild();

        static char dummy_str[] = "This is a dummy field to be able to tab-out of the widgets above.";
        ImGui::InputText("dummy", dummy_str, IM_ARRAYSIZE(dummy_str), ImGuiInputTextFlags_ReadOnly);

        // Calling IsItemHovered() after begin returns the hovered status of the title bar.
        // This is useful in particular if you want to create a context menu (with BeginPopupContextItem) associated to the title bar of a window.
        static bool test_window = false;
        ImGui::Checkbox("Hovered/Active tests after Begin() for title bar testing", &test_window);
        if (test_window)
        {
            ImGui::Begin("Title bar Hovered/Active tests", &test_window);
            if (ImGui::BeginPopupContextItem()) // <-- This is using IsItemHovered()
            {
                if (ImGui::MenuItem("Close")) { test_window = false; }
                ImGui::EndPopup();
            }
            ImGui::Text(
                "IsItemHovered() after begin = %d (== is title bar hovered)\n"
                "IsItemActive() after begin = %d (== is window being clicked/moved)\n",
                ImGui::IsItemHovered(), ImGui::IsItemActive());
            ImGui::End();
        }

        ImGui::TreePop();
    }
}

static void ShowDemoWindowLayout()
{
    if (!ImGui::CollapsingHeader("Layout"))
        return;

    if (ImGui::TreeNode("Child windows"))
    {
        HelpMarker("Use child windows to begin into a self-contained independent scrolling/clipping regions within a host window.");
        static bool disable_mouse_wheel = false;
        static bool disable_menu = false;
        ImGui::Checkbox("Disable Mouse Wheel", &disable_mouse_wheel);
        ImGui::Checkbox("Disable Menu", &disable_menu);

        static int line = 50;
        bool goto_line = ImGui::Button("Goto");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100);
        goto_line |= ImGui::InputInt("##Line", &line, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue);

        // Child 1: no border, enable horizontal scrollbar
        {
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar | (disable_mouse_wheel ? ImGuiWindowFlags_NoScrollWithMouse : 0);
            ImGui::BeginChild("ChildL", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 260), false, window_flags);
            for (int i = 0; i < 100; i++)
            {
                ImGui::Text("%04d: scrollable region", i);
                if (goto_line && line == i)
                    ImGui::SetScrollHereY();
            }
            if (goto_line && line >= 100)
                ImGui::SetScrollHereY();
            ImGui::EndChild();
        }

        ImGui::SameLine();

        // Child 2: rounded border
        {
            ImGuiWindowFlags window_flags = (disable_mouse_wheel ? ImGuiWindowFlags_NoScrollWithMouse : 0) | (disable_menu ? 0 : ImGuiWindowFlags_MenuBar);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
            ImGui::BeginChild("ChildR", ImVec2(0, 260), true, window_flags);
            if (!disable_menu && ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("Menu"))
                {
                    ShowExampleMenuFile();
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
            ImGui::Columns(2);
            for (int i = 0; i < 100; i++)
            {
                char buf[32];
                sprintf(buf, "%03d", i);
                ImGui::Button(buf, ImVec2(-FLT_MIN, 0.0f));
                ImGui::NextColumn();
            }
            ImGui::EndChild();
            ImGui::PopStyleVar();
        }

        ImGui::Separator();

        // Demonstrate a few extra things
        // - Changing ImGuiCol_ChildBg (which is transparent black in default styles)
        // - Using SetCursorPos() to position the child window (because the child window is an item from the POV of the parent window)
        //   You can also call SetNextWindowPos() to position the child window. The parent window will effectively layout from this position.
        // - Using ImGui::GetItemRectMin/Max() to query the "item" state (because the child window is an item from the POV of the parent window)
        //   See "Widgets" -> "Querying Status (Active/Focused/Hovered etc.)" section for more details about this.
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
            ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(255, 0, 0, 100));
            ImGui::BeginChild("Red", ImVec2(200, 100), true, ImGuiWindowFlags_None);
            for (int n = 0; n < 50; n++)
                ImGui::Text("Some test %d", n);
            ImGui::EndChild();
            ImVec2 child_rect_min = ImGui::GetItemRectMin();
            ImVec2 child_rect_max = ImGui::GetItemRectMax();
            ImGui::PopStyleColor();
            ImGui::Text("Rect of child window is: (%.0f,%.0f) (%.0f,%.0f)", child_rect_min.x, child_rect_min.y, child_rect_max.x, child_rect_max.y);
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Widgets Width"))
    {
        // Use SetNextItemWidth() to set the width of a single upcoming item.
        // Use PushItemWidth()/PopItemWidth() to set the width of a group of items.
        static float f = 0.0f;
        ImGui::Text("SetNextItemWidth/PushItemWidth(100)");
        ImGui::SameLine(); HelpMarker("Fixed width.");
        ImGui::SetNextItemWidth(100);
        ImGui::DragFloat("float##1", &f);

        ImGui::Text("SetNextItemWidth/PushItemWidth(GetWindowWidth() * 0.5f)");
        ImGui::SameLine(); HelpMarker("Half of window width.");
        ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.5f);
        ImGui::DragFloat("float##2", &f);

        ImGui::Text("SetNextItemWidth/PushItemWidth(GetContentRegionAvail().x * 0.5f)");
        ImGui::SameLine(); HelpMarker("Half of available width.\n(~ right-cursor_pos)\n(works within a column set)");
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
        ImGui::DragFloat("float##3", &f);

        ImGui::Text("SetNextItemWidth/PushItemWidth(-100)");
        ImGui::SameLine(); HelpMarker("Align to right edge minus 100");
        ImGui::SetNextItemWidth(-100);
        ImGui::DragFloat("float##4", &f);

        // Demonstrate using PushItemWidth to surround three items. Calling SetNextItemWidth() before each of them would have the same effect.
        ImGui::Text("SetNextItemWidth/PushItemWidth(-1)");
        ImGui::SameLine(); HelpMarker("Align to right edge");
        ImGui::PushItemWidth(-1);
        ImGui::DragFloat("##float5a", &f);
        ImGui::DragFloat("##float5b", &f);
        ImGui::DragFloat("##float5c", &f);
        ImGui::PopItemWidth();

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Basic Horizontal Layout"))
    {
        ImGui::TextWrapped("(Use ImGui::SameLine() to keep adding items to the right of the preceding item)");

        // Text
        ImGui::Text("Two items: Hello"); ImGui::SameLine();
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Sailor");

        // Adjust spacing
        ImGui::Text("More spacing: Hello"); ImGui::SameLine(0, 20);
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Sailor");

        // Button
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Normal buttons"); ImGui::SameLine();
        ImGui::Button("Banana"); ImGui::SameLine();
        ImGui::Button("Apple"); ImGui::SameLine();
        ImGui::Button("Corniflower");

        // Button
        ImGui::Text("Small buttons"); ImGui::SameLine();
        ImGui::SmallButton("Like this one"); ImGui::SameLine();
        ImGui::Text("can fit within a text block.");

        // Aligned to arbitrary position. Easy/cheap column.
        ImGui::Text("Aligned");
        ImGui::SameLine(150); ImGui::Text("x=150");
        ImGui::SameLine(300); ImGui::Text("x=300");
        ImGui::Text("Aligned");
        ImGui::SameLine(150); ImGui::SmallButton("x=150");
        ImGui::SameLine(300); ImGui::SmallButton("x=300");

        // Checkbox
        static bool c1 = false, c2 = false, c3 = false, c4 = false;
        ImGui::Checkbox("My", &c1); ImGui::SameLine();
        ImGui::Checkbox("Tailor", &c2); ImGui::SameLine();
        ImGui::Checkbox("Is", &c3); ImGui::SameLine();
        ImGui::Checkbox("Rich", &c4);

        // Various
        static float f0 = 1.0f, f1 = 2.0f, f2 = 3.0f;
        ImGui::PushItemWidth(80);
        const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD" };
        static int item = -1;
        ImGui::Combo("Combo", &item, items, IM_ARRAYSIZE(items)); ImGui::SameLine();
        ImGui::SliderFloat("X", &f0, 0.0f, 5.0f); ImGui::SameLine();
        ImGui::SliderFloat("Y", &f1, 0.0f, 5.0f); ImGui::SameLine();
        ImGui::SliderFloat("Z", &f2, 0.0f, 5.0f);
        ImGui::PopItemWidth();

        ImGui::PushItemWidth(80);
        ImGui::Text("Lists:");
        static int selection[4] = { 0, 1, 2, 3 };
        for (int i = 0; i < 4; i++)
        {
            if (i > 0) ImGui::SameLine();
            ImGui::PushID(i);
            ImGui::ListBox("", &selection[i], items, IM_ARRAYSIZE(items));
            ImGui::PopID();
            //if (ImGui::IsItemHovered()) ImGui::SetTooltip("ListBox %d hovered", i);
        }
        ImGui::PopItemWidth();

        // Dummy
        ImVec2 button_sz(40, 40);
        ImGui::Button("A", button_sz); ImGui::SameLine();
        ImGui::Dummy(button_sz); ImGui::SameLine();
        ImGui::Button("B", button_sz);

        // Manually wrapping (we should eventually provide this as an automatic layout feature, but for now you can do it manually)
        ImGui::Text("Manually wrapping:");
        ImGuiStyle& style = ImGui::GetStyle();
        int buttons_count = 20;
        float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
        for (int n = 0; n < buttons_count; n++)
        {
            ImGui::PushID(n);
            ImGui::Button("Box", button_sz);
            float last_button_x2 = ImGui::GetItemRectMax().x;
            float next_button_x2 = last_button_x2 + style.ItemSpacing.x + button_sz.x; // Expected position if next button was on same line
            if (n + 1 < buttons_count && next_button_x2 < window_visible_x2)
                ImGui::SameLine();
            ImGui::PopID();
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Tabs"))
    {
        if (ImGui::TreeNode("Basic"))
        {
            ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
            if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
            {
                if (ImGui::BeginTabItem("Avocado"))
                {
                    ImGui::Text("This is the Avocado tab!\nblah blah blah blah blah");
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Broccoli"))
                {
                    ImGui::Text("This is the Broccoli tab!\nblah blah blah blah blah");
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Cucumber"))
                {
                    ImGui::Text("This is the Cucumber tab!\nblah blah blah blah blah");
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::Separator();
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Advanced & Close Button"))
        {
            // Expose a couple of the available flags. In most cases you may just call BeginTabBar() with no flags (0).
            static ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_Reorderable;
            ImGui::CheckboxFlags("ImGuiTabBarFlags_Reorderable", (unsigned int*)&tab_bar_flags, ImGuiTabBarFlags_Reorderable);
            ImGui::CheckboxFlags("ImGuiTabBarFlags_AutoSelectNewTabs", (unsigned int*)&tab_bar_flags, ImGuiTabBarFlags_AutoSelectNewTabs);
            ImGui::CheckboxFlags("ImGuiTabBarFlags_TabListPopupButton", (unsigned int*)&tab_bar_flags, ImGuiTabBarFlags_TabListPopupButton);
            ImGui::CheckboxFlags("ImGuiTabBarFlags_NoCloseWithMiddleMouseButton", (unsigned int*)&tab_bar_flags, ImGuiTabBarFlags_NoCloseWithMiddleMouseButton);
            if ((tab_bar_flags & ImGuiTabBarFlags_FittingPolicyMask_) == 0)
                tab_bar_flags |= ImGuiTabBarFlags_FittingPolicyDefault_;
            if (ImGui::CheckboxFlags("ImGuiTabBarFlags_FittingPolicyResizeDown", (unsigned int*)&tab_bar_flags, ImGuiTabBarFlags_FittingPolicyResizeDown))
                tab_bar_flags &= ~(ImGuiTabBarFlags_FittingPolicyMask_ ^ ImGuiTabBarFlags_FittingPolicyResizeDown);
            if (ImGui::CheckboxFlags("ImGuiTabBarFlags_FittingPolicyScroll", (unsigned int*)&tab_bar_flags, ImGuiTabBarFlags_FittingPolicyScroll))
                tab_bar_flags &= ~(ImGuiTabBarFlags_FittingPolicyMask_ ^ ImGuiTabBarFlags_FittingPolicyScroll);

            // Tab Bar
            const char* names[4] = { "Artichoke", "Beetroot", "Celery", "Daikon" };
            static bool opened[4] = { true, true, true, true }; // Persistent user state
            for (int n = 0; n < IM_ARRAYSIZE(opened); n++)
            {
                if (n > 0) { ImGui::SameLine(); }
                ImGui::Checkbox(names[n], &opened[n]);
            }

            // Passing a bool* to BeginTabItem() is similar to passing one to Begin(): the underlying bool will be set to false when the tab is closed.
            if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
            {
                for (int n = 0; n < IM_ARRAYSIZE(opened); n++)
                    if (opened[n] && ImGui::BeginTabItem(names[n], &opened[n], ImGuiTabItemFlags_None))
                    {
                        ImGui::Text("This is the %s tab!", names[n]);
                        if (n & 1)
                            ImGui::Text("I am an odd tab.");
                        ImGui::EndTabItem();
                    }
                ImGui::EndTabBar();
            }
            ImGui::Separator();
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Groups"))
    {
        HelpMarker("BeginGroup() basically locks the horizontal position for new line. EndGroup() bundles the whole group so that you can use \"item\" functions such as IsItemHovered()/IsItemActive() or SameLine() etc. on the whole group.");
        ImGui::BeginGroup();
        {
            ImGui::BeginGroup();
            ImGui::Button("AAA");
            ImGui::SameLine();
            ImGui::Button("BBB");
            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::Button("CCC");
            ImGui::Button("DDD");
            ImGui::EndGroup();
            ImGui::SameLine();
            ImGui::Button("EEE");
            ImGui::EndGroup();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("First group hovered");
        }
        // Capture the group size and create widgets using the same size
        ImVec2 size = ImGui::GetItemRectSize();
        const float values[5] = { 0.5f, 0.20f, 0.80f, 0.60f, 0.25f };
        ImGui::PlotHistogram("##values", values, IM_ARRAYSIZE(values), 0, NULL, 0.0f, 1.0f, size);

        ImGui::Button("ACTION", ImVec2((size.x - ImGui::GetStyle().ItemSpacing.x) * 0.5f, size.y));
        ImGui::SameLine();
        ImGui::Button("REACTION", ImVec2((size.x - ImGui::GetStyle().ItemSpacing.x) * 0.5f, size.y));
        ImGui::EndGroup();
        ImGui::SameLine();

        ImGui::Button("LEVERAGE\nBUZZWORD", size);
        ImGui::SameLine();

        if (ImGui::ListBoxHeader("List", size))
        {
            ImGui::Selectable("Selected", true);
            ImGui::Selectable("Not Selected", false);
            ImGui::ListBoxFooter();
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Text Baseline Alignment"))
    {
        {
            ImGui::BulletText("Text baseline:");
            ImGui::SameLine();
            HelpMarker("This is testing the vertical alignment that gets applied on text to keep it aligned with widgets. Lines only composed of text or \"small\" widgets fit in less vertical spaces than lines with normal widgets.");
            ImGui::Indent();

            ImGui::Text("KO Blahblah"); ImGui::SameLine();
            ImGui::Button("Some framed item"); ImGui::SameLine();
            HelpMarker("Baseline of button will look misaligned with text..");

            // If your line starts with text, call AlignTextToFramePadding() to align text to upcoming widgets.
            // Because we don't know what's coming after the Text() statement, we need to move the text baseline down by FramePadding.y
            ImGui::AlignTextToFramePadding();
            ImGui::Text("OK Blahblah"); ImGui::SameLine();
            ImGui::Button("Some framed item"); ImGui::SameLine();
            HelpMarker("We call AlignTextToFramePadding() to vertically align the text baseline by +FramePadding.y");

            // SmallButton() uses the same vertical padding as Text
            ImGui::Button("TEST##1"); ImGui::SameLine();
            ImGui::Text("TEST"); ImGui::SameLine();
            ImGui::SmallButton("TEST##2");

            // If your line starts with text, call AlignTextToFramePadding() to align text to upcoming widgets.
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Text aligned to framed item"); ImGui::SameLine();
            ImGui::Button("Item##1"); ImGui::SameLine();
            ImGui::Text("Item"); ImGui::SameLine();
            ImGui::SmallButton("Item##2"); ImGui::SameLine();
            ImGui::Button("Item##3");

            ImGui::Unindent();
        }

        ImGui::Spacing();

        {
            ImGui::BulletText("Multi-line text:");
            ImGui::Indent();
            ImGui::Text("One\nTwo\nThree"); ImGui::SameLine();
            ImGui::Text("Hello\nWorld"); ImGui::SameLine();
            ImGui::Text("Banana");

            ImGui::Text("Banana"); ImGui::SameLine();
            ImGui::Text("Hello\nWorld"); ImGui::SameLine();
            ImGui::Text("One\nTwo\nThree");

            ImGui::Button("HOP##1"); ImGui::SameLine();
            ImGui::Text("Banana"); ImGui::SameLine();
            ImGui::Text("Hello\nWorld"); ImGui::SameLine();
            ImGui::Text("Banana");

            ImGui::Button("HOP##2"); ImGui::SameLine();
            ImGui::Text("Hello\nWorld"); ImGui::SameLine();
            ImGui::Text("Banana");
            ImGui::Unindent();
        }

        ImGui::Spacing();

        {
            ImGui::BulletText("Misc items:");
            ImGui::Indent();

            // SmallButton() sets FramePadding to zero. Text baseline is aligned to match baseline of previous Button
            ImGui::Button("80x80", ImVec2(80, 80));
            ImGui::SameLine();
            ImGui::Button("50x50", ImVec2(50, 50));
            ImGui::SameLine();
            ImGui::Button("Button()");
            ImGui::SameLine();
            ImGui::SmallButton("SmallButton()");

            // Tree
            const float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
            ImGui::Button("Button##1");
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::TreeNode("Node##1")) { for (int i = 0; i < 6; i++) ImGui::BulletText("Item %d..", i); ImGui::TreePop(); }    // Dummy tree data

            ImGui::AlignTextToFramePadding();           // Vertically align text node a bit lower so it'll be vertically centered with upcoming widget. Otherwise you can use SmallButton (smaller fit).
            bool node_open = ImGui::TreeNode("Node##2");// Common mistake to avoid: if we want to SameLine after TreeNode we need to do it before we add child content.
            ImGui::SameLine(0.0f, spacing); ImGui::Button("Button##2");
            if (node_open) { for (int i = 0; i < 6; i++) ImGui::BulletText("Item %d..", i); ImGui::TreePop(); }   // Dummy tree data

            // Bullet
            ImGui::Button("Button##3");
            ImGui::SameLine(0.0f, spacing);
            ImGui::BulletText("Bullet text");

            ImGui::AlignTextToFramePadding();
            ImGui::BulletText("Node");
            ImGui::SameLine(0.0f, spacing); ImGui::Button("Button##4");
            ImGui::Unindent();
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Scrolling"))
    {
        // Vertical scroll functions
        HelpMarker("Use SetScrollHereY() or SetScrollFromPosY() to scroll to a given vertical position.");

        static int track_item = 50;
        static bool enable_track = true;
        static bool enable_extra_decorations = false;
        static float scroll_to_off_px = 0.0f;
        static float scroll_to_pos_px = 200.0f;

        ImGui::Checkbox("Decoration", &enable_extra_decorations);
        ImGui::SameLine();
        HelpMarker("We expose this for testing because scrolling sometimes had issues with window decoration such as menu-bars.");

        ImGui::Checkbox("Track", &enable_track);
        ImGui::PushItemWidth(100);
        ImGui::SameLine(140); enable_track |= ImGui::DragInt("##item", &track_item, 0.25f, 0, 99, "Item = %d");

        bool scroll_to_off = ImGui::Button("Scroll Offset");
        ImGui::SameLine(140); scroll_to_off |= ImGui::DragFloat("##off", &scroll_to_off_px, 1.00f, 0, FLT_MAX, "+%.0f px");

        bool scroll_to_pos = ImGui::Button("Scroll To Pos");
        ImGui::SameLine(140); scroll_to_pos |= ImGui::DragFloat("##pos", &scroll_to_pos_px, 1.00f, -10, FLT_MAX, "X/Y = %.0f px");
        ImGui::PopItemWidth();

        if (scroll_to_off || scroll_to_pos)
            enable_track = false;

        ImGuiStyle& style = ImGui::GetStyle();
        float child_w = (ImGui::GetContentRegionAvail().x - 4 * style.ItemSpacing.x) / 5;
        if (child_w < 1.0f)
            child_w = 1.0f;
        ImGui::PushID("##VerticalScrolling");
        for (int i = 0; i < 5; i++)
        {
            if (i > 0) ImGui::SameLine();
            ImGui::BeginGroup();
            const char* names[] = { "Top", "25%", "Center", "75%", "Bottom" };
            ImGui::TextUnformatted(names[i]);

            ImGuiWindowFlags child_flags = enable_extra_decorations ? ImGuiWindowFlags_MenuBar : 0;
            bool window_visible = ImGui::BeginChild(ImGui::GetID((void*)(intptr_t)i), ImVec2(child_w, 200.0f), true, child_flags);
            if (ImGui::BeginMenuBar())
            {
                ImGui::TextUnformatted("abc");
                ImGui::EndMenuBar();
            }
            if (scroll_to_off)
                ImGui::SetScrollY(scroll_to_off_px);
            if (scroll_to_pos)
                ImGui::SetScrollFromPosY(ImGui::GetCursorStartPos().y + scroll_to_pos_px, i * 0.25f);
            if (window_visible) // Avoid calling SetScrollHereY when running with culled items
            {
                for (int item = 0; item < 100; item++)
                {
                    if (enable_track && item == track_item)
                    {
                        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Item %d", item);
                        ImGui::SetScrollHereY(i * 0.25f); // 0.0f:top, 0.5f:center, 1.0f:bottom
                    } else
                    {
                        ImGui::Text("Item %d", item);
                    }
                }
            }
            float scroll_y = ImGui::GetScrollY();
            float scroll_max_y = ImGui::GetScrollMaxY();
            ImGui::EndChild();
            ImGui::Text("%.0f/%.0f", scroll_y, scroll_max_y);
            ImGui::EndGroup();
        }
        ImGui::PopID();

        // Horizontal scroll functions
        ImGui::Spacing();
        HelpMarker("Use SetScrollHereX() or SetScrollFromPosX() to scroll to a given horizontal position.\n\nUsing the \"Scroll To Pos\" button above will make the discontinuity at edges visible: scrolling to the top/bottom/left/right-most item will add an additional WindowPadding to reflect on reaching the edge of the list.\n\nBecause the clipping rectangle of most window hides half worth of WindowPadding on the left/right, using SetScrollFromPosX(+1) will usually result in clipped text whereas the equivalent SetScrollFromPosY(+1) wouldn't.");
        ImGui::PushID("##HorizontalScrolling");
        for (int i = 0; i < 5; i++)
        {
            float child_height = ImGui::GetTextLineHeight() + style.ScrollbarSize + style.WindowPadding.y * 2.0f;
            ImGuiWindowFlags child_flags = ImGuiWindowFlags_HorizontalScrollbar | (enable_extra_decorations ? ImGuiWindowFlags_AlwaysVerticalScrollbar : 0);
            bool window_visible = ImGui::BeginChild(ImGui::GetID((void*)(intptr_t)i), ImVec2(-100, child_height), true, child_flags);
            if (scroll_to_off)
                ImGui::SetScrollX(scroll_to_off_px);
            if (scroll_to_pos)
                ImGui::SetScrollFromPosX(ImGui::GetCursorStartPos().x + scroll_to_pos_px, i * 0.25f);
            if (window_visible) // Avoid calling SetScrollHereY when running with culled items
            {
                for (int item = 0; item < 100; item++)
                {
                    if (enable_track && item == track_item)
                    {
                        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Item %d", item);
                        ImGui::SetScrollHereX(i * 0.25f); // 0.0f:left, 0.5f:center, 1.0f:right
                    } else
                    {
                        ImGui::Text("Item %d", item);
                    }
                    ImGui::SameLine();
                }
            }
            float scroll_x = ImGui::GetScrollX();
            float scroll_max_x = ImGui::GetScrollMaxX();
            ImGui::EndChild();
            ImGui::SameLine();
            const char* names[] = { "Left", "25%", "Center", "75%", "Right" };
            ImGui::Text("%s\n%.0f/%.0f", names[i], scroll_x, scroll_max_x);
            ImGui::Spacing();
        }
        ImGui::PopID();

        // Miscellaneous Horizontal Scrolling Demo
        HelpMarker("Horizontal scrolling for a window has to be enabled explicitly via the ImGuiWindowFlags_HorizontalScrollbar flag.\n\nYou may want to explicitly specify content width by calling SetNextWindowContentWidth() before Begin().");
        static int lines = 7;
        ImGui::SliderInt("Lines", &lines, 1, 15);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 1.0f));
        ImGui::BeginChild("scrolling", ImVec2(0, ImGui::GetFrameHeightWithSpacing() * 7 + 30), true, ImGuiWindowFlags_HorizontalScrollbar);
        for (int line = 0; line < lines; line++)
        {
            // Display random stuff (for the sake of this trivial demo we are using basic Button+SameLine. If you want to create your own time line for a real application you may be better off
            // manipulating the cursor position yourself, aka using SetCursorPos/SetCursorScreenPos to position the widgets yourself. You may also want to use the lower-level ImDrawList API)
            int num_buttons = 10 + ((line & 1) ? line * 9 : line * 3);
            for (int n = 0; n < num_buttons; n++)
            {
                if (n > 0) ImGui::SameLine();
                ImGui::PushID(n + line * 1000);
                char num_buf[16];
                sprintf(num_buf, "%d", n);
                const char* label = (!(n % 15)) ? "FizzBuzz" : (!(n % 3)) ? "Fizz" : (!(n % 5)) ? "Buzz" : num_buf;
                float hue = n * 0.05f;
                ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(hue, 0.6f, 0.6f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(hue, 0.7f, 0.7f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(hue, 0.8f, 0.8f));
                ImGui::Button(label, ImVec2(40.0f + sinf((float)(line + n)) * 20.0f, 0.0f));
                ImGui::PopStyleColor(3);
                ImGui::PopID();
            }
        }
        float scroll_x = ImGui::GetScrollX();
        float scroll_max_x = ImGui::GetScrollMaxX();
        ImGui::EndChild();
        ImGui::PopStyleVar(2);
        float scroll_x_delta = 0.0f;
        ImGui::SmallButton("<<"); if (ImGui::IsItemActive()) { scroll_x_delta = -ImGui::GetIO().DeltaTime * 1000.0f; } ImGui::SameLine();
        ImGui::Text("Scroll from code"); ImGui::SameLine();
        ImGui::SmallButton(">>"); if (ImGui::IsItemActive()) { scroll_x_delta = +ImGui::GetIO().DeltaTime * 1000.0f; } ImGui::SameLine();
        ImGui::Text("%.0f/%.0f", scroll_x, scroll_max_x);
        if (scroll_x_delta != 0.0f)
        {
            ImGui::BeginChild("scrolling"); // Demonstrate a trick: you can use Begin to set yourself in the context of another window (here we are already out of your child window)
            ImGui::SetScrollX(ImGui::GetScrollX() + scroll_x_delta);
            ImGui::EndChild();
        }
        ImGui::Spacing();

        static bool show_horizontal_contents_size_demo_window = false;
        ImGui::Checkbox("Show Horizontal contents size demo window", &show_horizontal_contents_size_demo_window);

        if (show_horizontal_contents_size_demo_window)
        {
            static bool show_h_scrollbar = true;
            static bool show_button = true;
            static bool show_tree_nodes = true;
            static bool show_text_wrapped = false;
            static bool show_columns = true;
            static bool show_tab_bar = true;
            static bool show_child = false;
            static bool explicit_content_size = false;
            static float contents_size_x = 300.0f;
            if (explicit_content_size)
                ImGui::SetNextWindowContentSize(ImVec2(contents_size_x, 0.0f));
            ImGui::Begin("Horizontal contents size demo window", &show_horizontal_contents_size_demo_window, show_h_scrollbar ? ImGuiWindowFlags_HorizontalScrollbar : 0);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 0));
            HelpMarker("Test of different widgets react and impact the work rectangle growing when horizontal scrolling is enabled.\n\nUse 'Metrics->Tools->Show windows rectangles' to visualize rectangles.");
            ImGui::Checkbox("H-scrollbar", &show_h_scrollbar);
            ImGui::Checkbox("Button", &show_button);            // Will grow contents size (unless explicitly overwritten)
            ImGui::Checkbox("Tree nodes", &show_tree_nodes);    // Will grow contents size and display highlight over full width
            ImGui::Checkbox("Text wrapped", &show_text_wrapped);// Will grow and use contents size
            ImGui::Checkbox("Columns", &show_columns);          // Will use contents size
            ImGui::Checkbox("Tab bar", &show_tab_bar);          // Will use contents size
            ImGui::Checkbox("Child", &show_child);              // Will grow and use contents size
            ImGui::Checkbox("Explicit content size", &explicit_content_size);
            ImGui::Text("Scroll %.1f/%.1f %.1f/%.1f", ImGui::GetScrollX(), ImGui::GetScrollMaxX(), ImGui::GetScrollY(), ImGui::GetScrollMaxY());
            if (explicit_content_size)
            {
                ImGui::SameLine();
                ImGui::SetNextItemWidth(100);
                ImGui::DragFloat("##csx", &contents_size_x);
                ImVec2 p = ImGui::GetCursorScreenPos();
                ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + 10, p.y + 10), IM_COL32_WHITE);
                ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(p.x + contents_size_x - 10, p.y), ImVec2(p.x + contents_size_x, p.y + 10), IM_COL32_WHITE);
                ImGui::Dummy(ImVec2(0, 10));
            }
            ImGui::PopStyleVar(2);
            ImGui::Separator();
            if (show_button)
            {
                ImGui::Button("this is a 300-wide button", ImVec2(300, 0));
            }
            if (show_tree_nodes)
            {
                bool open = true;
                if (ImGui::TreeNode("this is a tree node"))
                {
                    if (ImGui::TreeNode("another one of those tree node..."))
                    {
                        ImGui::Text("Some tree contents");
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
                ImGui::CollapsingHeader("CollapsingHeader", &open);
            }
            if (show_text_wrapped)
            {
                ImGui::TextWrapped("This text should automatically wrap on the edge of the work rectangle.");
            }
            if (show_columns)
            {
                ImGui::Columns(4);
                for (int n = 0; n < 4; n++)
                {
                    ImGui::Text("Width %.2f", ImGui::GetColumnWidth());
                    ImGui::NextColumn();
                }
                ImGui::Columns(1);
            }
            if (show_tab_bar && ImGui::BeginTabBar("Hello"))
            {
                if (ImGui::BeginTabItem("OneOneOne")) { ImGui::EndTabItem(); }
                if (ImGui::BeginTabItem("TwoTwoTwo")) { ImGui::EndTabItem(); }
                if (ImGui::BeginTabItem("ThreeThreeThree")) { ImGui::EndTabItem(); }
                if (ImGui::BeginTabItem("FourFourFour")) { ImGui::EndTabItem(); }
                ImGui::EndTabBar();
            }
            if (show_child)
            {
                ImGui::BeginChild("child", ImVec2(0, 0), true);
                ImGui::EndChild();
            }
            ImGui::End();
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Clipping"))
    {
        static ImVec2 size(100, 100), offset(50, 20);
        ImGui::TextWrapped("On a per-widget basis we are occasionally clipping text CPU-side if it won't fit in its frame. Otherwise we are doing coarser clipping + passing a scissor rectangle to the renderer. The system is designed to try minimizing both execution and CPU/GPU rendering cost.");
        ImGui::DragFloat2("size", (float*)&size, 0.5f, 1.0f, 200.0f, "%.0f");
        ImGui::TextWrapped("(Click and drag)");
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec4 clip_rect(pos.x, pos.y, pos.x + size.x, pos.y + size.y);
        ImGui::InvisibleButton("##dummy", size);
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0)) { offset.x += ImGui::GetIO().MouseDelta.x; offset.y += ImGui::GetIO().MouseDelta.y; }
        ImGui::GetWindowDrawList()->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(90, 90, 120, 255));
        ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize() * 2.0f, ImVec2(pos.x + offset.x, pos.y + offset.y), IM_COL32(255, 255, 255, 255), "Line 1 hello\nLine 2 clip me!", NULL, 0.0f, &clip_rect);
        ImGui::TreePop();
    }
}

static void ShowDemoWindowPopups()
{
    if (!ImGui::CollapsingHeader("Popups & Modal windows"))
        return;

    // The properties of popups windows are:
    // - They block normal mouse hovering detection outside them. (*)
    // - Unless modal, they can be closed by clicking anywhere outside them, or by pressing ESCAPE.
    // - Their visibility state (~bool) is held internally by Dear ImGui instead of being held by the programmer as we are used to with regular Begin() calls.
    //   User can manipulate the visibility state by calling OpenPopup().
    // (*) One can use IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) to bypass it and detect hovering even when normally blocked by a popup.
    // Those three properties are connected. The library needs to hold their visibility state because it can close popups at any time.

    // Typical use for regular windows:
    //   bool my_tool_is_active = false; if (ImGui::Button("Open")) my_tool_is_active = true; [...] if (my_tool_is_active) Begin("My Tool", &my_tool_is_active) { [...] } End();
    // Typical use for popups:
    //   if (ImGui::Button("Open")) ImGui::OpenPopup("MyPopup"); if (ImGui::BeginPopup("MyPopup") { [...] EndPopup(); }

    // With popups we have to go through a library call (here OpenPopup) to manipulate the visibility state.
    // This may be a bit confusing at first but it should quickly make sense. Follow on the examples below.

    if (ImGui::TreeNode("Popups"))
    {
        ImGui::TextWrapped("When a popup is active, it inhibits interacting with windows that are behind the popup. Clicking outside the popup closes it.");

        static int selected_fish = -1;
        const char* names[] = { "Bream", "Haddock", "Mackerel", "Pollock", "Tilefish" };
        static bool toggles[] = { true, false, false, false, false };

        // Simple selection popup
        // (If you want to show the current selection inside the Button itself, you may want to build a string using the "###" operator to preserve a constant ID with a variable label)
        if (ImGui::Button("Select.."))
            ImGui::OpenPopup("my_select_popup");
        ImGui::SameLine();
        ImGui::TextUnformatted(selected_fish == -1 ? "<None>" : names[selected_fish]);
        if (ImGui::BeginPopup("my_select_popup"))
        {
            ImGui::Text("Aquarium");
            ImGui::Separator();
            for (int i = 0; i < IM_ARRAYSIZE(names); i++)
                if (ImGui::Selectable(names[i]))
                    selected_fish = i;
            ImGui::EndPopup();
        }

        // Showing a menu with toggles
        if (ImGui::Button("Toggle.."))
            ImGui::OpenPopup("my_toggle_popup");
        if (ImGui::BeginPopup("my_toggle_popup"))
        {
            for (int i = 0; i < IM_ARRAYSIZE(names); i++)
                ImGui::MenuItem(names[i], "", &toggles[i]);
            if (ImGui::BeginMenu("Sub-menu"))
            {
                ImGui::MenuItem("Click me");
                ImGui::EndMenu();
            }

            ImGui::Separator();
            ImGui::Text("Tooltip here");
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("I am a tooltip over a popup");

            if (ImGui::Button("Stacked Popup"))
                ImGui::OpenPopup("another popup");
            if (ImGui::BeginPopup("another popup"))
            {
                for (int i = 0; i < IM_ARRAYSIZE(names); i++)
                    ImGui::MenuItem(names[i], "", &toggles[i]);
                if (ImGui::BeginMenu("Sub-menu"))
                {
                    ImGui::MenuItem("Click me");
                    if (ImGui::Button("Stacked Popup"))
                        ImGui::OpenPopup("another popup");
                    if (ImGui::BeginPopup("another popup"))
                    {
                        ImGui::Text("I am the last one here.");
                        ImGui::EndPopup();
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndPopup();
            }
            ImGui::EndPopup();
        }

        // Call the more complete ShowExampleMenuFile which we use in various places of this demo
        if (ImGui::Button("File Menu.."))
            ImGui::OpenPopup("my_file_popup");
        if (ImGui::BeginPopup("my_file_popup"))
        {
            ShowExampleMenuFile();
            ImGui::EndPopup();
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Context menus"))
    {
        // BeginPopupContextItem() is a helper to provide common/simple popup behavior of essentially doing:
        //    if (IsItemHovered() && IsMouseReleased(0))
        //       OpenPopup(id);
        //    return BeginPopup(id);
        // For more advanced uses you may want to replicate and cuztomize this code. This the comments inside BeginPopupContextItem() implementation.
        static float value = 0.5f;
        ImGui::Text("Value = %.3f (<-- right-click here)", value);
        if (ImGui::BeginPopupContextItem("item context menu"))
        {
            if (ImGui::Selectable("Set to zero")) value = 0.0f;
            if (ImGui::Selectable("Set to PI")) value = 3.1415f;
            ImGui::SetNextItemWidth(-1);
            ImGui::DragFloat("##Value", &value, 0.1f, 0.0f, 0.0f);
            ImGui::EndPopup();
        }

        // We can also use OpenPopupOnItemClick() which is the same as BeginPopupContextItem() but without the Begin call.
        // So here we will make it that clicking on the text field with the right mouse button (1) will toggle the visibility of the popup above.
        ImGui::Text("(You can also right-click me to open the same popup as above.)");
        ImGui::OpenPopupOnItemClick("item context menu", 1);

        // When used after an item that has an ID (here the Button), we can skip providing an ID to BeginPopupContextItem().
        // BeginPopupContextItem() will use the last item ID as the popup ID.
        // In addition here, we want to include your editable label inside the button label. We use the ### operator to override the ID (read FAQ about ID for details)
        static char name[32] = "Label1";
        char buf[64]; sprintf(buf, "Button: %s###Button", name); // ### operator override ID ignoring the preceding label
        ImGui::Button(buf);
        if (ImGui::BeginPopupContextItem())
        {
            ImGui::Text("Edit name:");
            ImGui::InputText("##edit", name, IM_ARRAYSIZE(name));
            if (ImGui::Button("Close"))
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
        ImGui::SameLine(); ImGui::Text("(<-- right-click here)");

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Modals"))
    {
        ImGui::TextWrapped("Modal windows are like popups but the user cannot close them by clicking outside the window.");

        if (ImGui::Button("Delete.."))
            ImGui::OpenPopup("Delete?");

        if (ImGui::BeginPopupModal("Delete?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("All those beautiful files will be deleted.\nThis operation cannot be undone!\n\n");
            ImGui::Separator();

            //static int dummy_i = 0;
            //ImGui::Combo("Combo", &dummy_i, "Delete\0Delete harder\0");

            static bool dont_ask_me_next_time = false;
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            ImGui::Checkbox("Don't ask me next time", &dont_ask_me_next_time);
            ImGui::PopStyleVar();

            if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
            ImGui::EndPopup();
        }

        if (ImGui::Button("Stacked modals.."))
            ImGui::OpenPopup("Stacked 1");
        if (ImGui::BeginPopupModal("Stacked 1", NULL, ImGuiWindowFlags_MenuBar))
        {
            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Dummy menu item")) {}
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
            ImGui::Text("Hello from Stacked The First\nUsing style.Colors[ImGuiCol_ModalWindowDimBg] behind it.");

            // Testing behavior of widgets stacking their own regular popups over the modal.
            static int item = 1;
            static float color[4] = { 0.4f,0.7f,0.0f,0.5f };
            ImGui::Combo("Combo", &item, "aaaa\0bbbb\0cccc\0dddd\0eeee\0\0");
            ImGui::ColorEdit4("color", color);

            if (ImGui::Button("Add another modal.."))
                ImGui::OpenPopup("Stacked 2");

            // Also demonstrate passing a bool* to BeginPopupModal(), this will create a regular close button which will close the popup.
            // Note that the visibility state of popups is owned by imgui, so the input value of the bool actually doesn't matter here.
            bool dummy_open = true;
            if (ImGui::BeginPopupModal("Stacked 2", &dummy_open))
            {
                ImGui::Text("Hello from Stacked The Second!");
                if (ImGui::Button("Close"))
                    ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
            }

            if (ImGui::Button("Close"))
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Menus inside a regular window"))
    {
        ImGui::TextWrapped("Below we are testing adding menu items to a regular window. It's rather unusual but should work!");
        ImGui::Separator();
        // NB: As a quirk in this very specific example, we want to differentiate the parent of this menu from the parent of the various popup menus above.
        // To do so we are encloding the items in a PushID()/PopID() block to make them two different menusets. If we don't, opening any popup above and hovering our menu here
        // would open it. This is because once a menu is active, we allow to switch to a sibling menu by just hovering on it, which is the desired behavior for regular menus.
        ImGui::PushID("foo");
        ImGui::MenuItem("Menu item", "CTRL+M");
        if (ImGui::BeginMenu("Menu inside a regular window"))
        {
            ShowExampleMenuFile();
            ImGui::EndMenu();
        }
        ImGui::PopID();
        ImGui::Separator();
        ImGui::TreePop();
    }
}

static void ShowDemoWindowColumns()
{
    if (!ImGui::CollapsingHeader("Columns"))
        return;

    ImGui::PushID("Columns");

    static bool disable_indent = false;
    ImGui::Checkbox("Disable tree indentation", &disable_indent);
    ImGui::SameLine();
    HelpMarker("Disable the indenting of tree nodes so demo columns can use the full window width.");
    if (disable_indent)
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0.0f);

    // Basic columns
    if (ImGui::TreeNode("Basic"))
    {
        ImGui::Text("Without border:");
        ImGui::Columns(3, "mycolumns3", false);  // 3-ways, no border
        ImGui::Separator();
        for (int n = 0; n < 14; n++)
        {
            char label[32];
            sprintf(label, "Item %d", n);
            if (ImGui::Selectable(label)) {}
            //if (ImGui::Button(label, ImVec2(-FLT_MIN,0.0f))) {}
            ImGui::NextColumn();
        }
        ImGui::Columns(1);
        ImGui::Separator();

        ImGui::Text("With border:");
        ImGui::Columns(4, "mycolumns"); // 4-ways, with border
        ImGui::Separator();
        ImGui::Text("ID"); ImGui::NextColumn();
        ImGui::Text("Name"); ImGui::NextColumn();
        ImGui::Text("Path"); ImGui::NextColumn();
        ImGui::Text("Hovered"); ImGui::NextColumn();
        ImGui::Separator();
        const char* names[3] = { "One", "Two", "Three" };
        const char* paths[3] = { "/path/one", "/path/two", "/path/three" };
        static int selected = -1;
        for (int i = 0; i < 3; i++)
        {
            char label[32];
            sprintf(label, "%04d", i);
            if (ImGui::Selectable(label, selected == i, ImGuiSelectableFlags_SpanAllColumns))
                selected = i;
            bool hovered = ImGui::IsItemHovered();
            ImGui::NextColumn();
            ImGui::Text(names[i]); ImGui::NextColumn();
            ImGui::Text(paths[i]); ImGui::NextColumn();
            ImGui::Text("%d", hovered); ImGui::NextColumn();
        }
        ImGui::Columns(1);
        ImGui::Separator();
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Borders"))
    {
        // NB: Future columns API should allow automatic horizontal borders.
        static bool h_borders = true;
        static bool v_borders = true;
        static int columns_count = 4;
        const int lines_count = 3;
        ImGui::SetNextItemWidth(ImGui::GetFontSize() * 8);
        ImGui::DragInt("##columns_count", &columns_count, 0.1f, 2, 10, "%d columns");
        if (columns_count < 2)
            columns_count = 2;
        ImGui::SameLine();
        ImGui::Checkbox("horizontal", &h_borders);
        ImGui::SameLine();
        ImGui::Checkbox("vertical", &v_borders);
        ImGui::Columns(columns_count, NULL, v_borders);
        for (int i = 0; i < columns_count * lines_count; i++)
        {
            if (h_borders && ImGui::GetColumnIndex() == 0)
                ImGui::Separator();
            ImGui::Text("%c%c%c", 'a' + i, 'a' + i, 'a' + i);
            ImGui::Text("Width %.2f", ImGui::GetColumnWidth());
            ImGui::Text("Avail %.2f", ImGui::GetContentRegionAvail().x);
            ImGui::Text("Offset %.2f", ImGui::GetColumnOffset());
            ImGui::Text("Long text that is likely to clip");
            ImGui::Button("Button", ImVec2(-FLT_MIN, 0.0f));
            ImGui::NextColumn();
        }
        ImGui::Columns(1);
        if (h_borders)
            ImGui::Separator();
        ImGui::TreePop();
    }

    // Create multiple items in a same cell before switching to next column
    if (ImGui::TreeNode("Mixed items"))
    {
        ImGui::Columns(3, "mixed");
        ImGui::Separator();

        ImGui::Text("Hello");
        ImGui::Button("Banana");
        ImGui::NextColumn();

        ImGui::Text("ImGui");
        ImGui::Button("Apple");
        static float foo = 1.0f;
        ImGui::InputFloat("red", &foo, 0.05f, 0, "%.3f");
        ImGui::Text("An extra line here.");
        ImGui::NextColumn();

        ImGui::Text("Sailor");
        ImGui::Button("Corniflower");
        static float bar = 1.0f;
        ImGui::InputFloat("blue", &bar, 0.05f, 0, "%.3f");
        ImGui::NextColumn();

        if (ImGui::CollapsingHeader("Category A")) { ImGui::Text("Blah blah blah"); } ImGui::NextColumn();
        if (ImGui::CollapsingHeader("Category B")) { ImGui::Text("Blah blah blah"); } ImGui::NextColumn();
        if (ImGui::CollapsingHeader("Category C")) { ImGui::Text("Blah blah blah"); } ImGui::NextColumn();
        ImGui::Columns(1);
        ImGui::Separator();
        ImGui::TreePop();
    }

    // Word wrapping
    if (ImGui::TreeNode("Word-wrapping"))
    {
        ImGui::Columns(2, "word-wrapping");
        ImGui::Separator();
        ImGui::TextWrapped("The quick brown fox jumps over the lazy dog.");
        ImGui::TextWrapped("Hello Left");
        ImGui::NextColumn();
        ImGui::TextWrapped("The quick brown fox jumps over the lazy dog.");
        ImGui::TextWrapped("Hello Right");
        ImGui::Columns(1);
        ImGui::Separator();
        ImGui::TreePop();
    }

    // Scrolling columns
    /*
    if (ImGui::TreeNode("Vertical Scrolling"))
    {
        ImGui::BeginChild("##header", ImVec2(0, ImGui::GetTextLineHeightWithSpacing()+ImGui::GetStyle().ItemSpacing.y));
        ImGui::Columns(3);
        ImGui::Text("ID"); ImGui::NextColumn();
        ImGui::Text("Name"); ImGui::NextColumn();
        ImGui::Text("Path"); ImGui::NextColumn();
        ImGui::Columns(1);
        ImGui::Separator();
        ImGui::EndChild();
        ImGui::BeginChild("##scrollingregion", ImVec2(0, 60));
        ImGui::Columns(3);
        for (int i = 0; i < 10; i++)
        {
            ImGui::Text("%04d", i); ImGui::NextColumn();
            ImGui::Text("Foobar"); ImGui::NextColumn();
            ImGui::Text("/path/foobar/%04d/", i); ImGui::NextColumn();
        }
        ImGui::Columns(1);
        ImGui::EndChild();
        ImGui::TreePop();
    }
    */

    if (ImGui::TreeNode("Horizontal Scrolling"))
    {
        ImGui::SetNextWindowContentSize(ImVec2(1500.0f, 0.0f));
        ImGui::BeginChild("##ScrollingRegion", ImVec2(0, ImGui::GetFontSize() * 20), false, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::Columns(10);
        int ITEMS_COUNT = 2000;
        ImGuiListClipper clipper(ITEMS_COUNT);  // Also demonstrate using the clipper for large list
        while (clipper.Step())
        {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
                for (int j = 0; j < 10; j++)
                {
                    ImGui::Text("Line %d Column %d...", i, j);
                    ImGui::NextColumn();
                }
        }
        ImGui::Columns(1);
        ImGui::EndChild();
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Tree"))
    {
        ImGui::Columns(2, "tree", true);
        for (int x = 0; x < 3; x++)
        {
            bool open1 = ImGui::TreeNode((void*)(intptr_t)x, "Node%d", x);
            ImGui::NextColumn();
            ImGui::Text("Node contents");
            ImGui::NextColumn();
            if (open1)
            {
                for (int y = 0; y < 3; y++)
                {
                    bool open2 = ImGui::TreeNode((void*)(intptr_t)y, "Node%d.%d", x, y);
                    ImGui::NextColumn();
                    ImGui::Text("Node contents");
                    if (open2)
                    {
                        ImGui::Text("Even more contents");
                        if (ImGui::TreeNode("Tree in column"))
                        {
                            ImGui::Text("The quick brown fox jumps over the lazy dog");
                            ImGui::TreePop();
                        }
                    }
                    ImGui::NextColumn();
                    if (open2)
                        ImGui::TreePop();
                }
                ImGui::TreePop();
            }
        }
        ImGui::Columns(1);
        ImGui::TreePop();
    }

    if (disable_indent)
        ImGui::PopStyleVar();
    ImGui::PopID();
}

static void ShowDemoWindowMisc()
{
    if (ImGui::CollapsingHeader("Filtering"))
    {
        // Helper class to easy setup a text filter.
        // You may want to implement a more feature-full filtering scheme in your own application.
        static ImGuiTextFilter filter;
        ImGui::Text("Filter usage:\n"
            "  \"\"         display all lines\n"
            "  \"xxx\"      display lines containing \"xxx\"\n"
            "  \"xxx,yyy\"  display lines containing \"xxx\" or \"yyy\"\n"
            "  \"-xxx\"     hide lines containing \"xxx\"");
        filter.Draw();
        const char* lines[] = { "aaa1.c", "bbb1.c", "ccc1.c", "aaa2.cpp", "bbb2.cpp", "ccc2.cpp", "abc.h", "hello, world" };
        for (int i = 0; i < IM_ARRAYSIZE(lines); i++)
            if (filter.PassFilter(lines[i]))
                ImGui::BulletText("%s", lines[i]);
    }

    if (ImGui::CollapsingHeader("Inputs, Navigation & Focus"))
    {
        ImGuiIO& io = ImGui::GetIO();

        // Display ImGuiIO output flags
        ImGui::Text("WantCaptureMouse: %d", io.WantCaptureMouse);
        ImGui::Text("WantCaptureKeyboard: %d", io.WantCaptureKeyboard);
        ImGui::Text("WantTextInput: %d", io.WantTextInput);
        ImGui::Text("WantSetMousePos: %d", io.WantSetMousePos);
        ImGui::Text("NavActive: %d, NavVisible: %d", io.NavActive, io.NavVisible);

        // Display Keyboard/Mouse state
        if (ImGui::TreeNode("Keyboard, Mouse & Navigation State"))
        {
            if (ImGui::IsMousePosValid())
                ImGui::Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
            else
                ImGui::Text("Mouse pos: <INVALID>");
            ImGui::Text("Mouse delta: (%g, %g)", io.MouseDelta.x, io.MouseDelta.y);
            ImGui::Text("Mouse down:");     for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) if (io.MouseDownDuration[i] >= 0.0f) { ImGui::SameLine(); ImGui::Text("b%d (%.02f secs)", i, io.MouseDownDuration[i]); }
            ImGui::Text("Mouse clicked:");  for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) if (ImGui::IsMouseClicked(i)) { ImGui::SameLine(); ImGui::Text("b%d", i); }
            ImGui::Text("Mouse dbl-clicked:"); for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) if (ImGui::IsMouseDoubleClicked(i)) { ImGui::SameLine(); ImGui::Text("b%d", i); }
            ImGui::Text("Mouse released:"); for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) if (ImGui::IsMouseReleased(i)) { ImGui::SameLine(); ImGui::Text("b%d", i); }
            ImGui::Text("Mouse wheel: %.1f", io.MouseWheel);

            ImGui::Text("Keys down:");      for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++) if (io.KeysDownDuration[i] >= 0.0f) { ImGui::SameLine(); ImGui::Text("%d (0x%X) (%.02f secs)", i, i, io.KeysDownDuration[i]); }
            ImGui::Text("Keys pressed:");   for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++) if (ImGui::IsKeyPressed(i)) { ImGui::SameLine(); ImGui::Text("%d (0x%X)", i, i); }
            ImGui::Text("Keys release:");   for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++) if (ImGui::IsKeyReleased(i)) { ImGui::SameLine(); ImGui::Text("%d (0x%X)", i, i); }
            ImGui::Text("Keys mods: %s%s%s%s", io.KeyCtrl ? "CTRL " : "", io.KeyShift ? "SHIFT " : "", io.KeyAlt ? "ALT " : "", io.KeySuper ? "SUPER " : "");
            ImGui::Text("Chars queue:");    for (int i = 0; i < io.InputQueueCharacters.Size; i++) { ImWchar c = io.InputQueueCharacters[i]; ImGui::SameLine();  ImGui::Text("\'%c\' (0x%04X)", (c > ' ' && c <= 255) ? (char)c : '?', c); } // FIXME: We should convert 'c' to UTF-8 here but the functions are not public.

            ImGui::Text("NavInputs down:"); for (int i = 0; i < IM_ARRAYSIZE(io.NavInputs); i++) if (io.NavInputs[i] > 0.0f) { ImGui::SameLine(); ImGui::Text("[%d] %.2f", i, io.NavInputs[i]); }
            ImGui::Text("NavInputs pressed:"); for (int i = 0; i < IM_ARRAYSIZE(io.NavInputs); i++) if (io.NavInputsDownDuration[i] == 0.0f) { ImGui::SameLine(); ImGui::Text("[%d]", i); }
            ImGui::Text("NavInputs duration:"); for (int i = 0; i < IM_ARRAYSIZE(io.NavInputs); i++) if (io.NavInputsDownDuration[i] >= 0.0f) { ImGui::SameLine(); ImGui::Text("[%d] %.2f", i, io.NavInputsDownDuration[i]); }

            ImGui::Button("Hovering me sets the\nkeyboard capture flag");
            if (ImGui::IsItemHovered())
                ImGui::CaptureKeyboardFromApp(true);
            ImGui::SameLine();
            ImGui::Button("Holding me clears the\nthe keyboard capture flag");
            if (ImGui::IsItemActive())
                ImGui::CaptureKeyboardFromApp(false);

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Tabbing"))
        {
            ImGui::Text("Use TAB/SHIFT+TAB to cycle through keyboard editable fields.");
            static char buf[32] = "dummy";
            ImGui::InputText("1", buf, IM_ARRAYSIZE(buf));
            ImGui::InputText("2", buf, IM_ARRAYSIZE(buf));
            ImGui::InputText("3", buf, IM_ARRAYSIZE(buf));
            ImGui::PushAllowKeyboardFocus(false);
            ImGui::InputText("4 (tab skip)", buf, IM_ARRAYSIZE(buf));
            //ImGui::SameLine(); HelpMarker("Use ImGui::PushAllowKeyboardFocus(bool)\nto disable tabbing through certain widgets.");
            ImGui::PopAllowKeyboardFocus();
            ImGui::InputText("5", buf, IM_ARRAYSIZE(buf));
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Focus from code"))
        {
            bool focus_1 = ImGui::Button("Focus on 1"); ImGui::SameLine();
            bool focus_2 = ImGui::Button("Focus on 2"); ImGui::SameLine();
            bool focus_3 = ImGui::Button("Focus on 3");
            int has_focus = 0;
            static char buf[128] = "click on a button to set focus";

            if (focus_1) ImGui::SetKeyboardFocusHere();
            ImGui::InputText("1", buf, IM_ARRAYSIZE(buf));
            if (ImGui::IsItemActive()) has_focus = 1;

            if (focus_2) ImGui::SetKeyboardFocusHere();
            ImGui::InputText("2", buf, IM_ARRAYSIZE(buf));
            if (ImGui::IsItemActive()) has_focus = 2;

            ImGui::PushAllowKeyboardFocus(false);
            if (focus_3) ImGui::SetKeyboardFocusHere();
            ImGui::InputText("3 (tab skip)", buf, IM_ARRAYSIZE(buf));
            if (ImGui::IsItemActive()) has_focus = 3;
            ImGui::PopAllowKeyboardFocus();

            if (has_focus)
                ImGui::Text("Item with focus: %d", has_focus);
            else
                ImGui::Text("Item with focus: <none>");

            // Use >= 0 parameter to SetKeyboardFocusHere() to focus an upcoming item
            static float f3[3] = { 0.0f, 0.0f, 0.0f };
            int focus_ahead = -1;
            if (ImGui::Button("Focus on X")) { focus_ahead = 0; } ImGui::SameLine();
            if (ImGui::Button("Focus on Y")) { focus_ahead = 1; } ImGui::SameLine();
            if (ImGui::Button("Focus on Z")) { focus_ahead = 2; }
            if (focus_ahead != -1) ImGui::SetKeyboardFocusHere(focus_ahead);
            ImGui::SliderFloat3("Float3", &f3[0], 0.0f, 1.0f);

            ImGui::TextWrapped("NB: Cursor & selection are preserved when refocusing last used item in code.");
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Dragging"))
        {
            ImGui::TextWrapped("You can use ImGui::GetMouseDragDelta(0) to query for the dragged amount on any widget.");
            for (int button = 0; button < 3; button++)
                ImGui::Text("IsMouseDragging(%d):\n  w/ default threshold: %d,\n  w/ zero threshold: %d\n  w/ large threshold: %d",
                    button, ImGui::IsMouseDragging(button), ImGui::IsMouseDragging(button, 0.0f), ImGui::IsMouseDragging(button, 20.0f));

            ImGui::Button("Drag Me");
            if (ImGui::IsItemActive())
                ImGui::GetForegroundDrawList()->AddLine(io.MouseClickedPos[0], io.MousePos, ImGui::GetColorU32(ImGuiCol_Button), 4.0f); // Draw a line between the button and the mouse cursor

            // Drag operations gets "unlocked" when the mouse has moved past a certain threshold (the default threshold is stored in io.MouseDragThreshold)
            // You can request a lower or higher threshold using the second parameter of IsMouseDragging() and GetMouseDragDelta()
            ImVec2 value_raw = ImGui::GetMouseDragDelta(0, 0.0f);
            ImVec2 value_with_lock_threshold = ImGui::GetMouseDragDelta(0);
            ImVec2 mouse_delta = io.MouseDelta;
            ImGui::Text("GetMouseDragDelta(0):\n  w/ default threshold: (%.1f, %.1f),\n  w/ zero threshold: (%.1f, %.1f)\nMouseDelta: (%.1f, %.1f)", value_with_lock_threshold.x, value_with_lock_threshold.y, value_raw.x, value_raw.y, mouse_delta.x, mouse_delta.y);
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Mouse cursors"))
        {
            const char* mouse_cursors_names[] = { "Arrow", "TextInput", "ResizeAll", "ResizeNS", "ResizeEW", "ResizeNESW", "ResizeNWSE", "Hand", "NotAllowed" };
            IM_ASSERT(IM_ARRAYSIZE(mouse_cursors_names) == ImGuiMouseCursor_COUNT);

            ImGui::Text("Current mouse cursor = %d: %s", ImGui::GetMouseCursor(), mouse_cursors_names[ImGui::GetMouseCursor()]);
            ImGui::Text("Hover to see mouse cursors:");
            ImGui::SameLine(); HelpMarker("Your application can render a different mouse cursor based on what ImGui::GetMouseCursor() returns. If software cursor rendering (io.MouseDrawCursor) is set ImGui will draw the right cursor for you, otherwise your backend needs to handle it.");
            for (int i = 0; i < ImGuiMouseCursor_COUNT; i++)
            {
                char label[32];
                sprintf(label, "Mouse cursor %d: %s", i, mouse_cursors_names[i]);
                ImGui::Bullet(); ImGui::Selectable(label, false);
                if (ImGui::IsItemHovered() || ImGui::IsItemFocused())
                    ImGui::SetMouseCursor(i);
            }
            ImGui::TreePop();
        }
    }
}

//-----------------------------------------------------------------------------
// [SECTION] About Window / ShowAboutWindow()
// Access from Dear ImGui Demo -> Tools -> About
//-----------------------------------------------------------------------------

void ImGui::ShowAboutWindow(bool* p_open)
{
    if (!ImGui::Begin("About Dear ImGui", p_open, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::End();
        return;
    }
    ImGui::Text("Dear ImGui %s", ImGui::GetVersion());
    ImGui::Separator();
    ImGui::Text("By Omar Cornut and all Dear ImGui contributors.");
    ImGui::Text("Dear ImGui is licensed under the MIT License, see LICENSE for more information.");

    static bool show_config_info = false;
    ImGui::Checkbox("Config/Build Information", &show_config_info);
    if (show_config_info)
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();

        bool copy_to_clipboard = ImGui::Button("Copy to clipboard");
        ImGui::BeginChildFrame(ImGui::GetID("cfginfos"), ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 18), ImGuiWindowFlags_NoMove);
        if (copy_to_clipboard)
        {
            ImGui::LogToClipboard();
            ImGui::LogText("```\n"); // Back quotes will make the text appears without formatting when pasting to GitHub
        }

        ImGui::Text("Dear ImGui %s (%d)", IMGUI_VERSION, IMGUI_VERSION_NUM);
        ImGui::Separator();
        ImGui::Text("sizeof(size_t): %d, sizeof(ImDrawIdx): %d, sizeof(ImDrawVert): %d", (int)sizeof(size_t), (int)sizeof(ImDrawIdx), (int)sizeof(ImDrawVert));
        ImGui::Text("define: __cplusplus=%d", (int)__cplusplus);
#ifdef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
        ImGui::Text("define: IMGUI_DISABLE_OBSOLETE_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS
        ImGui::Text("define: IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
        ImGui::Text("define: IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_WIN32_FUNCTIONS
        ImGui::Text("define: IMGUI_DISABLE_WIN32_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS
        ImGui::Text("define: IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_DEFAULT_MATH_FUNCTIONS
        ImGui::Text("define: IMGUI_DISABLE_DEFAULT_MATH_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS
        ImGui::Text("define: IMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_FILE_FUNCTIONS
        ImGui::Text("define: IMGUI_DISABLE_FILE_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_DEFAULT_ALLOCATORS
        ImGui::Text("define: IMGUI_DISABLE_DEFAULT_ALLOCATORS");
#endif
#ifdef IMGUI_USE_BGRA_PACKED_COLOR
        ImGui::Text("define: IMGUI_USE_BGRA_PACKED_COLOR");
#endif
#ifdef _WIN32
        ImGui::Text("define: _WIN32");
#endif
#ifdef _WIN64
        ImGui::Text("define: _WIN64");
#endif
#ifdef __linux__
        ImGui::Text("define: __linux__");
#endif
#ifdef __APPLE__
        ImGui::Text("define: __APPLE__");
#endif
#ifdef _MSC_VER
        ImGui::Text("define: _MSC_VER=%d", _MSC_VER);
#endif
#ifdef __MINGW32__
        ImGui::Text("define: __MINGW32__");
#endif
#ifdef __MINGW64__
        ImGui::Text("define: __MINGW64__");
#endif
#ifdef __GNUC__
        ImGui::Text("define: __GNUC__=%d", (int)__GNUC__);
#endif
#ifdef __clang_version__
        ImGui::Text("define: __clang_version__=%s", __clang_version__);
#endif
        ImGui::Separator();
        ImGui::Text("io.BackendPlatformName: %s", io.BackendPlatformName ? io.BackendPlatformName : "NULL");
        ImGui::Text("io.BackendRendererName: %s", io.BackendRendererName ? io.BackendRendererName : "NULL");
        ImGui::Text("io.ConfigFlags: 0x%08X", io.ConfigFlags);
        if (io.ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard)        ImGui::Text(" NavEnableKeyboard");
        if (io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad)         ImGui::Text(" NavEnableGamepad");
        if (io.ConfigFlags & ImGuiConfigFlags_NavEnableSetMousePos)     ImGui::Text(" NavEnableSetMousePos");
        if (io.ConfigFlags & ImGuiConfigFlags_NavNoCaptureKeyboard)     ImGui::Text(" NavNoCaptureKeyboard");
        if (io.ConfigFlags & ImGuiConfigFlags_NoMouse)                  ImGui::Text(" NoMouse");
        if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)      ImGui::Text(" NoMouseCursorChange");
        if (io.MouseDrawCursor)                                         ImGui::Text("io.MouseDrawCursor");
        if (io.ConfigMacOSXBehaviors)                                   ImGui::Text("io.ConfigMacOSXBehaviors");
        if (io.ConfigInputTextCursorBlink)                              ImGui::Text("io.ConfigInputTextCursorBlink");
        if (io.ConfigWindowsResizeFromEdges)                            ImGui::Text("io.ConfigWindowsResizeFromEdges");
        if (io.ConfigWindowsMoveFromTitleBarOnly)                       ImGui::Text("io.ConfigWindowsMoveFromTitleBarOnly");
        if (io.ConfigWindowsMemoryCompactTimer >= 0.0f)                 ImGui::Text("io.ConfigWindowsMemoryCompactTimer = %.1ff", io.ConfigWindowsMemoryCompactTimer);
        ImGui::Text("io.BackendFlags: 0x%08X", io.BackendFlags);
        if (io.BackendFlags & ImGuiBackendFlags_HasGamepad)             ImGui::Text(" HasGamepad");
        if (io.BackendFlags & ImGuiBackendFlags_HasMouseCursors)        ImGui::Text(" HasMouseCursors");
        if (io.BackendFlags & ImGuiBackendFlags_HasSetMousePos)         ImGui::Text(" HasSetMousePos");
        if (io.BackendFlags & ImGuiBackendFlags_RendererHasVtxOffset)   ImGui::Text(" RendererHasVtxOffset");
        ImGui::Separator();
        ImGui::Text("io.Fonts: %d fonts, Flags: 0x%08X, TexSize: %d,%d", io.Fonts->Fonts.Size, io.Fonts->Flags, io.Fonts->TexWidth, io.Fonts->TexHeight);
        ImGui::Text("io.DisplaySize: %.2f,%.2f", io.DisplaySize.x, io.DisplaySize.y);
        ImGui::Text("io.DisplayFramebufferScale: %.2f,%.2f", io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        ImGui::Separator();
        ImGui::Text("style.WindowPadding: %.2f,%.2f", style.WindowPadding.x, style.WindowPadding.y);
        ImGui::Text("style.WindowBorderSize: %.2f", style.WindowBorderSize);
        ImGui::Text("style.FramePadding: %.2f,%.2f", style.FramePadding.x, style.FramePadding.y);
        ImGui::Text("style.FrameRounding: %.2f", style.FrameRounding);
        ImGui::Text("style.FrameBorderSize: %.2f", style.FrameBorderSize);
        ImGui::Text("style.ItemSpacing: %.2f,%.2f", style.ItemSpacing.x, style.ItemSpacing.y);
        ImGui::Text("style.ItemInnerSpacing: %.2f,%.2f", style.ItemInnerSpacing.x, style.ItemInnerSpacing.y);

        if (copy_to_clipboard)
        {
            ImGui::LogText("\n```\n");
            ImGui::LogFinish();
        }
        ImGui::EndChildFrame();
    }
    ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Style Editor / ShowStyleEditor()
//-----------------------------------------------------------------------------
// - ShowStyleSelector()
// - ShowFontSelector()
// - ShowStyleEditor()
//-----------------------------------------------------------------------------

// Demo helper function to select among default colors. See ShowStyleEditor() for more advanced options.
// Here we use the simplified Combo() api that packs items into a single literal string. Useful for quick combo boxes where the choices are known locally.
bool ImGui::ShowStyleSelector(const char* label)
{
    static int style_idx = -1;
    if (ImGui::Combo(label, &style_idx, "Classic\0Dark\0Light\0"))
    {
        switch (style_idx)
        {
        case 0: ImGui::StyleColorsClassic(); break;
        case 1: ImGui::StyleColorsDark(); break;
        case 2: ImGui::StyleColorsLight(); break;
        }
        return true;
    }
    return false;
}

// Demo helper function to select among loaded fonts.
// Here we use the regular BeginCombo()/EndCombo() api which is more the more flexible one.
void ImGui::ShowFontSelector(const char* label)
{
    ImGuiIO& io = ImGui::GetIO();
    ImFont* font_current = ImGui::GetFont();
    if (ImGui::BeginCombo(label, font_current->GetDebugName()))
    {
        for (int n = 0; n < io.Fonts->Fonts.Size; n++)
        {
            ImFont* font = io.Fonts->Fonts[n];
            ImGui::PushID((void*)font);
            if (ImGui::Selectable(font->GetDebugName(), font == font_current))
                io.FontDefault = font;
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    HelpMarker(
        "- Load additional fonts with io.Fonts->AddFontFromFileTTF().\n"
        "- The font atlas is built when calling io.Fonts->GetTexDataAsXXXX() or io.Fonts->Build().\n"
        "- Read FAQ and docs/FONTS.txt for more details.\n"
        "- If you need to add/remove fonts at runtime (e.g. for DPI change), do it before calling NewFrame().");
}

void ImGui::ShowStyleEditor(ImGuiStyle* ref)
{
    // You can pass in a reference ImGuiStyle structure to compare to, revert to and save to (else it compares to an internally stored reference)
    ImGuiStyle& style = ImGui::GetStyle();
    static ImGuiStyle ref_saved_style;

    // Default to using internal storage as reference
    static bool init = true;
    if (init && ref == NULL)
        ref_saved_style = style;
    init = false;
    if (ref == NULL)
        ref = &ref_saved_style;

    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.50f);

    if (ImGui::ShowStyleSelector("Colors##Selector"))
        ref_saved_style = style;
    ImGui::ShowFontSelector("Fonts##Selector");

    // Simplified Settings
    if (ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f"))
        style.GrabRounding = style.FrameRounding; // Make GrabRounding always the same value as FrameRounding
    { bool window_border = (style.WindowBorderSize > 0.0f); if (ImGui::Checkbox("WindowBorder", &window_border)) style.WindowBorderSize = window_border ? 1.0f : 0.0f; }
    ImGui::SameLine();
    { bool frame_border = (style.FrameBorderSize > 0.0f); if (ImGui::Checkbox("FrameBorder", &frame_border)) style.FrameBorderSize = frame_border ? 1.0f : 0.0f; }
    ImGui::SameLine();
    { bool popup_border = (style.PopupBorderSize > 0.0f); if (ImGui::Checkbox("PopupBorder", &popup_border)) style.PopupBorderSize = popup_border ? 1.0f : 0.0f; }

    // Save/Revert button
    if (ImGui::Button("Save Ref"))
        *ref = ref_saved_style = style;
    ImGui::SameLine();
    if (ImGui::Button("Revert Ref"))
        style = *ref;
    ImGui::SameLine();
    HelpMarker("Save/Revert in local non-persistent storage. Default Colors definition are not affected. Use \"Export\" below to save them somewhere.");

    ImGui::Separator();

    if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("Sizes"))
        {
            ImGui::Text("Main");
            ImGui::SliderFloat2("WindowPadding", (float*)&style.WindowPadding, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("FramePadding", (float*)&style.FramePadding, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("ItemSpacing", (float*)&style.ItemSpacing, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("ItemInnerSpacing", (float*)&style.ItemInnerSpacing, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("TouchExtraPadding", (float*)&style.TouchExtraPadding, 0.0f, 10.0f, "%.0f");
            ImGui::SliderFloat("IndentSpacing", &style.IndentSpacing, 0.0f, 30.0f, "%.0f");
            ImGui::SliderFloat("ScrollbarSize", &style.ScrollbarSize, 1.0f, 20.0f, "%.0f");
            ImGui::SliderFloat("GrabMinSize", &style.GrabMinSize, 1.0f, 20.0f, "%.0f");
            ImGui::Text("Borders");
            ImGui::SliderFloat("WindowBorderSize", &style.WindowBorderSize, 0.0f, 1.0f, "%.0f");
            ImGui::SliderFloat("ChildBorderSize", &style.ChildBorderSize, 0.0f, 1.0f, "%.0f");
            ImGui::SliderFloat("PopupBorderSize", &style.PopupBorderSize, 0.0f, 1.0f, "%.0f");
            ImGui::SliderFloat("FrameBorderSize", &style.FrameBorderSize, 0.0f, 1.0f, "%.0f");
            ImGui::SliderFloat("TabBorderSize", &style.TabBorderSize, 0.0f, 1.0f, "%.0f");
            ImGui::Text("Rounding");
            ImGui::SliderFloat("WindowRounding", &style.WindowRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("ChildRounding", &style.ChildRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("PopupRounding", &style.PopupRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("ScrollbarRounding", &style.ScrollbarRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("GrabRounding", &style.GrabRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("TabRounding", &style.TabRounding, 0.0f, 12.0f, "%.0f");
            ImGui::Text("Alignment");
            ImGui::SliderFloat2("WindowTitleAlign", (float*)&style.WindowTitleAlign, 0.0f, 1.0f, "%.2f");
            int window_menu_button_position = style.WindowMenuButtonPosition + 1;
            if (ImGui::Combo("WindowMenuButtonPosition", (int*)&window_menu_button_position, "None\0Left\0Right\0"))
                style.WindowMenuButtonPosition = window_menu_button_position - 1;
            ImGui::Combo("ColorButtonPosition", (int*)&style.ColorButtonPosition, "Left\0Right\0");
            ImGui::SliderFloat2("ButtonTextAlign", (float*)&style.ButtonTextAlign, 0.0f, 1.0f, "%.2f"); ImGui::SameLine(); HelpMarker("Alignment applies when a button is larger than its text content.");
            ImGui::SliderFloat2("SelectableTextAlign", (float*)&style.SelectableTextAlign, 0.0f, 1.0f, "%.2f"); ImGui::SameLine(); HelpMarker("Alignment applies when a selectable is larger than its text content.");
            ImGui::Text("Safe Area Padding"); ImGui::SameLine(); HelpMarker("Adjust if you cannot see the edges of your screen (e.g. on a TV where scaling has not been configured).");
            ImGui::SliderFloat2("DisplaySafeAreaPadding", (float*)&style.DisplaySafeAreaPadding, 0.0f, 30.0f, "%.0f");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Colors"))
        {
            static int output_dest = 0;
            static bool output_only_modified = true;
            if (ImGui::Button("Export"))
            {
                if (output_dest == 0)
                    ImGui::LogToClipboard();
                else
                    ImGui::LogToTTY();
                ImGui::LogText("ImVec4* colors = ImGui::GetStyle().Colors;" IM_NEWLINE);
                for (int i = 0; i < ImGuiCol_COUNT; i++)
                {
                    const ImVec4& col = style.Colors[i];
                    const char* name = ImGui::GetStyleColorName(i);
                    if (!output_only_modified || memcmp(&col, &ref->Colors[i], sizeof(ImVec4)) != 0)
                        ImGui::LogText("colors[ImGuiCol_%s]%*s= ImVec4(%.2ff, %.2ff, %.2ff, %.2ff);" IM_NEWLINE, name, 23 - (int)strlen(name), "", col.x, col.y, col.z, col.w);
                }
                ImGui::LogFinish();
            }
            ImGui::SameLine(); ImGui::SetNextItemWidth(120); ImGui::Combo("##output_type", &output_dest, "To Clipboard\0To TTY\0");
            ImGui::SameLine(); ImGui::Checkbox("Only Modified Colors", &output_only_modified);

            static ImGuiTextFilter filter;
            filter.Draw("Filter colors", ImGui::GetFontSize() * 16);

            static ImGuiColorEditFlags alpha_flags = 0;
            if (ImGui::RadioButton("Opaque", alpha_flags == 0)) { alpha_flags = 0; } ImGui::SameLine();
            if (ImGui::RadioButton("Alpha", alpha_flags == ImGuiColorEditFlags_AlphaPreview)) { alpha_flags = ImGuiColorEditFlags_AlphaPreview; } ImGui::SameLine();
            if (ImGui::RadioButton("Both", alpha_flags == ImGuiColorEditFlags_AlphaPreviewHalf)) { alpha_flags = ImGuiColorEditFlags_AlphaPreviewHalf; } ImGui::SameLine();
            HelpMarker("In the color list:\nLeft-click on colored square to open color picker,\nRight-click to open edit options menu.");

            ImGui::BeginChild("##colors", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NavFlattened);
            ImGui::PushItemWidth(-160);
            for (int i = 0; i < ImGuiCol_COUNT; i++)
            {
                const char* name = ImGui::GetStyleColorName(i);
                if (!filter.PassFilter(name))
                    continue;
                ImGui::PushID(i);
                ImGui::ColorEdit4("##color", (float*)&style.Colors[i], ImGuiColorEditFlags_AlphaBar | alpha_flags);
                if (memcmp(&style.Colors[i], &ref->Colors[i], sizeof(ImVec4)) != 0)
                {
                    // Tips: in a real user application, you may want to merge and use an icon font into the main font, so instead of "Save"/"Revert" you'd use icons.
                    // Read the FAQ and docs/FONTS.txt about using icon fonts. It's really easy and super convenient!
                    ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Save")) ref->Colors[i] = style.Colors[i];
                    ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Revert")) style.Colors[i] = ref->Colors[i];
                }
                ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
                ImGui::TextUnformatted(name);
                ImGui::PopID();
            }
            ImGui::PopItemWidth();
            ImGui::EndChild();

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Fonts"))
        {
            ImGuiIO& io = ImGui::GetIO();
            ImFontAtlas* atlas = io.Fonts;
            HelpMarker("Read FAQ and docs/FONTS.txt for details on font loading.");
            ImGui::PushItemWidth(120);
            for (int i = 0; i < atlas->Fonts.Size; i++)
            {
                ImFont* font = atlas->Fonts[i];
                ImGui::PushID(font);
                bool font_details_opened = ImGui::TreeNode(font, "Font %d: \"%s\"\n%.2f px, %d glyphs, %d file(s)", i, font->ConfigData ? font->ConfigData[0].Name : "", font->FontSize, font->Glyphs.Size, font->ConfigDataCount);
                ImGui::SameLine(); if (ImGui::SmallButton("Set as default")) { io.FontDefault = font; }
                if (font_details_opened)
                {
                    ImGui::PushFont(font);
                    ImGui::Text("The quick brown fox jumps over the lazy dog");
                    ImGui::PopFont();
                    ImGui::DragFloat("Font scale", &font->Scale, 0.005f, 0.3f, 2.0f, "%.1f");   // Scale only this font
                    ImGui::SameLine(); HelpMarker("Note than the default embedded font is NOT meant to be scaled.\n\nFont are currently rendered into bitmaps at a given size at the time of building the atlas. You may oversample them to get some flexibility with scaling. You can also render at multiple sizes and select which one to use at runtime.\n\n(Glimmer of hope: the atlas system should hopefully be rewritten in the future to make scaling more natural and automatic.)");
                    ImGui::InputFloat("Font offset", &font->DisplayOffset.y, 1, 1, "%.0f");
                    ImGui::Text("Ascent: %f, Descent: %f, Height: %f", font->Ascent, font->Descent, font->Ascent - font->Descent);
                    ImGui::Text("Fallback character: '%c' (U+%04X)", font->FallbackChar, font->FallbackChar);
                    ImGui::Text("Ellipsis character: '%c' (U+%04X)", font->EllipsisChar, font->EllipsisChar);
                    const float surface_sqrt = sqrtf((float)font->MetricsTotalSurface);
                    ImGui::Text("Texture Area: about %d px ~%dx%d px", font->MetricsTotalSurface, (int)surface_sqrt, (int)surface_sqrt);
                    for (int config_i = 0; config_i < font->ConfigDataCount; config_i++)
                        if (font->ConfigData)
                            if (const ImFontConfig* cfg = &font->ConfigData[config_i])
                                ImGui::BulletText("Input %d: \'%s\', Oversample: (%d,%d), PixelSnapH: %d", config_i, cfg->Name, cfg->OversampleH, cfg->OversampleV, cfg->PixelSnapH);
                    if (ImGui::TreeNode("Glyphs", "Glyphs (%d)", font->Glyphs.Size))
                    {
                        // Display all glyphs of the fonts in separate pages of 256 characters
                        for (unsigned int base = 0; base <= IM_UNICODE_CODEPOINT_MAX; base += 256)
                        {
                            // Skip ahead if a large bunch of glyphs are not present in the font (test in chunks of 4k)
                            // This is only a small optimization to reduce the number of iterations when IM_UNICODE_MAX_CODEPOINT is large.
                            // (if ImWchar==ImWchar32 we will do at least about 272 queries here)
                            if (!(base & 4095) && font->IsGlyphRangeUnused(base, base + 4095))
                            {
                                base += 4096 - 256;
                                continue;
                            }

                            int count = 0;
                            for (unsigned int n = 0; n < 256; n++)
                                count += font->FindGlyphNoFallback((ImWchar)(base + n)) ? 1 : 0;
                            if (count > 0 && ImGui::TreeNode((void*)(intptr_t)base, "U+%04X..U+%04X (%d %s)", base, base + 255, count, count > 1 ? "glyphs" : "glyph"))
                            {
                                float cell_size = font->FontSize * 1;
                                float cell_spacing = style.ItemSpacing.y;
                                ImVec2 base_pos = ImGui::GetCursorScreenPos();
                                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                                for (unsigned int n = 0; n < 256; n++)
                                {
                                    ImVec2 cell_p1(base_pos.x + (n % 16) * (cell_size + cell_spacing), base_pos.y + (n / 16) * (cell_size + cell_spacing));
                                    ImVec2 cell_p2(cell_p1.x + cell_size, cell_p1.y + cell_size);
                                    const ImFontGlyph* glyph = font->FindGlyphNoFallback((ImWchar)(base + n));
                                    draw_list->AddRect(cell_p1, cell_p2, glyph ? IM_COL32(255, 255, 255, 100) : IM_COL32(255, 255, 255, 50));
                                    if (glyph)
                                        font->RenderChar(draw_list, cell_size, cell_p1, ImGui::GetColorU32(ImGuiCol_Text), (ImWchar)(base + n)); // We use ImFont::RenderChar as a shortcut because we don't have UTF-8 conversion functions available to generate a string.
                                    if (glyph && ImGui::IsMouseHoveringRect(cell_p1, cell_p2))
                                    {
                                        ImGui::BeginTooltip();
                                        ImGui::Text("Codepoint: U+%04X", base + n);
                                        ImGui::Separator();
                                        ImGui::Text("Visible: %d", glyph->Visible);
                                        ImGui::Text("AdvanceX: %.1f", glyph->AdvanceX);
                                        ImGui::Text("Pos: (%.2f,%.2f)->(%.2f,%.2f)", glyph->X0, glyph->Y0, glyph->X1, glyph->Y1);
                                        ImGui::Text("UV: (%.3f,%.3f)->(%.3f,%.3f)", glyph->U0, glyph->V0, glyph->U1, glyph->V1);
                                        ImGui::EndTooltip();
                                    }
                                }
                                ImGui::Dummy(ImVec2((cell_size + cell_spacing) * 16, (cell_size + cell_spacing) * 16));
                                ImGui::TreePop();
                            }
                        }
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
            if (ImGui::TreeNode("Atlas texture", "Atlas texture (%dx%d pixels)", atlas->TexWidth, atlas->TexHeight))
            {
                ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);
                ImGui::Image(atlas->TexID, ImVec2((float)atlas->TexWidth, (float)atlas->TexHeight), ImVec2(0, 0), ImVec2(1, 1), tint_col, border_col);
                ImGui::TreePop();
            }

            HelpMarker("Those are old settings provided for convenience.\nHowever, the _correct_ way of scaling your UI is currently to reload your font at the designed size, rebuild the font atlas, and call style.ScaleAllSizes() on a reference ImGuiStyle structure.");
            static float window_scale = 1.0f;
            if (ImGui::DragFloat("window scale", &window_scale, 0.005f, 0.3f, 2.0f, "%.2f"))   // scale only this window
                ImGui::SetWindowFontScale(window_scale);
            ImGui::DragFloat("global scale", &io.FontGlobalScale, 0.005f, 0.3f, 2.0f, "%.2f");      // scale everything
            ImGui::PopItemWidth();

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Rendering"))
        {
            ImGui::Checkbox("Anti-aliased lines", &style.AntiAliasedLines); ImGui::SameLine(); HelpMarker("When disabling anti-aliasing lines, you'll probably want to disable borders in your style as well.");
            ImGui::Checkbox("Anti-aliased fill", &style.AntiAliasedFill);
            ImGui::PushItemWidth(100);
            ImGui::DragFloat("Curve Tessellation Tolerance", &style.CurveTessellationTol, 0.02f, 0.10f, 10.0f, "%.2f");
            if (style.CurveTessellationTol < 0.10f) style.CurveTessellationTol = 0.10f;
            ImGui::DragFloat("Circle segment Max Error", &style.CircleSegmentMaxError, 0.01f, 0.10f, 10.0f, "%.2f");
            ImGui::DragFloat("Global Alpha", &style.Alpha, 0.005f, 0.20f, 1.0f, "%.2f"); // Not exposing zero here so user doesn't "lose" the UI (zero alpha clips all widgets). But application code could have a toggle to switch between zero and non-zero.
            ImGui::PopItemWidth();

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::PopItemWidth();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Main Menu Bar / ShowExampleAppMainMenuBar()
//-----------------------------------------------------------------------------
// - ShowExampleAppMainMenuBar()
// - ShowExampleMenuFile()
//-----------------------------------------------------------------------------

// Demonstrate creating a "main" fullscreen menu bar and populating it.
// Note the difference between BeginMainMenuBar() and BeginMenuBar():
// - BeginMenuBar() = menu-bar inside current window we Begin()-ed into (the window needs the ImGuiWindowFlags_MenuBar flag)
// - BeginMainMenuBar() = helper to create menu-bar-sized window at the top of the main viewport + call BeginMenuBar() into it.
static void ShowExampleAppMainMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            ShowExampleMenuFile();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) {}
            if (ImGui::MenuItem("Copy", "CTRL+C")) {}
            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

// Note that shortcuts are currently provided for display only (future version will add flags to BeginMenu to process shortcuts)
static void ShowExampleMenuFile()
{
    ImGui::MenuItem("(dummy menu)", NULL, false, false);
    if (ImGui::MenuItem("New")) {}
    if (ImGui::MenuItem("Open", "Ctrl+O")) {}
    if (ImGui::BeginMenu("Open Recent"))
    {
        ImGui::MenuItem("fish_hat.c");
        ImGui::MenuItem("fish_hat.inl");
        ImGui::MenuItem("fish_hat.h");
        if (ImGui::BeginMenu("More.."))
        {
            ImGui::MenuItem("Hello");
            ImGui::MenuItem("Sailor");
            if (ImGui::BeginMenu("Recurse.."))
            {
                ShowExampleMenuFile();
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Save", "Ctrl+S")) {}
    if (ImGui::MenuItem("Save As..")) {}

    ImGui::Separator();
    if (ImGui::BeginMenu("Options"))
    {
        static bool enabled = true;
        ImGui::MenuItem("Enabled", "", &enabled);
        ImGui::BeginChild("child", ImVec2(0, 60), true);
        for (int i = 0; i < 10; i++)
            ImGui::Text("Scrolling Text %d", i);
        ImGui::EndChild();
        static float f = 0.5f;
        static int n = 0;
        ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
        ImGui::InputFloat("Input", &f, 0.1f);
        ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Colors"))
    {
        float sz = ImGui::GetTextLineHeight();
        for (int i = 0; i < ImGuiCol_COUNT; i++)
        {
            const char* name = ImGui::GetStyleColorName((ImGuiCol)i);
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + sz, p.y + sz), ImGui::GetColorU32((ImGuiCol)i));
            ImGui::Dummy(ImVec2(sz, sz));
            ImGui::SameLine();
            ImGui::MenuItem(name);
        }
        ImGui::EndMenu();
    }

    // Here we demonstrate appending again to the "Options" menu (which we already created above)
    // Of course in this demo it is a little bit silly that this function calls BeginMenu("Options") twice.
    // In a real code-base using it would make senses to use this feature from very different code locations.
    if (ImGui::BeginMenu("Options")) // <-- Append!
    {
        static bool b = true;
        ImGui::Checkbox("SomeOption", &b);
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Disabled", false)) // Disabled
    {
        IM_ASSERT(0);
    }
    if (ImGui::MenuItem("Checked", NULL, true)) {}
    if (ImGui::MenuItem("Quit", "Alt+F4")) {}
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Debug Console / ShowExampleAppConsole()
//-----------------------------------------------------------------------------

// Demonstrate creating a simple console window, with scrolling, filtering, completion and history.
// For the console example, here we are using a more C++ like approach of declaring a class to hold the data and the functions.
struct ExampleAppConsole
{
    char                  InputBuf[256];
    ImVector<char*>       Items;
    ImVector<const char*> Commands;
    ImVector<char*>       History;
    int                   HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
    ImGuiTextFilter       Filter;
    bool                  AutoScroll;
    bool                  ScrollToBottom;

    ExampleAppConsole()
    {
        ClearLog();
        memset(InputBuf, 0, sizeof(InputBuf));
        HistoryPos = -1;
        Commands.push_back("HELP");
        Commands.push_back("HISTORY");
        Commands.push_back("CLEAR");
        Commands.push_back("CLASSIFY");  // "classify" is only here to provide an example of "C"+[tab] completing to "CL" and displaying matches.
        AutoScroll = true;
        ScrollToBottom = false;
        AddLog("Welcome to Dear ImGui!");
    }
    ~ExampleAppConsole()
    {
        ClearLog();
        for (int i = 0; i < History.Size; i++)
            free(History[i]);
    }

    // Portable helpers
    static int   Stricmp(const char* str1, const char* str2) { int d; while ((d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; } return d; }
    static int   Strnicmp(const char* str1, const char* str2, int n) { int d = 0; while (n > 0 && (d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; n--; } return d; }
    static char* Strdup(const char* str) { size_t len = strlen(str) + 1; void* buf = malloc(len); IM_ASSERT(buf); return (char*)memcpy(buf, (const void*)str, len); }
    static void  Strtrim(char* str) { char* str_end = str + strlen(str); while (str_end > str && str_end[-1] == ' ') str_end--; *str_end = 0; }

    void    ClearLog()
    {
        for (int i = 0; i < Items.Size; i++)
            free(Items[i]);
        Items.clear();
    }

    void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
    {
        // FIXME-OPT
        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
        buf[IM_ARRAYSIZE(buf) - 1] = 0;
        va_end(args);
        Items.push_back(Strdup(buf));
    }

    void    Draw(const char* title, bool* p_open)
    {
        ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(title, p_open))
        {
            ImGui::End();
            return;
        }

        // As a specific feature guaranteed by the library, after calling Begin() the last Item represent the title bar. So e.g. IsItemHovered() will return true when hovering the title bar.
        // Here we create a context menu only available from the title bar.
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Close Console"))
                *p_open = false;
            ImGui::EndPopup();
        }

        ImGui::TextWrapped("This example implements a console with basic coloring, completion and history. A more elaborate implementation may want to store entries along with extra data such as timestamp, emitter, etc.");
        ImGui::TextWrapped("Enter 'HELP' for help, press TAB to use text completion.");

        // TODO: display items starting from the bottom

        if (ImGui::SmallButton("Add Dummy Text")) { AddLog("%d some text", Items.Size); AddLog("some more text"); AddLog("display very important message here!"); } ImGui::SameLine();
        if (ImGui::SmallButton("Add Dummy Error")) { AddLog("[error] something went wrong"); } ImGui::SameLine();
        if (ImGui::SmallButton("Clear")) { ClearLog(); } ImGui::SameLine();
        bool copy_to_clipboard = ImGui::SmallButton("Copy");
        //static float t = 0.0f; if (ImGui::GetTime() - t > 0.02f) { t = ImGui::GetTime(); AddLog("Spam %f", t); }

        ImGui::Separator();

        // Options menu
        if (ImGui::BeginPopup("Options"))
        {
            ImGui::Checkbox("Auto-scroll", &AutoScroll);
            ImGui::EndPopup();
        }

        // Options, Filter
        if (ImGui::Button("Options"))
            ImGui::OpenPopup("Options");
        ImGui::SameLine();
        Filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
        ImGui::Separator();

        const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing(); // 1 separator, 1 input text
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar); // Leave room for 1 separator + 1 InputText
        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::Selectable("Clear")) ClearLog();
            ImGui::EndPopup();
        }

        // Display every line as a separate entry so we can change their color or add custom widgets. If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
        // NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping to only process visible items.
        // You can seek and display only the lines that are visible using the ImGuiListClipper helper, if your elements are evenly spaced and you have cheap random access to the elements.
        // To use the clipper we could replace the 'for (int i = 0; i < Items.Size; i++)' loop with:
        //     ImGuiListClipper clipper(Items.Size);
        //     while (clipper.Step())
        //         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
        // However, note that you can not use this code as is if a filter is active because it breaks the 'cheap random-access' property. We would need random-access on the post-filtered list.
        // A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices that passed the filtering test, recomputing this array when user changes the filter,
        // and appending newly elements as they are inserted. This is left as a task to the user until we can manage to improve this example code!
        // If your items are of variable size you may want to implement code similar to what ImGuiListClipper does. Or split your data into fixed height items to allow random-seeking into your list.
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
        if (copy_to_clipboard)
            ImGui::LogToClipboard();
        for (int i = 0; i < Items.Size; i++)
        {
            const char* item = Items[i];
            if (!Filter.PassFilter(item))
                continue;

            // Normally you would store more information in your item (e.g. make Items[] an array of structure, store color/type etc.)
            bool pop_color = false;
            if (strstr(item, "[error]")) { ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f)); pop_color = true; } else if (strncmp(item, "# ", 2) == 0) { ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.6f, 1.0f)); pop_color = true; }
            ImGui::TextUnformatted(item);
            if (pop_color)
                ImGui::PopStyleColor();
        }
        if (copy_to_clipboard)
            ImGui::LogFinish();

        if (ScrollToBottom || (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
            ImGui::SetScrollHereY(1.0f);
        ScrollToBottom = false;

        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::Separator();

        // Command-line
        bool reclaim_focus = false;
        if (ImGui::InputText("Input", InputBuf, IM_ARRAYSIZE(InputBuf), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory, &TextEditCallbackStub, (void*)this))
        {
            char* s = InputBuf;
            Strtrim(s);
            if (s[0])
                ExecCommand(s);
            strcpy(s, "");
            reclaim_focus = true;
        }

        // Auto-focus on window apparition
        ImGui::SetItemDefaultFocus();
        if (reclaim_focus)
            ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

        ImGui::End();
    }

    void    ExecCommand(const char* command_line)
    {
        AddLog("# %s\n", command_line);

        // Insert into history. First find match and delete it so it can be pushed to the back. This isn't trying to be smart or optimal.
        HistoryPos = -1;
        for (int i = History.Size - 1; i >= 0; i--)
            if (Stricmp(History[i], command_line) == 0)
            {
                free(History[i]);
                History.erase(History.begin() + i);
                break;
            }
        History.push_back(Strdup(command_line));

        // Process command
        if (Stricmp(command_line, "CLEAR") == 0)
        {
            ClearLog();
        } else if (Stricmp(command_line, "HELP") == 0)
        {
            AddLog("Commands:");
            for (int i = 0; i < Commands.Size; i++)
                AddLog("- %s", Commands[i]);
        } else if (Stricmp(command_line, "HISTORY") == 0)
        {
            int first = History.Size - 10;
            for (int i = first > 0 ? first : 0; i < History.Size; i++)
                AddLog("%3d: %s\n", i, History[i]);
        } else
        {
            AddLog("Unknown command: '%s'\n", command_line);
        }

        // On commad input, we scroll to bottom even if AutoScroll==false
        ScrollToBottom = true;
    }

    static int TextEditCallbackStub(ImGuiInputTextCallbackData* data) // In C++11 you are better off using lambdas for this sort of forwarding callbacks
    {
        ExampleAppConsole* console = (ExampleAppConsole*)data->UserData;
        return console->TextEditCallback(data);
    }

    int     TextEditCallback(ImGuiInputTextCallbackData* data)
    {
        //AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
        switch (data->EventFlag)
        {
        case ImGuiInputTextFlags_CallbackCompletion:
        {
            // Example of TEXT COMPLETION

            // Locate beginning of current word
            const char* word_end = data->Buf + data->CursorPos;
            const char* word_start = word_end;
            while (word_start > data->Buf)
            {
                const char c = word_start[-1];
                if (c == ' ' || c == '\t' || c == ',' || c == ';')
                    break;
                word_start--;
            }

            // Build a list of candidates
            ImVector<const char*> candidates;
            for (int i = 0; i < Commands.Size; i++)
                if (Strnicmp(Commands[i], word_start, (int)(word_end - word_start)) == 0)
                    candidates.push_back(Commands[i]);

            if (candidates.Size == 0)
            {
                // No match
                AddLog("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
            } else if (candidates.Size == 1)
            {
                // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing
                data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                data->InsertChars(data->CursorPos, candidates[0]);
                data->InsertChars(data->CursorPos, " ");
            } else
            {
                // Multiple matches. Complete as much as we can, so inputing "C" will complete to "CL" and display "CLEAR" and "CLASSIFY"
                int match_len = (int)(word_end - word_start);
                for (;;)
                {
                    int c = 0;
                    bool all_candidates_matches = true;
                    for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
                        if (i == 0)
                            c = toupper(candidates[i][match_len]);
                        else if (c == 0 || c != toupper(candidates[i][match_len]))
                            all_candidates_matches = false;
                    if (!all_candidates_matches)
                        break;
                    match_len++;
                }

                if (match_len > 0)
                {
                    data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                    data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
                }

                // List matches
                AddLog("Possible matches:\n");
                for (int i = 0; i < candidates.Size; i++)
                    AddLog("- %s\n", candidates[i]);
            }

            break;
        }
        case ImGuiInputTextFlags_CallbackHistory:
        {
            // Example of HISTORY
            const int prev_history_pos = HistoryPos;
            if (data->EventKey == ImGuiKey_UpArrow)
            {
                if (HistoryPos == -1)
                    HistoryPos = History.Size - 1;
                else if (HistoryPos > 0)
                    HistoryPos--;
            } else if (data->EventKey == ImGuiKey_DownArrow)
            {
                if (HistoryPos != -1)
                    if (++HistoryPos >= History.Size)
                        HistoryPos = -1;
            }

            // A better implementation would preserve the data on the current input line along with cursor position.
            if (prev_history_pos != HistoryPos)
            {
                const char* history_str = (HistoryPos >= 0) ? History[HistoryPos] : "";
                data->DeleteChars(0, data->BufTextLen);
                data->InsertChars(0, history_str);
            }
        }
        }
        return 0;
    }
};

static void ShowExampleAppConsole(bool* p_open)
{
    static ExampleAppConsole console;
    console.Draw("Example: Console", p_open);
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Debug Log / ShowExampleAppLog()
//-----------------------------------------------------------------------------

// Usage:
//  static ExampleAppLog my_log;
//  my_log.AddLog("Hello %d world\n", 123);
//  my_log.Draw("title");
struct ExampleAppLog
{
    ImGuiTextBuffer     Buf;
    ImGuiTextFilter     Filter;
    ImVector<int>       LineOffsets;        // Index to lines offset. We maintain this with AddLog() calls, allowing us to have a random access on lines
    bool                AutoScroll;     // Keep scrolling if already at the bottom

    ExampleAppLog()
    {
        AutoScroll = true;
        Clear();
    }

    void    Clear()
    {
        Buf.clear();
        LineOffsets.clear();
        LineOffsets.push_back(0);
    }

    void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
    {
        int old_size = Buf.size();
        va_list args;
        va_start(args, fmt);
        Buf.appendfv(fmt, args);
        va_end(args);
        for (int new_size = Buf.size(); old_size < new_size; old_size++)
            if (Buf[old_size] == '\n')
                LineOffsets.push_back(old_size + 1);
    }

    void    Draw(const char* title, bool* p_open = NULL)
    {
        if (!ImGui::Begin(title, p_open))
        {
            ImGui::End();
            return;
        }

        // Options menu
        if (ImGui::BeginPopup("Options"))
        {
            ImGui::Checkbox("Auto-scroll", &AutoScroll);
            ImGui::EndPopup();
        }

        // Main window
        if (ImGui::Button("Options"))
            ImGui::OpenPopup("Options");
        ImGui::SameLine();
        bool clear = ImGui::Button("Clear");
        ImGui::SameLine();
        bool copy = ImGui::Button("Copy");
        ImGui::SameLine();
        Filter.Draw("Filter", -100.0f);

        ImGui::Separator();
        ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        if (clear)
            Clear();
        if (copy)
            ImGui::LogToClipboard();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        const char* buf = Buf.begin();
        const char* buf_end = Buf.end();
        if (Filter.IsActive())
        {
            // In this example we don't use the clipper when Filter is enabled.
            // This is because we don't have a random access on the result on our filter.
            // A real application processing logs with ten of thousands of entries may want to store the result of search/filter.
            // especially if the filtering function is not trivial (e.g. reg-exp).
            for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
            {
                const char* line_start = buf + LineOffsets[line_no];
                const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                if (Filter.PassFilter(line_start, line_end))
                    ImGui::TextUnformatted(line_start, line_end);
            }
        } else
        {
            // The simplest and easy way to display the entire buffer:
            //   ImGui::TextUnformatted(buf_begin, buf_end);
            // And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward to skip non-visible lines.
            // Here we instead demonstrate using the clipper to only process lines that are within the visible area.
            // If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them on your side is recommended.
            // Using ImGuiListClipper requires A) random access into your data, and B) items all being the  same height,
            // both of which we can handle since we an array pointing to the beginning of each line of text.
            // When using the filter (in the block of code above) we don't have random access into the data to display anymore, which is why we don't use the clipper.
            // Storing or skimming through the search result would make it possible (and would be recommended if you want to search through tens of thousands of entries)
            ImGuiListClipper clipper;
            clipper.Begin(LineOffsets.Size);
            while (clipper.Step())
            {
                for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
                {
                    const char* line_start = buf + LineOffsets[line_no];
                    const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                    ImGui::TextUnformatted(line_start, line_end);
                }
            }
            clipper.End();
        }
        ImGui::PopStyleVar();

        if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);

        ImGui::EndChild();
        ImGui::End();
    }
};

// Demonstrate creating a simple log window with basic filtering.
static void ShowExampleAppLog(bool* p_open)
{
    static ExampleAppLog log;

    // For the demo: add a debug button _BEFORE_ the normal log window contents
    // We take advantage of a rarely used feature: multiple calls to Begin()/End() are appending to the _same_ window.
    // Most of the contents of the window will be added by the log.Draw() call.
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Example: Log", p_open);
    if (ImGui::SmallButton("[Debug] Add 5 entries"))
    {
        static int counter = 0;
        for (int n = 0; n < 5; n++)
        {
            const char* categories[3] = { "info", "warn", "error" };
            const char* words[] = { "Bumfuzzled", "Cattywampus", "Snickersnee", "Abibliophobia", "Absquatulate", "Nincompoop", "Pauciloquent" };
            log.AddLog("[%05d] [%s] Hello, current time is %.1f, here's a word: '%s'\n",
                ImGui::GetFrameCount(), categories[counter % IM_ARRAYSIZE(categories)], ImGui::GetTime(), words[counter % IM_ARRAYSIZE(words)]);
            counter++;
        }
    }
    ImGui::End();

    // Actually call in the regular Log helper (which will Begin() into the same window as we just did)
    log.Draw("Example: Log", p_open);
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Simple Layout / ShowExampleAppLayout()
//-----------------------------------------------------------------------------

// Demonstrate create a window with multiple child windows.
static void ShowExampleAppLayout(bool* p_open)
{
    ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Example: Simple layout", p_open, ImGuiWindowFlags_MenuBar))
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Close")) *p_open = false;
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        // left
        static int selected = 0;
        ImGui::BeginChild("left pane", ImVec2(150, 0), true);
        for (int i = 0; i < 100; i++)
        {
            char label[128];
            sprintf(label, "MyObject %d", i);
            if (ImGui::Selectable(label, selected == i))
                selected = i;
        }
        ImGui::EndChild();
        ImGui::SameLine();

        // right
        ImGui::BeginGroup();
        ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
        ImGui::Text("MyObject: %d", selected);
        ImGui::Separator();
        if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem("Description"))
            {
                ImGui::TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. ");
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Details"))
            {
                ImGui::Text("ID: 0123456789");
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::EndChild();
        if (ImGui::Button("Revert")) {}
        ImGui::SameLine();
        if (ImGui::Button("Save")) {}
        ImGui::EndGroup();
    }
    ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Property Editor / ShowExampleAppPropertyEditor()
//-----------------------------------------------------------------------------

// Demonstrate create a simple property editor.
static void ShowExampleAppPropertyEditor(bool* p_open)
{
    ImGui::SetNextWindowSize(ImVec2(430, 450), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Example: Property editor", p_open))
    {
        ImGui::End();
        return;
    }

    HelpMarker("This example shows how you may implement a property editor using two columns.\nAll objects/fields data are dummies here.\nRemember that in many simple cases, you can use ImGui::SameLine(xxx) to position\nyour cursor horizontally instead of using the Columns() API.");

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
    ImGui::Columns(2);
    ImGui::Separator();

    struct funcs
    {
        static void ShowDummyObject(const char* prefix, int uid)
        {
            ImGui::PushID(uid);                      // Use object uid as identifier. Most commonly you could also use the object pointer as a base ID.
            ImGui::AlignTextToFramePadding();  // Text and Tree nodes are less high than regular widgets, here we add vertical spacing to make the tree lines equal high.
            bool node_open = ImGui::TreeNode("Object", "%s_%u", prefix, uid);
            ImGui::NextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("my sailor is rich");
            ImGui::NextColumn();
            if (node_open)
            {
                static float dummy_members[8] = { 0.0f,0.0f,1.0f,3.1416f,100.0f,999.0f };
                for (int i = 0; i < 8; i++)
                {
                    ImGui::PushID(i); // Use field index as identifier.
                    if (i < 2)
                    {
                        ShowDummyObject("Child", 424242);
                    } else
                    {
                        // Here we use a TreeNode to highlight on hover (we could use e.g. Selectable as well)
                        ImGui::AlignTextToFramePadding();
                        ImGui::TreeNodeEx("Field", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet, "Field_%d", i);
                        ImGui::NextColumn();
                        ImGui::SetNextItemWidth(-1);
                        if (i >= 5)
                            ImGui::InputFloat("##value", &dummy_members[i], 1.0f);
                        else
                            ImGui::DragFloat("##value", &dummy_members[i], 0.01f);
                        ImGui::NextColumn();
                    }
                    ImGui::PopID();
                }
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
    };

    // Iterate dummy objects with dummy members (all the same data)
    for (int obj_i = 0; obj_i < 3; obj_i++)
        funcs::ShowDummyObject("Object", obj_i);

    ImGui::Columns(1);
    ImGui::Separator();
    ImGui::PopStyleVar();
    ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Long Text / ShowExampleAppLongText()
//-----------------------------------------------------------------------------

// Demonstrate/test rendering huge amount of text, and the incidence of clipping.
static void ShowExampleAppLongText(bool* p_open)
{
    ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Example: Long text display", p_open))
    {
        ImGui::End();
        return;
    }

    static int test_type = 0;
    static ImGuiTextBuffer log;
    static int lines = 0;
    ImGui::Text("Printing unusually long amount of text.");
    ImGui::Combo("Test type", &test_type, "Single call to TextUnformatted()\0Multiple calls to Text(), clipped\0Multiple calls to Text(), not clipped (slow)\0");
    ImGui::Text("Buffer contents: %d lines, %d bytes", lines, log.size());
    if (ImGui::Button("Clear")) { log.clear(); lines = 0; }
    ImGui::SameLine();
    if (ImGui::Button("Add 1000 lines"))
    {
        for (int i = 0; i < 1000; i++)
            log.appendf("%i The quick brown fox jumps over the lazy dog\n", lines + i);
        lines += 1000;
    }
    ImGui::BeginChild("Log");
    switch (test_type)
    {
    case 0:
        // Single call to TextUnformatted() with a big buffer
        ImGui::TextUnformatted(log.begin(), log.end());
        break;
    case 1:
    {
        // Multiple calls to Text(), manually coarsely clipped - demonstrate how to use the ImGuiListClipper helper.
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        ImGuiListClipper clipper(lines);
        while (clipper.Step())
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
                ImGui::Text("%i The quick brown fox jumps over the lazy dog", i);
        ImGui::PopStyleVar();
        break;
    }
    case 2:
        // Multiple calls to Text(), not clipped (slow)
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        for (int i = 0; i < lines; i++)
            ImGui::Text("%i The quick brown fox jumps over the lazy dog", i);
        ImGui::PopStyleVar();
        break;
    }
    ImGui::EndChild();
    ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Auto Resize / ShowExampleAppAutoResize()
//-----------------------------------------------------------------------------

// Demonstrate creating a window which gets auto-resized according to its content.
static void ShowExampleAppAutoResize(bool* p_open)
{
    if (!ImGui::Begin("Example: Auto-resizing window", p_open, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::End();
        return;
    }

    static int lines = 10;
    ImGui::Text("Window will resize every-frame to the size of its content.\nNote that you probably don't want to query the window size to\noutput your content because that would create a feedback loop.");
    ImGui::SliderInt("Number of lines", &lines, 1, 20);
    for (int i = 0; i < lines; i++)
        ImGui::Text("%*sThis is line %d", i * 4, "", i); // Pad with space to extend size horizontally
    ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Constrained Resize / ShowExampleAppConstrainedResize()
//-----------------------------------------------------------------------------

// Demonstrate creating a window with custom resize constraints.
static void ShowExampleAppConstrainedResize(bool* p_open)
{
    struct CustomConstraints // Helper functions to demonstrate programmatic constraints
    {
        static void Square(ImGuiSizeCallbackData* data) { data->DesiredSize.x = data->DesiredSize.y = (data->DesiredSize.x > data->DesiredSize.y ? data->DesiredSize.x : data->DesiredSize.y); }
        static void Step(ImGuiSizeCallbackData* data) { float step = (float)(int)(intptr_t)data->UserData; data->DesiredSize = ImVec2((int)(data->DesiredSize.x / step + 0.5f) * step, (int)(data->DesiredSize.y / step + 0.5f) * step); }
    };

    static bool auto_resize = false;
    static int type = 0;
    static int display_lines = 10;
    if (type == 0) ImGui::SetNextWindowSizeConstraints(ImVec2(-1, 0), ImVec2(-1, FLT_MAX));      // Vertical only
    if (type == 1) ImGui::SetNextWindowSizeConstraints(ImVec2(0, -1), ImVec2(FLT_MAX, -1));      // Horizontal only
    if (type == 2) ImGui::SetNextWindowSizeConstraints(ImVec2(100, 100), ImVec2(FLT_MAX, FLT_MAX)); // Width > 100, Height > 100
    if (type == 3) ImGui::SetNextWindowSizeConstraints(ImVec2(400, -1), ImVec2(500, -1));          // Width 400-500
    if (type == 4) ImGui::SetNextWindowSizeConstraints(ImVec2(-1, 400), ImVec2(-1, 500));          // Height 400-500
    if (type == 5) ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, FLT_MAX), CustomConstraints::Square);                     // Always Square
    if (type == 6) ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, FLT_MAX), CustomConstraints::Step, (void*)(intptr_t)100); // Fixed Step

    ImGuiWindowFlags flags = auto_resize ? ImGuiWindowFlags_AlwaysAutoResize : 0;
    if (ImGui::Begin("Example: Constrained Resize", p_open, flags))
    {
        const char* desc[] =
        {
            "Resize vertical only",
            "Resize horizontal only",
            "Width > 100, Height > 100",
            "Width 400-500",
            "Height 400-500",
            "Custom: Always Square",
            "Custom: Fixed Steps (100)",
        };
        if (ImGui::Button("200x200")) { ImGui::SetWindowSize(ImVec2(200, 200)); } ImGui::SameLine();
        if (ImGui::Button("500x500")) { ImGui::SetWindowSize(ImVec2(500, 500)); } ImGui::SameLine();
        if (ImGui::Button("800x200")) { ImGui::SetWindowSize(ImVec2(800, 200)); }
        ImGui::SetNextItemWidth(200);
        ImGui::Combo("Constraint", &type, desc, IM_ARRAYSIZE(desc));
        ImGui::SetNextItemWidth(200);
        ImGui::DragInt("Lines", &display_lines, 0.2f, 1, 100);
        ImGui::Checkbox("Auto-resize", &auto_resize);
        for (int i = 0; i < display_lines; i++)
            ImGui::Text("%*sHello, sailor! Making this line long enough for the example.", i * 4, "");
    }
    ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Simple Overlay / ShowExampleAppSimpleOverlay()
//-----------------------------------------------------------------------------

// Demonstrate creating a simple static window with no decoration + a context-menu to choose which corner of the screen to use.
static void ShowExampleAppSimpleOverlay(bool* p_open)
{
    const float DISTANCE = 10.0f;
    static int corner = 0;
    ImGuiIO& io = ImGui::GetIO();
    if (corner != -1)
    {
        ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE);
        ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    }
    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
    if (ImGui::Begin("Example: Simple overlay", p_open, (corner != -1 ? ImGuiWindowFlags_NoMove : 0) | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
    {
        ImGui::Text("Simple overlay\n" "in the corner of the screen.\n" "(right-click to change position)");
        ImGui::Separator();
        if (ImGui::IsMousePosValid())
            ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
        else
            ImGui::Text("Mouse Position: <invalid>");
        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::MenuItem("Custom", NULL, corner == -1)) corner = -1;
            if (ImGui::MenuItem("Top-left", NULL, corner == 0)) corner = 0;
            if (ImGui::MenuItem("Top-right", NULL, corner == 1)) corner = 1;
            if (ImGui::MenuItem("Bottom-left", NULL, corner == 2)) corner = 2;
            if (ImGui::MenuItem("Bottom-right", NULL, corner == 3)) corner = 3;
            if (p_open && ImGui::MenuItem("Close")) *p_open = false;
            ImGui::EndPopup();
        }
    }
    ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Manipulating Window Titles / ShowExampleAppWindowTitles()
//-----------------------------------------------------------------------------

// Demonstrate using "##" and "###" in identifiers to manipulate ID generation.
// This apply to all regular items as well. Read FAQ section "How can I have multiple widgets with the same label? Can I have widget without a label? (Yes). A primer on the purpose of labels/IDs." for details.
static void ShowExampleAppWindowTitles(bool*)
{
    // By default, Windows are uniquely identified by their title.
    // You can use the "##" and "###" markers to manipulate the display/ID.

    // Using "##" to display same title but have unique identifier.
    ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
    ImGui::Begin("Same title as another window##1");
    ImGui::Text("This is window 1.\nMy title is the same as window 2, but my identifier is unique.");
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(100, 200), ImGuiCond_FirstUseEver);
    ImGui::Begin("Same title as another window##2");
    ImGui::Text("This is window 2.\nMy title is the same as window 1, but my identifier is unique.");
    ImGui::End();

    // Using "###" to display a changing title but keep a static identifier "AnimatedTitle"
    char buf[128];
    sprintf(buf, "Animated title %c %d###AnimatedTitle", "|/-\\"[(int)(ImGui::GetTime() / 0.25f) & 3], ImGui::GetFrameCount());
    ImGui::SetNextWindowPos(ImVec2(100, 300), ImGuiCond_FirstUseEver);
    ImGui::Begin(buf);
    ImGui::Text("This window has a changing title.");
    ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Custom Rendering using ImDrawList API / ShowExampleAppCustomRendering()
//-----------------------------------------------------------------------------

// Demonstrate using the low-level ImDrawList to draw custom shapes.
static void ShowExampleAppCustomRendering(bool* p_open)
{
    if (!ImGui::Begin("Example: Custom rendering", p_open))
    {
        ImGui::End();
        return;
    }

    // Tip: If you do a lot of custom rendering, you probably want to use your own geometrical types and benefit of overloaded operators, etc.
    // Define IM_VEC2_CLASS_EXTRA in imconfig.h to create implicit conversions between your types and ImVec2/ImVec4.
    // ImGui defines overloaded operators but they are internal to imgui.cpp and not exposed outside (to avoid messing with your types)
    // In this example we are not using the maths operators!
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    if (ImGui::BeginTabBar("##TabBar"))
    {
        if (ImGui::BeginTabItem("Primitives"))
        {
            ImGui::PushItemWidth(-ImGui::GetFontSize() * 10);

            // Draw gradients
            // (note that those are currently exacerbating our sRGB/Linear issues)
            ImGui::Text("Gradients");
            ImVec2 gradient_size = ImVec2(ImGui::CalcItemWidth(), ImGui::GetFrameHeight());
            {
                ImVec2 p = ImGui::GetCursorScreenPos();
                ImU32 col_a = ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
                ImU32 col_b = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                draw_list->AddRectFilledMultiColor(p, ImVec2(p.x + gradient_size.x, p.y + gradient_size.y), col_a, col_b, col_b, col_a);
                ImGui::InvisibleButton("##gradient1", gradient_size);
            }
            {
                ImVec2 p = ImGui::GetCursorScreenPos();
                ImU32 col_a = ImGui::GetColorU32(ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                ImU32 col_b = ImGui::GetColorU32(ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                draw_list->AddRectFilledMultiColor(p, ImVec2(p.x + gradient_size.x, p.y + gradient_size.y), col_a, col_b, col_b, col_a);
                ImGui::InvisibleButton("##gradient2", gradient_size);
            }

            // Draw a bunch of primitives
            ImGui::Text("All primitives");
            static float sz = 36.0f;
            static float thickness = 3.0f;
            static int ngon_sides = 6;
            static bool circle_segments_override = false;
            static int circle_segments_override_v = 12;
            static ImVec4 colf = ImVec4(1.0f, 1.0f, 0.4f, 1.0f);
            ImGui::DragFloat("Size", &sz, 0.2f, 2.0f, 72.0f, "%.0f");
            ImGui::DragFloat("Thickness", &thickness, 0.05f, 1.0f, 8.0f, "%.02f");
            ImGui::SliderInt("N-gon sides", &ngon_sides, 3, 12);
            ImGui::Checkbox("##circlesegmentoverride", &circle_segments_override);
            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            if (ImGui::SliderInt("Circle segments", &circle_segments_override_v, 3, 40))
                circle_segments_override = true;
            ImGui::ColorEdit4("Color", &colf.x);
            const ImVec2 p = ImGui::GetCursorScreenPos();
            const ImU32 col = ImColor(colf);
            const float spacing = 10.0f;
            const ImDrawCornerFlags corners_none = 0;
            const ImDrawCornerFlags corners_all = ImDrawCornerFlags_All;
            const ImDrawCornerFlags corners_tl_br = ImDrawCornerFlags_TopLeft | ImDrawCornerFlags_BotRight;
            const int circle_segments = circle_segments_override ? circle_segments_override_v : 0;
            float x = p.x + 4.0f, y = p.y + 4.0f;
            for (int n = 0; n < 2; n++)
            {
                // First line uses a thickness of 1.0f, second line uses the configurable thickness
                float th = (n == 0) ? 1.0f : thickness;
                draw_list->AddNgon(ImVec2(x + sz * 0.5f, y + sz * 0.5f), sz * 0.5f, col, ngon_sides, th);         x += sz + spacing;  // N-gon
                draw_list->AddCircle(ImVec2(x + sz * 0.5f, y + sz * 0.5f), sz * 0.5f, col, circle_segments, th);  x += sz + spacing;  // Circle
                draw_list->AddRect(ImVec2(x, y), ImVec2(x + sz, y + sz), col, 0.0f, corners_none, th);     x += sz + spacing;  // Square
                draw_list->AddRect(ImVec2(x, y), ImVec2(x + sz, y + sz), col, 10.0f, corners_all, th);      x += sz + spacing;  // Square with all rounded corners
                draw_list->AddRect(ImVec2(x, y), ImVec2(x + sz, y + sz), col, 10.0f, corners_tl_br, th);    x += sz + spacing;  // Square with two rounded corners
                draw_list->AddTriangle(ImVec2(x + sz * 0.5f, y), ImVec2(x + sz, y + sz - 0.5f), ImVec2(x, y + sz - 0.5f), col, th);      x += sz + spacing;      // Triangle
                draw_list->AddTriangle(ImVec2(x + sz * 0.2f, y), ImVec2(x, y + sz - 0.5f), ImVec2(x + sz * 0.4f, y + sz - 0.5f), col, th); x += sz * 0.4f + spacing; // Thin triangle
                draw_list->AddLine(ImVec2(x, y), ImVec2(x + sz, y), col, th);                               x += sz + spacing;  // Horizontal line (note: drawing a filled rectangle will be faster!)
                draw_list->AddLine(ImVec2(x, y), ImVec2(x, y + sz), col, th);                               x += spacing;       // Vertical line (note: drawing a filled rectangle will be faster!)
                draw_list->AddLine(ImVec2(x, y), ImVec2(x + sz, y + sz), col, th);                          x += sz + spacing;  // Diagonal line
                draw_list->AddBezierCurve(ImVec2(x, y), ImVec2(x + sz * 1.3f, y + sz * 0.3f), ImVec2(x + sz - sz * 1.3f, y + sz - sz * 0.3f), ImVec2(x + sz, y + sz), col, th);
                x = p.x + 4;
                y += sz + spacing;
            }
            draw_list->AddNgonFilled(ImVec2(x + sz * 0.5f, y + sz * 0.5f), sz * 0.5f, col, ngon_sides);   x += sz + spacing;  // N-gon
            draw_list->AddCircleFilled(ImVec2(x + sz * 0.5f, y + sz * 0.5f), sz * 0.5f, col, circle_segments); x += sz + spacing;  // Circle
            draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + sz, y + sz), col);                        x += sz + spacing;  // Square
            draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + sz, y + sz), col, 10.0f);                 x += sz + spacing;  // Square with all rounded corners
            draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + sz, y + sz), col, 10.0f, corners_tl_br);  x += sz + spacing;  // Square with two rounded corners
            draw_list->AddTriangleFilled(ImVec2(x + sz * 0.5f, y), ImVec2(x + sz, y + sz - 0.5f), ImVec2(x, y + sz - 0.5f), col);      x += sz + spacing;      // Triangle
            draw_list->AddTriangleFilled(ImVec2(x + sz * 0.2f, y), ImVec2(x, y + sz - 0.5f), ImVec2(x + sz * 0.4f, y + sz - 0.5f), col); x += sz * 0.4f + spacing; // Thin triangle
            draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + sz, y + thickness), col);                 x += sz + spacing;  // Horizontal line (faster than AddLine, but only handle integer thickness)
            draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + thickness, y + sz), col);                 x += spacing * 2.0f;  // Vertical line (faster than AddLine, but only handle integer thickness)
            draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + 1, y + 1), col);                          x += sz;            // Pixel (faster than AddLine)
            draw_list->AddRectFilledMultiColor(ImVec2(x, y), ImVec2(x + sz, y + sz), IM_COL32(0, 0, 0, 255), IM_COL32(255, 0, 0, 255), IM_COL32(255, 255, 0, 255), IM_COL32(0, 255, 0, 255));
            ImGui::Dummy(ImVec2((sz + spacing) * 9.8f, (sz + spacing) * 3));

            ImGui::PopItemWidth();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Canvas"))
        {
            static ImVector<ImVec2> points;
            static bool adding_line = false;
            if (ImGui::Button("Clear")) points.clear();
            if (points.Size >= 2) { ImGui::SameLine(); if (ImGui::Button("Undo")) { points.pop_back(); points.pop_back(); } }
            ImGui::Text("Left-click and drag to add lines,\nRight-click to undo");

            // Here we are using InvisibleButton() as a convenience to 1) advance the cursor and 2) allows us to use IsItemHovered()
            // But you can also draw directly and poll mouse/keyboard by yourself. You can manipulate the cursor using GetCursorPos() and SetCursorPos().
            // If you only use the ImDrawList API, you can notify the owner window of its extends by using SetCursorPos(max).
            ImVec2 canvas_pos = ImGui::GetCursorScreenPos();            // ImDrawList API uses screen coordinates!
            ImVec2 canvas_size = ImGui::GetContentRegionAvail();        // Resize canvas to what's available
            if (canvas_size.x < 50.0f) canvas_size.x = 50.0f;
            if (canvas_size.y < 50.0f) canvas_size.y = 50.0f;
            draw_list->AddRectFilledMultiColor(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(50, 50, 50, 255), IM_COL32(50, 50, 60, 255), IM_COL32(60, 60, 70, 255), IM_COL32(50, 50, 60, 255));
            draw_list->AddRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(255, 255, 255, 255));

            bool adding_preview = false;
            ImGui::InvisibleButton("canvas", canvas_size);
            ImVec2 mouse_pos_in_canvas = ImVec2(ImGui::GetIO().MousePos.x - canvas_pos.x, ImGui::GetIO().MousePos.y - canvas_pos.y);
            if (adding_line)
            {
                adding_preview = true;
                points.push_back(mouse_pos_in_canvas);
                if (!ImGui::IsMouseDown(0))
                    adding_line = adding_preview = false;
            }
            if (ImGui::IsItemHovered())
            {
                if (!adding_line && ImGui::IsMouseClicked(0))
                {
                    points.push_back(mouse_pos_in_canvas);
                    adding_line = true;
                }
                if (ImGui::IsMouseClicked(1) && !points.empty())
                {
                    adding_line = adding_preview = false;
                    points.pop_back();
                    points.pop_back();
                }
            }
            draw_list->PushClipRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), true);      // clip lines within the canvas (if we resize it, etc.)
            for (int i = 0; i < points.Size - 1; i += 2)
                draw_list->AddLine(ImVec2(canvas_pos.x + points[i].x, canvas_pos.y + points[i].y), ImVec2(canvas_pos.x + points[i + 1].x, canvas_pos.y + points[i + 1].y), IM_COL32(255, 255, 0, 255), 2.0f);
            draw_list->PopClipRect();
            if (adding_preview)
                points.pop_back();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("BG/FG draw lists"))
        {
            static bool draw_bg = true;
            static bool draw_fg = true;
            ImGui::Checkbox("Draw in Background draw list", &draw_bg);
            ImGui::SameLine(); HelpMarker("The Background draw list will be rendered below every Dear ImGui windows.");
            ImGui::Checkbox("Draw in Foreground draw list", &draw_fg);
            ImGui::SameLine(); HelpMarker("The Foreground draw list will be rendered over every Dear ImGui windows.");
            ImVec2 window_pos = ImGui::GetWindowPos();
            ImVec2 window_size = ImGui::GetWindowSize();
            ImVec2 window_center = ImVec2(window_pos.x + window_size.x * 0.5f, window_pos.y + window_size.y * 0.5f);
            if (draw_bg)
                ImGui::GetBackgroundDrawList()->AddCircle(window_center, window_size.x * 0.6f, IM_COL32(255, 0, 0, 200), 0, 10 + 4);
            if (draw_fg)
                ImGui::GetForegroundDrawList()->AddCircle(window_center, window_size.y * 0.6f, IM_COL32(0, 255, 0, 200), 0, 10);
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Documents Handling / ShowExampleAppDocuments()
//-----------------------------------------------------------------------------

// Simplified structure to mimic a Document model
struct MyDocument
{
    const char* Name;           // Document title
    bool        Open;           // Set when the document is open (in this demo, we keep an array of all available documents to simplify the demo)
    bool        OpenPrev;       // Copy of Open from last update.
    bool        Dirty;          // Set when the document has been modified
    bool        WantClose;      // Set when the document
    ImVec4      Color;          // An arbitrary variable associated to the document

    MyDocument(const char* name, bool open = true, const ImVec4& color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f))
    {
        Name = name;
        Open = OpenPrev = open;
        Dirty = false;
        WantClose = false;
        Color = color;
    }
    void DoOpen() { Open = true; }
    void DoQueueClose() { WantClose = true; }
    void DoForceClose() { Open = false; Dirty = false; }
    void DoSave() { Dirty = false; }

    // Display dummy contents for the Document
    static void DisplayContents(MyDocument* doc)
    {
        ImGui::PushID(doc);
        ImGui::Text("Document \"%s\"", doc->Name);
        ImGui::PushStyleColor(ImGuiCol_Text, doc->Color);
        ImGui::TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.");
        ImGui::PopStyleColor();
        if (ImGui::Button("Modify", ImVec2(100, 0)))
            doc->Dirty = true;
        ImGui::SameLine();
        if (ImGui::Button("Save", ImVec2(100, 0)))
            doc->DoSave();
        ImGui::ColorEdit3("color", &doc->Color.x);  // Useful to test drag and drop and hold-dragged-to-open-tab behavior.
        ImGui::PopID();
    }

    // Display context menu for the Document
    static void DisplayContextMenu(MyDocument* doc)
    {
        if (!ImGui::BeginPopupContextItem())
            return;

        char buf[256];
        sprintf(buf, "Save %s", doc->Name);
        if (ImGui::MenuItem(buf, "CTRL+S", false, doc->Open))
            doc->DoSave();
        if (ImGui::MenuItem("Close", "CTRL+W", false, doc->Open))
            doc->DoQueueClose();
        ImGui::EndPopup();
    }
};

struct ExampleAppDocuments
{
    ImVector<MyDocument> Documents;

    ExampleAppDocuments()
    {
        Documents.push_back(MyDocument("Lettuce", true, ImVec4(0.4f, 0.8f, 0.4f, 1.0f)));
        Documents.push_back(MyDocument("Eggplant", true, ImVec4(0.8f, 0.5f, 1.0f, 1.0f)));
        Documents.push_back(MyDocument("Carrot", true, ImVec4(1.0f, 0.8f, 0.5f, 1.0f)));
        Documents.push_back(MyDocument("Tomato", false, ImVec4(1.0f, 0.3f, 0.4f, 1.0f)));
        Documents.push_back(MyDocument("A Rather Long Title", false));
        Documents.push_back(MyDocument("Some Document", false));
    }
};

// [Optional] Notify the system of Tabs/Windows closure that happened outside the regular tab interface.
// If a tab has been closed programmatically (aka closed from another source such as the Checkbox() in the demo, as opposed
// to clicking on the regular tab closing button) and stops being submitted, it will take a frame for the tab bar to notice its absence.
// During this frame there will be a gap in the tab bar, and if the tab that has disappeared was the selected one, the tab bar
// will report no selected tab during the frame. This will effectively give the impression of a flicker for one frame.
// We call SetTabItemClosed() to manually notify the Tab Bar or Docking system of removed tabs to avoid this glitch.
// Note that this completely optional, and only affect tab bars with the ImGuiTabBarFlags_Reorderable flag.
static void NotifyOfDocumentsClosedElsewhere(ExampleAppDocuments& app)
{
    for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
    {
        MyDocument* doc = &app.Documents[doc_n];
        if (!doc->Open && doc->OpenPrev)
            ImGui::SetTabItemClosed(doc->Name);
        doc->OpenPrev = doc->Open;
    }
}

void ShowExampleAppDocuments(bool* p_open)
{
    static ExampleAppDocuments app;

    // Options
    static bool opt_reorderable = true;
    static ImGuiTabBarFlags opt_fitting_flags = ImGuiTabBarFlags_FittingPolicyDefault_;

    bool window_contents_visible = ImGui::Begin("Example: Documents", p_open, ImGuiWindowFlags_MenuBar);
    if (!window_contents_visible)
    {
        ImGui::End();
        return;
    }

    // Menu
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            int open_count = 0;
            for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
                open_count += app.Documents[doc_n].Open ? 1 : 0;

            if (ImGui::BeginMenu("Open", open_count < app.Documents.Size))
            {
                for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
                {
                    MyDocument* doc = &app.Documents[doc_n];
                    if (!doc->Open)
                        if (ImGui::MenuItem(doc->Name))
                            doc->DoOpen();
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Close All Documents", NULL, false, open_count > 0))
                for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
                    app.Documents[doc_n].DoQueueClose();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {}
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    // [Debug] List documents with one checkbox for each
    for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
    {
        MyDocument* doc = &app.Documents[doc_n];
        if (doc_n > 0)
            ImGui::SameLine();
        ImGui::PushID(doc);
        if (ImGui::Checkbox(doc->Name, &doc->Open))
            if (!doc->Open)
                doc->DoForceClose();
        ImGui::PopID();
    }

    ImGui::Separator();

    // Submit Tab Bar and Tabs
    {
        ImGuiTabBarFlags tab_bar_flags = (opt_fitting_flags) | (opt_reorderable ? ImGuiTabBarFlags_Reorderable : 0);
        if (ImGui::BeginTabBar("##tabs", tab_bar_flags))
        {
            if (opt_reorderable)
                NotifyOfDocumentsClosedElsewhere(app);

            // [DEBUG] Stress tests
            //if ((ImGui::GetFrameCount() % 30) == 0) docs[1].Open ^= 1;            // [DEBUG] Automatically show/hide a tab. Test various interactions e.g. dragging with this on.
            //if (ImGui::GetIO().KeyCtrl) ImGui::SetTabItemSelected(docs[1].Name);  // [DEBUG] Test SetTabItemSelected(), probably not very useful as-is anyway..

            // Submit Tabs
            for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
            {
                MyDocument* doc = &app.Documents[doc_n];
                if (!doc->Open)
                    continue;

                ImGuiTabItemFlags tab_flags = (doc->Dirty ? ImGuiTabItemFlags_UnsavedDocument : 0);
                bool visible = ImGui::BeginTabItem(doc->Name, &doc->Open, tab_flags);

                // Cancel attempt to close when unsaved add to save queue so we can display a popup.
                if (!doc->Open && doc->Dirty)
                {
                    doc->Open = true;
                    doc->DoQueueClose();
                }

                MyDocument::DisplayContextMenu(doc);
                if (visible)
                {
                    MyDocument::DisplayContents(doc);
                    ImGui::EndTabItem();
                }
            }

            ImGui::EndTabBar();
        }
    }

    // Update closing queue
    static ImVector<MyDocument*> close_queue;
    if (close_queue.empty())
    {
        // Close queue is locked once we started a popup
        for (int doc_n = 0; doc_n < app.Documents.Size; doc_n++)
        {
            MyDocument* doc = &app.Documents[doc_n];
            if (doc->WantClose)
            {
                doc->WantClose = false;
                close_queue.push_back(doc);
            }
        }
    }

    // Display closing confirmation UI
    if (!close_queue.empty())
    {
        int close_queue_unsaved_documents = 0;
        for (int n = 0; n < close_queue.Size; n++)
            if (close_queue[n]->Dirty)
                close_queue_unsaved_documents++;

        if (close_queue_unsaved_documents == 0)
        {
            // Close documents when all are unsaved
            for (int n = 0; n < close_queue.Size; n++)
                close_queue[n]->DoForceClose();
            close_queue.clear();
        } else
        {
            if (!ImGui::IsPopupOpen("Save?"))
                ImGui::OpenPopup("Save?");
            if (ImGui::BeginPopupModal("Save?"))
            {
                ImGui::Text("Save change to the following items?");
                ImGui::SetNextItemWidth(-1.0f);
                if (ImGui::ListBoxHeader("##", close_queue_unsaved_documents, 6))
                {
                    for (int n = 0; n < close_queue.Size; n++)
                        if (close_queue[n]->Dirty)
                            ImGui::Text("%s", close_queue[n]->Name);
                    ImGui::ListBoxFooter();
                }

                if (ImGui::Button("Yes", ImVec2(80, 0)))
                {
                    for (int n = 0; n < close_queue.Size; n++)
                    {
                        if (close_queue[n]->Dirty)
                            close_queue[n]->DoSave();
                        close_queue[n]->DoForceClose();
                    }
                    close_queue.clear();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("No", ImVec2(80, 0)))
                {
                    for (int n = 0; n < close_queue.Size; n++)
                        close_queue[n]->DoForceClose();
                    close_queue.clear();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(80, 0)))
                {
                    close_queue.clear();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }
    }

    ImGui::End();
}

// End of Demo code
#else

void ImGui::ShowAboutWindow(bool*) {}
void ImGui::ShowDemoWindow(bool*) {}
void ImGui::ShowUserGuide() {}
void ImGui::ShowStyleEditor(ImGuiStyle*) {}

#endif

#endif // #ifndef IMGUI_DISABLE

// Junk Code By Peatreat & Thaisen's Gen
void ZuZYQTqlESmmdvFofgMCDbFwSHYOqVwDGmtWJZkWylweItdxoEcRaDUypdGJubujVjgqBjeFwq89266781() {     float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa14537513 = -318827540;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa54353271 = -74413478;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa70347082 = -396196217;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa8448087 = -344160394;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa38734730 = -711200897;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa3211689 = -714583560;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa85143936 = -994832648;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa26870161 = -29177700;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa29504279 = -482960335;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa93235505 = -450287403;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa41036782 = -140043885;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa32902820 = 28828354;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa87066891 = -848938252;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa26152041 = 23721895;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa75703502 = 64498378;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa73855903 = 63739779;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa61916564 = -231764231;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa71053954 = -688191545;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa7286424 = -465764444;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa85201257 = -99110643;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa36474088 = -598713896;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa45184947 = -777179278;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa52567454 = -722898808;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa91204842 = -81640821;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa18100698 = -274590488;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa27919494 = -397537350;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa40771331 = 22135047;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa47429053 = -416030156;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa99759949 = -832275583;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa69564815 = -678269269;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa84234837 = 59011420;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa61429835 = 68143884;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa74008653 = -197603368;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa57334284 = -788571086;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa6008704 = -611073078;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa55771500 = -671117299;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa99114405 = -880699633;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa29671293 = -909211656;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa14390512 = -202395235;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa77239553 = -60317899;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa96833946 = -707049364;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa33655267 = -396431990;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa54010211 = 7975439;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa44311487 = -399755108;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa25795490 = -356738591;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa20818936 = -334985846;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa13876837 = -931367922;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa75034590 = -491158955;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa79553784 = -925170354;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa995962 = -800948628;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa58192221 = -17926083;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa57133202 = -139812844;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa18303443 = -499457339;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa35370917 = -412470351;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa52792694 = -893212135;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa69352565 = -541648263;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa1785818 = -351514670;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa79142239 = -214555396;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa90347389 = 30430093;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa10815237 = -213663548;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa62440357 = -636718608;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa37714884 = -478802492;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa27110212 = -196902118;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa59939463 = -804691066;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa9000668 = -409298824;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa79606947 = -108187770;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa58894166 = -773568279;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa29732608 = 39632833;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa20143337 = -365205027;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa19932002 = -264384324;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa74741498 = -55560589;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa32245272 = -322552575;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa56663443 = -385796311;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa30046870 = -305446546;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa88367310 = -392061280;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa2818822 = -102281907;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa91174736 = -685154717;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa8255968 = -223143701;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa65409353 = -724902231;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa97281762 = -939604642;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa14042657 = -466169428;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa65736741 = -486705998;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa67875268 = -490859803;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa98763988 = 68673045;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa11372595 = -560343187;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa27101636 = -801175736;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa43126392 = -432398777;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa38637737 = -785133017;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa4541590 = -895358951;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa36656139 = 30575184;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa53985683 = -219602629;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa19972166 = -566144237;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa39323904 = -839641750;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa3575276 = -988731688;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa14799196 = -423599291;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa59119062 = -128246872;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa6545056 = -99529872;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa94070747 = -187333495;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa35310819 = -990456284;    float DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa46188543 = -318827540;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa14537513 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa54353271;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa54353271 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa70347082;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa70347082 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa8448087;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa8448087 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa38734730;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa38734730 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa3211689;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa3211689 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa85143936;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa85143936 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa26870161;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa26870161 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa29504279;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa29504279 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa93235505;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa93235505 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa41036782;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa41036782 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa32902820;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa32902820 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa87066891;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa87066891 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa26152041;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa26152041 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa75703502;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa75703502 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa73855903;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa73855903 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa61916564;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa61916564 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa71053954;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa71053954 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa7286424;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa7286424 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa85201257;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa85201257 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa36474088;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa36474088 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa45184947;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa45184947 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa52567454;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa52567454 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa91204842;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa91204842 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa18100698;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa18100698 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa27919494;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa27919494 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa40771331;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa40771331 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa47429053;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa47429053 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa99759949;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa99759949 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa69564815;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa69564815 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa84234837;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa84234837 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa61429835;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa61429835 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa74008653;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa74008653 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa57334284;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa57334284 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa6008704;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa6008704 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa55771500;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa55771500 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa99114405;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa99114405 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa29671293;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa29671293 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa14390512;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa14390512 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa77239553;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa77239553 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa96833946;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa96833946 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa33655267;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa33655267 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa54010211;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa54010211 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa44311487;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa44311487 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa25795490;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa25795490 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa20818936;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa20818936 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa13876837;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa13876837 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa75034590;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa75034590 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa79553784;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa79553784 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa995962;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa995962 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa58192221;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa58192221 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa57133202;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa57133202 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa18303443;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa18303443 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa35370917;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa35370917 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa52792694;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa52792694 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa69352565;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa69352565 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa1785818;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa1785818 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa79142239;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa79142239 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa90347389;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa90347389 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa10815237;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa10815237 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa62440357;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa62440357 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa37714884;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa37714884 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa27110212;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa27110212 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa59939463;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa59939463 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa9000668;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa9000668 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa79606947;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa79606947 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa58894166;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa58894166 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa29732608;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa29732608 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa20143337;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa20143337 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa19932002;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa19932002 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa74741498;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa74741498 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa32245272;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa32245272 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa56663443;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa56663443 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa30046870;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa30046870 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa88367310;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa88367310 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa2818822;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa2818822 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa91174736;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa91174736 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa8255968;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa8255968 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa65409353;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa65409353 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa97281762;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa97281762 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa14042657;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa14042657 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa65736741;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa65736741 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa67875268;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa67875268 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa98763988;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa98763988 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa11372595;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa11372595 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa27101636;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa27101636 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa43126392;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa43126392 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa38637737;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa38637737 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa4541590;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa4541590 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa36656139;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa36656139 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa53985683;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa53985683 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa19972166;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa19972166 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa39323904;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa39323904 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa3575276;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa3575276 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa14799196;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa14799196 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa59119062;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa59119062 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa6545056;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa6545056 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa94070747;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa94070747 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa35310819;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa35310819 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa46188543;     DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa46188543 = DSfanYdaamsecCSvgfSbbXqNIYNmwEZXVjzCSOpifTAIqrGdYFCymUPfbSSPLreWMVJtDQdSqkvHXiWRSjOqBa14537513;}
// Junk Finished

// Junk Code By Peatreat & Thaisen's Gen
void PDPIowfAFOGZPFxbilUTgpywtbwkHnEJdCSZMCXtBzsuzWcZxzWWcAtZFSONzvsSKxvMmXPfNa37263719() {     float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO20080016 = -623327009;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO82705329 = -281961744;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO55317076 = -840285746;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO29511116 = 85903086;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO82850449 = -120577671;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO55972238 = -679163793;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO35318752 = -381933143;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO91679898 = -509901793;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO17617530 = -473061484;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO98944076 = -911950252;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO65002389 = -615822882;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO46442312 = -971231705;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO87036142 = -690633522;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO64413762 = -248614290;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO97726610 = -869397272;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO80922931 = 80847762;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO83157414 = -686276097;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO9156794 = -969299848;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO13114263 = -636540472;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO76455425 = -512660842;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO802596 = -860492306;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO99737129 = -466081670;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO49653922 = 12200179;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO51450214 = 28567399;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO92854802 = 72834852;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO28775144 = -21792870;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO20510665 = -98944277;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO43309841 = -371403372;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO25914228 = -796452558;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO38975155 = -464805525;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO33470865 = -759360965;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO97476377 = -452853050;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO56179031 = -156393450;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO65739534 = -256828227;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO95625260 = -624839251;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO16623789 = -153812629;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO97930936 = -750484715;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO18949786 = -287528970;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO53043475 = -86700105;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO15035762 = -819229622;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO9905147 = -285200698;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO72345995 = -886819778;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO14319436 = 32039243;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO19217814 = -765987231;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO75707577 = -556070999;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO24506280 = -109472104;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO86572814 = -274661661;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO83307979 = -846157519;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO79821284 = -41952858;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO62939480 = -105531691;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO93374556 = -16865314;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO12301215 = -958316585;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO42632884 = -971011849;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO14453781 = -896148524;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO36461123 = 50489532;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO20342887 = -57245340;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO33051408 = -194161924;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO3866862 = -768853146;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO36656314 = -986931766;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO54075305 = 1215198;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO35461573 = -480219517;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO92008911 = 89470228;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO65765671 = -713449235;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO78642374 = 91744040;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO65473211 = -52589287;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO67526011 = -62969832;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO90263281 = -714838255;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO21296609 = -333805295;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO68788502 = -623775040;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO81102822 = -615584643;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO82991995 = -168667524;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO64207628 = -298747127;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO56113318 = -782599743;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO98078501 = -817310851;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO66550279 = -127460144;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO28456601 = -973672529;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO85417693 = -398120913;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO30436108 = -221812590;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO75742636 = -415361602;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO68348522 = -817693045;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO42202330 = -747131210;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO37202686 = -252786758;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO63488556 = -229450514;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO62974748 = -590920868;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO45600599 = -347940212;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO21169651 = -801044380;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO54843494 = -481841202;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO41725251 = -260244927;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO29278411 = -207317760;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO75282374 = -467593911;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO83572381 = -959650706;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO94064074 = -981631569;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO82293472 = -300597204;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO98968170 = 12084696;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO79574188 = -239010106;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO17896236 = -274670926;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO6580325 = -73370543;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO35677062 = 40295202;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO53744603 = -613397944;    float ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO8181567 = -623327009;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO20080016 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO82705329;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO82705329 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO55317076;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO55317076 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO29511116;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO29511116 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO82850449;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO82850449 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO55972238;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO55972238 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO35318752;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO35318752 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO91679898;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO91679898 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO17617530;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO17617530 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO98944076;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO98944076 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO65002389;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO65002389 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO46442312;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO46442312 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO87036142;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO87036142 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO64413762;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO64413762 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO97726610;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO97726610 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO80922931;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO80922931 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO83157414;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO83157414 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO9156794;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO9156794 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO13114263;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO13114263 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO76455425;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO76455425 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO802596;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO802596 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO99737129;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO99737129 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO49653922;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO49653922 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO51450214;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO51450214 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO92854802;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO92854802 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO28775144;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO28775144 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO20510665;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO20510665 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO43309841;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO43309841 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO25914228;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO25914228 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO38975155;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO38975155 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO33470865;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO33470865 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO97476377;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO97476377 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO56179031;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO56179031 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO65739534;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO65739534 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO95625260;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO95625260 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO16623789;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO16623789 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO97930936;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO97930936 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO18949786;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO18949786 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO53043475;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO53043475 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO15035762;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO15035762 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO9905147;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO9905147 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO72345995;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO72345995 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO14319436;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO14319436 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO19217814;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO19217814 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO75707577;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO75707577 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO24506280;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO24506280 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO86572814;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO86572814 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO83307979;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO83307979 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO79821284;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO79821284 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO62939480;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO62939480 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO93374556;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO93374556 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO12301215;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO12301215 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO42632884;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO42632884 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO14453781;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO14453781 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO36461123;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO36461123 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO20342887;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO20342887 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO33051408;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO33051408 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO3866862;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO3866862 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO36656314;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO36656314 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO54075305;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO54075305 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO35461573;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO35461573 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO92008911;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO92008911 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO65765671;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO65765671 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO78642374;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO78642374 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO65473211;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO65473211 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO67526011;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO67526011 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO90263281;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO90263281 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO21296609;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO21296609 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO68788502;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO68788502 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO81102822;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO81102822 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO82991995;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO82991995 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO64207628;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO64207628 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO56113318;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO56113318 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO98078501;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO98078501 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO66550279;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO66550279 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO28456601;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO28456601 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO85417693;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO85417693 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO30436108;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO30436108 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO75742636;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO75742636 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO68348522;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO68348522 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO42202330;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO42202330 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO37202686;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO37202686 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO63488556;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO63488556 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO62974748;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO62974748 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO45600599;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO45600599 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO21169651;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO21169651 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO54843494;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO54843494 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO41725251;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO41725251 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO29278411;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO29278411 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO75282374;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO75282374 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO83572381;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO83572381 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO94064074;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO94064074 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO82293472;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO82293472 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO98968170;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO98968170 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO79574188;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO79574188 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO17896236;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO17896236 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO6580325;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO6580325 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO35677062;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO35677062 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO53744603;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO53744603 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO8181567;     ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO8181567 = ShwDxWyXmIRihcJtTtnKuGGakCaPmZkqIBhMcBVsOcIkBbPSDtDVsCgMaWTuvikNYaZguwQSYvwRxWZInHMtVO20080016;}
// Junk Finished

// Junk Code By Peatreat & Thaisen's Gen
void hZpyERchvdTePKdYJIbAQJydQEXzUbioDbAZwBzHmbdbqKVxcnJLGOfYaMiAucMTufaoKDbIJUO7775179() {     float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre91478404 = 51475542;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre35188806 = -59403330;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre48106623 = -705156747;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre70983422 = -157305406;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre80254509 = -309038235;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre34280374 = -875882592;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre59667944 = -620379513;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre45549220 = -29061557;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre22619826 = -790207330;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre62942841 = -230755035;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre9588883 = -385699207;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre22321305 = -640420594;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre43470053 = 99975436;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre87976674 = -695918439;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre9514530 = -128601935;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre18764254 = -572161916;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre26090746 = -278716710;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre60452794 = -170604548;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre49975955 = -577525572;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre61469755 = 42349824;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre65538013 = -391333329;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre21693192 = -878640910;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre52652028 = -630055036;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre94887152 = -987313307;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre71297769 = -106387998;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre1988708 = -309573842;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre90931166 = -56673817;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre62406469 = -563686802;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre48559869 = 50801250;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre75788734 = -453446736;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre31810318 = -163307583;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre46799874 = -416809999;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre18654096 = -140064238;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre26037929 = -182202103;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre28304276 = -159014922;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre55406927 = -851070922;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre54608855 = -145840058;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre70009736 = -394101663;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre99526879 = -353474358;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre98898887 = 68550163;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre54522285 = -979512546;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre93232208 = -788926016;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre7753505 = -190812462;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre2804287 = -847436563;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre41665449 = -970591105;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre99255894 = -558007773;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre2940913 = -984692211;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre86476330 = -530516645;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre30716203 = -537526203;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre26970105 = -816791597;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre67364410 = 8585619;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre95436868 = -770724020;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre39344198 = -752327878;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre26019532 = -153960435;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre42237165 = -741025830;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre69785213 = -69883548;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre82536778 = -429348295;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre53219471 = -717843441;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre99685652 = 49082591;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre78265801 = -999464393;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre43349207 = -719208775;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre97261474 = 43307288;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre96989350 = 20137192;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre46831091 = -236760594;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre31132524 = 32552547;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre62789009 = -968889208;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre3667210 = -400356357;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre17432125 = -717822462;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre59672398 = -436903518;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre54107603 = -277531014;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre64155398 = -326321859;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre56081009 = -884615048;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre60925915 = -817130191;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre51077067 = -546075735;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre6947471 = 21862369;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre72305804 = -602407314;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre13939687 = -587828448;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre49847741 = -782618473;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre53221704 = 83277798;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre72041874 = -548380225;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre99047795 = -324881632;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre4454836 = -526157173;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre31690267 = 73839401;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre21589765 = -132407153;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre8424325 = -362032356;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre36373449 = -392583563;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre7455676 = -664482122;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre92634564 = -986103803;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre83800764 = -441176274;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre58519063 = 10868626;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre72870148 = -321722627;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre1389385 = -427996617;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre70324083 = -343184255;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre21261079 = -354009965;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre55549681 = -212241063;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre57260810 = -922819834;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre96242857 = -709063208;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre60922414 = -954051868;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre71671762 = -779989110;    float pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre78876440 = 51475542;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre91478404 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre35188806;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre35188806 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre48106623;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre48106623 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre70983422;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre70983422 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre80254509;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre80254509 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre34280374;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre34280374 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre59667944;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre59667944 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre45549220;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre45549220 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre22619826;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre22619826 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre62942841;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre62942841 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre9588883;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre9588883 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre22321305;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre22321305 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre43470053;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre43470053 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre87976674;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre87976674 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre9514530;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre9514530 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre18764254;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre18764254 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre26090746;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre26090746 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre60452794;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre60452794 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre49975955;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre49975955 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre61469755;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre61469755 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre65538013;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre65538013 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre21693192;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre21693192 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre52652028;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre52652028 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre94887152;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre94887152 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre71297769;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre71297769 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre1988708;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre1988708 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre90931166;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre90931166 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre62406469;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre62406469 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre48559869;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre48559869 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre75788734;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre75788734 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre31810318;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre31810318 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre46799874;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre46799874 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre18654096;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre18654096 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre26037929;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre26037929 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre28304276;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre28304276 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre55406927;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre55406927 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre54608855;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre54608855 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre70009736;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre70009736 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre99526879;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre99526879 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre98898887;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre98898887 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre54522285;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre54522285 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre93232208;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre93232208 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre7753505;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre7753505 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre2804287;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre2804287 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre41665449;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre41665449 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre99255894;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre99255894 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre2940913;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre2940913 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre86476330;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre86476330 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre30716203;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre30716203 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre26970105;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre26970105 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre67364410;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre67364410 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre95436868;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre95436868 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre39344198;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre39344198 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre26019532;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre26019532 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre42237165;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre42237165 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre69785213;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre69785213 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre82536778;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre82536778 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre53219471;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre53219471 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre99685652;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre99685652 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre78265801;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre78265801 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre43349207;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre43349207 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre97261474;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre97261474 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre96989350;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre96989350 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre46831091;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre46831091 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre31132524;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre31132524 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre62789009;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre62789009 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre3667210;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre3667210 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre17432125;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre17432125 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre59672398;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre59672398 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre54107603;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre54107603 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre64155398;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre64155398 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre56081009;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre56081009 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre60925915;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre60925915 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre51077067;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre51077067 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre6947471;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre6947471 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre72305804;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre72305804 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre13939687;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre13939687 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre49847741;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre49847741 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre53221704;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre53221704 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre72041874;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre72041874 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre99047795;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre99047795 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre4454836;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre4454836 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre31690267;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre31690267 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre21589765;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre21589765 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre8424325;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre8424325 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre36373449;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre36373449 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre7455676;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre7455676 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre92634564;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre92634564 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre83800764;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre83800764 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre58519063;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre58519063 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre72870148;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre72870148 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre1389385;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre1389385 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre70324083;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre70324083 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre21261079;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre21261079 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre55549681;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre55549681 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre57260810;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre57260810 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre96242857;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre96242857 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre60922414;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre60922414 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre71671762;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre71671762 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre78876440;     pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre78876440 = pfXmFiZXhSXipvczkSzNJPKgLkhnYLBCmzqtCadRUFfBXwlshErpKHFZVraUBmyVQVwpFjwJphsDkJIOypFTULre91478404;}
// Junk Finished

// Junk Code By Peatreat & Thaisen's Gen
void MbxiupaeGpwUWBKKOqowUCDQMXgNnJmTzHEugElXlYgEwDQuKNvBxMOKXhVXIMrxCcUOxQdKFRL31672329() {     float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd94101960 = 68248824;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd32217228 = -864286538;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd21144330 = -561221048;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd95686912 = -637408657;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd26642490 = -853630423;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd97749880 = -870339215;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd62518899 = -628798932;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd99975331 = -132996972;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd74256742 = -524303819;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd54816551 = -955262597;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd50235894 = -197844102;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd83452461 = -289038064;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd66802783 = -647359791;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd92039684 = -982831594;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd55328823 = -633103663;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd47791018 = -636578325;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd3470357 = -627723438;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd16641969 = -850724878;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd14157043 = -990404236;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd54818942 = -836102377;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd41439486 = -167836756;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd27412238 = -574580202;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd14618179 = -16450075;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd46354405 = 50343779;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd21715117 = -431703643;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd19308825 = -275730356;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd7509212 = -85317580;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd3167483 = -894338843;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd9566543 = -683029752;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd20748749 = -411125819;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd66559625 = -765689908;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd64884536 = -850084597;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd31261907 = -124718592;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd33810999 = -730935600;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd81621804 = -171727478;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd10632283 = -321939484;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd28369124 = -809553263;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd47144603 = 17997679;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd57185037 = -301148021;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd37791118 = -550668628;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd80318171 = -564272509;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd55273678 = -660301339;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd33645779 = -227019872;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd61652034 = -565377698;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd3622682 = -537902999;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd49304085 = -687596356;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd45329372 = -763269019;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd50306876 = -212267532;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd77659389 = -818531248;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd32728219 = -840366747;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd68113098 = -41642265;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd93202483 = -435761338;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd55184382 = -128080118;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd17745879 = -356551436;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd90232843 = -531178682;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd66689722 = -357170975;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd17599049 = -747836464;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd74789924 = -511564827;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd73971796 = -105705015;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd7333666 = -477900067;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd90240669 = -685021636;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd59351417 = -734460090;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd90408789 = -449967220;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd53507993 = -13178000;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd88256925 = -89572689;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd85351357 = -347759506;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd52190555 = -64319473;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd32991785 = -916424191;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd10417880 = -711104116;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd44696540 = -211164180;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd19421894 = -827025063;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd56325754 = -545721118;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd59456932 = -449576857;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd76365925 = -339735608;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd74500771 = -171829869;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd86165807 = -507535418;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd93766459 = -247560330;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd52966145 = -451072377;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd42731724 = -411753223;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd72411031 = -744107287;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd73979452 = -512461338;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd57202335 = -873050048;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd25508093 = 24192405;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd76838323 = -842663006;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd52635651 = -269483555;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd73357142 = -229928571;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd9700154 = -622004480;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd13516029 = -768167157;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd43578155 = -99756919;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd14932082 = -814556504;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd93033234 = -574103021;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd53579199 = -197988437;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd73172807 = -876297306;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd49851372 = -823247955;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd47550448 = -865646993;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd20966755 = -829812419;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd64864889 = -110334119;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd80137786 = -113841873;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd73395108 = -375805010;    float vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd18271324 = 68248824;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd94101960 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd32217228;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd32217228 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd21144330;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd21144330 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd95686912;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd95686912 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd26642490;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd26642490 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd97749880;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd97749880 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd62518899;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd62518899 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd99975331;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd99975331 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd74256742;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd74256742 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd54816551;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd54816551 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd50235894;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd50235894 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd83452461;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd83452461 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd66802783;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd66802783 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd92039684;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd92039684 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd55328823;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd55328823 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd47791018;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd47791018 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd3470357;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd3470357 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd16641969;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd16641969 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd14157043;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd14157043 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd54818942;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd54818942 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd41439486;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd41439486 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd27412238;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd27412238 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd14618179;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd14618179 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd46354405;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd46354405 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd21715117;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd21715117 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd19308825;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd19308825 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd7509212;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd7509212 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd3167483;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd3167483 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd9566543;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd9566543 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd20748749;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd20748749 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd66559625;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd66559625 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd64884536;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd64884536 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd31261907;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd31261907 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd33810999;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd33810999 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd81621804;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd81621804 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd10632283;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd10632283 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd28369124;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd28369124 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd47144603;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd47144603 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd57185037;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd57185037 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd37791118;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd37791118 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd80318171;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd80318171 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd55273678;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd55273678 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd33645779;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd33645779 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd61652034;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd61652034 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd3622682;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd3622682 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd49304085;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd49304085 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd45329372;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd45329372 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd50306876;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd50306876 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd77659389;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd77659389 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd32728219;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd32728219 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd68113098;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd68113098 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd93202483;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd93202483 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd55184382;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd55184382 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd17745879;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd17745879 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd90232843;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd90232843 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd66689722;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd66689722 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd17599049;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd17599049 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd74789924;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd74789924 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd73971796;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd73971796 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd7333666;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd7333666 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd90240669;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd90240669 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd59351417;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd59351417 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd90408789;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd90408789 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd53507993;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd53507993 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd88256925;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd88256925 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd85351357;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd85351357 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd52190555;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd52190555 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd32991785;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd32991785 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd10417880;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd10417880 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd44696540;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd44696540 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd19421894;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd19421894 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd56325754;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd56325754 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd59456932;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd59456932 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd76365925;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd76365925 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd74500771;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd74500771 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd86165807;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd86165807 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd93766459;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd93766459 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd52966145;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd52966145 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd42731724;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd42731724 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd72411031;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd72411031 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd73979452;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd73979452 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd57202335;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd57202335 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd25508093;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd25508093 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd76838323;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd76838323 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd52635651;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd52635651 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd73357142;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd73357142 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd9700154;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd9700154 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd13516029;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd13516029 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd43578155;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd43578155 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd14932082;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd14932082 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd93033234;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd93033234 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd53579199;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd53579199 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd73172807;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd73172807 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd49851372;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd49851372 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd47550448;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd47550448 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd20966755;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd20966755 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd64864889;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd64864889 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd80137786;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd80137786 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd73395108;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd73395108 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd18271324;     vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd18271324 = vWzTIHmCZNlWmstXyiOOYhjBTiBxujlvgBToYONQWKSjRGiVDsfbrCnOmxzaFoHpcpplZFzkVysNOzjNAfuBEJEd94101960;}
// Junk Finished

// Junk Code By Peatreat & Thaisen's Gen
void FWRTCHYrbMnAZzfCOfxDCUxBhIJIKdFNSQmUJGYOqqhgsoZRmIIfMlLlYPltxQhyHZhJNLsIGjM64401906() {     float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK38028851 = -51276155;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK66592683 = -312456345;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK54119875 = -201874268;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK252976 = -397788687;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK15931114 = -965035306;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK41218913 = -267958141;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK2127646 = -634769771;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK68086204 = -930566566;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK17623767 = -445094965;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK92623181 = -81960859;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK75381226 = -79792857;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK35676761 = -569183120;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK49790288 = -113769134;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK63796761 = -871801168;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK66583154 = -337105398;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK41875063 = -940961908;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK68555621 = -795100425;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK13418132 = -409177600;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK41753694 = -617658784;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK65741547 = -599287188;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK27612467 = -861698464;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK55922815 = -933090105;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK80471947 = -538531404;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK65520732 = -296800948;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK381241 = -108828664;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK97824102 = -438455760;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK40608827 = -904316893;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK59492946 = -35238419;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK34858192 = -905577188;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK44052955 = -385595044;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK15452926 = -864939605;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK71992120 = -839953520;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK9760439 = -3296163;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK72314018 = -406728466;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK90987805 = -978333593;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK85844157 = 48738858;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK89712365 = -750452884;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK29864446 = -238129193;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK71909772 = -160278958;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK66889944 = -205104144;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK26260626 = -917070229;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK74281130 = -70454800;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK57471973 = -236735386;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK33516382 = -427268777;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK57488031 = -722507682;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK12163945 = -333859249;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK39980238 = -773985425;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK45365841 = -798279011;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK35485480 = 28935769;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK55341307 = -329959654;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK5240436 = -285014392;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK74711107 = -39238100;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK81175880 = -459428442;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK89926834 = -780905429;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK36789440 = -543213363;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK82106036 = -118186050;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK86120736 = -773924942;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK88599143 = -905073320;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK99871735 = -188960023;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK18107011 = -426579546;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK610086 = -363641248;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK42634700 = -499531353;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK33228013 = 75010622;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK73570812 = 40500078;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK77170255 = -217021254;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK3389107 = -239839337;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK25916322 = -465886957;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK77476269 = -707040668;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK72808956 = -893467576;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK80738997 = -285844257;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK52162698 = -90509025;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK38691176 = -456971233;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK41508360 = -148898642;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK74863750 = -312554640;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK39480922 = -682216960;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK53331336 = -691243665;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK98450841 = -596354719;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK46955566 = -11262627;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK8032701 = -574293267;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK88217295 = -774969415;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK57843865 = -664470336;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK95242986 = -6037883;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK24007466 = 35825811;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK79516885 = -475617535;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK38812520 = -580652;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK40741818 = -725701506;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK90816239 = -280525079;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK19833605 = -222390735;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK35524579 = -863515104;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK8881770 = -760147543;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK99723421 = -177336201;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK1113222 = -845379564;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK29992711 = 50830830;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK53802761 = -733699412;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK66279858 = -841462896;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK83625925 = -317538876;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK41053118 = -45465423;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK83901161 = -177235465;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK56346126 = -110247523;    float MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK54098925 = -51276155;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK38028851 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK66592683;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK66592683 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK54119875;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK54119875 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK252976;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK252976 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK15931114;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK15931114 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK41218913;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK41218913 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK2127646;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK2127646 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK68086204;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK68086204 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK17623767;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK17623767 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK92623181;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK92623181 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK75381226;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK75381226 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK35676761;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK35676761 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK49790288;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK49790288 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK63796761;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK63796761 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK66583154;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK66583154 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK41875063;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK41875063 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK68555621;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK68555621 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK13418132;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK13418132 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK41753694;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK41753694 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK65741547;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK65741547 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK27612467;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK27612467 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK55922815;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK55922815 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK80471947;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK80471947 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK65520732;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK65520732 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK381241;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK381241 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK97824102;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK97824102 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK40608827;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK40608827 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK59492946;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK59492946 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK34858192;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK34858192 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK44052955;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK44052955 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK15452926;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK15452926 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK71992120;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK71992120 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK9760439;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK9760439 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK72314018;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK72314018 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK90987805;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK90987805 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK85844157;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK85844157 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK89712365;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK89712365 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK29864446;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK29864446 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK71909772;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK71909772 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK66889944;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK66889944 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK26260626;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK26260626 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK74281130;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK74281130 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK57471973;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK57471973 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK33516382;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK33516382 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK57488031;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK57488031 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK12163945;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK12163945 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK39980238;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK39980238 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK45365841;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK45365841 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK35485480;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK35485480 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK55341307;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK55341307 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK5240436;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK5240436 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK74711107;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK74711107 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK81175880;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK81175880 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK89926834;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK89926834 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK36789440;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK36789440 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK82106036;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK82106036 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK86120736;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK86120736 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK88599143;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK88599143 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK99871735;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK99871735 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK18107011;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK18107011 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK610086;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK610086 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK42634700;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK42634700 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK33228013;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK33228013 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK73570812;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK73570812 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK77170255;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK77170255 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK3389107;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK3389107 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK25916322;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK25916322 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK77476269;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK77476269 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK72808956;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK72808956 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK80738997;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK80738997 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK52162698;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK52162698 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK38691176;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK38691176 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK41508360;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK41508360 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK74863750;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK74863750 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK39480922;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK39480922 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK53331336;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK53331336 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK98450841;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK98450841 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK46955566;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK46955566 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK8032701;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK8032701 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK88217295;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK88217295 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK57843865;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK57843865 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK95242986;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK95242986 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK24007466;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK24007466 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK79516885;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK79516885 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK38812520;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK38812520 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK40741818;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK40741818 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK90816239;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK90816239 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK19833605;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK19833605 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK35524579;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK35524579 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK8881770;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK8881770 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK99723421;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK99723421 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK1113222;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK1113222 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK29992711;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK29992711 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK53802761;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK53802761 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK66279858;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK66279858 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK83625925;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK83625925 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK41053118;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK41053118 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK83901161;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK83901161 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK56346126;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK56346126 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK54098925;     MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK54098925 = MnRIsjsVBUaAgEBvuUeQaJiZSGQEaQAYdZzyHHtKFvfdgZBFLPyobSmDEsqmToukUBXGFDonbUKedFstFKsLRMwK38028851;}
// Junk Finished

// Junk Code By Peatreat & Thaisen's Gen
void bgiCpkWhDhHJeocQsqmuppbOJoaYrdjMWUjaAKJuDHpZbmPvAeuITYplEUtAJyqaxumKFBQigrr90488395() {     float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU37165328 = -33303562;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU78703930 = 29972145;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU82839892 = -906041334;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU37505313 = -246705729;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU30971484 = 34785509;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU41935292 = -292942340;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU78793253 = -808107165;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU57964486 = -47386165;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU73514940 = -431609208;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU38501427 = -465193634;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU76244353 = -967172990;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU11824355 = -786533496;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU78393715 = -46890024;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU34934885 = -568235858;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU17297205 = -692933836;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU83858651 = -763999532;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU73626081 = 43045803;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU5742794 = -606823995;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU87857757 = -419664748;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU99815993 = -287264220;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU11192794 = -848751490;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU2543620 = 92952910;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU31935061 = -205846153;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU40812755 = -318261267;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU29073595 = 40429931;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU91168584 = -583907537;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU5083557 = -760142502;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU92504321 = -21646558;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU78060837 = -404786174;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU89267477 = -808050970;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU59296313 = -669290085;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU25826966 = -505234138;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU39812688 = -978047285;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU92746756 = -775971964;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU53790173 = -105711674;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU84740767 = -799206412;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU44029601 = -924590047;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU80126286 = -747161819;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU78302235 = -449021372;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU85299572 = -260670817;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU84327507 = 66627638;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU80515953 = -449271086;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU70906184 = -769215368;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU4572237 = -286843666;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU85431978 = -821973913;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU59893161 = -795775628;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU79280790 = -831082882;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU89686409 = -643123898;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU1037735 = -883573470;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU33900461 = -703074258;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU57777764 = -964621483;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU44885361 = -286440669;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU89159443 = -627089327;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU67160376 = -333697434;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU79295602 = -766088009;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU34621709 = -26256472;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU46768870 = -764181702;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU42027138 = -487780067;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU8431719 = -187135660;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU39802900 = -381306954;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU36851735 = -532799838;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU86288932 = -686460607;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU79903649 = -642599992;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU84247463 = -623558239;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU79205114 = -795903549;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU50417387 = -361938852;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU72011667 = -808486212;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU85646959 = -270918061;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU81144712 = -362524184;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU32556437 = -893727424;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU39829051 = -839409486;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU93499794 = -209792378;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU27440558 = -57802623;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU2558186 = -58993931;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU15488487 = -253891858;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU30676841 = -299480405;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU31637436 = -137831722;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU27362824 = -919002488;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU55380776 = -496287354;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU69180433 = -163794441;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU11887794 = -752824656;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU15397148 = -17018605;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU91466586 = -138073089;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU44160376 = -701711916;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU31489713 = -843429487;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU14410952 = -282849416;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU36667523 = -878144812;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU72652311 = -544349852;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU13451155 = 90116044;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU19168464 = 20544797;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU37971898 = 64975289;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU2002464 = -336809980;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU71694568 = -460026159;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU38499336 = 32285581;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU48447837 = -727870979;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU98038574 = -246911756;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU612304 = -806671095;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU86658720 = -45657130;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU25367123 = -490940117;    float iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU35014591 = -33303562;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU37165328 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU78703930;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU78703930 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU82839892;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU82839892 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU37505313;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU37505313 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU30971484;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU30971484 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU41935292;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU41935292 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU78793253;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU78793253 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU57964486;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU57964486 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU73514940;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU73514940 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU38501427;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU38501427 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU76244353;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU76244353 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU11824355;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU11824355 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU78393715;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU78393715 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU34934885;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU34934885 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU17297205;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU17297205 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU83858651;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU83858651 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU73626081;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU73626081 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU5742794;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU5742794 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU87857757;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU87857757 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU99815993;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU99815993 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU11192794;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU11192794 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU2543620;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU2543620 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU31935061;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU31935061 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU40812755;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU40812755 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU29073595;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU29073595 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU91168584;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU91168584 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU5083557;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU5083557 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU92504321;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU92504321 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU78060837;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU78060837 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU89267477;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU89267477 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU59296313;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU59296313 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU25826966;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU25826966 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU39812688;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU39812688 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU92746756;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU92746756 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU53790173;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU53790173 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU84740767;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU84740767 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU44029601;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU44029601 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU80126286;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU80126286 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU78302235;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU78302235 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU85299572;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU85299572 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU84327507;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU84327507 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU80515953;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU80515953 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU70906184;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU70906184 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU4572237;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU4572237 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU85431978;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU85431978 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU59893161;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU59893161 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU79280790;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU79280790 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU89686409;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU89686409 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU1037735;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU1037735 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU33900461;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU33900461 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU57777764;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU57777764 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU44885361;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU44885361 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU89159443;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU89159443 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU67160376;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU67160376 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU79295602;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU79295602 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU34621709;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU34621709 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU46768870;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU46768870 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU42027138;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU42027138 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU8431719;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU8431719 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU39802900;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU39802900 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU36851735;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU36851735 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU86288932;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU86288932 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU79903649;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU79903649 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU84247463;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU84247463 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU79205114;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU79205114 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU50417387;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU50417387 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU72011667;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU72011667 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU85646959;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU85646959 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU81144712;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU81144712 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU32556437;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU32556437 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU39829051;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU39829051 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU93499794;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU93499794 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU27440558;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU27440558 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU2558186;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU2558186 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU15488487;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU15488487 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU30676841;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU30676841 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU31637436;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU31637436 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU27362824;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU27362824 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU55380776;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU55380776 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU69180433;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU69180433 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU11887794;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU11887794 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU15397148;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU15397148 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU91466586;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU91466586 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU44160376;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU44160376 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU31489713;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU31489713 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU14410952;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU14410952 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU36667523;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU36667523 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU72652311;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU72652311 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU13451155;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU13451155 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU19168464;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU19168464 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU37971898;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU37971898 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU2002464;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU2002464 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU71694568;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU71694568 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU38499336;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU38499336 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU48447837;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU48447837 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU98038574;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU98038574 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU612304;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU612304 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU86658720;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU86658720 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU25367123;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU25367123 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU35014591;     iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU35014591 = iACmXYeXMibsyexWgRIkERPbhAFqyOMbbhHlCiVcebdrWBngHfkQXGmRkaFXToYyWARQWMTxvgGdnYFfMZiTevDU37165328;}
// Junk Finished

// Junk Code By Peatreat & Thaisen's Gen
void PWoeRxkIiAJoFtNNdqPSlIqLvXFJJxEzcPhQtgOKdgQOwrOlBeXWHbxHgdiusaylvJMqMwUwcEo8611009() {     float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ10154868 = 62152835;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ2252179 = -884879824;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ44982120 = 13253755;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ52864249 = -926406106;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ34209876 = -170487647;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ17374877 = -464163911;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ2729196 = -238235937;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ9454390 = -669019430;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ62067247 = -716675751;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ36421175 = -272938513;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ85988032 = -23522502;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ5787389 = -544872097;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ80470670 = -674249781;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ61942896 = -439937768;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ47678831 = -378224097;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ34553737 = -595146853;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ94597753 = -729225788;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ72000172 = -109635445;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ75638558 = -167739253;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ39285451 = 66599748;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ10200322 = -259462941;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ59099944 = -522260055;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ67373259 = -629536640;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ3343516 = -386518353;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ77031884 = -868806252;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ98009750 = 5902887;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ28006769 = -668972673;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ73583509 = -649902751;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ48051398 = -317354950;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ29774471 = -839799975;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ71261187 = -825858511;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ11460085 = 75374483;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ50232468 = -67265550;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ72601435 = 47577600;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ78613380 = -688059062;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ85315523 = -238694697;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ42373312 = -335057722;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ70168362 = 76234434;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ38186638 = -944444004;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ1474013 = -912876300;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ26933749 = 88396051;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ77886134 = -151710212;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ96815701 = -49960073;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ73869603 = -910963254;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ91318567 = -573334478;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ52114269 = -900325269;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ18334707 = -217668434;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ75443493 = -522366258;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ69053661 = -499924277;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ67590639 = -171688193;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ73141763 = -195105517;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ3064813 = -609049983;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ4253689 = -22508568;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ38108452 = -870793982;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ98410686 = -455840231;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ51054924 = -415587111;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ34878920 = -155343184;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ41638604 = -600227892;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ75832364 = 42400146;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ36200126 = -76390535;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ89368107 = -795191239;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ29145686 = -588333186;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ61402991 = -251664480;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ32292776 = -876875777;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ65159987 = -447080003;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ74527947 = 1103015;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ55554920 = -377606548;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ7869236 = -621827382;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ83329515 = -751878707;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ62363307 = -39529400;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ92180424 = -160089131;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ24429391 = -705460222;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ33813534 = -165191441;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ74164546 = -254862954;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ12351702 = 78203696;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ32314188 = -7752729;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ62284242 = -372299982;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ93503656 = -718573386;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ12024948 = -813183876;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ24917616 = -968480984;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ79675043 = -776428679;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ52563276 = -46606415;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ4529848 = -49978475;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ80460759 = -45666758;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ56632708 = -544694458;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ68196374 = -116808528;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ7206397 = -902116950;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ12124016 = -196471569;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ74190748 = -496582169;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ27558457 = -172471951;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ50436603 = 16648487;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ734708 = -734829830;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ94335997 = -966165713;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ1986512 = -768053470;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ12105905 = -17685061;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ97788063 = -323270763;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ16483143 = -900045733;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ64522925 = -173084297;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ8709616 = -363883252;    float FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ16790621 = 62152835;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ10154868 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ2252179;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ2252179 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ44982120;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ44982120 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ52864249;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ52864249 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ34209876;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ34209876 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ17374877;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ17374877 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ2729196;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ2729196 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ9454390;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ9454390 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ62067247;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ62067247 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ36421175;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ36421175 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ85988032;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ85988032 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ5787389;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ5787389 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ80470670;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ80470670 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ61942896;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ61942896 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ47678831;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ47678831 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ34553737;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ34553737 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ94597753;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ94597753 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ72000172;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ72000172 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ75638558;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ75638558 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ39285451;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ39285451 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ10200322;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ10200322 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ59099944;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ59099944 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ67373259;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ67373259 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ3343516;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ3343516 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ77031884;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ77031884 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ98009750;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ98009750 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ28006769;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ28006769 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ73583509;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ73583509 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ48051398;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ48051398 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ29774471;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ29774471 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ71261187;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ71261187 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ11460085;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ11460085 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ50232468;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ50232468 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ72601435;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ72601435 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ78613380;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ78613380 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ85315523;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ85315523 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ42373312;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ42373312 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ70168362;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ70168362 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ38186638;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ38186638 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ1474013;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ1474013 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ26933749;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ26933749 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ77886134;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ77886134 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ96815701;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ96815701 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ73869603;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ73869603 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ91318567;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ91318567 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ52114269;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ52114269 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ18334707;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ18334707 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ75443493;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ75443493 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ69053661;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ69053661 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ67590639;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ67590639 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ73141763;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ73141763 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ3064813;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ3064813 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ4253689;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ4253689 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ38108452;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ38108452 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ98410686;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ98410686 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ51054924;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ51054924 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ34878920;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ34878920 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ41638604;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ41638604 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ75832364;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ75832364 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ36200126;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ36200126 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ89368107;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ89368107 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ29145686;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ29145686 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ61402991;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ61402991 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ32292776;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ32292776 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ65159987;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ65159987 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ74527947;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ74527947 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ55554920;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ55554920 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ7869236;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ7869236 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ83329515;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ83329515 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ62363307;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ62363307 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ92180424;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ92180424 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ24429391;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ24429391 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ33813534;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ33813534 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ74164546;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ74164546 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ12351702;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ12351702 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ32314188;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ32314188 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ62284242;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ62284242 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ93503656;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ93503656 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ12024948;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ12024948 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ24917616;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ24917616 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ79675043;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ79675043 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ52563276;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ52563276 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ4529848;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ4529848 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ80460759;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ80460759 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ56632708;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ56632708 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ68196374;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ68196374 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ7206397;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ7206397 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ12124016;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ12124016 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ74190748;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ74190748 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ27558457;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ27558457 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ50436603;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ50436603 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ734708;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ734708 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ94335997;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ94335997 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ1986512;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ1986512 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ12105905;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ12105905 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ97788063;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ97788063 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ16483143;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ16483143 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ64522925;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ64522925 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ8709616;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ8709616 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ16790621;     FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ16790621 = FsbPDITHfaisARakPyTSCheTmfYZEJatCnsOEOelYTSLMQLZmLEyImlXNHcYktCVzpqNYLgBThtjOSGWTyntEclQ10154868;}
// Junk Finished

// Junk Code By Peatreat & Thaisen's Gen
void HMETIznBnQkDlatCUiKwmSmrSAiiTddvAbHBDffRsBzbxCUASBNkdmaoZNAHQEhJnoCKSryoCsD64026086() {     float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt53995414 = -646856232;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt5210171 = 62443617;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt17995007 = -32472837;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt102253 = -88487944;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt12344507 = -139502645;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt53953702 = -375874410;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt15212054 = -951916539;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt36360353 = 36144791;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt34361365 = -630795460;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt51079683 = -194439853;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt57760364 = -278568639;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt99174316 = -847805031;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt2364684 = -312223226;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt70259268 = -32746097;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt27181048 = -814973008;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt26814174 = -142580608;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt73191884 = 29181162;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt82804830 = -342375645;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt46047243 = -214040720;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt55152296 = -745784358;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt31112876 = -337676677;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt41261058 = -160518031;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt16350061 = -615319816;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt43280357 = -492122999;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt31561271 = -720718516;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt72160427 = -397153734;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt32905637 = -664119692;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt93184972 = -453954122;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt79127609 = -9427346;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt43620975 = -171320819;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt89235518 = -499711113;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt49634876 = -518434240;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt80325833 = 50967616;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt50153484 = -18534824;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt68411494 = -466446876;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt45325966 = -705445735;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt23362309 = -393417842;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt5599146 = -498395323;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt36674188 = -788561530;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt38521624 = -311940576;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt50922453 = -622013291;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt71432406 = -409087751;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt94111371 = -901353190;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt41325858 = -515917138;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt31090869 = -8982844;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt50952463 = -531070551;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt48796983 = -52652527;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt89804789 = -459614095;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt64719814 = -123635801;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt81678897 = -565210241;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt24331428 = -611332072;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt50501867 = -401428075;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt1720600 = -648878489;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt37793892 = -379917021;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt32913211 = -309704130;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt12734356 = -386338201;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt88860109 = -322236567;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt74714649 = -540349839;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt68540981 = -367769428;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt40184080 = -742348911;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt21048065 = -711754718;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt22027082 = -397962418;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt57232744 = -954427864;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt90740390 = -359474642;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt61844165 = -694728741;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt8125488 = -760134399;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt18848484 = -798772647;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt52211199 = -193688403;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt1847775 = -566299222;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt81855082 = -9527273;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt3451865 = -749162766;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt67592738 = -472423515;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt46130643 = -553814116;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt7525619 = -902100145;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt4229843 = -23771068;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt59680470 = -928588926;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt47149687 = -259164841;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt75024203 = 597322;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt12189489 = -383140156;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt80608808 = -89647966;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt23363444 = -244501207;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt43100848 = -104505597;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt28465158 = -230318321;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt97448711 = -444217105;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt19289547 = -559988747;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt38733652 = 1716962;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt47914276 = -869555751;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt42531941 = -569115364;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt17240274 = -708830694;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt55677138 = 19891325;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt56465856 = -283209168;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt48647659 = -853068004;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt37058165 = -30625896;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt96490108 = 53787381;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt17473560 = -600185858;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt28895372 = -124050873;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt14199662 = -454659888;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt3370981 = -441878549;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt79481693 = -821188398;    float kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt22965382 = -646856232;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt53995414 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt5210171;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt5210171 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt17995007;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt17995007 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt102253;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt102253 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt12344507;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt12344507 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt53953702;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt53953702 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt15212054;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt15212054 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt36360353;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt36360353 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt34361365;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt34361365 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt51079683;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt51079683 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt57760364;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt57760364 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt99174316;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt99174316 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt2364684;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt2364684 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt70259268;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt70259268 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt27181048;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt27181048 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt26814174;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt26814174 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt73191884;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt73191884 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt82804830;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt82804830 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt46047243;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt46047243 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt55152296;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt55152296 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt31112876;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt31112876 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt41261058;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt41261058 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt16350061;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt16350061 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt43280357;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt43280357 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt31561271;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt31561271 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt72160427;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt72160427 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt32905637;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt32905637 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt93184972;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt93184972 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt79127609;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt79127609 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt43620975;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt43620975 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt89235518;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt89235518 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt49634876;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt49634876 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt80325833;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt80325833 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt50153484;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt50153484 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt68411494;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt68411494 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt45325966;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt45325966 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt23362309;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt23362309 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt5599146;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt5599146 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt36674188;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt36674188 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt38521624;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt38521624 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt50922453;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt50922453 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt71432406;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt71432406 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt94111371;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt94111371 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt41325858;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt41325858 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt31090869;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt31090869 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt50952463;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt50952463 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt48796983;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt48796983 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt89804789;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt89804789 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt64719814;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt64719814 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt81678897;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt81678897 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt24331428;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt24331428 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt50501867;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt50501867 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt1720600;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt1720600 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt37793892;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt37793892 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt32913211;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt32913211 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt12734356;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt12734356 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt88860109;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt88860109 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt74714649;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt74714649 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt68540981;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt68540981 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt40184080;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt40184080 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt21048065;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt21048065 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt22027082;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt22027082 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt57232744;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt57232744 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt90740390;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt90740390 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt61844165;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt61844165 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt8125488;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt8125488 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt18848484;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt18848484 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt52211199;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt52211199 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt1847775;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt1847775 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt81855082;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt81855082 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt3451865;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt3451865 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt67592738;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt67592738 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt46130643;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt46130643 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt7525619;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt7525619 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt4229843;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt4229843 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt59680470;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt59680470 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt47149687;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt47149687 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt75024203;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt75024203 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt12189489;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt12189489 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt80608808;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt80608808 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt23363444;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt23363444 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt43100848;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt43100848 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt28465158;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt28465158 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt97448711;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt97448711 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt19289547;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt19289547 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt38733652;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt38733652 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt47914276;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt47914276 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt42531941;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt42531941 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt17240274;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt17240274 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt55677138;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt55677138 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt56465856;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt56465856 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt48647659;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt48647659 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt37058165;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt37058165 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt96490108;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt96490108 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt17473560;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt17473560 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt28895372;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt28895372 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt14199662;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt14199662 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt3370981;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt3370981 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt79481693;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt79481693 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt22965382;     kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt22965382 = kTKtQgiPSQsSOASbqfXAoUpnJAKhbCZhliqVPUkbioPtuWoJAPIJerPQdXjPVBoPrLgfKwMVuwSAFvsnLAgMtPCt53995414;}
// Junk Finished

// Junk Code By Peatreat & Thaisen's Gen
void voJXPOuRXIzIBdaKUONWHwYlTeQjzffxGySSVkGBINudDRmjfEKlDvryQYElxLRrdCCCLIpkHMN2889405() {     float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri79882868 = -733376046;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri10442085 = -393507567;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri71337460 = -386870971;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri63330275 = -649779288;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri42040968 = -181715182;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri31801985 = -640376228;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri34250697 = -725469904;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri73179545 = -550853433;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri78295539 = -516216389;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri51912711 = -483962351;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri16864607 = -416836486;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri91993636 = -547890899;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri34600130 = -929333068;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri81459707 = -461618456;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri30580844 = -808473959;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri72884301 = -148789402;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri19546263 = -812342920;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri80727605 = -461657838;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri16866673 = -78454957;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri76766464 = -266097855;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri77473067 = -942678826;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri26113382 = 48832283;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri6763366 = 65870737;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri6452089 = -550820648;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri71295060 = -418968738;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri41402876 = -817111582;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri22210280 = -542984490;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri44445392 = 63925799;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri5470727 = -977070619;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri30870372 = -77078960;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri24278318 = -480322610;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri3582413 = -748968681;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri88028555 = -493705079;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri31559564 = -738724535;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri47197354 = -792618135;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri90719269 = -690999463;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri60571726 = -158830873;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri85017306 = -550019434;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri42322585 = -511829971;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri62186944 = -659491971;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri41334929 = -604453918;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri98969243 = -260363610;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri25358761 = -289000007;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri44185474 = -348462156;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri47990984 = -555456978;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri28203622 = -494663046;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri58176242 = -18117031;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri90001356 = -515578540;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri71699821 = -732996617;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri86702103 = -172662970;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri67222026 = -620831225;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri63362302 = -94477494;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri95511234 = 35724473;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri79411864 = -451840258;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri25416337 = -675524486;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri53769487 = -682208330;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri3678720 = -359378305;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri64885371 = -836050323;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri92035215 = -130810551;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri638093 = -364603600;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri9591705 = 2608262;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri89805305 = -689395704;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri67708819 = -573782815;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri47425168 = -339137430;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri27634393 = 96360259;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri13282194 = -667867805;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri3965081 = 45814180;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri3040567 = -90608534;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri34262354 = -669000322;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri39861575 = -17474496;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri12312575 = -989958529;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri34528956 = -162323486;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri38405021 = -949827868;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri54679728 = -418962987;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri35431536 = -661643937;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri78503823 = -582315216;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri754622 = -662167710;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri62577892 = -585667107;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri58461105 = -995363671;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri43091438 = -924305692;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri83226633 = -698994551;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri32208924 = 72594049;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri72745571 = -203077585;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri18768623 = -704407649;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri63648345 = -456247735;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri60916016 = -285845117;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri8071178 = -684693155;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri8616692 = 58135178;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri6143227 = 36799951;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri93427867 = -10409806;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri87040550 = -231621159;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri95686354 = -322780550;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri92982091 = -319208884;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri41684493 = -47226371;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri52595239 = -562100233;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri51529623 = -915058214;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri31260425 = -686580796;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri77933592 = -949862578;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri16551081 = -344822416;    float tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri34708790 = -733376046;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri79882868 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri10442085;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri10442085 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri71337460;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri71337460 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri63330275;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri63330275 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri42040968;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri42040968 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri31801985;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri31801985 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri34250697;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri34250697 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri73179545;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri73179545 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri78295539;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri78295539 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri51912711;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri51912711 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri16864607;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri16864607 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri91993636;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri91993636 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri34600130;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri34600130 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri81459707;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri81459707 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri30580844;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri30580844 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri72884301;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri72884301 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri19546263;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri19546263 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri80727605;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri80727605 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri16866673;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri16866673 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri76766464;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri76766464 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri77473067;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri77473067 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri26113382;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri26113382 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri6763366;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri6763366 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri6452089;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri6452089 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri71295060;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri71295060 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri41402876;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri41402876 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri22210280;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri22210280 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri44445392;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri44445392 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri5470727;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri5470727 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri30870372;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri30870372 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri24278318;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri24278318 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri3582413;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri3582413 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri88028555;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri88028555 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri31559564;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri31559564 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri47197354;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri47197354 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri90719269;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri90719269 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri60571726;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri60571726 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri85017306;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri85017306 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri42322585;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri42322585 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri62186944;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri62186944 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri41334929;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri41334929 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri98969243;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri98969243 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri25358761;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri25358761 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri44185474;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri44185474 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri47990984;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri47990984 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri28203622;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri28203622 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri58176242;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri58176242 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri90001356;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri90001356 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri71699821;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri71699821 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri86702103;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri86702103 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri67222026;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri67222026 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri63362302;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri63362302 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri95511234;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri95511234 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri79411864;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri79411864 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri25416337;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri25416337 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri53769487;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri53769487 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri3678720;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri3678720 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri64885371;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri64885371 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri92035215;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri92035215 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri638093;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri638093 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri9591705;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri9591705 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri89805305;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri89805305 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri67708819;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri67708819 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri47425168;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri47425168 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri27634393;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri27634393 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri13282194;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri13282194 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri3965081;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri3965081 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri3040567;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri3040567 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri34262354;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri34262354 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri39861575;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri39861575 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri12312575;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri12312575 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri34528956;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri34528956 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri38405021;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri38405021 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri54679728;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri54679728 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri35431536;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri35431536 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri78503823;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri78503823 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri754622;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri754622 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri62577892;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri62577892 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri58461105;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri58461105 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri43091438;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri43091438 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri83226633;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri83226633 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri32208924;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri32208924 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri72745571;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri72745571 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri18768623;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri18768623 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri63648345;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri63648345 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri60916016;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri60916016 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri8071178;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri8071178 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri8616692;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri8616692 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri6143227;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri6143227 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri93427867;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri93427867 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri87040550;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri87040550 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri95686354;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri95686354 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri92982091;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri92982091 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri41684493;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri41684493 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri52595239;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri52595239 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri51529623;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri51529623 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri31260425;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri31260425 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri77933592;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri77933592 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri16551081;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri16551081 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri34708790;     tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri34708790 = tCviBshEvVttLHTBxgVDxvCzAPjYWOZerhxvVpCtQGWWkIqvrGrEmIQxzvimIIhgvOCeJXfIdcmqZgOsYohPXVri79882868;}
// Junk Finished

// Junk Code By Peatreat & Thaisen's Gen
void xdFfVbaGIuhjVsgKReZMJUQBiJMUdfPbAidMuiGUAcNFKgdAgLplujicNDxLOGcdfGjnOUQiXEL90807495() {     float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr74980312 = -802967226;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr37611828 = -352632928;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr70071356 = -732447264;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr59340198 = -424805104;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr70013601 = -386964639;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr9289570 = -347957161;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr84501132 = -795983157;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr71310666 = 90213214;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr72321150 = -329455625;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr28036107 = -279783660;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr70677047 = -919641014;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr14853248 = -615453769;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr70749190 = 75760858;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr36336218 = -299636618;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr54372952 = -289567609;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr82410432 = -225555938;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr84227759 = -356359495;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr19735577 = 47334718;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr67392765 = -171657219;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr10012004 = 21838583;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr65664579 = -359940096;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr57630172 = -49143479;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr80144729 = -544394802;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr29046500 = -615663335;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr59767463 = -331086043;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr78896427 = -108738069;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr20085714 = -493610073;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr28568456 = -636298242;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr46516800 = -572205756;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr60819559 = -969367308;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr17264888 = -136866369;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr85069861 = -186429272;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr8901192 = -167924925;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr77236726 = -526807344;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr59373331 = -802096073;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr75514845 = -255510553;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr21205738 = 39245617;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr18882524 = -503575957;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr38894477 = -187637552;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr8853118 = -153868125;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr81579945 = 92627983;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr5995876 = -688594080;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr47585394 = -355928284;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr57636380 = -47184432;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr85818085 = -618103896;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr23864126 = -86021014;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr52364016 = -93771159;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr1285864 = -387955711;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr31148552 = -84882053;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr16935182 = -458302183;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr24621858 = -261520557;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr99388085 = -54112615;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr57649940 = -331420948;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr89390637 = -406321815;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr25142916 = -537740002;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr17350141 = -653823748;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr57467098 = -808238126;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr41024856 = -16783929;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr99572734 = 6280939;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr91117173 = -178226571;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr89203856 = -854347089;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr55932677 = -59684916;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr24793866 = -337581030;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr11501591 = -360088317;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr10771220 = -42917292;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr85607185 = -633211742;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr5952056 = -347528844;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr93512464 = -397431799;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr76962887 = -497540546;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr78858107 = 65942944;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr61204694 = -164801556;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr65345236 = -852783539;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr80841099 = -765027730;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr58539648 = 82210905;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr28432058 = 29210600;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr59668703 = -671346016;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr10044779 = -693215195;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr22508350 = -397210370;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr43228415 = -997559440;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr35903337 = -145065030;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr26532411 = 85033089;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr18799851 = -5654362;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr97419903 = -451416190;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr29581619 = -13903573;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr36197701 = -607846752;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr17876803 = 17246245;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr27419922 = -855008324;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr19510554 = -761603111;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr52093810 = -989067342;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr42023190 = -48272325;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr18047747 = -447272428;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr80180882 = -943970454;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr19309789 = -409856896;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr47777304 = 90589018;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr19649262 = -299521037;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr25647268 = -847687102;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr81202009 = -251013050;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr36083803 = -995839968;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr46865161 = 95732859;    float gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr210901 = -802967226;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr74980312 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr37611828;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr37611828 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr70071356;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr70071356 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr59340198;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr59340198 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr70013601;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr70013601 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr9289570;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr9289570 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr84501132;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr84501132 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr71310666;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr71310666 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr72321150;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr72321150 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr28036107;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr28036107 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr70677047;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr70677047 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr14853248;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr14853248 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr70749190;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr70749190 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr36336218;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr36336218 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr54372952;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr54372952 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr82410432;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr82410432 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr84227759;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr84227759 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr19735577;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr19735577 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr67392765;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr67392765 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr10012004;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr10012004 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr65664579;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr65664579 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr57630172;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr57630172 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr80144729;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr80144729 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr29046500;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr29046500 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr59767463;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr59767463 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr78896427;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr78896427 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr20085714;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr20085714 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr28568456;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr28568456 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr46516800;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr46516800 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr60819559;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr60819559 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr17264888;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr17264888 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr85069861;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr85069861 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr8901192;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr8901192 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr77236726;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr77236726 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr59373331;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr59373331 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr75514845;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr75514845 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr21205738;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr21205738 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr18882524;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr18882524 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr38894477;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr38894477 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr8853118;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr8853118 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr81579945;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr81579945 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr5995876;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr5995876 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr47585394;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr47585394 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr57636380;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr57636380 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr85818085;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr85818085 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr23864126;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr23864126 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr52364016;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr52364016 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr1285864;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr1285864 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr31148552;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr31148552 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr16935182;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr16935182 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr24621858;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr24621858 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr99388085;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr99388085 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr57649940;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr57649940 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr89390637;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr89390637 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr25142916;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr25142916 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr17350141;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr17350141 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr57467098;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr57467098 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr41024856;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr41024856 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr99572734;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr99572734 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr91117173;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr91117173 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr89203856;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr89203856 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr55932677;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr55932677 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr24793866;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr24793866 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr11501591;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr11501591 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr10771220;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr10771220 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr85607185;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr85607185 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr5952056;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr5952056 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr93512464;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr93512464 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr76962887;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr76962887 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr78858107;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr78858107 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr61204694;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr61204694 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr65345236;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr65345236 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr80841099;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr80841099 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr58539648;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr58539648 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr28432058;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr28432058 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr59668703;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr59668703 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr10044779;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr10044779 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr22508350;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr22508350 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr43228415;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr43228415 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr35903337;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr35903337 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr26532411;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr26532411 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr18799851;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr18799851 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr97419903;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr97419903 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr29581619;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr29581619 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr36197701;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr36197701 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr17876803;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr17876803 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr27419922;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr27419922 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr19510554;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr19510554 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr52093810;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr52093810 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr42023190;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr42023190 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr18047747;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr18047747 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr80180882;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr80180882 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr19309789;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr19309789 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr47777304;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr47777304 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr19649262;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr19649262 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr25647268;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr25647268 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr81202009;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr81202009 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr36083803;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr36083803 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr46865161;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr46865161 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr210901;     gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr210901 = gHKqBgMnSpegQzAyrlvGuAkaExplMvGNpYCVDMweBTKnBaSoOVYxcyRgWjmDqbbWjQqswevepQFbpxZdfAWqWIkr74980312;}
// Junk Finished

// Junk Code By Peatreat & Thaisen's Gen
void lzDncsxiiLbCEnulMBmZqahjYNuJYeoelWVhquDBPgoikwCXXYHZPKNKSUOPynKDWwRhRMuSykJ5233002() {     float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw25008132 = -165704448;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw61791354 = -716890328;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw72372115 = -966754939;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw53680693 = -477285839;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw50297969 = 10092303;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw31189385 = -358654351;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw5967185 = -752260141;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw62216557 = -254673586;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw87181080 = -952394612;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw98671181 = -477474578;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw6734183 = -577707535;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw34629782 = -678700933;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw19578098 = -434685760;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw75847522 = -483727389;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw74299983 = -590813640;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw54411239 = -784928191;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw82780926 = -689318020;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw21066225 = -719319701;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw57889933 = -649636531;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw81669180 = -528310275;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw39564314 = -185455194;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw18402437 = 59437193;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw73785281 = -310388223;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw41330471 = -887414356;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw3239961 = -898141730;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw73568573 = 79366383;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw77199062 = -865270871;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw86509856 = -265544243;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw69636822 = -982311684;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw92964077 = 50114165;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw41932435 = -17170090;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw51597711 = -865299415;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw10741731 = -993976407;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw11277403 = -195846306;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw36109079 = -695720104;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw26523107 = -848272684;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw43664333 = -654423318;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw10486779 = -519365536;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw15513458 = -172395526;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw55136114 = -955322079;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw2830793 = 4696622;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw21397938 = -853370026;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw50186966 = -744274018;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw14988214 = 79170443;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw61681031 = 2244763;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw99733101 = -897611700;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw34037938 = -326464659;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw42760722 = -855766112;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw95296345 = -982561677;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw2573479 = -102468114;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw30598298 = -393573873;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw28074573 = -963680068;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw70510766 = -734693881;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw83166151 = -830945582;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw65668688 = -642554660;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw6605695 = -125141642;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw88006073 = -306502106;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw31041644 = 20659416;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw50440733 = -579144110;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw76729395 = 30725919;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw53990323 = -493383481;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw19457328 = -386715899;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw92579734 = -272361902;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw94217003 = -902508778;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw56738746 = -360304488;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw55136472 = -712408120;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw23888052 = -684724527;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw8300695 = -138839454;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw39738443 = -788007285;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw47776876 = -742540956;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw10746906 = -30504873;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw72294148 = -69952484;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw5552768 = -446924175;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw2753820 = -694314452;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw78838388 = -433006898;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw18166377 = -332085169;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw68215470 = -196288789;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw58797067 = -289558666;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw79649440 = -789659120;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw3506859 = 99469970;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw39530635 = -594168959;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw34438340 = 90495241;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw91213511 = -282982566;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw67063343 = -779843571;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw62365779 = -556311962;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw13857862 = -53490023;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw81086944 = -30605534;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw27575579 = -63030826;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw45608715 = -553291647;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw29503385 = -470578463;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw38517034 = -441770579;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw12622690 = -575082735;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw60046045 = -940221427;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw38784062 = -103121446;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw1145791 = -361938599;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw83373464 = -608587479;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw28818203 = -481008124;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw55969963 = -841765241;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw58249468 = -560525070;    float KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw6544559 = -165704448;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw25008132 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw61791354;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw61791354 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw72372115;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw72372115 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw53680693;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw53680693 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw50297969;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw50297969 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw31189385;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw31189385 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw5967185;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw5967185 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw62216557;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw62216557 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw87181080;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw87181080 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw98671181;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw98671181 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw6734183;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw6734183 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw34629782;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw34629782 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw19578098;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw19578098 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw75847522;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw75847522 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw74299983;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw74299983 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw54411239;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw54411239 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw82780926;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw82780926 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw21066225;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw21066225 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw57889933;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw57889933 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw81669180;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw81669180 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw39564314;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw39564314 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw18402437;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw18402437 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw73785281;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw73785281 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw41330471;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw41330471 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw3239961;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw3239961 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw73568573;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw73568573 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw77199062;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw77199062 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw86509856;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw86509856 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw69636822;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw69636822 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw92964077;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw92964077 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw41932435;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw41932435 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw51597711;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw51597711 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw10741731;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw10741731 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw11277403;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw11277403 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw36109079;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw36109079 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw26523107;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw26523107 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw43664333;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw43664333 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw10486779;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw10486779 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw15513458;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw15513458 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw55136114;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw55136114 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw2830793;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw2830793 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw21397938;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw21397938 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw50186966;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw50186966 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw14988214;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw14988214 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw61681031;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw61681031 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw99733101;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw99733101 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw34037938;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw34037938 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw42760722;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw42760722 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw95296345;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw95296345 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw2573479;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw2573479 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw30598298;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw30598298 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw28074573;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw28074573 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw70510766;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw70510766 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw83166151;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw83166151 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw65668688;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw65668688 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw6605695;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw6605695 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw88006073;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw88006073 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw31041644;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw31041644 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw50440733;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw50440733 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw76729395;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw76729395 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw53990323;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw53990323 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw19457328;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw19457328 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw92579734;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw92579734 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw94217003;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw94217003 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw56738746;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw56738746 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw55136472;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw55136472 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw23888052;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw23888052 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw8300695;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw8300695 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw39738443;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw39738443 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw47776876;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw47776876 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw10746906;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw10746906 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw72294148;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw72294148 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw5552768;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw5552768 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw2753820;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw2753820 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw78838388;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw78838388 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw18166377;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw18166377 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw68215470;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw68215470 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw58797067;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw58797067 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw79649440;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw79649440 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw3506859;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw3506859 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw39530635;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw39530635 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw34438340;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw34438340 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw91213511;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw91213511 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw67063343;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw67063343 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw62365779;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw62365779 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw13857862;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw13857862 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw81086944;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw81086944 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw27575579;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw27575579 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw45608715;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw45608715 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw29503385;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw29503385 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw38517034;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw38517034 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw12622690;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw12622690 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw60046045;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw60046045 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw38784062;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw38784062 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw1145791;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw1145791 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw83373464;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw83373464 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw28818203;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw28818203 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw55969963;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw55969963 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw58249468;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw58249468 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw6544559;     KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw6544559 = KmzycKWmzWPPbJKPWfnDcWkLoeVGKepbeADYSVlEcBKhHgqYGxXmKhhoBnKxpPcVhnlaGmwAuiaaMDEHowANGDQw25008132;}
// Junk Finished

// Junk Code By Peatreat & Thaisen's Gen
void FJUoPBAmkEWjbUHBuBsPnGSPTzKyzykLxYkSSvwDGnhFthYKWBHoJvszqFUQrCmlULlDkpjyFoI39184811() {     float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs12235223 = -481580296;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs10987705 = 93347254;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs34874801 = -421242613;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs6510051 = -576781166;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs56961461 = -704833764;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs88091352 = 83847945;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs85456984 = -323072104;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs32134948 = -39441093;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs42801570 = -144279164;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs55526456 = -878893323;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs61025299 = -579900203;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs54612084 = -189441597;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs19932759 = -815081289;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs61884243 = -561997328;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs22557706 = -901177757;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs40757209 = -717199268;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs51250984 = -402634860;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs65589587 = -349174966;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs68190077 = -958606747;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs62837828 = -206270009;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs12546197 = -518203368;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs51293480 = -534605524;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs11641930 = -903032893;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs57320057 = 19108460;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs79814251 = -289816662;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs49213747 = -385561305;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs55895433 = -180024743;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs56756264 = -541167862;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs54666297 = -156858581;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs18559687 = -770238797;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs97342513 = 57983161;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs80435415 = -797048843;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs8804272 = 76452410;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs68878091 = -507664949;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs65089881 = -562768587;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs78670680 = 93055360;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs31226945 = -407403005;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs51701789 = -604876052;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs15812645 = 6163891;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs11868389 = -758580481;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs35325584 = -730634516;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs96917280 = -455866822;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs28542630 = -717229378;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs85983281 = -656472437;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs96576104 = -274470271;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs92936876 = -937984456;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs50449873 = -594524506;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs49408670 = -62936785;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs82361465 = 89891985;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs36363558 = -406403576;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs98080826 = -424491648;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs11315843 = -336890116;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs43645226 = -994085050;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs5305049 = -387937272;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs26638615 = -205948206;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs60941742 = -946974773;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs99345775 = -3619853;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs77554744 = -340351074;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs26695799 = -186964504;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs7747715 = -219272460;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs32195920 = -736127312;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs28700721 = -781904242;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs77468651 = -882582513;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs24241884 = -374040367;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs58183943 = -836876484;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs80589883 = -782851360;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs45807812 = -165894008;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs51054668 = -207416340;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs96794362 = -999228742;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs43887026 = -894233117;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs9530264 = -209796264;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs99549195 = -797758808;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs49776943 = -255338858;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs56321689 = -100026266;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs27512245 = -475635494;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs15628917 = 37663453;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs22750851 = -817376146;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs25658648 = -146560457;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs60743952 = -706421270;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs86877374 = -351832207;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs98763874 = -791036799;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs6486763 = -17087958;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs74394798 = -531059848;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs18302739 = -750455005;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs20478861 = -245747150;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs86026671 = -605126724;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs36790190 = -802963793;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs3499224 = -535610319;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs42239477 = -201716743;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs4148139 = -615793814;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs79324904 = -903324787;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs53672201 = 32948068;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs25005990 = -317911549;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs8064930 = -774563650;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs79672469 = 77546830;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs6624864 = -948730274;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs19448629 = -573284310;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs4300746 = -243189012;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs27799338 = -819595954;    float zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs15986221 = -481580296;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs12235223 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs10987705;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs10987705 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs34874801;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs34874801 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs6510051;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs6510051 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs56961461;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs56961461 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs88091352;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs88091352 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs85456984;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs85456984 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs32134948;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs32134948 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs42801570;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs42801570 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs55526456;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs55526456 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs61025299;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs61025299 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs54612084;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs54612084 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs19932759;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs19932759 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs61884243;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs61884243 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs22557706;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs22557706 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs40757209;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs40757209 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs51250984;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs51250984 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs65589587;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs65589587 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs68190077;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs68190077 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs62837828;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs62837828 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs12546197;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs12546197 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs51293480;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs51293480 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs11641930;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs11641930 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs57320057;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs57320057 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs79814251;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs79814251 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs49213747;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs49213747 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs55895433;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs55895433 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs56756264;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs56756264 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs54666297;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs54666297 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs18559687;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs18559687 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs97342513;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs97342513 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs80435415;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs80435415 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs8804272;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs8804272 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs68878091;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs68878091 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs65089881;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs65089881 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs78670680;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs78670680 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs31226945;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs31226945 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs51701789;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs51701789 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs15812645;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs15812645 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs11868389;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs11868389 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs35325584;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs35325584 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs96917280;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs96917280 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs28542630;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs28542630 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs85983281;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs85983281 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs96576104;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs96576104 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs92936876;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs92936876 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs50449873;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs50449873 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs49408670;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs49408670 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs82361465;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs82361465 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs36363558;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs36363558 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs98080826;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs98080826 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs11315843;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs11315843 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs43645226;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs43645226 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs5305049;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs5305049 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs26638615;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs26638615 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs60941742;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs60941742 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs99345775;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs99345775 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs77554744;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs77554744 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs26695799;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs26695799 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs7747715;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs7747715 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs32195920;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs32195920 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs28700721;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs28700721 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs77468651;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs77468651 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs24241884;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs24241884 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs58183943;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs58183943 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs80589883;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs80589883 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs45807812;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs45807812 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs51054668;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs51054668 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs96794362;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs96794362 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs43887026;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs43887026 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs9530264;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs9530264 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs99549195;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs99549195 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs49776943;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs49776943 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs56321689;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs56321689 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs27512245;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs27512245 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs15628917;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs15628917 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs22750851;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs22750851 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs25658648;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs25658648 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs60743952;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs60743952 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs86877374;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs86877374 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs98763874;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs98763874 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs6486763;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs6486763 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs74394798;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs74394798 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs18302739;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs18302739 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs20478861;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs20478861 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs86026671;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs86026671 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs36790190;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs36790190 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs3499224;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs3499224 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs42239477;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs42239477 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs4148139;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs4148139 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs79324904;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs79324904 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs53672201;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs53672201 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs25005990;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs25005990 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs8064930;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs8064930 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs79672469;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs79672469 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs6624864;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs6624864 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs19448629;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs19448629 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs4300746;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs4300746 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs27799338;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs27799338 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs15986221;     zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs15986221 = zYMVmazbyqwBCglbMbgMEBvkhvVahSJiUnsaejipranvYJjWzNWXYcEycboqFZughNhPnKxdsrVeNAMXMyaTwlKs12235223;}
// Junk Finished

// Junk Code By Peatreat & Thaisen's Gen
void CPtQFwsZoeOsLKHYrmXSbECqcIMjdMgAufbkgKsMLFGClpDYStWfePCxMDnxkJvcxNxgiWrhuKH19919153() {     float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll89306173 = -773118029;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll31869273 = -788369234;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll51349182 = -466306593;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll91022210 = -411636636;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll7661729 = 95819768;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll30347323 = 8249744;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll78042645 = -752094949;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll60546288 = -879448441;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll71914002 = -494856043;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll1397222 = -363925125;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll8155439 = -320079679;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll89483361 = -205988554;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll41623933 = -917370912;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll48515675 = -249612171;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll48151060 = -746344919;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll53140328 = -930199715;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll85496076 = -981569390;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll59743105 = -706868638;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll21082409 = -53004165;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll62627404 = -651413354;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll1586071 = -425672332;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll82559486 = -144737302;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll35826036 = -492110867;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll36619657 = 4553896;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll72468939 = -469671462;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll44286316 = -118008379;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll69532730 = -788916385;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll65007234 = -364678430;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll71406945 = -304948924;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll60274326 = -42953315;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll9653225 = -905735930;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll39315102 = -791531428;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll87976252 = -959974127;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll56892638 = -486781138;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll81969640 = -673183819;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll47991305 = -768156308;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll43844398 = -934533168;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll42429964 = -363404455;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll32681006 = -308418363;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll9355271 = -627209143;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll44329286 = -587307726;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll41077976 = -937635525;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll37623737 = -143828450;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll11965659 = -111919766;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll29401867 = -852467837;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll22132744 = -122582685;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll3970456 = -762365486;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll15063618 = -387744210;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll53647333 = -491293561;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll8915909 = -535852838;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll34725180 = -674197991;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll49598922 = -721806397;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll22686372 = -561171168;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll64220296 = -26148369;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll18333083 = -908150841;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll6746688 = -528380728;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll96043236 = -196258368;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll14729526 = -370860489;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll18553271 = -941965175;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll63375413 = -786171853;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll60814593 = -202833872;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll13035411 = -287416520;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll89139342 = -474499517;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll11639677 = -351902728;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll91743997 = -458189196;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll68840337 = -528548251;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll1507110 = -246014427;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll84731295 = -330589775;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll66546035 = -576428352;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll159755 = -978188612;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll9295930 = -995666547;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll43066112 = -518164936;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll27062099 = -298450276;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll11727139 = -425795023;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll18298119 = 35894371;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll60508094 = -488036808;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll44935749 = 99091148;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll23860378 = -280191101;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll7217790 = -142978268;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll50336196 = -247088778;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll40315860 = -355642893;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll54469113 = -301172175;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll11359901 = -873384869;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll62491037 = -769096087;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll25549146 = -368755324;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll60054302 = -83929533;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll16628731 = -130360261;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll23755956 = -833825758;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll38559556 = -578630297;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll75222953 = -44803092;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll51948069 = -471897940;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll29114873 = -463672680;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll23876693 = -421439281;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll69305593 = -522246510;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll48540677 = -324375271;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll31293875 = -199891206;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll51938633 = -363136008;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll25984061 = -791925722;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll20221662 = -653730571;    float WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll60561530 = -773118029;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll89306173 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll31869273;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll31869273 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll51349182;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll51349182 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll91022210;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll91022210 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll7661729;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll7661729 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll30347323;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll30347323 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll78042645;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll78042645 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll60546288;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll60546288 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll71914002;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll71914002 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll1397222;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll1397222 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll8155439;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll8155439 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll89483361;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll89483361 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll41623933;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll41623933 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll48515675;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll48515675 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll48151060;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll48151060 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll53140328;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll53140328 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll85496076;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll85496076 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll59743105;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll59743105 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll21082409;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll21082409 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll62627404;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll62627404 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll1586071;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll1586071 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll82559486;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll82559486 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll35826036;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll35826036 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll36619657;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll36619657 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll72468939;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll72468939 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll44286316;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll44286316 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll69532730;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll69532730 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll65007234;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll65007234 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll71406945;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll71406945 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll60274326;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll60274326 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll9653225;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll9653225 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll39315102;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll39315102 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll87976252;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll87976252 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll56892638;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll56892638 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll81969640;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll81969640 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll47991305;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll47991305 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll43844398;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll43844398 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll42429964;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll42429964 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll32681006;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll32681006 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll9355271;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll9355271 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll44329286;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll44329286 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll41077976;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll41077976 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll37623737;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll37623737 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll11965659;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll11965659 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll29401867;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll29401867 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll22132744;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll22132744 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll3970456;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll3970456 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll15063618;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll15063618 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll53647333;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll53647333 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll8915909;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll8915909 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll34725180;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll34725180 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll49598922;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll49598922 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll22686372;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll22686372 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll64220296;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll64220296 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll18333083;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll18333083 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll6746688;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll6746688 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll96043236;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll96043236 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll14729526;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll14729526 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll18553271;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll18553271 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll63375413;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll63375413 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll60814593;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll60814593 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll13035411;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll13035411 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll89139342;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll89139342 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll11639677;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll11639677 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll91743997;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll91743997 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll68840337;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll68840337 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll1507110;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll1507110 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll84731295;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll84731295 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll66546035;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll66546035 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll159755;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll159755 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll9295930;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll9295930 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll43066112;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll43066112 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll27062099;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll27062099 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll11727139;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll11727139 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll18298119;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll18298119 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll60508094;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll60508094 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll44935749;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll44935749 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll23860378;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll23860378 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll7217790;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll7217790 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll50336196;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll50336196 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll40315860;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll40315860 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll54469113;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll54469113 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll11359901;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll11359901 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll62491037;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll62491037 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll25549146;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll25549146 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll60054302;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll60054302 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll16628731;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll16628731 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll23755956;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll23755956 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll38559556;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll38559556 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll75222953;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll75222953 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll51948069;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll51948069 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll29114873;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll29114873 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll23876693;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll23876693 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll69305593;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll69305593 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll48540677;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll48540677 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll31293875;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll31293875 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll51938633;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll51938633 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll25984061;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll25984061 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll20221662;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll20221662 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll60561530;     WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll60561530 = WxAFnEoQkqBVecdYdWucBJeEXrXpvBpkTIgAMyztvtLmwsTDFyNKziFNTvhvhyUiAQCWwdLfXBgIxsIfYTkxnfll89306173;}
// Junk Finished

// Junk Code By Peatreat & Thaisen's Gen
void ZAQecBkmfqATBCgLrcJQamNSFEkEOtpfcPCUDbXPbVEIBKKraWAzoCmnnyZXQfYmBUrmzMHQEto36856367() {     float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz53584486 = -802139211;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz4552269 = -442346833;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz51383975 = -60139804;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz52673406 = -242830660;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz73690992 = -180065557;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz64098656 = 10142044;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz17967884 = -339982191;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz31452566 = -979114849;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz88361787 = -597320900;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz37688797 = -781676166;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz48118801 = 94954788;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz6695811 = -550681056;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz25024719 = -37536353;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz39367249 = 50035133;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz95609970 = -647239103;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz11662925 = -925104422;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz19842110 = -616344023;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz93338670 = -806423629;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz93242380 = -955914654;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz31836247 = -9337012;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz90121311 = -652291405;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz63073938 = -95002615;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz58128244 = -609142787;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz15062236 = -630576178;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz87672058 = -865778191;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz74614315 = -68230183;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz41976073 = 35098541;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz41308804 = -623357881;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz84208987 = -884892343;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz25327719 = -461213052;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz10940619 = -880131505;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz79599821 = -99116097;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz6554706 = -122490096;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz68113650 = -437824841;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz98146906 = -432881096;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz64916006 = -820570617;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz31639159 = -671407359;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz45371795 = -197575445;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz79087577 = -393831455;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz52450315 = -260852644;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz79757347 = -156450840;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz63233877 = -614884975;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz72104735 = -106124331;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz52551397 = -983159295;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz66237191 = -910954844;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz18128450 = -145907145;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz7606811 = -913972037;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz26662179 = -65960212;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz99527540 = -472640993;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz54563261 = -741290524;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz66301410 = -249575033;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz74252712 = 82316912;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz42690715 = -944308012;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz88686090 = -606827576;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz46735056 = -272808268;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz90510548 = -607136597;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz46424024 = -833204046;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz36321740 = -429563626;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz65001348 = -377052470;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz99076677 = -11835374;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz22122584 = 75043503;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz76659079 = -716624311;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz47243578 = 5777494;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz63034068 = -36107848;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz26748178 = -901544661;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz68518979 = -805929115;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz141106 = -328190961;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz56911068 = -599711513;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz41220343 = -517083772;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz30693965 = -826668486;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz80023766 = -153697064;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz74470315 = -318768579;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz14251093 = -312592174;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz40792065 = -595062010;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz52078899 = -852886172;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz26887435 = 62593570;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz90969203 = -988878285;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz5576848 = -625983492;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz48825044 = -719621334;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz69543608 = -619871047;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz67007504 = -154258146;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz15313895 = -898941247;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz41781263 = -50716888;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz29645726 = -43601820;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz59026309 = -111638019;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz36687906 = -862448417;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz36909107 = -154808086;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz17868616 = -515662520;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz21378595 = -65016574;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz7636359 = -825744500;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz18491982 = -987366572;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz95317419 = -141843734;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz80370447 = -820522976;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz80010900 = -281996081;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz30327732 = -235896147;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz3098268 = -439826530;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz15990299 = -520662469;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz9070667 = 29983517;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz25803220 = 18385365;    float EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz97718211 = -802139211;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz53584486 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz4552269;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz4552269 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz51383975;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz51383975 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz52673406;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz52673406 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz73690992;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz73690992 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz64098656;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz64098656 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz17967884;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz17967884 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz31452566;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz31452566 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz88361787;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz88361787 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz37688797;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz37688797 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz48118801;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz48118801 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz6695811;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz6695811 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz25024719;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz25024719 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz39367249;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz39367249 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz95609970;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz95609970 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz11662925;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz11662925 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz19842110;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz19842110 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz93338670;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz93338670 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz93242380;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz93242380 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz31836247;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz31836247 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz90121311;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz90121311 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz63073938;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz63073938 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz58128244;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz58128244 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz15062236;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz15062236 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz87672058;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz87672058 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz74614315;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz74614315 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz41976073;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz41976073 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz41308804;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz41308804 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz84208987;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz84208987 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz25327719;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz25327719 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz10940619;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz10940619 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz79599821;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz79599821 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz6554706;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz6554706 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz68113650;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz68113650 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz98146906;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz98146906 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz64916006;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz64916006 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz31639159;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz31639159 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz45371795;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz45371795 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz79087577;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz79087577 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz52450315;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz52450315 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz79757347;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz79757347 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz63233877;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz63233877 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz72104735;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz72104735 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz52551397;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz52551397 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz66237191;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz66237191 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz18128450;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz18128450 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz7606811;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz7606811 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz26662179;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz26662179 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz99527540;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz99527540 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz54563261;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz54563261 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz66301410;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz66301410 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz74252712;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz74252712 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz42690715;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz42690715 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz88686090;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz88686090 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz46735056;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz46735056 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz90510548;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz90510548 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz46424024;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz46424024 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz36321740;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz36321740 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz65001348;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz65001348 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz99076677;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz99076677 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz22122584;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz22122584 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz76659079;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz76659079 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz47243578;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz47243578 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz63034068;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz63034068 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz26748178;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz26748178 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz68518979;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz68518979 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz141106;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz141106 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz56911068;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz56911068 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz41220343;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz41220343 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz30693965;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz30693965 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz80023766;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz80023766 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz74470315;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz74470315 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz14251093;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz14251093 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz40792065;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz40792065 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz52078899;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz52078899 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz26887435;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz26887435 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz90969203;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz90969203 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz5576848;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz5576848 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz48825044;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz48825044 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz69543608;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz69543608 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz67007504;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz67007504 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz15313895;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz15313895 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz41781263;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz41781263 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz29645726;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz29645726 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz59026309;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz59026309 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz36687906;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz36687906 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz36909107;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz36909107 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz17868616;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz17868616 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz21378595;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz21378595 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz7636359;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz7636359 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz18491982;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz18491982 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz95317419;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz95317419 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz80370447;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz80370447 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz80010900;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz80010900 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz30327732;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz30327732 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz3098268;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz3098268 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz15990299;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz15990299 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz9070667;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz9070667 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz25803220;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz25803220 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz97718211;     EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz97718211 = EhEQuusertprZcPgFCAyvxRHgTYsNOjvuNiFiiedYBgtLdRJkfCNDvtGQxjbWAQcJuwLtJxXXWZtRYvDBOHiMVrz53584486;}
// Junk Finished

// Junk Code By Peatreat & Thaisen's Gen
void QboiXXKGivXqatZmyxZnpvUqfPXbFONBecGCZoAOCoGCgBcofhkwdsRUHyDIziBTiWMxmWMKxKk99489273() {     float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW37051757 = -674935220;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW49533122 = -837932765;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW76078149 = -330819167;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW98616048 = -79516854;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW66726761 = -491142597;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW10606947 = -654201408;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW36883756 = -89572838;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW38616437 = -539292667;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW73806541 = 28046210;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW38349158 = -350348349;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW48832329 = -272831659;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW396503 = -953155239;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW65861088 = -630888068;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW23888611 = -731210784;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW60270546 = 85457245;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW50976304 = -147152738;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW54036731 = -17358502;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW17771165 = -193186641;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW86500892 = -246055536;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW96916188 = -998090201;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW79413501 = -400185314;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW69715721 = -635067625;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW83253301 = 65756213;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW44790346 = -334842987;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW19052532 = -937507026;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW52059168 = -241789893;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW26210348 = -190246385;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW65534474 = -229284006;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW2918939 = -817056150;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW55372980 = -811154960;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW78762988 = -138512353;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW97776991 = -437047641;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW66023449 = -995871917;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW44524966 = -980977273;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW65377914 = -718655307;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW60434594 = -986180964;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW56667958 = -493665509;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW79993141 = 81717446;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW81186833 = -822998289;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW27133116 = -796973885;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW25548870 = -156889097;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW18396970 = -398913304;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW68485897 = -914874998;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW21232095 = -280710524;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW47058172 = -648707690;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW63924397 = -11142258;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW36069507 = 10069767;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW19345318 = -164063349;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW83516238 = -253745596;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW40568110 = -282745575;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW5479826 = -286446027;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW19350437 = -897119164;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW1250262 = -103368767;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW64826066 = -323804619;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW20075773 = -543783507;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW67336036 = 60132405;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW66279820 = -803688978;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW31287804 = -995976181;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW79563517 = -142009828;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW14667593 = -149352705;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW84396599 = -363955024;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW71349281 = -860288832;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW35697498 = -722236518;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW18433561 = -160798831;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW59586169 = -111835996;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW51055338 = -835784019;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW34373054 = -957283323;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW21336122 = -649910796;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW58510697 = 87444522;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW99835952 = 71638208;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW94308346 = -653487230;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW74043589 = 924051;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW36584331 = -370188352;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW59367776 = -449081651;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW71367319 = -741201104;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW61016531 = 98727990;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW1229824 = -720192628;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW62021206 = -653533263;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW97732173 = -686135298;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW55128134 = -826364769;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW15989661 = -151859660;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW6865030 = 73816964;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW82018235 = -975538411;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW62350829 = -434310575;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW49893155 = -424708933;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW59412551 = -241393190;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW96526730 = -233678874;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW1197384 = -572067298;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW24449194 = -337193767;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW98041877 = -678787713;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW94154773 = -82491986;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW25380155 = -497689329;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW429625 = -776272727;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW66519241 = -573645585;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW42736517 = -333018861;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW54199588 = -296600266;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW82699472 = -676676787;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW50052336 = -654076168;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW61645925 = -68874528;    float BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW96002834 = -674935220;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW37051757 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW49533122;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW49533122 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW76078149;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW76078149 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW98616048;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW98616048 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW66726761;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW66726761 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW10606947;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW10606947 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW36883756;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW36883756 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW38616437;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW38616437 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW73806541;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW73806541 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW38349158;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW38349158 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW48832329;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW48832329 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW396503;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW396503 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW65861088;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW65861088 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW23888611;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW23888611 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW60270546;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW60270546 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW50976304;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW50976304 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW54036731;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW54036731 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW17771165;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW17771165 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW86500892;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW86500892 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW96916188;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW96916188 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW79413501;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW79413501 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW69715721;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW69715721 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW83253301;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW83253301 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW44790346;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW44790346 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW19052532;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW19052532 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW52059168;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW52059168 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW26210348;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW26210348 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW65534474;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW65534474 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW2918939;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW2918939 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW55372980;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW55372980 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW78762988;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW78762988 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW97776991;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW97776991 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW66023449;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW66023449 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW44524966;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW44524966 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW65377914;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW65377914 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW60434594;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW60434594 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW56667958;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW56667958 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW79993141;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW79993141 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW81186833;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW81186833 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW27133116;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW27133116 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW25548870;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW25548870 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW18396970;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW18396970 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW68485897;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW68485897 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW21232095;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW21232095 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW47058172;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW47058172 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW63924397;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW63924397 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW36069507;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW36069507 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW19345318;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW19345318 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW83516238;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW83516238 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW40568110;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW40568110 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW5479826;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW5479826 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW19350437;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW19350437 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW1250262;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW1250262 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW64826066;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW64826066 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW20075773;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW20075773 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW67336036;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW67336036 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW66279820;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW66279820 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW31287804;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW31287804 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW79563517;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW79563517 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW14667593;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW14667593 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW84396599;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW84396599 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW71349281;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW71349281 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW35697498;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW35697498 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW18433561;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW18433561 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW59586169;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW59586169 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW51055338;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW51055338 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW34373054;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW34373054 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW21336122;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW21336122 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW58510697;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW58510697 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW99835952;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW99835952 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW94308346;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW94308346 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW74043589;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW74043589 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW36584331;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW36584331 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW59367776;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW59367776 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW71367319;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW71367319 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW61016531;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW61016531 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW1229824;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW1229824 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW62021206;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW62021206 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW97732173;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW97732173 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW55128134;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW55128134 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW15989661;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW15989661 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW6865030;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW6865030 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW82018235;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW82018235 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW62350829;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW62350829 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW49893155;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW49893155 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW59412551;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW59412551 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW96526730;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW96526730 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW1197384;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW1197384 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW24449194;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW24449194 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW98041877;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW98041877 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW94154773;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW94154773 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW25380155;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW25380155 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW429625;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW429625 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW66519241;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW66519241 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW42736517;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW42736517 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW54199588;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW54199588 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW82699472;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW82699472 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW50052336;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW50052336 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW61645925;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW61645925 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW96002834;     BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW96002834 = BpRJemjvXdQXlLlvalsOUnmUJoeBIUazSHQfppYVQmdqRyamBbmZEwQOCxzpIhmCDRFdvFGlMXyhJwCLOszhiBuW37051757;}
// Junk Finished

// Junk Code By Peatreat & Thaisen's Gen
void vDIRVHxSTVwDWAsBZSPXpIdOnyGvNgzAgKszsZURFWOscNtsLFKlmPumZueWACEnVwWLIsIAxxS72589936() {     float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG33314902 = -323985707;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG73426388 = -239960022;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG66555306 = -524164992;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG74879726 = -559594079;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG40013480 = -591503120;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG48217159 = -446318956;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG24186382 = -278030855;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG68678344 = -987722765;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG30545861 = -287875668;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG50268954 = -817897858;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG46552331 = -816096015;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG4610183 = -59744694;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG29061389 = -545534729;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG27267721 = -227142322;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG80479512 = -244762960;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG56867931 = 29101033;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG60204031 = -452725594;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG41261345 = -189552522;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG277630 = -718692213;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG56539982 = -568050852;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG29375612 = 26304073;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG37960924 = -30003159;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG91821058 = -151911918;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG53550423 = -578074025;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG63663229 = -29637006;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG35961551 = -243471909;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG66527786 = -598098021;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG4007783 = -195557754;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG19228273 = -544331724;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG74034552 = -177835962;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG40902336 = -192670099;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG42187198 = -165966221;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG43359506 = -892192198;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG97588398 = 36138675;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG92132395 = -125490511;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG86935012 = -270705795;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG50548766 = -506352941;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG67441992 = -665715736;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG36091080 = -574993576;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG70271307 = -113364442;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG50988198 = -950885500;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG2716028 = -890564844;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG59325489 = -908363728;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG95693483 = -583987330;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG81860566 = -119341393;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG22706522 = -781628804;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG567534 = 58517452;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG78393052 = -54377231;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG33581402 = -302806382;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG43455245 = 45300241;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG43096679 = -108368819;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG70803823 = -124521955;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG31122435 = -150351793;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG70290360 = -482353627;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG43214324 = -73854316;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG95353978 = -193982549;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG81605330 = 11951895;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG13004883 = -946090968;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG11216497 = -429957073;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG4051930 = -248031211;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG81689373 = -848220936;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG20178600 = 17526899;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG49450071 = -343391042;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG56511308 = -10039706;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG9366618 = -525227759;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG4365134 = -550129795;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG61250677 = -167552496;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG31472991 = -481673405;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG35135325 = -1651811;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG93544499 = -974057165;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG6319165 = -464546026;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG92762038 = -787009858;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG5170265 = -614558947;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG30006323 = -505327771;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG5551785 = -617165352;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG26659584 = -83131083;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG78635434 = -121639431;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG96127574 = -567924588;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG71689857 = -358732632;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG40956708 = -248008203;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG35394017 = -201989362;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG88134733 = -443720790;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG70426380 = -892751373;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG75773028 = -489631966;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG30937873 = 30532856;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG70098513 = 31851855;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG11064763 = 84385571;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG73069146 = -309838572;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG54374074 = -890007009;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG96778417 = -931507962;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG5329683 = -182657691;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG37543884 = -560261974;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG56225495 = -135758663;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG32039151 = -226962365;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG88581934 = -265143507;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG30809599 = -868412400;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG53265957 = -447173802;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG2814182 = -798324023;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG86326865 = 41240429;    float SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG77495432 = -323985707;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG33314902 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG73426388;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG73426388 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG66555306;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG66555306 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG74879726;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG74879726 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG40013480;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG40013480 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG48217159;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG48217159 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG24186382;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG24186382 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG68678344;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG68678344 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG30545861;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG30545861 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG50268954;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG50268954 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG46552331;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG46552331 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG4610183;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG4610183 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG29061389;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG29061389 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG27267721;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG27267721 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG80479512;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG80479512 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG56867931;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG56867931 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG60204031;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG60204031 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG41261345;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG41261345 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG277630;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG277630 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG56539982;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG56539982 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG29375612;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG29375612 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG37960924;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG37960924 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG91821058;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG91821058 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG53550423;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG53550423 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG63663229;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG63663229 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG35961551;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG35961551 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG66527786;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG66527786 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG4007783;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG4007783 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG19228273;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG19228273 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG74034552;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG74034552 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG40902336;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG40902336 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG42187198;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG42187198 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG43359506;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG43359506 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG97588398;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG97588398 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG92132395;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG92132395 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG86935012;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG86935012 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG50548766;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG50548766 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG67441992;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG67441992 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG36091080;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG36091080 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG70271307;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG70271307 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG50988198;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG50988198 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG2716028;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG2716028 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG59325489;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG59325489 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG95693483;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG95693483 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG81860566;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG81860566 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG22706522;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG22706522 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG567534;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG567534 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG78393052;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG78393052 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG33581402;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG33581402 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG43455245;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG43455245 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG43096679;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG43096679 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG70803823;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG70803823 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG31122435;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG31122435 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG70290360;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG70290360 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG43214324;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG43214324 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG95353978;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG95353978 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG81605330;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG81605330 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG13004883;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG13004883 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG11216497;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG11216497 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG4051930;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG4051930 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG81689373;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG81689373 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG20178600;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG20178600 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG49450071;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG49450071 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG56511308;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG56511308 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG9366618;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG9366618 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG4365134;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG4365134 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG61250677;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG61250677 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG31472991;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG31472991 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG35135325;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG35135325 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG93544499;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG93544499 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG6319165;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG6319165 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG92762038;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG92762038 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG5170265;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG5170265 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG30006323;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG30006323 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG5551785;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG5551785 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG26659584;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG26659584 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG78635434;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG78635434 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG96127574;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG96127574 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG71689857;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG71689857 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG40956708;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG40956708 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG35394017;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG35394017 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG88134733;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG88134733 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG70426380;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG70426380 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG75773028;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG75773028 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG30937873;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG30937873 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG70098513;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG70098513 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG11064763;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG11064763 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG73069146;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG73069146 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG54374074;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG54374074 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG96778417;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG96778417 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG5329683;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG5329683 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG37543884;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG37543884 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG56225495;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG56225495 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG32039151;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG32039151 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG88581934;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG88581934 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG30809599;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG30809599 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG53265957;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG53265957 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG2814182;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG2814182 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG86326865;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG86326865 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG77495432;     SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG77495432 = SuklaFOkIVOOQQFZyvxQDRbXgqhkTxKnoxbspJERmRwRGhGuCSJFAhPBTMEjXdRKLaZTpNkprgdlUJQyknhAnFnG33314902;}
// Junk Finished

// Junk Code By Peatreat & Thaisen's Gen
void LapewaEnhkwRWvNscUYLqtUMdQzcFFJpOszfOVuvPCstddTkJlikclSSFdBfbbhbbHCdwieHZcv75684657() {     float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq79573193 = -702429301;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq1248893 = -773933624;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq83017371 = -960357277;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq39953299 = -630076927;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq19930276 = -493130134;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq11931446 = -13011113;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq37899510 = -519891222;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq650790 = -664285852;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq99340302 = -114032098;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq59668385 = -188052521;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq59512788 = -778964424;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq19542622 = -617942920;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq66151375 = -751425245;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq96029995 = -531938648;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq84567559 = -547017804;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq43682969 = -869241934;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq8260900 = -202803612;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq7001924 = -758722119;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq54375570 = -4815591;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq20219099 = -47029842;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq39089791 = -980056320;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq39928326 = -82432605;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq28047613 = -988798431;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq45048082 = -181995454;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq54605945 = -266787376;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq7294491 = -726308372;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq84511410 = -131549442;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq69033734 = -68556743;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq95046442 = 68839966;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq74763526 = -401090495;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq28101193 = 12852228;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq75140298 = -738751122;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq34784881 = -114970642;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq50863959 = -229256675;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq30655407 = -826811158;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq45556573 = -440054936;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq78385600 = -868780407;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq57329101 = -309595927;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq67480524 = -586499924;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq92314167 = -511828764;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq67319278 = -985839827;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq76308331 = -427560445;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq20377602 = -771200149;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq89578795 = -554987469;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq29676499 = -219919648;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq11809623 = -586148853;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq35838905 = -803995329;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq68978473 = -676262130;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq72640357 = -849690063;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq11376448 = -416922608;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq40658061 = -714207866;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq83167651 = -463534058;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq92580870 = -941375324;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq33442358 = -214842523;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq36594867 = -521599583;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq39644867 = -519996697;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq73201279 = -785135194;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq37969289 = -678361823;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq85347354 = -263289551;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq12635786 = -766821763;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq27420035 = -881461672;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq68865776 = -351334479;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq5604348 = -633125818;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq24576776 = -712941604;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq31567192 = -100904750;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq84372489 = 59786697;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq84757741 = -402972279;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq15287417 = -422168570;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq65374589 = -705127490;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq39010987 = -6962868;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq65297368 = 99538473;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq50931798 = -893207685;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq39521399 = -72222195;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq62061402 = -492986827;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq52899821 = -61190016;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq62781459 = -452495875;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq19550724 = -311232456;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq38468818 = -333810962;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq15371584 = -962075807;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq42796322 = -680638523;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq71455585 = -922313043;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq15532938 = -455287312;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq96393376 = -218866681;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq83669994 = -514237427;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq34105466 = -686882629;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq44933541 = -523613714;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq82559427 = -797375798;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq1342524 = -900128119;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq14269092 = -707657093;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq91010539 = -206814462;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq72355293 = -654919743;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq40416312 = -90418585;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq71981747 = 53693623;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq54844739 = -819678162;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq64894133 = -630367093;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq98453502 = -534505348;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq70703984 = -794434628;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq95800826 = 41741454;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq58011603 = -354082720;    float oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq45304009 = -702429301;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq79573193 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq1248893;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq1248893 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq83017371;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq83017371 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq39953299;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq39953299 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq19930276;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq19930276 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq11931446;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq11931446 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq37899510;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq37899510 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq650790;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq650790 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq99340302;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq99340302 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq59668385;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq59668385 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq59512788;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq59512788 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq19542622;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq19542622 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq66151375;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq66151375 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq96029995;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq96029995 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq84567559;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq84567559 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq43682969;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq43682969 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq8260900;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq8260900 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq7001924;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq7001924 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq54375570;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq54375570 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq20219099;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq20219099 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq39089791;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq39089791 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq39928326;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq39928326 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq28047613;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq28047613 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq45048082;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq45048082 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq54605945;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq54605945 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq7294491;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq7294491 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq84511410;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq84511410 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq69033734;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq69033734 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq95046442;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq95046442 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq74763526;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq74763526 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq28101193;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq28101193 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq75140298;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq75140298 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq34784881;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq34784881 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq50863959;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq50863959 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq30655407;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq30655407 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq45556573;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq45556573 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq78385600;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq78385600 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq57329101;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq57329101 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq67480524;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq67480524 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq92314167;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq92314167 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq67319278;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq67319278 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq76308331;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq76308331 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq20377602;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq20377602 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq89578795;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq89578795 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq29676499;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq29676499 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq11809623;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq11809623 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq35838905;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq35838905 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq68978473;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq68978473 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq72640357;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq72640357 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq11376448;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq11376448 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq40658061;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq40658061 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq83167651;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq83167651 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq92580870;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq92580870 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq33442358;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq33442358 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq36594867;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq36594867 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq39644867;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq39644867 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq73201279;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq73201279 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq37969289;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq37969289 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq85347354;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq85347354 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq12635786;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq12635786 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq27420035;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq27420035 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq68865776;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq68865776 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq5604348;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq5604348 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq24576776;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq24576776 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq31567192;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq31567192 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq84372489;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq84372489 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq84757741;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq84757741 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq15287417;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq15287417 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq65374589;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq65374589 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq39010987;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq39010987 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq65297368;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq65297368 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq50931798;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq50931798 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq39521399;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq39521399 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq62061402;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq62061402 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq52899821;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq52899821 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq62781459;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq62781459 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq19550724;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq19550724 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq38468818;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq38468818 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq15371584;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq15371584 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq42796322;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq42796322 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq71455585;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq71455585 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq15532938;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq15532938 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq96393376;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq96393376 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq83669994;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq83669994 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq34105466;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq34105466 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq44933541;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq44933541 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq82559427;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq82559427 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq1342524;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq1342524 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq14269092;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq14269092 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq91010539;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq91010539 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq72355293;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq72355293 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq40416312;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq40416312 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq71981747;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq71981747 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq54844739;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq54844739 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq64894133;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq64894133 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq98453502;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq98453502 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq70703984;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq70703984 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq95800826;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq95800826 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq58011603;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq58011603 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq45304009;     oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq45304009 = oAaiAwOSUXYMdagZlTOUoURRWapobHaLzpdSeuBjMINEDuZDPIsnLIHUjIMZHarNwMNnwOuqSfHfGMObBmYSOnAq79573193;}
// Junk Finished

// Junk Code By Peatreat & Thaisen's Gen
void EchHrhLbUkfcQUuuvDiVXDXXzrpUUXmnuRmNYCzbNKnrQqHnroKKoylAEJIzrrGwxriyihWtEGH94811210() {     float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL40364427 = -730251172;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL89014713 = -380599525;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL38734475 = -302293254;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL14153343 = -930084741;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL54611930 = -324602455;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL82929650 = -41646389;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL51639401 = -272696439;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL7009238 = -876836444;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL20042345 = -468914708;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL49964496 = -264528774;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL59692266 = -339165194;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL51771508 = -431368329;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL54822858 = -157376350;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL53956682 = -741812879;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL36926227 = -299238698;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL15162390 = -622767857;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL70297782 = -850425290;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL76732978 = -375803176;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL8458518 = -296853380;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL30153200 = -414478330;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL35303887 = -317224991;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL61344535 = -410715611;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL39846783 = -286750062;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL47315432 = -776242932;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL48084071 = -188319865;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL13646856 = -855825439;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL4851439 = -234716363;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL37585667 = 17007707;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL90044455 = -376481436;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL40071428 = -184127074;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL38482667 = -263511500;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL51175201 = -378341811;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL70807773 = -267583378;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL74744639 = -810379;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL56317511 = -801173961;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL6152527 = -769545954;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL46737328 = -116078556;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL33397907 = 35101114;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL62621403 = 87018233;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL14926612 = -681820148;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL35018335 = 13474888;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL42657585 = -612250858;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL42400536 = -129768602;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL42372642 = -467860752;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL32498538 = -810560993;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL5486356 = -941801109;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL36387354 = -134122528;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL61067057 = -517572131;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL37129633 = -362541691;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL29824839 = -971899748;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL24022931 = -918964115;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL80230080 = -241575999;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL4728593 = 83579188;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL43415347 = -145722734;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL59507323 = -318978802;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL79019891 = -219535562;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL49167930 = 6150536;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL91419043 = -526050322;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL66069272 = -641764877;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL40965075 = -468777017;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL78078212 = -806930027;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL14053735 = -189704146;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL16964783 = -400355009;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL79970916 = -184787635;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL11481830 = 98982725;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL8517065 = -960823384;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL80963734 = -63784951;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL80078219 = -56565972;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL97639171 = -940638919;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL30773700 = -529692744;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL68425062 = -406689302;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL36899875 = -785526404;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL14111575 = -362821409;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL93531905 = -615033233;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL95134865 = -327953218;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL92646302 = -704974133;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL18944000 = -180947009;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL97474141 = -818889310;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL14816894 = -965681940;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL42597715 = -246518756;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL77259502 = -621702911;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL43784381 = -717144232;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL456034 = -620450603;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL60219616 = -404581688;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL16048497 = -265162959;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL58252586 = 78064499;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL46446609 = -361920999;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL27392427 = -21860645;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL15237316 = -681831577;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL77297619 = -481638400;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL56984597 = -675696491;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL55318284 = -590028235;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL67328635 = -323134009;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL21656329 = -444204751;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL36848400 = -874890122;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL20964600 = -796820967;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL25692803 = -111895850;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL62429619 = -944980968;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL30890813 = -466843478;    float MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL23981474 = -730251172;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL40364427 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL89014713;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL89014713 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL38734475;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL38734475 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL14153343;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL14153343 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL54611930;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL54611930 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL82929650;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL82929650 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL51639401;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL51639401 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL7009238;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL7009238 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL20042345;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL20042345 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL49964496;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL49964496 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL59692266;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL59692266 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL51771508;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL51771508 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL54822858;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL54822858 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL53956682;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL53956682 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL36926227;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL36926227 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL15162390;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL15162390 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL70297782;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL70297782 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL76732978;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL76732978 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL8458518;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL8458518 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL30153200;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL30153200 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL35303887;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL35303887 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL61344535;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL61344535 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL39846783;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL39846783 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL47315432;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL47315432 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL48084071;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL48084071 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL13646856;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL13646856 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL4851439;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL4851439 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL37585667;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL37585667 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL90044455;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL90044455 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL40071428;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL40071428 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL38482667;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL38482667 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL51175201;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL51175201 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL70807773;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL70807773 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL74744639;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL74744639 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL56317511;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL56317511 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL6152527;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL6152527 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL46737328;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL46737328 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL33397907;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL33397907 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL62621403;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL62621403 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL14926612;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL14926612 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL35018335;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL35018335 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL42657585;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL42657585 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL42400536;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL42400536 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL42372642;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL42372642 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL32498538;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL32498538 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL5486356;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL5486356 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL36387354;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL36387354 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL61067057;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL61067057 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL37129633;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL37129633 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL29824839;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL29824839 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL24022931;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL24022931 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL80230080;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL80230080 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL4728593;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL4728593 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL43415347;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL43415347 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL59507323;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL59507323 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL79019891;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL79019891 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL49167930;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL49167930 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL91419043;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL91419043 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL66069272;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL66069272 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL40965075;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL40965075 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL78078212;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL78078212 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL14053735;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL14053735 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL16964783;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL16964783 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL79970916;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL79970916 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL11481830;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL11481830 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL8517065;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL8517065 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL80963734;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL80963734 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL80078219;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL80078219 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL97639171;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL97639171 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL30773700;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL30773700 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL68425062;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL68425062 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL36899875;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL36899875 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL14111575;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL14111575 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL93531905;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL93531905 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL95134865;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL95134865 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL92646302;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL92646302 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL18944000;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL18944000 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL97474141;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL97474141 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL14816894;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL14816894 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL42597715;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL42597715 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL77259502;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL77259502 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL43784381;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL43784381 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL456034;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL456034 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL60219616;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL60219616 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL16048497;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL16048497 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL58252586;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL58252586 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL46446609;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL46446609 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL27392427;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL27392427 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL15237316;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL15237316 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL77297619;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL77297619 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL56984597;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL56984597 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL55318284;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL55318284 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL67328635;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL67328635 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL21656329;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL21656329 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL36848400;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL36848400 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL20964600;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL20964600 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL25692803;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL25692803 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL62429619;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL62429619 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL30890813;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL30890813 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL23981474;     MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL23981474 = MQgGIkCcgMczvgmIOtzkVDrtZIIFJSRFhrYkYCzPpOajVsmSwYHzFWlJYmhAbxSOvWREHCjLtjuHdHYgVqJkKRAL40364427;}
// Junk Finished

// Junk Code By Peatreat & Thaisen's Gen
void mWGfTDBJdjLQblXbWfwZYicUUHcMLTdSQkcMaXSYcOCnhoRwhZMfYJzDtlLLapxKGtArJKBzNXx8051319() {     float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb81681019 = -217465972;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb62328987 = -583982556;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb78927799 = 50270771;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb54786098 = -134059123;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb97687170 = 1842319;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb63141213 = -979229709;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb89094750 = -386731892;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb17331503 = -699756388;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb62797753 = -909252009;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb58971397 = 27774147;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb25969086 = -525847736;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb94797457 = 19030605;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb84975596 = -260628652;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb57311549 = -754554436;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb73930541 = -816088652;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb94990709 = -245897497;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb82225311 = -45886857;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb45401813 = -639201136;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb83334855 = -829668676;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb31549763 = -676414813;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb98731335 = -958647710;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb46074928 = -737187287;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb20351345 = -846084916;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb75511220 = -514866942;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb58801397 = -242246096;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb31805836 = -107753215;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb11484917 = -973532066;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb90749449 = -342661553;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb55975959 = -353962007;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb96762344 = -651156332;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb52472734 = 38357630;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb72354651 = -945405243;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb80806986 = -66932564;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb40151650 = -444442608;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb24407317 = -972147882;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb73510733 = -875234110;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb58646972 = -36154008;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb37901918 = -638255709;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb25762554 = -587730201;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb88130212 = -464712121;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb49091002 = -765368001;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb82845365 = -751837128;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb53573588 = -99665512;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb71012847 = -588625818;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb39310219 = -497338776;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb85129929 = -572166614;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb82643714 = -31837026;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb28383393 = -784356174;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb79141706 = -525217941;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb27420312 = -252889431;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb46211602 = -295910440;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb75390906 = -24410829;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb22499517 = -207411348;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb90708578 = -613929160;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb9319984 = -98698664;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb35606091 = -480278686;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb41977643 = -737897640;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb3416579 = -434862288;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb95984701 = -891813027;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb65881335 = -890404466;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb51656297 = 94302356;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb98345301 = 55929660;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb61355544 = -245794382;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb66035409 = -158095678;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb6498664 = 89416516;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb53614434 = -580442494;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb13990472 = -914036832;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb44823947 = -816186045;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb32904232 = -782406555;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb419809 = -940854542;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb36343737 = -109743489;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb44323393 = -407631148;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb19639260 = 48529065;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb95204642 = -264956555;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb82458761 = -911046813;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb15885970 = -106810583;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb92501339 = -537521776;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb49338497 = -157459099;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb36201002 = 82471834;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb73671468 = -670079482;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb49162121 = 24083811;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb83101524 = -89175892;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb11607743 = -817443612;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb28555647 = -1072576;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb50550743 = -255245893;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb77081828 = -937231541;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb49855135 = -637993896;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb90098407 = -453003405;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb30831667 = -245743945;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb88801226 = -391869197;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb31533090 = -37336471;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb55230393 = -601291720;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb41917217 = -746442682;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb59881219 = -697325735;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb36473916 = -459014478;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb50745700 = -721297661;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb21489822 = -406042747;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb87538179 = -941569834;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb64514184 = -578042335;    float KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb85695784 = -217465972;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb81681019 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb62328987;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb62328987 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb78927799;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb78927799 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb54786098;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb54786098 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb97687170;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb97687170 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb63141213;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb63141213 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb89094750;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb89094750 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb17331503;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb17331503 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb62797753;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb62797753 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb58971397;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb58971397 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb25969086;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb25969086 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb94797457;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb94797457 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb84975596;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb84975596 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb57311549;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb57311549 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb73930541;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb73930541 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb94990709;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb94990709 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb82225311;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb82225311 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb45401813;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb45401813 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb83334855;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb83334855 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb31549763;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb31549763 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb98731335;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb98731335 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb46074928;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb46074928 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb20351345;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb20351345 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb75511220;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb75511220 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb58801397;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb58801397 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb31805836;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb31805836 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb11484917;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb11484917 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb90749449;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb90749449 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb55975959;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb55975959 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb96762344;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb96762344 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb52472734;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb52472734 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb72354651;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb72354651 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb80806986;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb80806986 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb40151650;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb40151650 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb24407317;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb24407317 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb73510733;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb73510733 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb58646972;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb58646972 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb37901918;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb37901918 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb25762554;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb25762554 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb88130212;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb88130212 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb49091002;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb49091002 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb82845365;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb82845365 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb53573588;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb53573588 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb71012847;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb71012847 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb39310219;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb39310219 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb85129929;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb85129929 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb82643714;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb82643714 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb28383393;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb28383393 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb79141706;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb79141706 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb27420312;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb27420312 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb46211602;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb46211602 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb75390906;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb75390906 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb22499517;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb22499517 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb90708578;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb90708578 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb9319984;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb9319984 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb35606091;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb35606091 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb41977643;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb41977643 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb3416579;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb3416579 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb95984701;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb95984701 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb65881335;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb65881335 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb51656297;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb51656297 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb98345301;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb98345301 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb61355544;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb61355544 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb66035409;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb66035409 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb6498664;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb6498664 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb53614434;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb53614434 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb13990472;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb13990472 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb44823947;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb44823947 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb32904232;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb32904232 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb419809;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb419809 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb36343737;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb36343737 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb44323393;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb44323393 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb19639260;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb19639260 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb95204642;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb95204642 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb82458761;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb82458761 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb15885970;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb15885970 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb92501339;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb92501339 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb49338497;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb49338497 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb36201002;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb36201002 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb73671468;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb73671468 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb49162121;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb49162121 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb83101524;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb83101524 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb11607743;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb11607743 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb28555647;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb28555647 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb50550743;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb50550743 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb77081828;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb77081828 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb49855135;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb49855135 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb90098407;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb90098407 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb30831667;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb30831667 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb88801226;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb88801226 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb31533090;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb31533090 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb55230393;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb55230393 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb41917217;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb41917217 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb59881219;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb59881219 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb36473916;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb36473916 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb50745700;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb50745700 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb21489822;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb21489822 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb87538179;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb87538179 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb64514184;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb64514184 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb85695784;     KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb85695784 = KgpngqgemHytHkYToMGymQsgonZmXXNSBLpXDyeUuLLCHAtHlrMnGZKtoIysQuHOugrvvBPqhmDbFpYouSAfLaLb81681019;}
// Junk Finished

// Junk Code By Peatreat & Thaisen's Gen
void JfNPvWPDMEwjDwcyxeNLLCZxiSXSZxKfNXxOvKAGEUWDgIAnFlRbKLhHFPgdyxeOtRVfyNDmdGG27619974() {     float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF77652008 = -142208357;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF82640880 = -266930400;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF79853769 = 17450182;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF66853795 = -444135916;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF15214082 = -444148146;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF57527174 = -945963865;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF47650803 = -802660019;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF56828871 = 76755436;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF93327572 = -296442681;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF50610284 = 41275799;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF70857200 = -237886850;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF588122 = -816994550;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF78945649 = -167421259;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF53632846 = -379399571;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF41158930 = -140060238;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF64586564 = -599186701;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF93984604 = -814948252;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF81667235 = -966163563;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF59431794 = -548295519;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF50580881 = -183717442;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF96333171 = 31846420;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF82057909 = -130103071;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF37325737 = -892163192;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF5135378 = 47468233;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF65271339 = -669085780;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF91205763 = -463389776;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF20108319 = -67192047;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF97162232 = -710382673;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF61805139 = -711687453;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF47888670 = -308943559;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF18168330 = -208645766;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF24525712 = -823965203;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF58462075 = -422799363;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF214553 = -568195647;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF27001280 = -260985063;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF28195481 = -128917863;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF45261104 = -906656081;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF55383168 = -85541552;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF39673353 = -544427236;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF85320208 = 94087306;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF19123070 = -556761047;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF59707805 = -398955484;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF55746824 = 89932261;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF8647453 = -183070549;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF6105963 = -316634158;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF20107105 = -271321734;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF45373787 = 43391092;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF19127037 = 71207411;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF63496547 = -587624381;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF69857446 = -873898407;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF43513812 = -430072266;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF59933881 = -322311642;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF71998443 = -238317340;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF95417567 = -978022070;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF31286803 = -274327206;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF95594099 = 87894714;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF45315143 = -374767209;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF74718392 = 69981948;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF1582456 = -775050136;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF24008319 = -980758371;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF37418855 = -778771818;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF50488571 = 7722654;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF95023732 = -211557112;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF45438902 = -987499123;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF32441954 = -750078436;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF46331488 = -413921647;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF42126046 = -294195187;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF78731096 = -599225613;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF26631566 = -18414509;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF12963449 = 88857625;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF19325460 = -692530621;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF38601436 = -629406700;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF41993883 = -321736327;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF74111585 = -542382826;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF31457812 = -626956396;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF36625367 = -569198097;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF26311086 = -120035332;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF28678284 = -609092643;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF99029415 = -635897610;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF45164235 = -297764046;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF45831976 = -406780868;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF981282 = -38399458;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF33665685 = -22758292;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF91947692 = -837789046;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF4374859 = -878871293;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF58234449 = -886334124;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF52527268 = -485647863;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF63044507 = -444777294;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF68927750 = -193868441;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF31407181 = -248879777;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF82880338 = -754150655;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF70542712 = -876638029;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF53800713 = -310491417;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF15665035 = -563668866;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF47901354 = -127140876;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF68634498 = -464483701;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF64684072 = -87398373;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF10307922 = 77431383;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF76205499 = -432992114;    float rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF59774474 = -142208357;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF77652008 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF82640880;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF82640880 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF79853769;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF79853769 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF66853795;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF66853795 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF15214082;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF15214082 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF57527174;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF57527174 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF47650803;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF47650803 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF56828871;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF56828871 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF93327572;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF93327572 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF50610284;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF50610284 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF70857200;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF70857200 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF588122;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF588122 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF78945649;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF78945649 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF53632846;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF53632846 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF41158930;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF41158930 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF64586564;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF64586564 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF93984604;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF93984604 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF81667235;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF81667235 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF59431794;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF59431794 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF50580881;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF50580881 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF96333171;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF96333171 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF82057909;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF82057909 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF37325737;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF37325737 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF5135378;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF5135378 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF65271339;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF65271339 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF91205763;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF91205763 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF20108319;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF20108319 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF97162232;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF97162232 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF61805139;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF61805139 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF47888670;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF47888670 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF18168330;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF18168330 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF24525712;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF24525712 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF58462075;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF58462075 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF214553;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF214553 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF27001280;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF27001280 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF28195481;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF28195481 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF45261104;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF45261104 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF55383168;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF55383168 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF39673353;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF39673353 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF85320208;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF85320208 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF19123070;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF19123070 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF59707805;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF59707805 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF55746824;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF55746824 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF8647453;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF8647453 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF6105963;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF6105963 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF20107105;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF20107105 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF45373787;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF45373787 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF19127037;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF19127037 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF63496547;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF63496547 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF69857446;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF69857446 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF43513812;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF43513812 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF59933881;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF59933881 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF71998443;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF71998443 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF95417567;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF95417567 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF31286803;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF31286803 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF95594099;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF95594099 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF45315143;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF45315143 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF74718392;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF74718392 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF1582456;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF1582456 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF24008319;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF24008319 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF37418855;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF37418855 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF50488571;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF50488571 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF95023732;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF95023732 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF45438902;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF45438902 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF32441954;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF32441954 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF46331488;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF46331488 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF42126046;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF42126046 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF78731096;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF78731096 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF26631566;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF26631566 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF12963449;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF12963449 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF19325460;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF19325460 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF38601436;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF38601436 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF41993883;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF41993883 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF74111585;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF74111585 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF31457812;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF31457812 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF36625367;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF36625367 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF26311086;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF26311086 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF28678284;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF28678284 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF99029415;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF99029415 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF45164235;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF45164235 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF45831976;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF45831976 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF981282;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF981282 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF33665685;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF33665685 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF91947692;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF91947692 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF4374859;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF4374859 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF58234449;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF58234449 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF52527268;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF52527268 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF63044507;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF63044507 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF68927750;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF68927750 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF31407181;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF31407181 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF82880338;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF82880338 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF70542712;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF70542712 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF53800713;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF53800713 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF15665035;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF15665035 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF47901354;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF47901354 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF68634498;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF68634498 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF64684072;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF64684072 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF10307922;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF10307922 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF76205499;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF76205499 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF59774474;     rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF59774474 = rgVRaSkMngjVCqpJgeXIHpHJPmUosiexDEpbjyFKOkqiKwTpJEXIFyzBYLCoxvUEBlRxAOZhYNtWyZzTSffnLWsF77652008;}
// Junk Finished
