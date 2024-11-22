#include <iostream>
#include <string>
#include <thread>

#include <windows.h>

HWND last_focused_window = NULL;

void check_window_focus()
{
    while (true)
    {
        HWND current_focused_window = GetForegroundWindow();
        if (current_focused_window != last_focused_window)
        {
            last_focused_window = current_focused_window;
            std::cout << "RAHH";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nc-winuser-wineventproc
//
void CALLBACK win_event_proc
(
    HWINEVENTHOOK event_hook,
    DWORD dw_event,
    HWND hwnd,
    LONG object_id,
    LONG child_id,
    DWORD dw_event_thread,
    DWORD dw_ms_event_time
)
{
    if (dw_event == EVENT_SYSTEM_FOREGROUND)
    {
        std::cout << "WINDOW CHANGE\n";
    }
}

int main()
{
    // Set up the hook to listen for foreground window changes
    // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwineventhook
    //
    HWINEVENTHOOK event_hook = SetWinEventHook
    (
        EVENT_SYSTEM_FOREGROUND,    // Event to listen for
        EVENT_SYSTEM_FOREGROUND,    // Only care about this event
        NULL,                       // No module to inject
        win_event_proc,             // Callback procedure
        0,                          // Process ID 
        0,                          // Thread ID
        WINEVENT_OUTOFCONTEXT       // Listen for events in the background
    );

    if (event_hook == NULL)
    {
        std::cerr << "Failed to set event hook!" << std::endl;
        return -1;
    }

    MSG message;
    while (GetMessage(&message, NULL, 0, 0))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    UnhookWinEvent(event_hook);
}
    // std::string window_name = "SpeedCrunch";

    // std::wstring wide_window_name(window_name.begin(), window_name.end());

    // HWND hwnd = FindWindowW(NULL, wide_window_name.c_str());
    // if (hwnd)
    // {
    //     std::cout << "JOO";
    //     MoveWindow(hwnd, 0, 0, 500, 500, TRUE);
    // }

    // std::thread focus_thread(check_window_focus);

    // focus_thread.join();

// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-movewindow
