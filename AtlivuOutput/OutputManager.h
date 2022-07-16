#pragma once

//--------------------------------------------------------------------

typedef OUTPUT_PLUGIN_TABLE* (CALLBACK* Type_GetOutputPluginTable)();

class OutputManager
{
public:

	HINSTANCE m_auo = 0;
	OUTPUT_PLUGIN_TABLE* m_outputPluginTable = 0;

public:

	OutputManager(LPCTSTR fileName);
	~OutputManager();

	OUTPUT_PLUGIN_TABLE* getOutputPlugin() const;

	BOOL config(HWND hwnd);
};

typedef std::shared_ptr<OutputManager> OutputManagerPtr;

//--------------------------------------------------------------------
