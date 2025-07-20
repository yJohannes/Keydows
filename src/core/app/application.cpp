#include "defines.h"
#include "application.h"

WNDCLASSEXW CoreApplication::m_wcex;
HWND CoreApplication::h_wnd = nullptr;

std::vector<CoreApplication::ToolStruct> CoreApplication::m_loaded_tools;
std::unordered_map<Event, int> CoreApplication::m_hotkey_ids;

CoreApplication::CoreApplication(HINSTANCE h_instance)
{
    const wchar_t class_name[] = L"Keydows";
    const wchar_t window_name[] = L"Keydows";

    WNDCLASSEXW m_wcex = {0};
    m_wcex.cbSize         = sizeof(m_wcex);
    m_wcex.style          = CS_HREDRAW | CS_VREDRAW;
    m_wcex.lpfnWndProc    = wnd_proc;
    m_wcex.hInstance      = h_instance;
    m_wcex.hCursor        = ::LoadCursorW(NULL, IDC_ARROW);
    m_wcex.hbrBackground  = (HBRUSH)::GetStockObject(BLACK_BRUSH);
    m_wcex.lpszClassName  = class_name;
    ::RegisterClassExW(&m_wcex);

    DEVMODE dm;
    dm.dmSize = sizeof(dm);
    EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);

    h_wnd = ::CreateWindowExW(
        WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT, // Transparent to mouse press
        class_name,
        window_name,
        WS_POPUP | WS_VISIBLE,
        0, 0,
        dm.dmPelsWidth, dm.dmPelsHeight,
        NULL, NULL,
        h_instance,
        this
    );
    ::SetLayeredWindowAttributes(h_wnd, RGB(0, 0, 0), 200, LWA_ALPHA | LWA_COLORKEY);

    m_hotkey_ids[Event::QUIT] = HotkeyManager::register_hotkey(h_wnd, MOD_CONTROL | MOD_ALT, L'Q');

    load_config();
    load_tools();
}

int CoreApplication::run()
{
    MSG msg;
    while (::GetMessageW(&msg, NULL, 0, 0))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessageW(&msg);
    }

    return (int)msg.wParam;
}

void CoreApplication::shutdown()
{
    LLInput::detach_hooks();
    HotkeyManager::unregister_all_hotkeys();

    unload_tools();

    ::PostQuitMessage(0);
    ::UnregisterClassW(m_wcex.lpszClassName, m_wcex.hInstance);
}

void CoreApplication::load_tool(const std::wstring& dll_path, const std::wstring& tool_name)
{
    // std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    // converter.to_bytes(tool_name)
    std::wcout << L"Loading " << tool_name << "...\n";

    ToolStruct ts;
    ts.h_dll = ::LoadLibraryW(dll_path.c_str());
    
    if (!ts.h_dll)
    {
        std::wcerr << L"Failed to load DLL!" << std::endl;
        return;
    }
    std::wcout << L"Loaded DLL!\n";

    ts.create_tool = (CreateToolFn)::GetProcAddress(ts.h_dll, "create_tool");
    ts.destroy_tool = (DestroyToolFn)::GetProcAddress(ts.h_dll, "destroy_tool");

    if (ts.create_tool && ts.destroy_tool)
    {
        ts.tool_ptr = ts.create_tool();
        m_loaded_tools.push_back(ts);

        ts.tool_ptr->toggle(true);
        std::thread(&ITool::run, ts.tool_ptr).detach();
    }
}

void CoreApplication::load_tools()
{
    wchar_t path_buffer[MAX_PATH];
    ::GetModuleFileName(nullptr, path_buffer, MAX_PATH);
    auto exe_path = std::filesystem::path(path_buffer).parent_path();

    for (const auto& entry : std::filesystem::directory_iterator(exe_path / L"tools"))
    {
        if (entry.path().extension() == L".dll")
        {
            std::wstring dll_path = entry.path().wstring();
            std::wstring tool_name = entry.path().filename();
            load_tool(dll_path, tool_name);
        }
    }
}

void CoreApplication::unload_tools()
{
    for (auto& ts : m_loaded_tools)
    {
        ts.destroy_tool(ts.tool_ptr);
        ::FreeLibrary(ts.h_dll);
    }
}

void CoreApplication::load_config()
{
    // std::ifstream config_file("../config.json");
    
    // json json;
    // config_file >> json;
}

LRESULT CALLBACK CoreApplication::wnd_proc(HWND h_wnd, UINT message, WPARAM w_param, LPARAM l_param)
{
    switch (message) {
    case WM_HOTKEY:
        process_hotkey(w_param);
        return 0;

    case WM_DESTROY:
        shutdown();
        return 0;

    default:
        return ::DefWindowProc(h_wnd, message, w_param, l_param);
    }
}

void CoreApplication::process_hotkey(WPARAM w_param)
{
    if (w_param == m_hotkey_ids[Event::QUIT])
    {
        ::DestroyWindow(h_wnd);   // Send WM_DESTROY message
    }
}