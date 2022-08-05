#include "pch.h"
#include "aui.h"
#include "Plugin.h"
#include "Common/Tracer2.h"

//--------------------------------------------------------------------

using namespace Input;

//--------------------------------------------------------------------

HWND g_hostWindow = 0;
HANDLE g_pipe = 0;
PluginArray g_pluginArray;
MediaMap g_mediaMap;

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

void addMedia(MediaPtr media)
{
	g_mediaMap[media.get()] = media;
}

void eraseMedia(Media* mediaPointer)
{
	g_mediaMap.erase(mediaPointer);
}

MediaPtr getMedia(Media* mediaPointer)
{
	auto it = g_mediaMap.find(mediaPointer);
	if (it == g_mediaMap.end())
		return 0;
	else
		return it->second;
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
	if (result) g_pluginArray.push_back(plugin);
	MY_TRACE_INT(result);
	writePipe(&result, sizeof(result));

	MY_TRACE(_T("loadPlugin() end\n"));
}

void unloadPlugin()
{
	MY_TRACE(_T("unloadPlugin()\n"));

	for (auto a : g_mediaMap)
		a.second->close();

	for (auto plugin : g_pluginArray)
		plugin->unload();

	g_mediaMap.clear();
	g_pluginArray.clear();

	int32_t result = TRUE;
	writePipe(&result, sizeof(result));

	MY_TRACE(_T("unloadPlugin() end\n"));
}

void configPlugin()
{
}

void openMedia()
{
	MY_TRACE(_T("openMedia()\n"));

	// サーバーからファイル名を読み込む。
	TCHAR fileName[TextMaxSize] = {};
	readPipe(fileName, sizeof(fileName));

	for (auto plugin : g_pluginArray)
	{
		// メディアを作成する。
		MediaPtr media(new Media());

		// メディアファイルを開く。
		if (!media->open(plugin, fileName))
			continue;

		// メディアマップに追加する。
		addMedia(media);

		// サーバーに結果 (メディアのポインタ) を書き込む。
		int32_t result = (int32_t)media.get();
		writePipe(&result, sizeof(result));

		MY_TRACE(_T("openMedia() succeeded\n"));

		return;
	}

	// サーバーに結果 (メディアのポインタ) を書き込む。
	int32_t result = 0;
	writePipe(&result, sizeof(result));

	MY_TRACE(_T("openMedia() failed\n"));
}

void closeMedia()
{
	MY_TRACE(_T("closeMedia()\n"));

	// サーバーからメディアのポインタを読み込む。
	Media* mediaPointer = 0;
	readPipe(&mediaPointer, sizeof(mediaPointer));

	// ポインタから共有ポインタを取得する。
	MediaPtr media = getMedia(mediaPointer);

	if (!media)
	{
		int32_t result = FALSE;
		writePipe(&result, sizeof(result));

		MY_TRACE(_T("closeMedia() failed\n"));

		return;
	}

	// メディアファイルを閉じる。
	if (!media->close())
	{
		int32_t result = FALSE;
		writePipe(&result, sizeof(result));

		MY_TRACE(_T("closeMedia() failed\n"));

		return;
	}

	// メディアマップから削除する。
	eraseMedia(mediaPointer);

	int32_t result = TRUE;
	writePipe(&result, sizeof(result));

	MY_TRACE(_T("closeMedia() succeeded\n"));
}

void getMediaInfo()
{
	MY_TRACE(_T("getMediaInfo()\n"));

	// サーバーからメディアのポインタを読み込む。
	Media* mediaPointer = 0;
	readPipe(&mediaPointer, sizeof(mediaPointer));

	// ポインタから共有ポインタを取得する。
	MediaPtr media = getMedia(mediaPointer);

	if (!media)
	{
		MediaInfo result = {};
		writePipe(&result, sizeof(result));

		MY_TRACE(_T("getMediaInfo() failed\n"));

		return;
	}

	MediaInfo* mediaInfo = media->getMediaInfo();
	writePipe(mediaInfo, sizeof(*mediaInfo));

	MY_TRACE(_T("getMediaInfo() succeeded\n"));
}

void readVideo()
{
	MY_TRACE(_T("readVideo()\n"));

	// サーバーからメディアのポインタを読み込む。
	Media* mediaPointer = 0;
	readPipe(&mediaPointer, sizeof(mediaPointer));
	MY_TRACE_HEX(mediaPointer);

	int32_t frame = 0;
	readPipe(&frame, sizeof(frame));
	MY_TRACE_INT(frame);

	// ポインタから共有ポインタを取得する。
	MediaPtr media = getMedia(mediaPointer);

	if (!media)
	{
		int32_t bufferSize = 0;
		writePipe(&bufferSize, sizeof(bufferSize));

		MY_TRACE(_T("readVideo() failed\n"));

		return;
	}

	int32_t bufferSize = 0;
	void* buffer = media->readVideo(frame, &bufferSize);
	writePipe(&bufferSize, sizeof(bufferSize));

	if (bufferSize > 0)
		writePipe(buffer, bufferSize);

	MY_TRACE(_T("readVideo() succeeded\n"));
}

void readAudio()
{
	MY_TRACE(_T("readAudio()\n"));

	// サーバーからメディアのポインタを読み込む。
	Media* mediaPointer = 0;
	readPipe(&mediaPointer, sizeof(mediaPointer));
	MY_TRACE_HEX(mediaPointer);

	int32_t start = 0;
	readPipe(&start, sizeof(start));
	MY_TRACE_INT(start);

	int32_t length = 0;
	readPipe(&length, sizeof(length));
	MY_TRACE_INT(length);

	// ポインタから共有ポインタを取得する。
	MediaPtr media = getMedia(mediaPointer);

	if (!media)
	{
		int32_t bufferLength = 0;
		writePipe(&bufferLength, sizeof(bufferLength));

		MY_TRACE(_T("readAudio() failed\n"));

		return;
	}

	int32_t bufferLength = 0;
	void* buffer = media->readAudio(start, length, &bufferLength);
	MY_TRACE_INT(bufferLength);
	writePipe(&bufferLength, sizeof(bufferLength));
	MY_TRACE(_T("bufferLength 書き込み完了\n"));

	if (bufferLength > 0)
	{
		int bufferSize = media->calcAudioBufferSize(bufferLength);
		MY_TRACE_INT(bufferSize);
		writePipe(buffer, bufferSize);
		MY_TRACE(_T("buffer 書き込み完了\n"));
	}

	MY_TRACE(_T("readAudio() succeeded\n"));
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
		case CommandID::OpenMedia:
			{
				openMedia();

				break;
			}
		case CommandID::CloseMedia:
			{
				closeMedia();

				break;
			}
		case CommandID::GetMediaInfo:
			{
				getMediaInfo();

				break;
			}
		case CommandID::ReadVideo:
			{
				readVideo();

				break;
			}
		case CommandID::ReadAudio:
			{
				readAudio();

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
	::StringCbPrintf(pipeName, sizeof(pipeName), _T("\\\\.\\pipe\\aui%d"), tid);
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
