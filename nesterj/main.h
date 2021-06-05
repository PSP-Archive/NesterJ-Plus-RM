#ifndef MAIN_H
#define MAIN_H

#include <pspkernel.h>
//#include "syscall.h"
//#include "nes/types.h"
#include "filer.h"
#include "menu.h"
#include "pg.h"
#include "pggbk.h"
#include "sound.h"
#include "string.h"

// 拲堄丗壓婰峔憿懱傪曄峏偟偨傜 NESTERJ_CFG_FORMATVERSION 偺抣傪曄峏偟偰壓偝偄丅
// 曄峏偟側偄偲偦傟埲慜偺NESTERJ.CFG偺擖偭偰偄傞PSP偱僴儞僌傾僢僾偡傞嫲傟偁傝丅

typedef struct
{
	char vercnf[16];
	int version;		// CFG file format version
	int vsync;			// wait vbrank start
	int screenmode;		// screen mode
	int showfps;		// show fps
	int key_config[25]; // 梋暘偵妋曐
	u32 color[4];	// color
	int bgbright;
	int bToggle;		// toggle flag(speed mode1 speed mode2)
	int speedmode1;		// speed mode 1
	int speedmode2;		// speed mode 2
	int savethumbnail;	// save thumbnail
	int samplingrate;	// sampling rate
	int rapidmode;		// rapid mode
	int cpufrequency;   // CPU frequency(MHz)
	u32 language;		//系统语言
	u32 fatread;		//磁盘读取模式
	u32 Reserved[30];// Reserved
	char szLastFile[MAX_PATH];
	char szLastPath[MAX_PATH];
	char szLastGeniePath[MAX_PATH];
} SETTING;

// NESTERJ.CFG偺僼僅乕儅僢僩幆暿梡(嵟戝15暥帤傑偱)
// 枅夞儕儕乕僗偛偲偵曄峏偡傞昁梫側偟丅
#define NESTERJ_CFG_TAG "NesterJCFG"
//#define NESTERJ_CFG_FORMATVERSION 110
#define NESTERJ_CFG_FORMATVERSION 112

enum{
	DEF_COLOR0=0x9063,
	DEF_COLOR1=RGB(85,85,95),
	DEF_COLOR2=RGB(105,105,115),
	DEF_COLOR3=0xffff,
};

enum{
	LANG_ENGLISH=0,
	LANG_CHINESE=1,
	LANG_MAX,
};

enum{
	READFILE_SCEIO=0,
	READFILE_FAT,
};

#define BGBRIGHT_DEFAULT 100

#define SPEEDMODE_MIN 20
#define SPEEDMODE_MAX 300
#define SPEEDMODE1_DEFAULT 80
#define SPEEDMODE2_DEFAULT 40

extern char RomPath[];
extern char szLastGeniePath[];
extern SETTING setting;
extern const EXTENTIONS stExtRom[];
extern const EXTENTIONS stExtCheat[];

#endif
