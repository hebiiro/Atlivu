#include "pch.h"
#include "AtlivuOutput.h"
#include "MainFrame.h"
#include "Common/Tracer2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CAtlivuOutputApp theApp;

BEGIN_MESSAGE_MAP(CAtlivuOutputApp, CWinApp)
END_MESSAGE_MAP()

CAtlivuOutputApp::CAtlivuOutputApp() noexcept
{
	_tsetlocale(LC_CTYPE, _T(""));

	trace_init(0, 0, TRUE);
}

CAtlivuOutputApp::~CAtlivuOutputApp()
{
	trace_term();
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

	theApp.postMessage(WM_GET_VIDEO, 0, 0); // 読み込み待ちにさせる。

	GetVideoInput input = { frame, format };
	DWORD write1 = writeFile(theApp.m_pipe, &input, sizeof(input)); // ここで読み込み待ちを解除。
	MY_TRACE_INT(write1);

	DWORD bufferSize = 0;
	DWORD read1 = readFile(theApp.m_pipe, &bufferSize, sizeof(bufferSize));
	MY_TRACE_INT(read1);
	MY_TRACE_INT(bufferSize);

	theApp.m_videoBuffer.resize(bufferSize);
	DWORD read2 = readFile(theApp.m_pipe, theApp.m_videoBuffer.data(), bufferSize);
	MY_TRACE_INT(read2);

	GetVideoOutput* output = (GetVideoOutput*)theApp.m_videoBuffer.data();

	return output->buffer;
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

	theApp.postMessage(WM_GET_AUDIO, 0, 0); // 読み込み待ちにさせる。

	GetAudioInput input = { start, length };
	DWORD write1 = writeFile(theApp.m_pipe, &input, sizeof(input)); // ここで読み込み待ちを解除。
	MY_TRACE_INT(write1);

	DWORD bufferSize = 0;
	DWORD read1 = readFile(theApp.m_pipe, &bufferSize, sizeof(bufferSize));
	MY_TRACE_INT(read1);
	MY_TRACE_INT(bufferSize);

	theApp.m_audioBuffer.resize(bufferSize);
	DWORD read2 = readFile(theApp.m_pipe, theApp.m_audioBuffer.data(), bufferSize);
	MY_TRACE_INT(read2);

	GetAudioOutput* output = (GetAudioOutput*)theApp.m_audioBuffer.data();

	*readed = output->readed;

	return output->buffer;
}

//	中断するか調べます。
//	戻り値	: TRUEなら中断
BOOL func_is_abort()
{
	return (BOOL)theApp.sendMessage(WM_IS_ABORT, 0, 0);
}

//	残り時間を表示させます。
//	now		: 処理しているフレーム番号
//	total	: 処理する総フレーム数
//	戻り値	: TRUEなら成功
BOOL func_rest_time_disp(int now, int total)
{
	return (BOOL)theApp.sendMessage(WM_REST_TIME_DISP, now, total);
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
	return (BOOL)theApp.sendMessage(WM_UPDATE_PREVIEW, 0, 0);
}

BOOL CAtlivuOutputApp::save(LPCTSTR fileName, MediaInfo* mediaInfo)
{
	MY_TRACE(_T("CAtlivuOutputApp::save(%s)\n"), fileName);

	m_mediaInfo = *mediaInfo;

	OUTPUT_PLUGIN_TABLE* op = m_outputManager->getOutputPlugin();

	_bstr_t fileNameBSTR = fileName;

	::MessageBoxA(0, fileNameBSTR, "AtlivuOutput", MB_OK);

	OUTPUT_INFO oi = {};
	oi.flag |= OUTPUT_INFO_FLAG_VIDEO;
	oi.flag |= OUTPUT_INFO_FLAG_AUDIO;
	oi.w = mediaInfo->format.biWidth;
	oi.h = mediaInfo->format.biHeight;
	oi.rate = mediaInfo->rate;
	oi.scale = mediaInfo->scale;
	oi.size = oi.w * oi.h * 3;
	oi.audio_rate = mediaInfo->audio_format.nSamplesPerSec;
	oi.audio_ch = mediaInfo->audio_format.nChannels;
	oi.audio_size = mediaInfo->audio_format.nChannels * mediaInfo->audio_format.wBitsPerSample / 8;
	oi.savefile = fileNameBSTR;
	oi.func_get_video = func_get_video;
	oi.func_get_audio = func_get_audio;
	oi.func_is_abort = func_is_abort;
	oi.func_rest_time_disp = func_rest_time_disp;
	oi.func_get_flag = func_get_flag;
	oi.func_update_preview = func_update_preview;
	oi.func_get_video_ex = func_get_video_ex;
#if 1
	oi.n = mediaInfo->n;
	oi.audio_n = mediaInfo->audio_n;
#else
	// test
	int sec = 3;
	oi.n = sec * mediaInfo->rate / mediaInfo->scale;
	oi.audio_n = sec * oi.audio_rate;
#endif
	BOOL result = op->func_output(&oi);
	postMessage(WM_SAVE_FILE_FINISHED, 0, 0);
	return result;
}

BOOL CAtlivuOutputApp::InitInstance()
{
	MY_TRACE(_T("CAtlivuOutputApp::InitInstance()\n"));

	{
		m_hostProcessWindow = (HWND)_tcstoul(::GetCommandLine(), 0, 0);
		MY_TRACE_HWND(m_hostProcessWindow);

		if (!::IsWindow(m_hostProcessWindow))
			return FALSE;

		::GetWindowThreadProcessId(m_hostProcessWindow, &m_hostProcessId);
		MY_TRACE_INT(m_hostProcessId);

		m_hostProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_hostProcessId);
		MY_TRACE_HEX(m_hostProcess);
	}

	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	m_pipe = ::CreateFile(_T("\\\\.\\pipe\\AtlivuOutput"),
		GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
	MY_TRACE_HEX(m_pipe);

	DWORD dwMode = PIPE_READMODE_BYTE;
	BOOL fSuccess = SetNamedPipeHandleState(m_pipe, &dwMode, NULL, NULL);
	MY_TRACE_HEX(dwMode);
	MY_TRACE_INT(fSuccess);

	AfxEnableControlContainer();
	EnableTaskbarInteraction(FALSE);

	CFrameWnd* pFrame = new CMainFrame;
	if (!pFrame) return FALSE;
	m_pMainWnd = pFrame;
	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,
		nullptr, nullptr);

//	pFrame->ShowWindow(SW_SHOW);
//	pFrame->UpdateWindow();

	::PostMessage(m_hostProcessWindow,
		WM_ATLIVU_OUTPUT_INITED, (WPARAM)pFrame->GetSafeHwnd(), 0);

	return TRUE;
}

int CAtlivuOutputApp::ExitInstance()
{
	MY_TRACE(_T("CAtlivuOutputApp::ExitInstance()\n"));

	::CloseHandle(m_pipe), m_pipe = 0;

	AfxOleTerm(FALSE);

	return CWinApp::ExitInstance();
}
