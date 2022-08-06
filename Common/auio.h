#pragma once

//--------------------------------------------------------------------

static const int32_t TextMaxSize = 1024;

//--------------------------------------------------------------------

struct MediaInfo
{
	int32_t				flag;				//	�t���O
	int32_t				rate,scale;			//	�t���[�����[�g
	int32_t				n;					//	�t���[����
	BITMAPINFOHEADER	format;				//	�摜�t�H�[�}�b�g�ւ̃|�C���^(���Ɋ֐����Ă΂��܂œ��e��L���ɂ��Ă���)
	int32_t				format_size;		//	�摜�t�H�[�}�b�g�̃T�C�Y
	int32_t				audio_n;			//	�����T���v����
	WAVEFORMATEX		audio_format;		//	�����t�H�[�}�b�g�ւ̃|�C���^(���Ɋ֐����Ă΂��܂œ��e��L���ɂ��Ă���)
	int32_t				audio_format_size;	//	�����t�H�[�}�b�g�̃T�C�Y
	DWORD				handler;			//	�摜codec�n���h��
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
