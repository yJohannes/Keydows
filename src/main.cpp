#include <windows.h>
#include "core/application.h"

int APIENTRY WinMain(
    HINSTANCE h_instance,
    HINSTANCE /*hPrevInstance*/,
    LPSTR     /*lpCmdLine*/,
    int       /*n_cmd_show*/)
{
    Application app(h_instance);
    return app.run();
}