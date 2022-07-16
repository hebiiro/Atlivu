#pragma once

//--------------------------------------------------------------------

static const LPCTSTR PROP_NAME_RET_VALUE = _T("Atlivu.RetValue");

static const UINT WM_LOAD_INPUT_PLUGIN			= WM_APP + 1;
static const UINT WM_UNLOAD_INPUT_PLUGIN		= WM_APP + 2;
static const UINT WM_CONFIG_INPUT_PLUGIN		= WM_APP + 3;
static const UINT WM_OPEN_MEDIA					= WM_APP + 4;
static const UINT WM_CLOSE_MEDIA				= WM_APP + 5;
static const UINT WM_GET_MEDIA_INFO				= WM_APP + 6;
static const UINT WM_READ_VIDEO					= WM_APP + 7;
static const UINT WM_READ_AUDIO					= WM_APP + 8;

static const UINT WM_LOAD_OUTPUT_PLUGIN			= WM_APP + 20;
static const UINT WM_UNLOAD_OUTPUT_PLUGIN		= WM_APP + 21;
static const UINT WM_CONFIG_OUTPUT_PLUGIN		= WM_APP + 22;
static const UINT WM_SAVE_FILE					= WM_APP + 23;
static const UINT WM_IS_ABORT					= WM_APP + 24;
static const UINT WM_REST_TIME_DISP				= WM_APP + 25;
static const UINT WM_UPDATE_PREVIEW				= WM_APP + 26;
static const UINT WM_GET_VIDEO					= WM_APP + 27;
static const UINT WM_GET_AUDIO					= WM_APP + 28;
static const UINT WM_SAVE_FILE_FINISHED			= WM_APP + 29;
static const UINT WM_CREATE_OUTPUT_VIDEO		= WM_APP + 30;

static const UINT WM_ATLIVU_INPUT_INITED		= WM_APP + 100;
static const UINT WM_ATLIVU_OUTPUT_INITED		= WM_APP + 101;

static const UINT WM_VIDEO_PROCESS_SEEK			= WM_APP + 120;
static const UINT WM_VIDEO_PROCESSED_SEEK		= WM_APP + 121;
static const UINT WM_VIDEO_PROCESS_PLAY			= WM_APP + 122;
static const UINT WM_VIDEO_PROCESSED_PLAY		= WM_APP + 123;
static const UINT WM_VIDEO_PROCESSED_PLAY_STOP	= WM_APP + 125;

//--------------------------------------------------------------------

struct MediaInfo
{
	int32_t				flag;				//	フラグ
	int32_t				rate,scale;			//	フレームレート
	int32_t				n;					//	フレーム数
	BITMAPINFOHEADER	format;				//	画像フォーマットへのポインタ(次に関数が呼ばれるまで内容を有効にしておく)
	int32_t				format_size;		//	画像フォーマットのサイズ
	int32_t				audio_n;			//	音声サンプル数
	WAVEFORMATEX		audio_format;		//	音声フォーマットへのポインタ(次に関数が呼ばれるまで内容を有効にしておく)
	int32_t				audio_format_size;	//	音声フォーマットのサイズ
	DWORD				handler;			//	画像codecハンドラ
};

struct ReadVideoInput
{
	int32_t media;
	int32_t frame;
};

struct ReadVideoOutput
{
	int32_t structSize;
	BYTE buffer[1];
};

struct ReadAudioInput
{
	int32_t media;
	int32_t start;
	int32_t length;
};

struct ReadAudioOutput
{
	int32_t structSize;
	int32_t length;
	BYTE buffer[1];
};

//--------------------------------------------------------------------

struct LoadOutputPluginInput
{
	TCHAR fileName[MAX_PATH];
};

struct SaveFileInput
{
	TCHAR fileName[MAX_PATH];
	MediaInfo mediaInfo;
};

struct GetVideoInput
{
	int32_t frame;
	DWORD format;
};

struct GetVideoOutput
{
	BYTE buffer[1];
};

struct GetAudioInput
{
	int32_t start;
	int32_t length;
};

struct GetAudioOutput
{
	int32_t structSize;
	int32_t readed;
	BYTE buffer[1];
};

//--------------------------------------------------------------------
