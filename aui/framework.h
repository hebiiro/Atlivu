#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include <comdef.h>

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <locale.h>
#include <strsafe.h>
#include <stdint.h>

#include <memory>
#include <vector>
#include <map>

#include "AviUtl/aviutl_plugin_sdk/input.h"
#include "Common/Tracer.h"
#include "Common/WinUtility.h"
#include "../Common/auio.h"
