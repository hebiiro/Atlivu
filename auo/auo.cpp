#include "pch.h"
#include "auo.h"
#include "Plugin.h"
#include "Common/Tracer2.h"

//--------------------------------------------------------------------

using namespace Output;

//--------------------------------------------------------------------

HWND g_hostWindow = 0;
HANDLE g_pipe = 0;
PluginPtr g_plugin;
MediaInfo g_mediaInfo = {};
std::vector<BYTE> g_videoBuffer;
std::vector<BYTE> g_audioBuffer;

//--------------------------------------------------------------------

inline DWORD readPipe(void* x, DWORD bufferSize)
{
	return readFile(g_pipe, x, bufferSize);
}

inline DWORD writePipe(const void* x, DWORD bufferSize)
{
	return writeFile(g_pipe, x, bufferSize);
}

//--------------------------------------------------------------------

void loadPlugin()
{
	MY_TRACE(_T("loadPlugin()\n"));

	TCHAR fileName[TextMaxSize] = {};
	readPipe(fileName, sizeof(fileName));
	MY_TRACE_TSTR(fileName);

	PluginPtr plugin(new Plugin());

	int32_t result = plugin->load(fileName);
	if (result) g_plugin = plugin;
	MY_TRACE_INT(result);
	writePipe(&result, sizeof(result));

	MY_TRACE(_T("loadPlugin() end\n"));
}

void unloadPlugin()
{
	MY_TRACE(_T("unloadPlugin()\n"));

	g_plugin = 0;

	int32_t result = TRUE;
	writePipe(&result, sizeof(result));

	MY_TRACE(_T("unloadPlugin() end\n"));
}

void configPlugin()
{
	MY_TRACE(_T("configPlugin()\n"));

	if (g_plugin)
		g_plugin->config(0);

	int32_t result = TRUE;
	writePipe(&result, sizeof(result));

	MY_TRACE(_T("configPlugin() end\n"));
}

//	DIB形式の画像データを取得します。
//	frame	: フレーム番号
//	format	: 画像フォーマット( NULL = RGB24bit / 'Y''U''Y''2' = YUY2 / 'Y''C''4''8' = PIXEL_YC )
//			  ※PIXEL_YC形式 は YUY2フィルタモードでは使用出来ません。
//	戻り値	: データへのポインタ
//			  画像データポインタの内容は次に外部関数を使うかメインに処理を戻すまで有効
void* func_get_video_ex(int frame, DWORD format)
{
	MY_TRACE(_T("func_get_video_ex(%d, 0x%08X)\n"), frame, format);

	int32_t commandId = CommandID::GetVideo;
	writePipe(&commandId, sizeof(commandId));

	writePipe(&frame, sizeof(frame));
	writePipe(&format, sizeof(format));

	int32_t bufferSize = 0;
	readPipe(&bufferSize, sizeof(bufferSize));

	g_videoBuffer.resize(bufferSize);
	readPipe(g_videoBuffer.data(), bufferSize);

	return g_videoBuffer.data();
}

//	DIB形式(RGB24bit)の画像データへのポインタを取得します。
//	frame	: フレーム番号
//	戻り値	: データへのポインタ
//			  画像データポインタの内容は次に外部関数を使うかメインに処理を戻すまで有効
void* func_get_video(int frame)
{
	return func_get_video_ex(frame, 0);
}

//	16bitPCM形式の音声データへのポインタを取得します。
//	start	: 開始サンプル番号
//	length	: 読み込むサンプル数
//	readed	: 読み込まれたサンプル数
//	戻り値	: データへのポインタ
//			  音声データポインタの内容は次に外部関数を使うかメインに処理を戻すまで有効
void* func_get_audio(int start, int length, int *readed)
{
	MY_TRACE(_T("func_get_audio(%d, %d)\n"), start, length);

	int32_t commandId = CommandID::GetAudio;
	writePipe(&commandId, sizeof(commandId));

	writePipe(&start, sizeof(start));
	writePipe(&length, sizeof(length));

	int32_t bufferSize = 0;
	readPipe(&bufferSize, sizeof(bufferSize));

	if (bufferSize > 0)
	{
		g_audioBuffer.resize(bufferSize);
		readPipe(g_audioBuffer.data(), bufferSize);
	}

	int32_t bytesPerBlock = g_mediaInfo.audio_format.wBitsPerSample / 8 * g_mediaInfo.audio_format.nChannels;

	*readed = bufferSize / bytesPerBlock;

	return g_audioBuffer.data();
}

//	中断するか調べます。
//	戻り値	: TRUEなら中断
BOOL func_is_abort()
{
	MY_TRACE(_T("func_is_abort()\n"));

	int32_t commandId = CommandID::IsAbort;
	writePipe(&commandId, sizeof(commandId));

	int32_t result = FALSE;
	readPipe(&result, sizeof(result));

	MY_TRACE(_T("func_is_abort() end %d\n"), result);

	return result;
}

//	残り時間を表示させます。
//	now		: 処理しているフレーム番号
//	total	: 処理する総フレーム数
//	戻り値	: TRUEなら成功
BOOL func_rest_time_disp(int now, int total)
{
	int32_t commandId = CommandID::UpdatePreview;
	writePipe(&commandId, sizeof(commandId));

	writePipe(&now, sizeof(now));
	writePipe(&total, sizeof(total));

	return TRUE;
}

//	フラグを取得します。
//	frame	: フレーム番号
//	戻り値	: フラグ
//  OUTPUT_INFO_FRAME_FLAG_KEYFRAME		: キーフレーム推奨
//  OUTPUT_INFO_FRAME_FLAG_COPYFRAME	: コピーフレーム推奨
int func_get_flag(int frame)
{
	return 0;
}

//	プレビュー画面を更新します。
//	最後にfunc_get_videoで読み込まれたフレームが表示されます。
//	戻り値	: TRUEなら成功
BOOL func_update_preview()
{
	MY_TRACE(_T("func_update_preview()\n"));

	int32_t commandId = CommandID::UpdatePreview;
	writePipe(&commandId, sizeof(commandId));

	MY_TRACE(_T("func_update_preview() end\n"));

	return TRUE;
}

void saveFile()
{
	MY_TRACE(_T("saveFile()\n"));

	// ホストからファイル名を読み込む。
	TCHAR fileName[TextMaxSize] = {};
	readPipe(fileName, sizeof(fileName));

	// ホストからメディア情報を読み込む。
	readPipe(&g_mediaInfo, sizeof(g_mediaInfo));

	if (!g_plugin)
		return;

	OUTPUT_PLUGIN_TABLE* op = g_plugin->getOutputPlugin();

	_bstr_t fileNameBSTR = fileName;

	OUTPUT_INFO oi = {};
	oi.flag |= OUTPUT_INFO_FLAG_VIDEO;
	oi.flag |= OUTPUT_INFO_FLAG_AUDIO;
	oi.w = g_mediaInfo.format.biWidth;
	oi.h = g_mediaInfo.format.biHeight;
	oi.rate = g_mediaInfo.rate;
	oi.scale = g_mediaInfo.scale;
	oi.size = oi.w * oi.h * 3;
	oi.audio_rate = g_mediaInfo.audio_format.nSamplesPerSec;
	oi.audio_ch = g_mediaInfo.audio_format.nChannels;
	oi.audio_size = g_mediaInfo.audio_format.nChannels * g_mediaInfo.audio_format.wBitsPerSample / 8;
	oi.savefile = fileNameBSTR;
	oi.func_get_video = func_get_video;
	oi.func_get_audio = func_get_audio;
	oi.func_is_abort = func_is_abort;
	oi.func_rest_time_disp = func_rest_time_disp;
	oi.func_get_flag = func_get_flag;
	oi.func_update_preview = func_update_preview;
	oi.func_get_video_ex = func_get_video_ex;
#if 1
	oi.n = g_mediaInfo.n;
	oi.audio_n = g_mediaInfo.audio_n;
#else
	// test
	int sec = 3;
	oi.n = sec * g_mediaInfo.rate / g_mediaInfo.scale;
	oi.audio_n = sec * oi.audio_rate;
#endif
	BOOL result = op->func_output(&oi);

	MY_TRACE(_T("op->func_output() end %d\n"), result);

	int32_t commandId = CommandID::End;
	writePipe(&commandId, sizeof(commandId));

	MY_TRACE(_T("saveFile() end\n"));
}

BOOL mainLoop()
{
	MY_TRACE(_T("mainLoop()\n"));

	UINT_PTR timerId = ::SetTimer(0, 0, 1000, 0);

	MSG msg = {};
	while (::GetMessage(&msg, 0, 0, 0))
	{
		switch (msg.message)
		{
		case WM_TIMER:
			{
				if (!msg.hwnd && msg.wParam == timerId)
				{
					if (!::IsWindow(g_hostWindow))
					{
						MY_TRACE(_T("ホストウィンドウが無効なのでメインループを終了します\n"));

						return FALSE;
					}
				}

				break;
			}
		case WM_CLOSE:
			{
				if (!msg.hwnd)
				{
					MY_TRACE(_T("WM_CLOSE が投げられたのでメインループを終了します\n"));

					return FALSE;
				}

				break;
			}
		case CommandID::End:
			{
				return TRUE;
			}
		case CommandID::LoadPlugin:
			{
				loadPlugin();

				break;
			}
		case CommandID::UnloadPlugin:
			{
				unloadPlugin();

				break;
			}
		case CommandID::ConfigPlugin:
			{
				configPlugin();

				break;
			}
		case CommandID::SaveFile:
			{
				saveFile();

				break;
			}
		}

		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}

	return TRUE;
}

BOOL onWinMain(LPTSTR cmdLine)
{
	MY_TRACE(_T("onWinMain(%s)\n"), cmdLine);

	LPTSTR commandLine = ::GetCommandLine();
	MY_TRACE_TSTR(commandLine);
	g_hostWindow = (HWND)_tcstoul(commandLine, 0, 0);
	MY_TRACE_HWND(g_hostWindow);

	DWORD pid = 0;
	DWORD tid = ::GetWindowThreadProcessId(g_hostWindow, &pid);
	MY_TRACE_INT(pid);
	MY_TRACE_INT(tid);

	TCHAR pipeName[MAX_PATH] = {};
	::StringCbPrintf(pipeName, sizeof(pipeName), _T("\\\\.\\pipe\\auo%d"), tid);
	MY_TRACE_TSTR(pipeName);

	g_pipe = ::CreateFile(pipeName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
	MY_TRACE_HEX(g_pipe);

	DWORD dwMode = PIPE_READMODE_BYTE;
	BOOL fSuccess = ::SetNamedPipeHandleState(g_pipe, &dwMode, NULL, NULL);
	MY_TRACE_HEX(dwMode);
	MY_TRACE_INT(fSuccess);

	mainLoop();

	::CloseHandle(g_pipe);

	return TRUE;
}

int APIENTRY _tWinMain(HINSTANCE instance, HINSTANCE prevInstance, LPTSTR cmdLine, int cmdShow)
{
	::CoInitialize(0);

	_tsetlocale(LC_CTYPE, _T(""));

	trace_init(0, 0, TRUE);

	onWinMain(cmdLine);

	trace_term();

	::CoUninitialize();

	return 0;
}

//--------------------------------------------------------------------
