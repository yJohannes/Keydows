// https://stackoverflow.com/questions/29091028/windows-api-write-to-screen-as-on-screen-display
// https://forums.unrealengine.com/t/how-do-i-include-winuser-h-identifier-wm_touch-is-undefined-dword-is-ambiguous/69946/5
#pragma once

#include "defines.h"
#include <SDKDDKVer.h>
#include <windows.h>
#include <shellscalingapi.h>

#include <iostream>
#include <filesystem>
#include <fstream>
#include <thread>
#include <vector>
#include <unordered_map>
#include <cwchar>

#include "event_types.h"
#include "tool_interface.h"
#include "hotkeys/hotkey_manager.h"
#include "input/ll_input.h"
#include "input/hl_input.h"

#include "json.hpp"

typedef ITool* (*CreateToolFn)();
typedef void (*DestroyToolFn)(ITool*);

class CoreApplication
{
private:
    static HWND h_wnd;
    static WNDCLASSEXW m_wcex;

    struct ToolStruct
    {
        HMODULE h_dll;
        ITool* tool_ptr;
        std::wstring tool_name;
        CreateToolFn create_tool = nullptr;
        DestroyToolFn destroy_tool = nullptr;
    };

    static std::vector<ToolStruct> m_loaded_tools;
    static std::unordered_map<Event, int> m_hotkey_ids;

public:
    CoreApplication(HINSTANCE h_instance);
    ~CoreApplication() = default;

    static int run();
    static void shutdown();

    static void load_tool(const std::wstring& dll_path, const std::wstring& tool_name);
    static void load_tools();
    static void unload_tools();

private:
    static LRESULT CALLBACK wnd_proc(HWND h_wnd, UINT message, WPARAM w_param, LPARAM l_param);
    static void process_hotkey(WPARAM w_param);
    static void load_config();
};