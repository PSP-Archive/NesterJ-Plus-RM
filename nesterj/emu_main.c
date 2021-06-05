/*
	Emulation main by ruka
*/
#include <pspkernel.h>
#include <psppower.h>
#include <stdio.h>
#include "emu_main.h"
#include "main.h"
#include "syscall.h"
#include "nes/nes.h"
#include "pg.h"
#include "menu_submenu.h"
#include "nes/nes_config.h"
#include "nes/apu/nes_apu_wrapper.h"
#include "inputmanager.h"
#include "screenmanager.h"

#define GetMicroSec() sceKernelLibcClock()

#define NTSC_FRAMERATE 5994   // (60000.0/1001.0)*100
#define PAL_FRAMERATE  4995   // (50000.0/1001.0)*100

// �P�ʂ̓ʕb
uint32 Mode1FramePeriodTime;
uint32 NormalFramePeriodTime;
uint32 Mode2FramePeriodTime;

uint32 last_frame_time = 0;

uint32 g_ulFPS = 0;	// FPS

//>>>davex
int framecount = 0;
int f60framecount = 0;
//<<<

// NES init
void PSPEMU_NES_Init(void)
{
	// major struct zero clear
	_memset(&g_NES, 0x00, sizeof(g_NES));
	_memset(&g_PPU, 0x00, sizeof(g_PPU));
	_memset(&g_APU, 0x00, sizeof(g_APU));
	_memset(&g_NESConfig, 0x00, sizeof(g_NESConfig));
}

// �w�肵���ԍ��̃X�e�[�g�Ăяo��
boolean PSPEMU_LoadState(uint32 ulNum)
{
	char szFilePath[MAX_PATH], szTemp[32];

	if (ulNum >= STATE_SLOT_MAX) return FALSE;
	GetStatePath(szFilePath, sizeof(szFilePath));
	_strcat(szFilePath, NES_ROM_GetRomName());
	_strcat(szFilePath, ".ss");
	_itoa(ulNum, szTemp);
	_strcat(szFilePath, szTemp);
	return NES_loadState(szFilePath);
}

// �w�肵���ԍ��ɃX�e�[�g�ۑ�
boolean PSPEMU_SaveState(uint32 ulNum)
{
	char szFilePath[MAX_PATH], szThumbnailPath[MAX_PATH], szTemp[32];
	boolean bRet;

	if (ulNum >= STATE_SLOT_MAX) return FALSE;
	GetStatePath(szFilePath, sizeof(szFilePath));
	_strcat(szFilePath, NES_ROM_GetRomName());
	_strcpy(szThumbnailPath, szFilePath);
	_strcat(szFilePath, ".ss");
	_itoa(ulNum, szTemp);
	_strcat(szFilePath, szTemp);
	bRet = NES_saveState(szFilePath);
	if (bRet) {
		_strcat(szThumbnailPath, ".tn");
		_strcat(szThumbnailPath, szTemp);
		// delete old thumbnail file
		sceIoRemove(szThumbnailPath);
		_strcat(szThumbnailPath, ".png");
		if (setting.savethumbnail) {
			// create thumbnail(png)
			CreateThumnailFile(szThumbnailPath);
		}
		else {
			// delete thumbnail file
			sceIoRemove(szThumbnailPath);
		}
	}
	if (!bRet) sceIoRemove(szFilePath);
	return bRet;
}

// �w�肵���ԍ��ɃX�e�[�g�폜
boolean PSPEMU_DeleteState(uint32 ulNum)
{
	char szFilePath[MAX_PATH], szThumbnailPath[MAX_PATH], szTemp[32], *pszExt;

	if (ulNum >= STATE_SLOT_MAX) return FALSE;
	GetStatePath(szFilePath, sizeof(szFilePath));
	_strcat(szFilePath, NES_ROM_GetRomName());
	_strcpy(szThumbnailPath, szFilePath);
	_strcat(szFilePath, ".ss");
	_itoa(ulNum, szTemp);
	_strcat(szFilePath, szTemp);
	// delete ss? file
	sceIoRemove(szFilePath);
	pszExt = _strrchr(szFilePath, '.');
	if (pszExt) {
		pszExt[1] = 'z';
		// delete zs? file
		sceIoRemove(szFilePath);
	}
	_strcat(szThumbnailPath, ".tn");
	_strcat(szThumbnailPath, szTemp);
	// delete thumbnail file
	sceIoRemove(szThumbnailPath);
	_strcat(szThumbnailPath, ".png");
	// delete thumbnail file(png)
	sceIoRemove(szThumbnailPath);
	return TRUE;
}

// Screen shot�ۑ�
boolean PSPEMU_SaveScreenShot(void)
{
	char szFullPath[MAX_PATH];
	uint32 nCount = 0;
	SceIoStat stat;
	// �ۑ�����t�@�C�����̃p�X�����
	sceIoMkdir("ms0:/PSP/PHOTO" ,0777);
	sceIoMkdir("ms0:/PSP/PHOTO/NesterJ" ,0777);
	do {
		sprintf(szFullPath, "ms0:/PSP/PHOTO/NesterJ/%s%04d.png", NES_ROM_GetRomName(), nCount);
		nCount++;
	} while (sceIoGetstat(szFullPath, &stat) >= 0);
	// szFullPath�ɕۑ�����
	return CreateScreenShotFile(szFullPath);
}

// ���j���[����Ƃ��Ăяo��
void PSPEMU_Freeze(void)
{
	wavout_enable=0;
	Snd_ClearBuffer();
}

// Freese���ď����I������Ăяo��
void PSPEMU_Thaw(void)
{
	if(g_NESConfig.sound.enabled) wavout_enable=1;
	last_frame_time = GetMicroSec();
	PSPEMU_SetFrameSpeed();
}

// Save ram
void PSPEMU_SaveRAM(void)
{
	NES_Save_SaveRAM();
	NES_Save_Disk();
}

// get disk side number
uint8 PSPEMU_GetDiskSideNum(void)
{
	return NES_GetDiskSideNum();
}

// set disk side
void PSPEMU_SetDiskSide(uint8 side)
{
	NES_SetDiskSide(side);
}

// get disk side
uint8 PSPEMU_GetDiskSide(void)
{
	return NES_GetDiskSide();
}

// apply game genie code
int PSPEMU_ApplyGameGenie(const char *pszFile)
{
	int nRet = NES_Load_Genie(pszFile);
	if (nRet) {
		// �����K�p
		g_NESmapper.set_genie();
	}
	return nRet;
}

int PSPEMU_ApplyGameCheat(const char *pszFile)
{
	int nRet = NES_Load_Cheat(pszFile);
	return nRet;
}

// Apply graphics config
void PSPEMU_ApplyGraphicsConfig(void)
{
	NES_ppu_rgb();
}

// Apply sound config
void PSPEMU_ApplySoundConfig(void)
{
	NES_APU_freeze();
	NES_APU_snd_mgr_changed();
	NES_APU_thaw();
}

// ���݂̐ݒ�ł̃t���[�����[�g�ݒ���s��
void PSPEMU_SetFrameSpeed(void)
{
	// 1 = NTSC, 2 = PAL
	if(1==NES_GetScreenMode()){
		NormalFramePeriodTime = 100000000 / NTSC_FRAMERATE;// 16.68ms/frame
	}
	else{
		NormalFramePeriodTime = 100000000 / PAL_FRAMERATE;// 20.02ms/frame
	}
	Mode1FramePeriodTime = 1000000 / setting.speedmode1; // 20-180
	Mode2FramePeriodTime = 1000000 / setting.speedmode2; // 20-180
}

// �G�~�����C��
void PSPEMU_DoFrame(void)
{
	static uint32 ulRenderFrame = 0;
	static uint32 ulFPSCounter = 0;
	uint32 cur_time;
	uint32 frames_since_last, i;
	uint32 FramePeriod;
	uint32 Diff;

	// input key update
	InputUpdatePad();
	// skip frames while disk accessed
	while(g_NESConfig.preferences.UseFDSDiskASkip && NES_DiskAccessed())
	{
		NES_emulate_frame(FALSE);
		last_frame_time = cur_time = GetMicroSec();
	}
	// get the current time
	cur_time = GetMicroSec();
	if (cur_time < last_frame_time) {
		Diff = cur_time + (0xFFFFFFFF-last_frame_time);
	}
	else {
		Diff = cur_time - last_frame_time;
	}

	if (stKeyState.nSpeedMode == EMU_SPEED_NORMAL) {
		// NORMAL
		FramePeriod = NormalFramePeriodTime;
	}
	else if (stKeyState.nSpeedMode == EMU_SPEED_MODE1) {
		// MODE1
		FramePeriod = Mode1FramePeriodTime;
	}
	else {
		// MODE2
		FramePeriod = Mode2FramePeriodTime;
	}

	if (Diff < FramePeriod) {
		// �O�񂩂�A���̃t���[���܂Ŏ��Ԃ��o���ĂȂ��̂Ŕ�����
		return;
	}

	frames_since_last = Diff / FramePeriod;

	for(i = 1; i < frames_since_last; i+=1)
	{
		InputUpdateRapid();
		NES_emulate_frame(FALSE);
		last_frame_time += FramePeriod;
	}

	InputUpdateRapid();
	// emulate current frame
	NES_emulate_frame(TRUE);
	//>>davex
	framecount++;
	f60framecount++;	
	//<<<	
	ulRenderFrame++;
	if ((ulFPSCounter+1000000) < cur_time || ulFPSCounter > cur_time) {
		ulFPSCounter = cur_time;
		g_ulFPS = ulRenderFrame;
		ulRenderFrame = 0;
	}
	Scr_BltScreen();

	last_frame_time += FramePeriod;
}
