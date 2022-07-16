#pragma once

//--------------------------------------------------------------------

typedef INPUT_PLUGIN_TABLE* (CALLBACK* Type_GetInputPluginTable)();

class InputManager
{
public:

	HINSTANCE m_aui = 0;
	INPUT_PLUGIN_TABLE* m_inputPluginTable = 0;

public:

	InputManager(LPCTSTR fileName);
	~InputManager();

	INPUT_PLUGIN_TABLE* getInputPlugin() const;
};

typedef std::shared_ptr<InputManager> InputManagerPtr;

extern InputManager* getInputManager();

//--------------------------------------------------------------------

class Media
{
private:

	CString m_fileName;
	INPUT_HANDLE m_inputHandle = 0;
	INPUT_INFO m_inputInfo = {};
	MediaInfo m_mediaInfo = {};
	std::vector<BYTE> m_videoBuffer;
	std::vector<BYTE> m_audioBuffer;

public:

	Media(LPCTSTR fileName);
	~Media();

	LPCTSTR getFileName();
	INPUT_HANDLE getInputHandle();
	INPUT_INFO* getInputInfo();
	MediaInfo* getMediaInfo();
	ReadVideoOutput* readVideo(ReadVideoInput* input);
	ReadAudioOutput* readAudio(ReadAudioInput* input);
};

typedef std::shared_ptr<Media> MediaPtr;
typedef std::map<Media*, MediaPtr> MediaMap;

//--------------------------------------------------------------------
