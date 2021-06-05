#ifndef _SCREEN_MANAGER
#define _SCREEN_MANAGER

//#include "nes/types.h"
#include "nes/pixmap.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// ���Lenum��char*array�͓��������ŕ���ł���K�v������܂��B
enum {
	SCREEN_NORMAL = 0,
	SCREEN_FULL,
	SCREEN_X15,
	SCREEN_X15_CROPPED,
    SCREEN_GPU_NORMAL,
    SCREEN_GPU_X15,
    SCREEN_GPU_FULL_CROPPED,
    SCREEN_GPU_FULL,
    SCREEN_GPU_NORMAL_MIRROR,
    SCREEN_GPU_X15_MIRROR,
    SCREEN_GPU_FULL_CROPPED_MIRROR,
    SCREEN_GPU_FULL_MIRROR,
    SCREEN_GPU_X15_ROTATED,	
	SCREEN_COUNT	// Last
};

extern const char *aszScreenName[];

// ��ʂɃ��b�Z�[�W�\���p
void Scr_SetMessage(const char *pszMsg, int nTimer, int nColor);

// �p���b�g�ʒm pal[NES_MAX_COLORS][3]
void Scr_AssertPalette(uint8 pal[][3]);

// ��ʃ��[�h PAL or NTSC
void Scr_SetScreenMode(const unsigned char ScreenMode);

// �w�肵���F�œh��Ԃ�
void Scr_ClearScreen(const uint8 PalNum);

// ��ʂ֓]��
void Scr_BltScreen(void);

// �`�惍�b�N
boolean Scr_Lock(pixmap *p);

// �`��A�����b�N
void Scr_Unlock(void);

void pgGeInit(void);

void DrawCurrentScreen(int nWidth, int nHeight);

// create screen shot from current emu
boolean CreateScreenShotFile(char *pszFilePath);
// create thumbnail from current emu
boolean CreateThumnailFile(char *szThumbnailPath);
// load thumbnail
boolean LoadThumnailFile(const char *szThumbnailPath, uint16 *pBuf);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif
