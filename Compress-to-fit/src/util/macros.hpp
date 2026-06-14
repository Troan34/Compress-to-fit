#pragma once

#ifdef _WIN32
#include <windows.h>
#define WIN_CALL(x) x
#else
#define WIN_CALL(x)
#endif