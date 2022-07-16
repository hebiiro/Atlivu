#include "pch.h"
#include "OutputManager.h"

//--------------------------------------------------------------------

OutputManager::OutputManager(LPCTSTR fileName)
{
	MY_TRACE(_T("OutputManager::OutputManager(%s)\n"), fileName);

	m_auo = 0;
	m_outputPluginTable = 0;

	{
		// プラグイン DLL をロードする。
		m_auo = ::LoadLibrary(fileName);
		if (!m_auo)
			throw _T("プラグインのロードに失敗しました");

		// プラグイン関数を取得する。
		Type_GetOutputPluginTable GetOutputPluginTable =
			(Type_GetOutputPluginTable)::GetProcAddress(m_auo, "GetOutputPluginTable");
		if (!GetOutputPluginTable)
			throw _T("GetOutputPluginTable() が見つかりませんでした");

		// プラグインテーブルを取得する。
		m_outputPluginTable = GetOutputPluginTable();
		if (!m_outputPluginTable)
			throw _T("プラグインテーブルが無効です");

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

			CFile file;
			if (file.Open(fileName, CFile::modeRead | CFile::typeBinary))
			{
				std::vector<BYTE> data((int)file.GetLength());
				file.Read(data.data(), data.size());
				m_outputPluginTable->func_config_set(data.data(), data.size());
			}
		}
	}
}

OutputManager::~OutputManager()
{
	MY_TRACE(_T("OutputManager::~OutputManager()\n"));

	if (m_auo)
	{
		// プラグインの後始末をする。
		if (m_outputPluginTable && m_outputPluginTable->func_exit)
		{
			BOOL result = m_outputPluginTable->func_exit();
		}

		// プラグイン DLL をアンロードする。
		::FreeLibrary(m_auo), m_auo = 0;
	}
}

OUTPUT_PLUGIN_TABLE* OutputManager::getOutputPlugin() const
{
	return m_outputPluginTable;
}

BOOL OutputManager::config(HWND hwnd)
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

	CFile file;
	if (!file.Open(fileName, CFile::modeWrite | CFile::modeCreate | CFile::typeBinary))
		return FALSE;

	file.Write(data.data(), size);

	return TRUE;
}

//--------------------------------------------------------------------
