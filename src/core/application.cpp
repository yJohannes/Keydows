#include "defines.h"
#include "application.h"

WNDCLASSEXW CoreApplication::m_wcex;
HWND CoreApplication::h_wnd = nullptr;

std::vector<CoreApplication::ToolStruct> CoreApplication::m_tools;

std::unordered_map<int, int> CoreApplication::m_hotkey_ids;


// Overlay CoreApplication::m_overlay;

CoreApplication::CoreApplication(HINSTANCE h_instance)
{
    const wchar_t class_name[] = L"Keydows";
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
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT, // Transparent to mouse press
        class_name,
        L"Keydows",
        WS_POPUP | WS_VISIBLE,
        0, 0,
        dm.dmPelsWidth, dm.dmPelsHeight,
        NULL, NULL,
        h_instance,
        this
    );
    ::SetLayeredWindowAttributes(h_wnd, RGB(0, 0, 0), 200, LWA_ALPHA | LWA_COLORKEY);

    m_hotkey_ids[QUIT] = HotkeyManager::register_hotkey(h_wnd, MOD_CONTROL | MOD_ALT, L'Q');

    load_config();
    // m_overlay.activate(false);
    load_tool(L"tools\\libsmooth_scroll.dll", L"smooth_scroll");
    load_tool(L"tools\\liboverlay.dll", L"overlay");
}

CoreApplication::~CoreApplication()
{
    unload_tools();
    ::UnregisterClassW(m_wcex.lpszClassName, m_wcex.hInstance);
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
    ::PostQuitMessage(0);
}

void CoreApplication::load_tool(const std::wstring& dll_path, const std::wstring& tool_name)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

    std::cout << "Loading DLL " << converter.to_bytes(tool_name) << "...\n";

    ToolStruct ts;
    ts.h_dll = ::LoadLibraryW(dll_path.c_str());
    
    if (!ts.h_dll)
    {
        std::cerr << "Failed to load DLL!" << std::endl;
        return;
    }
    std::cout << "Loaded DLL!\n";

    ts.create_tool = (CreateToolFn)::GetProcAddress(ts.h_dll, "create_tool");
    ts.destroy_tool = (DestroyToolFn)::GetProcAddress(ts.h_dll, "destroy_tool");

    if (ts.create_tool && ts.destroy_tool)
    {
        std::cout << "Found create/destroy fns!\n";

        ts.tool_ptr = ts.create_tool();
        m_tools.push_back(ts);

        std::cout << "Created tool!\n";
        ts.tool_ptr->activate(true);
        std::thread(&ITool::run, ts.tool_ptr).detach();
    }
}

void CoreApplication::unload_tools()
{
    for (auto& ts : m_tools)
    {
        ts.destroy_tool(ts.tool_ptr);
        ::FreeLibrary(ts.h_dll);
    }
}

void CoreApplication::load_config()
{
    // std::ifstream config_file("../config.json");
    
    // if (!config_file.is_open())
    {
        // std::cerr << "Failed to open config.json" << std::endl;
        // m_overlay.set_size(::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN));
        // m_overlay.set_resolution(24, 19);
        // return;
    }

    // json json;
    // config_file >> json;

    // Overlay
    // {
    //     auto overlay = json.at("overlay");

    //     auto resolution = overlay.at("resolution");
    //     int x = resolution.at(0);
    //     int y = resolution.at(1);

    //     m_overlay.set_size(::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN));
    //     m_overlay.set_resolution(x, y);
        
    //     std::string charset = overlay.at("charset");
    //     std::string dir_charset = overlay.at("click_direction_charset");
        
    //     m_overlay.set_charset(std::wstring(charset.begin(), charset.end()).c_str());
    //     m_overlay.set_click_direction_charset(std::wstring(dir_charset.begin(), dir_charset.end()).c_str());

    //     // Hotkeys
    //     auto hk = overlay.at("hotkeys");
    //     auto activate = hk.at("activate");
    //     hotkey::register_hotkey(h_wnd, Hotkeys::OVERLAY, activate.at("mod"), activate.at("key"));
    // }
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
    if (w_param == m_hotkey_ids[QUIT])
    {
        ::DestroyWindow(h_wnd);   // Send WM_DESTROY message
    }
}