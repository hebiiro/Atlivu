#pragma once

#define VC_EXTRALEAN
#include "targetver.h"
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC のコアおよび標準コンポーネント
#include <afxext.h>         // MFC の拡張部分
#include <afxdisp.h>        // MFC オートメーション クラス
#include <afxdtctl.h>           // MFC の Internet Explorer 4 コモン コントロール サポート
#include <afxcmn.h>             // MFC の Windows コモン コントロール サポート
#include <atlimage.h>
#include <afxmt.h>

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;

#include <strsafe.h>

#include <algorithm>
#include <memory>
#include <vector>
#include <map>

#include "Common/Tracer.h"
#include "Common/WinUtility.h"
#include "../Atlivu/Common.h"

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif
