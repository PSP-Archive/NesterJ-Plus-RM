#ifndef EMU_MAIN_H
#define EMU_MAIN_H
//#include "nes/types.h"

enum {
	EMU_SPEED_NORMAL,
	EMU_SPEED_MODE1,
	EMU_SPEED_MODE2
};

// NES init
void PSPEMU_NES_Init(void);
// �w�肵���ԍ��̃X�e�[�g�Ăяo��
u8 PSPEMU_LoadState(u32 ulNum);
// �w�肵���ԍ��ɃX�e�[�g�ۑ�
u8 PSPEMU_SaveState(u32 ulNum);
// �w�肵���ԍ��ɃX�e�[�g�폜
u8 PSPEMU_DeleteState(u32 ulNum);
// Screen shot�ۑ�
u8 PSPEMU_SaveScreenShot(void);

// ���j���[����Ƃ��Ăяo��
void PSPEMU_Freeze(void);

// Freese���ď����I������Ăяo��
void PSPEMU_Thaw(void);

// get disk side number
u8 PSPEMU_GetDiskSideNum(void);

// set disk side
void PSPEMU_SetDiskSide(u8 side);

// get disk side
u8 PSPEMU_GetDiskSide(void);

// apply game genie code
int PSPEMU_ApplyGameGenie(const char *pszFile);
int PSPEMU_ApplyGameCheat(const char *pszFile);

// Apply graphics config
void PSPEMU_ApplyGraphicsConfig(void);

// Apply sound config
void PSPEMU_ApplySoundConfig(void);

// Save ram
void PSPEMU_SaveRAM(void);

// ���݂̐ݒ�ł̃t���[�����[�g�ݒ���s��(internal)
void PSPEMU_SetFrameSpeed(void);

// �G�~�����C��
void PSPEMU_DoFrame(void);

#endif
