#pragma once

#include "Resource.h"
#include "MainFrame.h"
#include "VideoProcessor.h"

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
	HWND m_inputWindow = 0;
	HANDLE m_inputPipe = 0;

	PROCESS_INFORMATION m_outputPi = {};
	HWND m_outputWindow = 0;
	HANDLE m_outputPipe = 0;

	int32_t m_media = 0;
	MediaInfo m_mediaInfo = {};

	CMainFrame m_mainFrame;

	BOOL m_isPlaying = FALSE;
	BOOL m_isProcessing = FALSE;
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
	BOOL openMedia(LPCTSTR fileName);
	BOOL closeMedia();
	BOOL raw_loadInputPlugin(LPCTSTR fileName);
	BOOL raw_unloadInputPlugin();
	BOOL raw_configInputPlugin();
	int32_t raw_openMedia(LPCTSTR fileName);
	BOOL raw_closeMedia(int32_t media);
	BOOL raw_getMediaInfo(int32_t media, MediaInfo* mediaInfo);
	BOOL raw_readVideo(int32_t media, int32_t frame, std::vector<BYTE>& output);
	BOOL raw_readAudio(int32_t media, int32_t start, int32_t length, std::vector<BYTE>& output);
	BOOL raw_loadOutputPlugin(LPCTSTR fileName);
	BOOL raw_unloadOutputPlugin();
	BOOL raw_configOutputPlugin();
	BOOL raw_saveFile(LPCTSTR fileName);

	LRESULT sendInputMessage(UINT message, WPARAM wParam, LPARAM lParam)
	{
		return ::SendMessage(m_inputWindow, message, wParam, lParam);
	}

	BOOL postInputMessage(UINT message, WPARAM wParam, LPARAM lParam)
	{
		return ::PostMessage(m_inputWindow, message, wParam, lParam);
	}

	LRESULT sendOutputMessage(UINT message, WPARAM wParam, LPARAM lParam)
	{
		return ::SendMessage(m_outputWindow, message, wParam, lParam);
	}

	BOOL postOutputMessage(UINT message, WPARAM wParam, LPARAM lParam)
	{
		return ::PostMessage(m_outputWindow, message, wParam, lParam);
	}

	BOOL readInputProcessMemory(void* address, void* buffer, SIZE_T bufferSize)
	{
		return ::ReadProcessMemory(m_inputPi.hProcess, address, buffer, bufferSize, 0);
	}

	BOOL writeInputProcessMemory(void* address, void* buffer, SIZE_T bufferSize)
	{
		return ::WriteProcessMemory(m_inputPi.hProcess, address, buffer, bufferSize, 0);
	}

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
	BOOL OnIsAbort();
	BOOL OnRestTimeDisp(int now, int total);
	BOOL OnUpdatePreview();
	BOOL OnGetVideo();
	BOOL OnGetAudio();
	BOOL OnSaveFileFinished();
	BOOL OnCreateOutputVideo(int frame);

	virtual BOOL InitInstance();
	virtual int ExitInstance();
	DECLARE_MESSAGE_MAP()
};

//--------------------------------------------------------------------

extern CAtlivuApp theApp;

//--------------------------------------------------------------------
