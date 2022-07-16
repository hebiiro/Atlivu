#include "pch.h"
#include "Atlivu.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//--------------------------------------------------------------------

BOOL CAtlivuApp::raw_loadInputPlugin(LPCTSTR fileName)
{
	COPYDATASTRUCT cds = {};
	cds.dwData = WM_LOAD_INPUT_PLUGIN;
	cds.cbData = (::lstrlen(fileName) + 1) * sizeof(TCHAR);
	cds.lpData = (void*)fileName;
	return (BOOL)sendInputMessage(WM_COPYDATA, (WPARAM)m_mainFrame.GetSafeHwnd(), (LPARAM)&cds);
}

BOOL CAtlivuApp::raw_unloadInputPlugin()
{
	COPYDATASTRUCT cds = {};
	cds.dwData = WM_UNLOAD_INPUT_PLUGIN;
	cds.cbData = 0;
	cds.lpData = this;
	return (BOOL)sendInputMessage(WM_COPYDATA, (WPARAM)m_mainFrame.GetSafeHwnd(), (LPARAM)&cds);
}

BOOL CAtlivuApp::raw_configInputPlugin()
{
	COPYDATASTRUCT cds = {};
	cds.dwData = WM_CONFIG_INPUT_PLUGIN;
	cds.cbData = 0;
	cds.lpData = this;
	return (BOOL)sendInputMessage(WM_COPYDATA, (WPARAM)m_mainFrame.GetSafeHwnd(), (LPARAM)&cds);
}

int32_t CAtlivuApp::raw_openMedia(LPCTSTR fileName)
{
	COPYDATASTRUCT cds = {};
	cds.dwData = WM_OPEN_MEDIA;
	cds.cbData = (::lstrlen(fileName) + 1) * sizeof(TCHAR);
	cds.lpData = (void*)fileName;
	return (int32_t)sendInputMessage(WM_COPYDATA, (WPARAM)m_mainFrame.GetSafeHwnd(), (LPARAM)&cds);
}

BOOL CAtlivuApp::raw_closeMedia(int32_t media)
{
	COPYDATASTRUCT cds = {};
	cds.dwData = WM_CLOSE_MEDIA;
	cds.cbData = sizeof(media);
	cds.lpData = &media;
	return (BOOL)sendInputMessage(WM_COPYDATA, (WPARAM)m_mainFrame.GetSafeHwnd(), (LPARAM)&cds);
}

BOOL CAtlivuApp::raw_getMediaInfo(int32_t media, MediaInfo* mediaInfo)
{
	COPYDATASTRUCT cds = {};
	cds.dwData = WM_GET_MEDIA_INFO;
	cds.cbData = sizeof(media);
	cds.lpData = &media;
	void* p = (void*)sendInputMessage(WM_COPYDATA, (WPARAM)m_mainFrame.GetSafeHwnd(), (LPARAM)&cds);
	return readInputProcessMemory(p, mediaInfo, sizeof(*mediaInfo));
}

BOOL CAtlivuApp::raw_readVideo(int32_t media, int frame, std::vector<BYTE>& output)
{
	ReadVideoInput input = { media, frame };
	COPYDATASTRUCT cds = {};
	cds.dwData = WM_READ_VIDEO;
	cds.cbData = sizeof(input);
	cds.lpData = &input;
	void* p = (void*)sendInputMessage(WM_COPYDATA, (WPARAM)m_mainFrame.GetSafeHwnd(), (LPARAM)&cds);
	int32_t structSize = 0;
	if (!readInputProcessMemory(p, &structSize, sizeof(structSize))) return FALSE;
	output.resize(structSize);
	return readInputProcessMemory(p, output.data(), output.size());
}

BOOL CAtlivuApp::raw_readAudio(int32_t media, int start, int length, std::vector<BYTE>& output)
{
	ReadAudioInput input = { media, start, length };
	COPYDATASTRUCT cds = {};
	cds.dwData = WM_READ_AUDIO;
	cds.cbData = sizeof(input);
	cds.lpData = &input;
	void* p = (void*)sendInputMessage(WM_COPYDATA, (WPARAM)m_mainFrame.GetSafeHwnd(), (LPARAM)&cds);
	int32_t structSize = 0;
	if (!readInputProcessMemory(p, &structSize, sizeof(structSize))) return FALSE;
	output.resize(structSize);
	return readInputProcessMemory(p, output.data(), output.size());
}

BOOL CAtlivuApp::raw_loadOutputPlugin(LPCTSTR fileName)
{
	LoadOutputPluginInput input = {};
	::StringCbCopy(input.fileName, sizeof(input.fileName), fileName);

	postOutputMessage(WM_LOAD_OUTPUT_PLUGIN, 0, 0);
	writeFile(m_outputPipe, &input, sizeof(input));
	return TRUE;
}

BOOL CAtlivuApp::raw_unloadOutputPlugin()
{
	postOutputMessage(WM_UNLOAD_OUTPUT_PLUGIN, 0, 0);
	return TRUE;
}

BOOL CAtlivuApp::raw_configOutputPlugin()
{
	postOutputMessage(WM_CONFIG_OUTPUT_PLUGIN, 0, 0);
	return TRUE;
}

BOOL CAtlivuApp::raw_saveFile(LPCTSTR fileName)
{
	SaveFileInput input = {};
	::StringCbCopy(input.fileName, sizeof(input.fileName), fileName);
	input.mediaInfo = m_mediaInfo;

	postOutputMessage(WM_SAVE_FILE, 0, 0);
	writeFile(m_outputPipe, &input, sizeof(input));
	return TRUE;
}

//--------------------------------------------------------------------
