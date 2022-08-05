#pragma once

#include "Resource.h"
#include "MainFrame.h"
#include "VideoProcessor.h"

//--------------------------------------------------------------------

static const UINT WM_VIDEO_PROCESS_SEEK			= WM_APP + 120;
static const UINT WM_VIDEO_PROCESSED_SEEK		= WM_APP + 121;
static const UINT WM_VIDEO_PROCESS_PLAY			= WM_APP + 122;
static const UINT WM_VIDEO_PROCESSED_PLAY		= WM_APP + 123;
static const UINT WM_VIDEO_PROCESSED_PLAY_STOP	= WM_APP + 125;

//--------------------------------------------------------------------

typedef std::shared_ptr<CVideoProcessor> CVideoProcessorPtr;
typedef std::vector<CVideoProcessorPtr> CVideoProcessorArray;

//--------------------------------------------------------------------

class CAtlivuApp : public CWinApp
{
public:

	GdiplusStartupInput m_gdiSI;
	GdiplusStartupOutput m_gdiSO;
	ULONG_PTR m_gdiToken = 0;
	ULONG_PTR m_gdiHookToken = 0;

	PROCESS_INFORMATION m_inputPi = {};
	HANDLE m_inputPipe = 0;

	PROCESS_INFORMATION m_outputPi = {};
	HANDLE m_outputPipe = 0;

	int32_t m_media = 0;
	MediaInfo m_mediaInfo = {};

	CMainFrame m_mainFrame;
	CCriticalSection m_csMedia;

	BOOL m_isPlaying = FALSE;
	BOOL m_isSubThreadProcessing = FALSE;
	DWORD m_startTime = 0;
	LONG m_startFrame = 0;
	LONG m_endFrame = 0;
	LONG m_currentFrame = 0;
	LONG m_totalSkipCount = 0;
	DWORD m_videoTimerId = 0;
	CVideoProcessorPtr m_videoProcessor;

	HWAVEOUT m_waveOut = 0;
	WAVEHDR m_waveHeader[2] = {};
	int m_waveCurrentSample = -1;
	int m_waveCurrentBuffer = 0;

	BOOL m_isSaving = FALSE;
	std::vector<BYTE> m_outputVideoBuffer[2];
	std::vector<BYTE> m_outputAudioBuffer;
	std::vector<BYTE> m_outputVideoRawBufferTemp;

	typedef std::shared_ptr<CImage> CImagePtr;
	CImagePtr m_tempImage;

public:

	CAtlivuApp() noexcept;
	virtual ~CAtlivuApp();

	BOOL createInputProcess();
	BOOL createOutputProcess();
	BOOL initInputPlugin();
	BOOL termInputPlugin();
	BOOL initOutputPlugin();
	BOOL termOutputPlugin();
	BOOL openMedia(LPCTSTR fileName);
	BOOL closeMedia();
	BOOL raw_loadInputPlugin(LPCTSTR fileName);
	BOOL raw_unloadInputPlugin();
	BOOL raw_configInputPlugin();
	int32_t raw_openMedia(LPCTSTR fileName);
	BOOL raw_closeMedia(int32_t media);
	BOOL raw_getMediaInfo(int32_t media, MediaInfo* mediaInfo);
	BOOL raw_readVideo(int32_t media, int32_t frame, std::vector<BYTE>& output);
	BOOL raw_readAudio(int32_t media, int32_t start, int32_t length, int32_t* bufferLength, std::vector<BYTE>& output);
	BOOL raw_loadOutputPlugin(LPCTSTR fileName);
	BOOL raw_unloadOutputPlugin();
	BOOL raw_configOutputPlugin();
	BOOL raw_saveFile(LPCTSTR fileName);

	CString getIniPath()
	{
		TCHAR path[MAX_PATH] = {};
		::GetModuleFileName(0, path, MAX_PATH);
		::PathRenameExtension(path, _T(".ini"));
		return path;
	}

	LRESULT sendMessage(UINT message, WPARAM wParam, LPARAM lParam)
	{
		return ::SendMessage(m_mainFrame, message, wParam, lParam);
	}

	BOOL postMessage(UINT message, WPARAM wParam, LPARAM lParam)
	{
		return ::PostMessage(m_mainFrame, message, wParam, lParam);
	}

	void seek(int frame);
	void play();
	void stop();
	LONG frameToTime(LONG frame);
	LONG timeToFrame(LONG time);
	int getRGBBufferSize();
	void getRGBBuffer(BYTE* src, BYTE* dst);
	int getYUY2BufferSize();
	void getYUY2Buffer(BYTE* src, BYTE* dst);
	void OnVideoProcessedSeek(CVideoProcessor* videoProcessor);
	void OnVideoProcessedPlay(CVideoProcessor* videoProcessor);
	void OnVideoProcessedPlayStop();
	void OnVideoTimerProc(UINT timerId);
	static void CALLBACK videoTimerProc(UINT timerId, UINT message, DWORD_PTR user, DWORD_PTR, DWORD_PTR);

	void audioInit();
	void audioPlay();
	void audioStop();
	void audioTerm();
	void audioWrite(WAVEHDR& waveHeader);

	void CALLBACK OnWaveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR wParam, DWORD_PTR lParam);
	static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR wParam, DWORD_PTR lParam);

	BOOL save(LPCTSTR fileName);
	BOOL abort();
	UINT outputPipeLoop(LPVOID param);
	UINT OnOutputThreadProc(LPVOID param);
	static UINT outputThreadProc(LPVOID param);
	BOOL OnIsAbort();
	BOOL OnRestTimeDisp(int now, int total);
	BOOL OnUpdatePreview();
	BOOL OnGetVideo(int32_t frame, DWORD format);
	BOOL OnGetAudio(int32_t start, int32_t length);
	BOOL OnCreateOutputVideo(int frame);

	virtual BOOL InitInstance();
	virtual int ExitInstance();
	DECLARE_MESSAGE_MAP()
};

//--------------------------------------------------------------------

extern CAtlivuApp theApp;

//--------------------------------------------------------------------
