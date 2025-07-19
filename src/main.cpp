#include "defines.h"
#include <windows.h>

#include "core/app/application.h"

int APIENTRY WinMain(
    HINSTANCE h_instance,
    HINSTANCE /*hPrevInstance*/,
    LPSTR     /*lpCmdLine*/,
    int       /*n_cmd_show*/)
{
    #if NTDDI_VERSION >= NTDDI_WINBLUE
        ::SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    #else
        ::SetProcessDPIAware();
    #endif

    CoreApplication app(h_instance);
    return app.run();
}