#pragma once

#include "aui.h"

//--------------------------------------------------------------------
namespace Input {
//--------------------------------------------------------------------

class Plugin;
class Media;

typedef std::shared_ptr<Plugin> PluginPtr;
typedef std::vector<PluginPtr> PluginArray;

typedef std::shared_ptr<Media> MediaPtr;
typedef std::map<Media*, MediaPtr> MediaMap;

//--------------------------------------------------------------------

typedef INPUT_PLUGIN_TABLE* (CALLBACK* Type_GetInputPluginTable)();

class Plugin
{
public:

	HINSTANCE m_aui = 0;
	INPUT_PLUGIN_TABLE* m_inputPluginTable = 0;

public:

	Plugin();
	~Plugin();

	BOOL load(LPCTSTR fileName);
	BOOL unload();

	INPUT_PLUGIN_TABLE* getInputPlugin() const;
};

//--------------------------------------------------------------------

class Media
{
private:

	PluginPtr m_plugin;
	_bstr_t m_fileName;
	INPUT_HANDLE m_inputHandle = 0;
	INPUT_INFO m_inputInfo = {};
	MediaInfo m_mediaInfo = {};
	std::vector<BYTE> m_videoBuffer;
	std::vector<BYTE> m_audioBuffer;

public:

	Media();
	~Media();

	BOOL open(PluginPtr plugin, LPCTSTR fileName);
	BOOL close();

	PluginPtr getPlugin();
	LPCTSTR getFileName();
	INPUT_HANDLE getInputHandle();
	INPUT_INFO* getInputInfo();
	MediaInfo* getMediaInfo();
	int32_t calcAudioBufferSize(int32_t length);
	void* readVideo(int32_t frame, int32_t* bufferSize);
	void* readAudio(int32_t start, int32_t length, int32_t* bufferLength);
};

//--------------------------------------------------------------------
} // namespace Input
//--------------------------------------------------------------------
