#define STRICT 1
#define WIN32_LEAN_AND_MEAN
#define NTDDI_VERSION NTDDI_WINBLUE
#include <Windows.h>

#include "core/application.hpp"

int APIENTRY WinMain(
    HINSTANCE hInstance,
    HINSTANCE /*hPrevInstance*/,
    LPSTR    /*lpCmdLine*/,
    int       nCmdShow
)
{
    Application app(hInstance, nCmdShow);
    return app.run();
}