#pragma once

#define STRICT 1
#define WIN32_LEAN_AND_MEAN
#ifdef NTDDI_VERSION
    #undef NTDDI_VERSION
#endif
#define NTDDI_VERSION NTDDI_WINBLUE

#define NULL_CHAR 0
