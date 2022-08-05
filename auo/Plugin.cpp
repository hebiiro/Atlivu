#include "pch.h"
#include "Plugin.h"

//--------------------------------------------------------------------
namespace Output {
//--------------------------------------------------------------------

Plugin::Plugin()
{
	MY_TRACE(_T("Plugin::Plugin()\n"));
}

Plugin::~Plugin()
{
	MY_TRACE(_T("Plugin::~Plugin()\n"));
}

BOOL Plugin::load(LPCTSTR fileName)
{
	MY_TRACE(_T("Plugin::load(%s)\n"), fileName);

	// プラグイン DLL をロードする。
	m_auo = ::LoadLibrary(fileName);
	if (!m_auo)
		return FALSE;

	// プラグイン関数を取得する。
	Type_GetOutputPluginTable GetOutputPluginTable =
		(Type_GetOutputPluginTable)::GetProcAddress(m_auo, "GetOutputPluginTable");
	if (!GetOutputPluginTable)
		return FALSE;

	// プラグインテーブルを取得する。
	m_outputPluginTable = GetOutputPluginTable();
	if (!m_outputPluginTable)
		return FALSE;

	// プラグインを初期化する。
	if (m_outputPluginTable->func_init)
	{
		BOOL result = m_outputPluginTable->func_init();
	}

	if (m_outputPluginTable->func_config_set)
	{
		TCHAR fileName[MAX_PATH] = {};
		::GetModuleFileName(m_auo, fileName, MAX_PATH);
		::PathRenameExtension(fileName, _T(".bin"));
		MY_TRACE_TSTR(fileName);

		Handle file = createFileForRead(fileName);
		if (file != INVALID_HANDLE_VALUE)
		{
			std::vector<BYTE> data((int)::GetFileSize(file, 0));
			readFile(file, data.data(), data.size());
			m_outputPluginTable->func_config_set(data.data(), data.size());
		}
	}

	return TRUE;
}

BOOL Plugin::unload()
{
	MY_TRACE(_T("Plugin::unload()\n"));

	if (!m_auo)
		return FALSE;

	// プラグインの後始末をする。
	if (m_outputPluginTable && m_outputPluginTable->func_exit)
	{
		BOOL result = m_outputPluginTable->func_exit();
	}

	// プラグイン DLL をアンロードする。
	::FreeLibrary(m_auo), m_auo = 0;

	return TRUE;
}

BOOL Plugin::config(HWND hwnd)
{
	MY_TRACE(_T("OutputManager::config(0x%08X)\n"), hwnd);

	if (!m_outputPluginTable->func_config)
		return FALSE;

	if (!m_outputPluginTable->func_config(hwnd, m_auo))
		return FALSE;

	int size = m_outputPluginTable->func_config_get(0, 0);

	std::vector<BYTE> data(size);
	m_outputPluginTable->func_config_get(data.data(), size);

	TCHAR fileName[MAX_PATH] = {};
	::GetModuleFileName(m_auo, fileName, MAX_PATH);
	::PathRenameExtension(fileName, _T(".bin"));
	MY_TRACE_TSTR(fileName);

	Handle file = createFileForWrite(fileName);
	if (file == INVALID_HANDLE_VALUE)
		return FALSE;

	writeFile(file, data.data(), size);

	return TRUE;
}

OUTPUT_PLUGIN_TABLE* Plugin::getOutputPlugin() const
{
	return m_outputPluginTable;
}

//--------------------------------------------------------------------
} // namespace Output
//--------------------------------------------------------------------
