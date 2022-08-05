#pragma once

#include "auo.h"

//--------------------------------------------------------------------
namespace Output {
//--------------------------------------------------------------------

class Plugin;
class Media;

typedef std::shared_ptr<Plugin> PluginPtr;
typedef std::vector<PluginPtr> PluginArray;

typedef std::shared_ptr<Media> MediaPtr;
typedef std::map<Media*, MediaPtr> MediaMap;

//--------------------------------------------------------------------

typedef OUTPUT_PLUGIN_TABLE* (CALLBACK* Type_GetOutputPluginTable)();

class Plugin
{
public:

	HINSTANCE m_auo = 0;
	OUTPUT_PLUGIN_TABLE* m_outputPluginTable = 0;

public:

	Plugin();
	~Plugin();

	BOOL load(LPCTSTR fileName);
	BOOL unload();
	BOOL config(HWND hwnd);

	OUTPUT_PLUGIN_TABLE* getOutputPlugin() const;
};

//--------------------------------------------------------------------
} // namespace Output
//--------------------------------------------------------------------
