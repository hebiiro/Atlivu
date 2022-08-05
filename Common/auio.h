#pragma once

//--------------------------------------------------------------------

static const int32_t TextMaxSize = 1024;

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

//--------------------------------------------------------------------
namespace Input {
//--------------------------------------------------------------------

struct CommandID
{
	static const int32_t None			= 0;
	static const int32_t End			= 1;
	static const int32_t LoadPlugin		= 2;
	static const int32_t UnloadPlugin	= 3;
	static const int32_t ConfigPlugin	= 4;
	static const int32_t OpenMedia		= 5;
	static const int32_t CloseMedia		= 6;
	static const int32_t GetMediaInfo	= 7;
	static const int32_t ReadVideo		= 8;
	static const int32_t ReadAudio		= 9;
};

//--------------------------------------------------------------------
} // namespace Input
//--------------------------------------------------------------------

//--------------------------------------------------------------------
namespace Output {
//--------------------------------------------------------------------

struct CommandID
{
	static const int32_t None			= WM_APP + 100;
	static const int32_t End			= WM_APP + 101;
	static const int32_t LoadPlugin		= WM_APP + 102;
	static const int32_t UnloadPlugin	= WM_APP + 103;
	static const int32_t ConfigPlugin	= WM_APP + 104;
	static const int32_t SaveFile		= WM_APP + 105;

	static const int32_t IsAbort		= WM_APP + 110;
	static const int32_t RestTimeDisp	= WM_APP + 111;
	static const int32_t UpdatePreview	= WM_APP + 112;
	static const int32_t GetVideo		= WM_APP + 113;
	static const int32_t GetAudio		= WM_APP + 114;
};

//--------------------------------------------------------------------
} // namespace Output
//--------------------------------------------------------------------
