/*
    sub menu
*/
#include <pspkernel.h>
#include <pspctrl.h>
#include <psppower.h>
#include "main.h"
#include "emu_main.h"
#include "filer.h"
#include "menu_submenu.h"
#include "screenmanager.h"
#include "inputmanager.h"
#include "nes/nes.h"
#include "nes/nes_config.h"
#include "nes/nes_crc32.h"
#include "convfile.h"

extern u32 new_pad; // pg.c

void submenu_diskchange(void)
{
	enum
	{
		// 弴斣曄峏偡傞偲堎忢摦嶌偡傞偺偱拲堄
		CONFIG_EJECT = 0,
		CONFIG_1STDISKSIDEA,
		CONFIG_1STDISKSIDEB,
		CONFIG_2NDDISKSIDEA,
		CONFIG_2NDDISKSIDEB,
	};
	char msg[256];
	int sel, x, y;
	int nCursor = 0;
	uint8 disksidenum = PSPEMU_GetDiskSideNum();
	sel = PSPEMU_GetDiskSide();

	pgWaitVn(15);

	for(;;){
		readpad();
		if(new_pad & PSP_CTRL_AD_UP){
			if(sel!=0)	sel--;
			else		sel=CONFIG_2NDDISKSIDEB;
		}else if(new_pad & PSP_CTRL_AD_DOWN){
			if(sel!=CONFIG_2NDDISKSIDEB)sel++;
			else				sel=0;
		}else if(new_pad & PSP_CTRL_CROSS){
			break;
		}else if(new_pad & PSP_CTRL_CIRCLE){
			if(sel==CONFIG_EJECT) {
				PSPEMU_SetDiskSide(0x00);
				break;
			} else if(sel==CONFIG_1STDISKSIDEA && disksidenum >= 1) {
				PSPEMU_SetDiskSide(0x01);
				break;
			} else if(sel==CONFIG_1STDISKSIDEB && disksidenum >= 2) {
				PSPEMU_SetDiskSide(0x02);
				break;
			} else if(sel==CONFIG_2NDDISKSIDEA && disksidenum >= 3) {
				PSPEMU_SetDiskSide(0x03);
				break;
			} else if(sel==CONFIG_2NDDISKSIDEB && disksidenum >= 4) {
				PSPEMU_SetDiskSide(0x04);
				break;
			}
		}else if(new_pad & PSP_CTRL_AD_LEFT){
			if(sel<=CONFIG_EJECT) sel=CONFIG_1STDISKSIDEA;
			else if(sel<=CONFIG_2NDDISKSIDEB) sel=CONFIG_EJECT;
		}else if(new_pad & PSP_CTRL_AD_RIGHT){
			if(sel<=CONFIG_EJECT) sel=CONFIG_1STDISKSIDEA;
			else if(sel<=CONFIG_1STDISKSIDEA) sel=CONFIG_EJECT;
		}

		_strcpy(msg,"仜丗OK  亊丗Return to Main Menu");

		menu_frame(0, msg);

		x=2; y=5;
		pgPrint(x,y++,setting.color[3],"  DISK EJECT");
		y++;
		pgPrint(x,y++,(disksidenum >= 1) ? setting.color[3]: setting.color[2],"  CHANGE 1ST DISK SIDE A");
		pgPrint(x,y++,(disksidenum >= 2) ? setting.color[3]: setting.color[2],"  CHANGE 1ST DISK SIDE B");
		pgPrint(x,y++,(disksidenum >= 3) ? setting.color[3]: setting.color[2],"  CHANGE 2ND DISK SIDE A");
		pgPrint(x,y++,(disksidenum >= 4) ? setting.color[3]: setting.color[2],"  CHANGE 2ND DISK SIDE B");

		x = 2;
		y = sel + 5;
		if(sel >= CONFIG_1STDISKSIDEA)       y++;
		if (nCursor/5) pgPutChar((x+1)*8,y*8,setting.color[3],0,127,1,0,1);
		nCursor = (nCursor + 1 ) %10;

		pgScreenFlipV();
	}
}

uint8 g_BiosCheckBuf[0x2000];

void DiskSystemBiosCheck(void)
{
	extern char pg_workdir[];
	uint32 ulReadSize;
	int hFile;
	uint32 ulAccBytes;

	char szBiosPath[MAX_PATH];
	_strcpy(szBiosPath, pg_workdir);
	_strcat(szBiosPath, "DISKSYS.ROM");

	if((hFile = sceIoOpen(szBiosPath, PSP_O_RDONLY, 0777)) >= 0) {
		uint8 head[3];
		ulReadSize = sceIoRead(hFile, head, sizeof(head));
		if(head[0] == 'N' && head[1] == 'E' && head[2] == 'S') {
			sceIoLseek(hFile, 0x6010, 0);
		}
		else {
			sceIoLseek(hFile, 0, 0);
		}
		_memset(g_BiosCheckBuf, 0x00, sizeof(g_BiosCheckBuf));
		ulReadSize = sceIoRead(hFile, g_BiosCheckBuf, sizeof(g_BiosCheckBuf));
		sceIoClose(hFile);
		ulAccBytes = CrcCalc(g_BiosCheckBuf, sizeof(g_BiosCheckBuf));
		if (ulAccBytes == 0x5e607dcf) {
			MessageBox("FDS BIOS is correct", RGB(64, 64, 255), MB_OK);
		}
		else {
			MessageBox("FDS BIOS is Incorrect", RGB(255, 64, 64), MB_OK);
		}
	}
	else
	{
		MessageBox("FDS BIOS FILE NOT FOUND!", RGB(255, 64, 64), MB_OK);
		return;
	}
}

const char *aszRapidModeName[] =
{
	"30count/sec(NTSC)",
	"20count/sec(NTSC)",
	"15count/sec(NTSC)",
	"10count/sec(NTSC)",
	NULL
};

void submenu_preferencesconfig(void)
{
	enum
	{
		CONFIG_USEROMDATABASE = 0,
		CONFIG_SKIPDISKACCESS,
		CONFIG_SAVESTATETHUMBNAIL,
		CONFIG_TVMODE,
		CONFIG_RAPIDMODE,
		CONFIG_SPEEDMODETOGGLE,
		CONFIG_SPEEDMODE1,
		CONFIG_SPEEDMODE2,
		CONFIG_CPUFREQUENCY,
		CONFIG_SHOWFPS,
		CONFIG_LANG,
		CONFIG_FATREAD,
		CONFIG_DISKSYSROMCHECK,
		CONFIG_CONVERTTHUMBNAIL,
		CONFIG_END = CONFIG_CONVERTTHUMBNAIL
	};
	char msg[256], szTemp[256];
	int sel=0, x, y;
	int nCursor = 0;
	int nPushCounter = 0;
	int nDelta;

	pgWaitVn(15);

	for(;;){
		readpad();
		if (!now_pad) nPushCounter = 0;
		if(new_pad & PSP_CTRL_AD_LEFT) {
			nPushCounter++;
			nDelta = (nPushCounter >=20) ? 5:1;
			if (sel == CONFIG_SPEEDMODE1) {
				setting.speedmode1-=nDelta;
				if (setting.speedmode1 < SPEEDMODE_MIN) setting.speedmode1 = SPEEDMODE_MIN;
			}
			if (sel == CONFIG_SPEEDMODE2) {
				setting.speedmode2-=nDelta;
				if (setting.speedmode2 < SPEEDMODE_MIN) setting.speedmode2 = SPEEDMODE_MIN;
			}
			if (sel == CONFIG_CPUFREQUENCY) {
				setting.cpufrequency-=11;
				if (setting.cpufrequency == 322) setting.cpufrequency = 321;
				if (setting.cpufrequency < 222) setting.cpufrequency = 222;
			}
		}
		else if (new_pad & PSP_CTRL_AD_RIGHT){
			nPushCounter++;
			nDelta = (nPushCounter >=20) ? 5:1;
			if (sel == CONFIG_SPEEDMODE1) {
				setting.speedmode1+=nDelta;
				if (setting.speedmode1 > SPEEDMODE_MAX) setting.speedmode1 = SPEEDMODE_MAX;
			}
			if (sel == CONFIG_SPEEDMODE2) {
				setting.speedmode2+=nDelta;
				if (setting.speedmode2 > SPEEDMODE_MAX) setting.speedmode2 = SPEEDMODE_MAX;
			}
			if (sel == CONFIG_CPUFREQUENCY) {
				setting.cpufrequency+=11;
				if (setting.cpufrequency > 330) setting.cpufrequency = 333;
			}
		}else if(new_pad & PSP_CTRL_AD_UP){
			if(sel!=0)	sel--;
			else		sel=CONFIG_END;
		}else if(new_pad & PSP_CTRL_AD_DOWN){
			if(sel!=CONFIG_END)sel++;
			else				sel=0;
		}else if(new_pad & PSP_CTRL_CROSS){
			break;
		}else if(new_pad & PSP_CTRL_CIRCLE){
			if(sel==CONFIG_USEROMDATABASE)
				g_NESConfig.preferences.UseRomDataBase = g_NESConfig.preferences.UseRomDataBase ? FALSE: TRUE;
			else if(sel==CONFIG_SKIPDISKACCESS)
				g_NESConfig.preferences.UseFDSDiskASkip = g_NESConfig.preferences.UseFDSDiskASkip ? FALSE: TRUE;
			else if(sel==CONFIG_SAVESTATETHUMBNAIL)
				setting.savethumbnail = setting.savethumbnail ? FALSE: TRUE;
			else if(sel==CONFIG_TVMODE)
				g_NESConfig.preferences.TV_Mode = (g_NESConfig.preferences.TV_Mode + 1) & 3;
			else if(sel==CONFIG_RAPIDMODE)
				setting.rapidmode = (setting.rapidmode + 1) % RAPID_COUNT;
			else if(sel==CONFIG_SPEEDMODETOGGLE)
				setting.bToggle = setting.bToggle ? FALSE: TRUE;
			else if(sel==CONFIG_DISKSYSROMCHECK)
				DiskSystemBiosCheck();
			else if(sel==CONFIG_SHOWFPS)
				setting.showfps = setting.showfps ? FALSE: TRUE;
			else if(sel==CONFIG_LANG)
			{
				setting.language = (setting.language+1)>=LANG_MAX ? LANG_ENGLISH: setting.language+1;
			}
			else if(sel==CONFIG_FATREAD)
				setting.fatread = setting.fatread ? FALSE: TRUE;
			else if(sel==CONFIG_CONVERTTHUMBNAIL)
				ConvertThumbnailFile();
		}

		_strcpy(msg,"仜丗OK  亊丗Return to Main Menu");

		menu_frame(0, msg);

		x=2; y=5;
		_strcpy(msg, "  EXTERNAL ROM DATABASE : ");
		_strcat(msg, g_NESConfig.preferences.UseRomDataBase ? "ENABLE":"DISABLE");
		pgPrint(x,y++,setting.color[3],msg);
		_strcpy(msg, "  SKIP DISK ACCESS      : ");
		_strcat(msg, g_NESConfig.preferences.UseFDSDiskASkip ? "ENABLE":"DISABLE");
		pgPrint(x,y++,setting.color[3],msg);
		_strcpy(msg, "  SAVE STATE THUMBNAIL  : ");
		_strcat(msg, setting.savethumbnail ? "ENABLE":"DISABLE");
		pgPrint(x,y++,setting.color[3],msg);
		_strcpy(msg, "  TV MODE               : ");
		_strcat(msg, g_NESConfig.preferences.TV_Mode ?
			(g_NESConfig.preferences.TV_Mode == 2)? "PAL":"NTSC":"AUTO");
		pgPrint(x,y++,setting.color[3],msg);
		y++;
		_strcpy(msg, "  RAPID MODE            : ");
		_strcat(msg, aszRapidModeName[setting.rapidmode]);
		pgPrint(x,y++,setting.color[3],msg);
		y++;
		_strcpy(msg, "  SPEED MODE TOGGLE     : ");
		_strcat(msg, setting.bToggle ? "ENABLE":"DISABLE");
		pgPrint(x,y++,setting.color[3],msg);
		_strcpy(msg, "  SPEED MODE 1          : ");
		_itoa(setting.speedmode1, szTemp);
		_strcat(msg, szTemp);
		_strcat(msg, " FPS");
		pgPrint(x,y++,setting.color[3],msg);
		_strcpy(msg, "  SPEED MODE 2          : ");
		_itoa(setting.speedmode2, szTemp);
		_strcat(msg, szTemp);
		_strcat(msg, " FPS");
		pgPrint(x,y++,setting.color[3],msg);
		y+=2;
		_strcpy(msg, "  CPU FREQUENCY         : ");
		pgPrint(x,y,setting.color[3],msg);
		_itoa(setting.cpufrequency, msg);
		_strcat(msg, " MHz");
		pgPrint(x+26,y++,RGB(255, (255+(222-setting.cpufrequency)*2), (255+(222-setting.cpufrequency)*2)),msg);
		_strcpy(msg, "  SHOW FPS              : ");
		_strcat(msg, setting.showfps ? "ENABLE":"DISABLE");
		pgPrint(x,y++,setting.color[3],msg);
		y++;
		_strcpy(msg, "  UI LANGUAGE           : ");
		_strcat(msg, setting.language ? "CHINESE":"ENGLISH");
		pgPrint(x,y++,setting.color[3],msg);
		_strcpy(msg, "  FAT READ MODE         : ");
		_strcat(msg, setting.fatread ? "ENABLE":"DISABLE");
		pgPrint(x,y++,setting.color[3],msg);
		y++;
		pgPrint(x,y++,setting.color[3],"  CHECK FDS BIOS");
		pgPrint(x,y++,setting.color[3],"  CONVERT OLD THUMBNAIL FILES TO PNG FORMAT");

		x = 2;
		y = sel + 5;
		if(sel >= CONFIG_RAPIDMODE) y++;
		if(sel >= CONFIG_SPEEDMODETOGGLE) y++;
		if(sel >= CONFIG_CPUFREQUENCY) y+=2;
		if(sel >= CONFIG_DISKSYSROMCHECK) y++;
		if(sel >= CONFIG_LANG) y++;
		if (nCursor/5) pgPutChar((x+1)*8,y*8,setting.color[3],0,127,1,0,1);
		nCursor = (nCursor + 1 ) %10;

		pgScreenFlipV();
	}
}

//>>>DAVEX REWIND CONFIG SUBMENU
void submenu_rewindconfig(void)
{
	enum
	{
		CONFIG_REWIND_SAVE_RATE = 0,
		CONFIG_REWIND_REPLAY_DELAY,
		CONFIG_END = CONFIG_REWIND_REPLAY_DELAY
	};
	char msg[256], szTemp[256];
	int sel=0, x, y;
	int nCursor = 0;
	int nPushCounter = 0;
	int nDelta;

	pgWaitVn(15);

	for(;;){
		readpad();
		if (!now_pad)
			 nPushCounter = 0;
			 
		if(new_pad & PSP_CTRL_AD_LEFT) {
			nPushCounter++;
			nDelta = (nPushCounter >=20) ? 2:1;
			if( sel == CONFIG_REWIND_SAVE_RATE){
				if(  setting.Reserved[0] > (5*nDelta) )
					setting.Reserved[0] -= (5*nDelta);	
			}
			if( sel == CONFIG_REWIND_REPLAY_DELAY){
				if(  setting.Reserved[1] > (50*nDelta) )
					setting.Reserved[1] -= (50*nDelta);
			}
		}
		else if (new_pad & PSP_CTRL_AD_RIGHT){
			
			nPushCounter++;
			nDelta = (nPushCounter >=20) ? 2:1;
			if( sel == CONFIG_REWIND_SAVE_RATE){
				setting.Reserved[0] += (5*nDelta);
				if( setting.Reserved[0] > 360)
					setting.Reserved[0] = 360;
			}
			if( sel == CONFIG_REWIND_REPLAY_DELAY){
				setting.Reserved[1] += (25*nDelta);
				if(  setting.Reserved[1] > 5000 )
					setting.Reserved[1] = 5000;
			}
			
		}else if(new_pad & PSP_CTRL_AD_UP){
			if(sel != 0)	
				sel--;
			else		
				sel = CONFIG_END;
				
		}else if(new_pad & PSP_CTRL_AD_DOWN){
			if(sel!=CONFIG_END)
				sel++;
			else
				sel=0;
		}else if(new_pad & PSP_CTRL_CROSS){
			break;
		}else if(new_pad & PSP_CTRL_CIRCLE){
			//no se hace nada
		}

		_strcpy(msg,"仜丗OK  亊丗Return to Main Menu");

		menu_frame(0, msg);

		x=2; y=5;
		_strcpy(msg, "  SAVE RATE (IN FRAMES)      : ");
		_itoa(setting.Reserved[0], szTemp);
		_strcat(msg, szTemp);
		pgPrint(x,y++,setting.color[3],msg);
		_strcpy(msg, "  REPLAY DELAY (IN MILISECS) : ");
		_itoa(setting.Reserved[1], szTemp);
		_strcat(msg, szTemp);
		pgPrint(x,y++,setting.color[3],msg);
		
		x = 2;
		y = sel + 5;
		
		//if(sel >= CONFIG_REWIND_SAVE_RATE) y++;
		//if(sel >= CONFIG_REWIND_REPLAY_DELAY) y++;
		
		if (nCursor/5) pgPutChar((x+1)*8,y*8,setting.color[3],0,127,1,0,1);
		nCursor = (nCursor + 1 ) %10;

		pgScreenFlipV();
	}
}
//<<<DAVEX REWIND CONFIG SUBMENU

u32 MessageBox(char*pszMsg, u32 color, u32 ulTypes)
{
	int cbI;
	int nMsgLen, nBtnLen;
	uint32 ulEnableBtn;
	boolean bKeyReleased = FALSE;
	typedef struct {
		uint32 ulType;
		char *pszBtnMsg;
		uint32 ulEnableBtn;
	}MSGTYPE;
	MSGTYPE stMsgType[] = {
		{MB_OK, "仜丗OK", IDOK},
		{MB_OKCANCEL, "仜丗OK  亊丗Cancel", IDOK | IDCANCEL},
		{MB_YESNO, "仜丗YES  亊丗NO", IDYES | IDNO}
	};

	for (cbI = 0; cbI < (sizeof(stMsgType) /sizeof(MSGTYPE)); cbI++) {
		if (stMsgType[cbI].ulType == ulTypes) {
			ulEnableBtn = stMsgType[cbI].ulEnableBtn;
			break;
		}
	}
	if (cbI == (sizeof(stMsgType) /sizeof(MSGTYPE))) {
		return 0x00000000;
	}

	nMsgLen = _strlen(pszMsg);
	nBtnLen = _strlen(stMsgType[cbI].pszBtnMsg);
	// msg
	pgDrawFrame( 240-nMsgLen*5/2-30-1, 110-1, 240+nMsgLen*5/2+30+1, 167+1, RGB(64,64,64));
	pgDrawFrame( 240-nMsgLen*5/2-30-1, 110-1, 240+nMsgLen*5/2+30, 167, RGB(212,208,200));
	pgFillBox( 240-nMsgLen*5/2-30, 110, 240+nMsgLen*5/2+30, 167, RGB(0, 0, 0));
	mh_print(240-nMsgLen*5/2, 125, pszMsg, color);
	mh_print(240-nBtnLen*5/2, 145, stMsgType[cbI].pszBtnMsg, RGB(208, 208, 208));
	pgScreenFlipV();

	for(;;){
		readpad();
		if (!bKeyReleased) {
			pgWaitVn(1);
			if (now_pad & (PSP_CTRL_CIRCLE | PSP_CTRL_CROSS)) {
				continue;
			}
			else {
				bKeyReleased = TRUE;
			}
		}
		if(new_pad & PSP_CTRL_CIRCLE){
			if (ulEnableBtn & IDOK) return IDOK;
		}
		if(new_pad & PSP_CTRL_CROSS){
			if (ulEnableBtn & IDCANCEL) return IDCANCEL;
		}
		pgWaitVn(1);
	}
}

// Screen mode name
const char *aszScreenName[] =
{
	"NORMAL",
	"FULL SCREEN",
	"x1.5 [4:2.8333]",
	"x1.5 [4:2.9166](cropped)",
    "GPU NORMAL",
    "GPU 4:3",
    "GPU FULL SCREEN(cropped)",
    "GPU FULL SCREEN",
    "MIRROR NORMAL", //davex: mirror modes
    "MIRROR 4:3",
    "MIRROR FULL SCREEN(cropped)",
    "MIRROR FULL SCREEN",
    "GPU 4:3 ROTATED",	
	NULL
};

void submenu_graphicsconfig(void)
{
	enum
	{
		CONFIG_EXTENDSPRITES = 0,
		CONFIG_BLACKANDWHITE,
		CONFIG_DEFAULTPALETTE,
		CONFIG_SCREENMODE,
		CONFIG_WAITVBLANKSTART,
	};
	char msg[256];
	int sel=0, x, y;
	int nCursor = 0;

	pgWaitVn(15);

	for(;;){
		readpad();
		if(new_pad & PSP_CTRL_AD_UP){
			if(sel!=0)	sel--;
			else		sel=CONFIG_WAITVBLANKSTART;
		}else if(new_pad & PSP_CTRL_AD_DOWN){
			if(sel!=CONFIG_WAITVBLANKSTART)sel++;
			else				sel=0;
		}else if(new_pad & PSP_CTRL_CROSS){
			break;
		}else if(new_pad & PSP_CTRL_CIRCLE){
			if(sel==CONFIG_EXTENDSPRITES)
				g_NESConfig.graphics.show_more_than_8_sprites = g_NESConfig.graphics.show_more_than_8_sprites ? FALSE: TRUE;
			else if(sel==CONFIG_BLACKANDWHITE)
				g_NESConfig.graphics.black_and_white = g_NESConfig.graphics.black_and_white ? FALSE: TRUE;
			else if(sel==CONFIG_SCREENMODE)
				setting.screenmode = (setting.screenmode + 1) % SCREEN_COUNT;
			else if(sel==CONFIG_WAITVBLANKSTART)
				setting.vsync = setting.vsync ? FALSE: TRUE;
			else if(sel==CONFIG_DEFAULTPALETTE)
				g_NESConfig.graphics.DefaultPaletteNo = (g_NESConfig.graphics.DefaultPaletteNo + 1) % 3;//davex: add a palette
		}else if(new_pad & PSP_CTRL_AD_LEFT){
			if(sel<=CONFIG_DEFAULTPALETTE) sel=CONFIG_WAITVBLANKSTART;
			else if(sel<=CONFIG_SCREENMODE) sel=CONFIG_EXTENDSPRITES;
			else if(sel<=CONFIG_WAITVBLANKSTART) sel=CONFIG_SCREENMODE;
		}else if(new_pad & PSP_CTRL_AD_RIGHT){
			if(sel<=CONFIG_DEFAULTPALETTE) sel=CONFIG_SCREENMODE;
			else if(sel<=CONFIG_SCREENMODE) sel=CONFIG_WAITVBLANKSTART;
			else if(sel<=CONFIG_WAITVBLANKSTART) sel=CONFIG_EXTENDSPRITES;
		}

		_strcpy(msg,"仜丗OK  亊丗Return to Main Menu");

		menu_frame(0, msg);

		x=2; y=5;
		_strcpy(msg, "  EXTEND SPRITES MODE  : ");
		_strcat(msg, g_NESConfig.graphics.show_more_than_8_sprites ? "ENABLE":"DISABLE");
		pgPrint(x,y++,setting.color[3],msg);
		_strcpy(msg, "  BLACK AND WHITE MODE : ");
		_strcat(msg, g_NESConfig.graphics.black_and_white ? "ENABLE":"DISABLE");
		pgPrint(x,y++,setting.color[3],msg);
		_strcpy(msg, "  DEFAULT PALETTE TYPE : ");
		//_strcat(msg, g_NESConfig.graphics.DefaultPaletteNo ? "Nintendulator": "NesterJ");
		if( g_NESConfig.graphics.DefaultPaletteNo == 0)
			_strcat(msg,"Nintendulator");
		else if( g_NESConfig.graphics.DefaultPaletteNo == 1)
			_strcat(msg,"NesterJ");
		else 
			_strcat(msg,"Sepia"); //davex: new palette added
		pgPrint(x,y++,setting.color[3],msg);
		y++;
		_strcpy(msg, "  SCREEN SIZE          : ");
		_strcat(msg, aszScreenName[setting.screenmode]);
		pgPrint(x,y++,setting.color[3],msg);
		y++;
		_strcpy(msg, "  WAIT VBLANK(VSYNC)   : ");
		_strcat(msg, setting.vsync ? "ENABLE":"DISABLE");
		pgPrint(x,y++,setting.color[3],msg);

		x = 2;
		y = sel + 5;
		if(sel >= CONFIG_SCREENMODE)            y++;
		if(sel >= CONFIG_WAITVBLANKSTART)       y++;
		if (nCursor/5) pgPutChar((x+1)*8,y*8,setting.color[3],0,127,1,0,1);
		nCursor = (nCursor + 1 ) %10;

		pgScreenFlipV();
	}
	PSPEMU_ApplyGraphicsConfig();
}

const char *aszSamplingRateName[] =
{
	"44100 Hz",
	"22050 Hz",
	"11025 Hz",
	NULL
};

void submenu_soundconfig(void)
{
	enum
	{
		CONFIG_ENABLESOUND = 0,
		CONFIG_ENABLERECTANGLE1,
		CONFIG_ENABLERECTANGLE2,
		CONFIG_ENABLETRIANGLE,
		CONFIG_ENABLENOISE,
		CONFIG_ENABLEDPCM,
		CONFIG_ENABLEEXTRACHANNEL,
		CONFIG_SAMPLINGRATE,
		CONFIG_LAST = CONFIG_SAMPLINGRATE
	};
	char msg[256];
	int sel=0, x, y;
	int nCursor = 0;
	NES_CONFIG sOld;

	_memcpy(&sOld, &g_NESConfig, sizeof(NES_CONFIG));
	pgWaitVn(15);

	for(;;){
		readpad();
		if(new_pad & PSP_CTRL_AD_UP){
			if(sel!=0)	sel--;
			else		sel=CONFIG_LAST;
		}else if(new_pad & PSP_CTRL_AD_DOWN){
			if(sel!=CONFIG_LAST)sel++;
			else				sel=0;
		}else if(new_pad & PSP_CTRL_CROSS){
			break;
		}else if(new_pad & PSP_CTRL_CIRCLE){
			if(sel==CONFIG_ENABLESOUND)
				g_NESConfig.sound.enabled = g_NESConfig.sound.enabled ? FALSE: TRUE;
			else if(sel==CONFIG_ENABLERECTANGLE1)
				g_NESConfig.sound.rectangle1_enabled = g_NESConfig.sound.rectangle1_enabled ? FALSE: TRUE;
			else if(sel==CONFIG_ENABLERECTANGLE2)
				g_NESConfig.sound.rectangle2_enabled = g_NESConfig.sound.rectangle2_enabled ? FALSE: TRUE;
			else if(sel==CONFIG_ENABLETRIANGLE)
				g_NESConfig.sound.triangle_enabled = g_NESConfig.sound.triangle_enabled ? FALSE: TRUE;
			else if(sel==CONFIG_ENABLENOISE)
				g_NESConfig.sound.noise_enabled = g_NESConfig.sound.noise_enabled ? FALSE: TRUE;
			else if(sel==CONFIG_ENABLEDPCM)
				g_NESConfig.sound.dpcm_enabled = g_NESConfig.sound.dpcm_enabled ? FALSE: TRUE;
			else if(sel==CONFIG_ENABLEEXTRACHANNEL)
				g_NESConfig.sound.ext_enabled = g_NESConfig.sound.ext_enabled ? FALSE: TRUE;
			else if(sel==CONFIG_SAMPLINGRATE)
				setting.samplingrate = (setting.samplingrate + 1) % SAMPLINGRATE_COUNT;
		}else if(new_pad & PSP_CTRL_AD_LEFT){
			if(sel<=CONFIG_ENABLESOUND) sel=CONFIG_SAMPLINGRATE;
			else if(sel<=CONFIG_ENABLEEXTRACHANNEL) sel=CONFIG_ENABLESOUND;
			else if(sel<=CONFIG_SAMPLINGRATE) sel=CONFIG_ENABLERECTANGLE1;
		}else if(new_pad & PSP_CTRL_AD_RIGHT){
			if(sel<=CONFIG_ENABLESOUND) sel=CONFIG_ENABLERECTANGLE1;
			else if(sel<=CONFIG_ENABLEEXTRACHANNEL) sel=CONFIG_SAMPLINGRATE;
			else if(sel<=CONFIG_SAMPLINGRATE) sel=CONFIG_ENABLESOUND;
		}

		_strcpy(msg,"仜丗OK  亊丗Return to Main Menu");

		menu_frame(0, msg);

		x=2; y=5;
		_strcpy(msg, "  SOUND  : ");
		_strcat(msg, g_NESConfig.sound.enabled ? "ENABLE":"DISABLE");
		pgPrint(x,y++,setting.color[3],msg);
		y++;
		_strcpy(msg, "  RECTANGLE 1   : ");
		_strcat(msg, g_NESConfig.sound.rectangle1_enabled ? "ENABLE":"DISABLE");
		pgPrint(x+2,y++,setting.color[3],msg);
		_strcpy(msg, "  RECTANGLE 2   : ");
		_strcat(msg, g_NESConfig.sound.rectangle2_enabled ? "ENABLE":"DISABLE");
		pgPrint(x+2,y++,setting.color[3],msg);
		_strcpy(msg, "  TRIANGLE      : ");
		_strcat(msg, g_NESConfig.sound.triangle_enabled ? "ENABLE":"DISABLE");
		pgPrint(x+2,y++,setting.color[3],msg);
		_strcpy(msg, "  NOISE         : ");
		_strcat(msg, g_NESConfig.sound.noise_enabled ? "ENABLE":"DISABLE");
		pgPrint(x+2,y++,setting.color[3],msg);
		_strcpy(msg, "  DPCM          : ");
		_strcat(msg, g_NESConfig.sound.dpcm_enabled ? "ENABLE":"DISABLE");
		pgPrint(x+2,y++,setting.color[3],msg);
		_strcpy(msg, "  EXTRA CHANNEL : ");
		_strcat(msg, g_NESConfig.sound.ext_enabled ? "ENABLE":"DISABLE");
		pgPrint(x+2,y++,setting.color[3],msg);
		y++;
		_strcpy(msg, "  SAMPLING RATE   : ");
		_strcat(msg, aszSamplingRateName[setting.samplingrate]);
		pgPrint(x,y++,setting.color[3],msg);

		x = 2;
		y = sel + 5;
		if(sel >= CONFIG_ENABLERECTANGLE1)       y++;
		if (sel >= CONFIG_ENABLERECTANGLE1 && sel <= CONFIG_ENABLEEXTRACHANNEL) x+=2;
		if(sel >= CONFIG_SAMPLINGRATE)       y++;
		if (nCursor/5) pgPutChar((x+1)*8,y*8,setting.color[3],0,127,1,0,1);
		nCursor = (nCursor + 1 ) %10;

		pgScreenFlipV();
	}
	// 曄峏偑偁偭偨偲偒偩偗峏怴
	if (_memcmp(&sOld, &g_NESConfig, sizeof(NES_CONFIG))) {
		PSPEMU_ApplySoundConfig();
	}
}




void submenu_menucolorconfig(void)
{
	enum
	{
		COLOR0_R=0,
		COLOR0_G,
		COLOR0_B,
		COLOR1_R,
		COLOR1_G,
		COLOR1_B,
		COLOR2_R,
		COLOR2_G,
		COLOR2_B,
		COLOR3_R,
		COLOR3_G,
		COLOR3_B,
		BG_BRIGHT,
		EXIT,
		INIT,
	};
	char tmp[4], msg[256];
	int color[4][3];
	int sel=0, x, y, i;
	int nCursor = 0;

	_memset(color, 0, sizeof(int)*4*3);
	for(i=0; i<4; i++){
		color[i][2] = setting.color[i]>>10 & 0x1F;
		color[i][1] = setting.color[i]>>5 & 0x1F;
		color[i][0] = setting.color[i] & 0x1F;
	}

	for(;;){
		readpad();
		if(new_pad & PSP_CTRL_CIRCLE){
			if(sel==EXIT){
				break;
			}else if(sel==INIT){
				if (MessageBox("Are you sure to Initialize ?", RGB(255, 64, 64), MB_OKCANCEL) == IDOK) {
					color[0][2] = DEF_COLOR0>>10 & 0x1F;
					color[0][1] = DEF_COLOR0>>5 & 0x1F;
					color[0][0] = DEF_COLOR0 & 0x1F;
					color[1][2] = DEF_COLOR1>>10 & 0x1F;
					color[1][1] = DEF_COLOR1>>5 & 0x1F;
					color[1][0] = DEF_COLOR1 & 0x1F;
					color[2][2] = DEF_COLOR2>>10 & 0x1F;
					color[2][1] = DEF_COLOR2>>5 & 0x1F;
					color[2][0] = DEF_COLOR2 & 0x1F;
					color[3][2] = DEF_COLOR3>>10 & 0x1F;
					color[3][1] = DEF_COLOR3>>5 & 0x1F;
					color[3][0] = DEF_COLOR3 & 0x1F;
					setting.bgbright = 100;
					if(g_bEnableBitmap){
						load_menu_bg();
						bgbright_change();
					}
				}
			}else if(sel == BG_BRIGHT) {
				//婸搙曄峏
				setting.bgbright += 10;
				if(setting.bgbright > 100) setting.bgbright=0;
				if(g_bEnableBitmap){
					load_menu_bg();
					bgbright_change();
				}
			}else{
				if(color[sel/3][sel%3]<31)
					color[sel/3][sel%3]++;
			}
		}else if(new_pad & PSP_CTRL_CROSS){
			if(sel == BG_BRIGHT) {
				//婸搙曄峏
				setting.bgbright -= 10;
				if(setting.bgbright < 0) setting.bgbright=100;
				if(g_bEnableBitmap){
					load_menu_bg();
					bgbright_change();
				}
			}else if(sel>=COLOR0_R && sel<=COLOR3_B){
				if(color[sel/3][sel%3]>0)
					color[sel/3][sel%3]--;
			}
		}else if(new_pad & PSP_CTRL_AD_UP){
			if(sel!=0)	sel--;
			else		sel=INIT;
		}else if(new_pad & PSP_CTRL_AD_DOWN){
			if(sel!=INIT)	sel++;
			else			sel=0;
		}else if(new_pad & PSP_CTRL_AD_RIGHT){
			if(sel<COLOR1_R) 		sel=COLOR1_R;
			else if(sel<COLOR2_R)	sel=COLOR2_R;
			else if(sel<COLOR3_R)	sel=COLOR3_R;
			else if(sel<BG_BRIGHT)	sel=BG_BRIGHT;
			else if(sel<EXIT)		sel=EXIT;
		}else if(new_pad & PSP_CTRL_AD_LEFT){
			if(sel>BG_BRIGHT)		sel=BG_BRIGHT;
			else if(sel>COLOR3_B)	sel=COLOR3_R;
			else if(sel>COLOR2_B)	sel=COLOR2_R;
			else if(sel>COLOR1_B)	sel=COLOR1_R;
			else					sel=COLOR0_R;
		}

		for(i=0; i<4; i++)
			setting.color[i]=color[i][2]<<10|color[i][1]<<5|color[i][0]|0x8000;

		x = 2;
		y = 5;

		if(sel>=COLOR0_R && sel<=BG_BRIGHT)
			_strcpy(msg, "仜丗Add  亊丗Sub");
		else
			_strcpy(msg, "仜丗OK");

		menu_frame(0, msg);

		pgPrint(x,y++,setting.color[3],"  COLOR0 R:");
		pgPrint(x,y++,setting.color[3],"  COLOR0 G:");
		pgPrint(x,y++,setting.color[3],"  COLOR0 B:");
		y++;
		pgPrint(x,y++,setting.color[3],"  COLOR1 R:");
		pgPrint(x,y++,setting.color[3],"  COLOR1 G:");
		pgPrint(x,y++,setting.color[3],"  COLOR1 B:");
		y++;
		pgPrint(x,y++,setting.color[3],"  COLOR2 R:");
		pgPrint(x,y++,setting.color[3],"  COLOR2 G:");
		pgPrint(x,y++,setting.color[3],"  COLOR2 B:");
		y++;
		pgPrint(x,y++,setting.color[3],"  COLOR3 R:");
		pgPrint(x,y++,setting.color[3],"  COLOR3 G:");
		pgPrint(x,y++,setting.color[3],"  COLOR3 B:");
		y++;
		if(setting.bgbright / 100 == 1)
			pgPrint(x,y++,setting.color[3],"  BG BRIGHT:100%");
		else
			pgPrint(x,y++,setting.color[3],"  BG BRIGHT:  0%");
		if(setting.bgbright % 100 != 0)			// 10%乣90%
			pgPutChar((x+13)*8,(y-1)*8,setting.color[3],0,'0'+setting.bgbright/10,1,0,1);
		y++;
		pgPrint(x,y++,setting.color[3],"  Return to Main Menu");
		pgPrint(x,y++,setting.color[3],"  Initialize");

		x=14; y=5;
		for(i=0; i<12; i++){
			if(i!=0 && i%3==0) y++;
			_itoa(color[i/3][i%3], tmp);
			pgPrint(x,y++,setting.color[3],tmp);
		}

		x = 2;
		y = sel + 5;
		if(sel>=COLOR1_R) y++;
		if(sel>=COLOR2_R) y++;
		if(sel>=COLOR3_R) y++;
		if(sel>=BG_BRIGHT) y++;
		if(sel>=EXIT) y++;
		if (nCursor/5) pgPutChar((x+1)*8,y*8,setting.color[3],0,127,1,0,1);
		nCursor = (nCursor + 1 ) %10;

		pgScreenFlipV();
	}
}

// by kwn
void submenu_keyconfig(void)
{
	enum
	{
		CONFIG_UP = 0,
		CONFIG_DOWN,
		CONFIG_LEFT,
		CONFIG_RIGHT,
		CONFIG_A,
		CONFIG_B,
		CONFIG_RAPIDA,
		CONFIG_RAPIDB,
		CONFIG_SELECT,
		CONFIG_START,
		CONFIG_MENU,
		CONFIG_PLAYER2MIC,
		CONFIG_SPEEDMODE1,
		CONFIG_SPEEDMODE2,
		CONFIG_SOUND,
		CONFIG_SCREENSIZE,
		CONFIG_QUICKSAVE,
		CONFIG_QUICKLOAD,
		CONFIG_STATE_SLOT,
		CONFIG_BATTERY_METER,
		CONFIG_SCREEN_SHOT,
		CONFIG_REWIND, //davex
		CONFIG_EXIT,
		CONFIG_INIT,
	};
	char msg[256];
	int sel=0, x, y, i, bPad = 0;
	int nCursor = 0;

	pgWaitVn(15);

	for(;;){
		readpad();
		if(now_pad==PSP_CTRL_LEFT || now_pad==PSP_CTRL_RIGHT){
			if(sel!=CONFIG_EXIT && sel!=CONFIG_MENU && sel!=CONFIG_INIT)
				setting.key_config[sel] = 0;
		}else if(now_pad==PSP_CTRL_UP){
			if(bPad==0){
				if(sel!=0)	sel--;
				else		sel=CONFIG_INIT;
				bPad++;
			}else if(bPad >= 5){
				if(sel!=0)	sel--;
				else		sel=CONFIG_INIT;
				bPad=4;
			}else
				bPad++;
		}else if(now_pad==PSP_CTRL_DOWN){
			if(bPad==0){
				if(sel!=CONFIG_INIT)sel++;
				else				sel=0;
				bPad++;
			}else if(bPad >= 5){
				if(sel!=CONFIG_INIT)sel++;
				else				sel=0;
				bPad=4;
			}else
				bPad++;
		}else if(new_pad != 0){
			if(sel==CONFIG_INIT) {
				if (new_pad&PSP_CTRL_CIRCLE) {
					if (MessageBox("Are you sure to Initialize ?", RGB(255, 64, 64), MB_OKCANCEL) == IDOK) {
						setkeydefault();
					}
				}
			}
			else if(sel==CONFIG_EXIT) {
				if (new_pad&PSP_CTRL_CIRCLE) {
					break;
				}
			}
			else
				setting.key_config[sel] = now_pad;
		}else{
			bPad=0;
		}

		if(sel>=CONFIG_EXIT)
			_strcpy(msg,"仜丗OK");
		else
			_strcpy(msg,"");

		menu_frame(0, msg);

		x=2; y=5;
		pgPrint(x,y++,setting.color[3],"  UP BUTTON      :");
		pgPrint(x,y++,setting.color[3],"  DOWN BUTTON    :");
		pgPrint(x,y++,setting.color[3],"  LEFT BUTTON    :");
		pgPrint(x,y++,setting.color[3],"  RIGHT BUTTON   :");
		pgPrint(x,y++,setting.color[3],"  A BUTTON       :");
		pgPrint(x,y++,setting.color[3],"  B BUTTON       :");
		pgPrint(x,y++,setting.color[3],"  A BUTTON(RAPID):");
		pgPrint(x,y++,setting.color[3],"  B BUTTON(RAPID):");
		pgPrint(x,y++,setting.color[3],"  SELECT BUTTON  :");
		pgPrint(x,y++,setting.color[3],"  START BUTTON   :");
		pgPrint(x,y++,setting.color[3],"  MENU BUTTON    :");
		pgPrint(x,y++,setting.color[3],"  PLAYER2 MIC    :");
		pgPrint(x,y++,setting.color[3],"  SPEED MODE 1   :");
		pgPrint(x,y++,setting.color[3],"  SPEED MODE 2   :");
		pgPrint(x,y++,setting.color[3],"  SOUND ON/OFF   :");
		pgPrint(x,y++,setting.color[3],"  SCREEN SIZE    :");
		pgPrint(x,y++,setting.color[3],"  QUICK SAVE     :");
		pgPrint(x,y++,setting.color[3],"  QUICK LOAD     :");
		pgPrint(x,y++,setting.color[3],"  STATE SLOT     :");
		pgPrint(x,y++,setting.color[3],"  BATTERY LIFE   :");
		pgPrint(x,y++,setting.color[3],"  SCREEN SHOT    :");
		pgPrint(x,y++,setting.color[3],"  REWIND         :");
		y++;
		pgPrint(x,y++,setting.color[3],"  Return to Main Menu");
		pgPrint(x,y++,setting.color[3],"  Initialize");

		for (i=0; i<CONFIG_EXIT; i++){
			y = i + 5;
			int j = 0;
			msg[0]=0;
			if(setting.key_config[i] == 0){
				_strcpy(msg,"UNDEFINED");
			}else{
				if (setting.key_config[i] & PSP_CTRL_LTRIGGER){
					msg[j++]='L'; msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & PSP_CTRL_RTRIGGER){
					msg[j++]='R'; msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & PSP_CTRL_CIRCLE){
					msg[j++]=1; msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & PSP_CTRL_CROSS){
					msg[j++]=2; msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & PSP_CTRL_SQUARE){
					msg[j++]=3; msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & PSP_CTRL_TRIANGLE){
					msg[j++]=4; msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & PSP_CTRL_START){
					_strcat(msg,"START+"); j+=6;
				}
				if (setting.key_config[i] & PSP_CTRL_SELECT){
					_strcat(msg,"SELECT+"); j+=7;
				}
				if (setting.key_config[i] & PSP_CTRL_UP){
					msg[j++]=5; msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & PSP_CTRL_RIGHT){
					msg[j++]=6; msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & PSP_CTRL_DOWN){
					msg[j++]=7; msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & PSP_CTRL_LEFT){
					msg[j++]=8; msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & PSP_CTRL_A_UP){
					msg[j++]=5; _strcpy(msg+j,"(analog)+"); j+=9;
				}
				if (setting.key_config[i] & PSP_CTRL_A_RIGHT){
					msg[j++]=6; _strcpy(msg+j,"(analog)+"); j+=9;
				}
				if (setting.key_config[i] & PSP_CTRL_A_DOWN){
					msg[j++]=7; _strcpy(msg+j,"(analog)+"); j+=9;
				}
				if (setting.key_config[i] & PSP_CTRL_A_LEFT){
					msg[j++]=8; _strcpy(msg+j,"(analog)+"); j+=9;
				}
				msg[_strlen(msg)-1]=0;
			}
			pgPrint(21,y,setting.color[3],msg);
		}

		x = 2;
		y = sel + 5;
		if(sel >= CONFIG_EXIT)        y++;
		if (nCursor) pgPutChar((x+1)*8,y*8,setting.color[3],0,127,1,0,1);
		nCursor = (nCursor + 1 ) %2;

		pgScreenFlipV();
		pgWaitVn(4);
	}

}



void submenu_configcheat(void)
{
#define MAX_CHEAT_LINE (17) //最大显示记录条数
#define MAX_LR_LINE (8)	//按LR最大跳过行数
	//新代码
	CHEATINFO *pCheatCurrent = g_NES.pCheatInfo;//指向当前作弊码
	int iCount = 1;//计数器
	int iSel = 0;//光标
	CHEATINFO *pCheatShow = g_NES.pCheatInfo;//显示指针

	uint8 cheat_num = NES_GetCheatCodeNum(); //总共作弊码条目
	CHEATINFO *pCheatTemp = NULL;//临时用
	CHEATOPTION *pOptionShow;

	char msg[256];

	int y;
	int nCursor = 0;

	int i;

	if(cheat_num==0)return;

	pgWaitVn(15);

	for(;;)
	{
		readpad();

		//计算当前选择的作弊码
		if(new_pad & PSP_CTRL_AD_UP)//上
		{
			if(iCount!=1)
			{
				iCount--;
				pCheatCurrent = pCheatCurrent->pPrevious;

				if(cheat_num<MAX_CHEAT_LINE+1)
					iSel--;
				else if((iCount<9)||(iCount>cheat_num-8))//9=MAX_CHEAT_LINE/2+1
					iSel--;
				else
					pCheatShow = pCheatShow->pPrevious;
			}
		}
		else if(new_pad & PSP_CTRL_AD_DOWN)//下
		{
			if(iCount<cheat_num)
			{
				iCount++;
				pCheatCurrent = pCheatCurrent->pNext;

				if(cheat_num<MAX_CHEAT_LINE+1)
					iSel++;
				else if((iCount<9)||(iCount>cheat_num-8))//9=MAX_CHEAT_LINE/2+1
					iSel++;
				else
					pCheatShow = pCheatShow->pNext;
			}
		}
		else if( new_pad&PSP_CTRL_LTRIGGER )
		{
			for(y=0; y<MAX_LR_LINE; y++)
			if(iCount!=1)
			{
				iCount--;
				pCheatCurrent = pCheatCurrent->pPrevious;

				if(cheat_num<MAX_CHEAT_LINE+1)
					iSel--;
				else if((iCount<9)||(iCount>cheat_num-8))//9=MAX_CHEAT_LINE/2+1
					iSel--;
				else
					pCheatShow = pCheatShow->pPrevious;
			}
		}
		else if( new_pad&PSP_CTRL_RTRIGGER )
		{
			for(y=0; y<MAX_LR_LINE; y++)
			if(iCount<cheat_num)
			{
				iCount++;
				pCheatCurrent = pCheatCurrent->pNext;

				if(cheat_num<MAX_CHEAT_LINE+1)
					iSel++;
				else if((iCount<9)||(iCount>cheat_num-8))//9=MAX_CHEAT_LINE/2+1
					iSel++;
				else
					pCheatShow = pCheatShow->pNext;
			}
		}

		else if(new_pad & PSP_CTRL_AD_LEFT)//左
		{
			if(pCheatCurrent->nCurrent)
				pCheatCurrent->nCurrent-=1;
		}
		else if(new_pad & PSP_CTRL_AD_RIGHT)//→
		{
			pCheatCurrent->nCurrent+=1;
			if( pCheatCurrent->pOption[pCheatCurrent->nCurrent]==NULL )
				pCheatCurrent->nCurrent-=1;
		}
		else if(new_pad & PSP_CTRL_CROSS)//X
		{
			break;//返回
		}else if(new_pad & PSP_CTRL_CIRCLE)//圆圈
		{//启用禁用作弊码
			if(pCheatCurrent->nStatus)
				pCheatCurrent->nStatus =0;
			else
			{
				pCheatCurrent->nStatus =1;
			}
		}

		_strcpy(msg,"○：开启/关闭作弊码  →：作弊功能选择  ×：返回");

		menu_frame_cn(0, msg);

		y=3;

		//显示作弊码表
		pCheatTemp = pCheatShow;
		for(i=0; i<MAX_CHEAT_LINE; i++)
		{
			if(pCheatTemp==NULL)
				break;

			_strcpy(msg,"   ");
			_strcat(msg,pCheatTemp->szCheatName);
			pgPrintGBK12(3,y++,setting.color[3],msg);
			//显示状态(开启或者关闭)+选择项目
			if(pCheatTemp->nStatus==0)
				pgPrintGBK12(40,y-1,setting.color[3],"关闭");
			else
			{
				pOptionShow = pCheatTemp->pOption[pCheatTemp->nCurrent];
				pgPrintGBK12(40,y-1,setting.color[3],pOptionShow->szOptionName);
			}


			pCheatTemp = pCheatTemp->pNext;
		}

		//显示状态或者作弊码项


		//显示光标，代码需要处理，cheat_num
		y = iSel + 3;

		if (nCursor/5) pgPutChar(24,y*12+2,setting.color[3],0,127,1,0,1);
		nCursor = (nCursor + 1 ) %10;

		pgScreenFlipV();
	}
}


void emu_config_cn(void)
{
	enum
	{
		PREFERENCES_CONFIG,
		GRAPHIC_CONFIG,
		SOUND_CONFIG,
		MENU_COLOR_CONFIG,
		KEY_CONFIG,
		KEY_CONFIG_EN,
		LOAD_FONT,
		ABOUT_EMU,
		EXIT_CONFIG,
	};
	
	char msg[256];
	int sel=0,  y;
	int nCursor = 0;

	pgWaitVn(15);

	for(;;){
		readpad();
		if(new_pad & PSP_CTRL_AD_UP)//上
		{
			if(sel!=0)	sel--;
			else		sel=EXIT_CONFIG;
		}
		else if(new_pad & PSP_CTRL_AD_DOWN)//下
		{
			if(sel!=EXIT_CONFIG)sel++;
			else				sel=0;
		}
		else if(new_pad & PSP_CTRL_CROSS)//X
		{
			break;
		}else if(new_pad & PSP_CTRL_CIRCLE)//圆圈
		{				
			if(sel==PREFERENCES_CONFIG)
				submenu_preferencesconfig_cn();
			else if(sel==GRAPHIC_CONFIG)
				submenu_graphicsconfig_cn();
			else if(sel==SOUND_CONFIG)
				submenu_soundconfig_cn();
			else if(sel==MENU_COLOR_CONFIG)
				submenu_menucolorconfig_cn();
			else if(sel==KEY_CONFIG)
				submenu_keyconfig_cn();
			else if(sel==KEY_CONFIG_EN)
				submenu_keyconfig();
			else if(sel==LOAD_FONT)
				MessageBoxGbk12("本功能为支持《霸王的大陆》等模拟器ROM一体版保留！",RGB(255,0,0), MB_OK);
			else if(sel==ABOUT_EMU)
				MessageBoxGbk12("问题联系：tensai_wang@msn.com！",RGB(0,255,0), MB_OK);
			else if(sel==EXIT_CONFIG)
				break;
		}

		_strcpy(msg,"○：确定  ×：返回主菜单");

		menu_frame_cn(0, msg);

		y=3;
		
		pgPrintGBK12(3,y++,setting.color[3],"   参数设置");
		pgPrintGBK12(3,y++,setting.color[3],"   图形设置");
		pgPrintGBK12(3,y++,setting.color[3],"   音频设置");
		pgPrintGBK12(3,y++,setting.color[3],"   菜单颜色设置");
		pgPrintGBK12(3,y++,setting.color[3],"   按键设置");
		pgPrintGBK12(3,y++,setting.color[3],"   按键设置(英文原版)");

		pgPrintGBK12(3,y+1,setting.color[3],"   读取额外字库");

		pgPrintGBK12(3,y+3,setting.color[3],"   关于本模拟器");
		pgPrintGBK12(3,y+4,setting.color[3],"   退出设置");

		y = sel + 3;
		
		if(sel >= LOAD_FONT) y++;
		if(sel >= ABOUT_EMU) y++;

		if (nCursor/5) pgPutChar(24,y*12+2,setting.color[3],0,127,1,0,1);
		nCursor = (nCursor + 1 ) %10;

		pgScreenFlipV();
	}
}

u32 MessageBoxGbk12(char*pszMsg, u32 color, u32 ulTypes)
{
	int cbI;
	int nMsgLen, nBtnLen;
	uint32 ulEnableBtn;
	boolean bKeyReleased = FALSE;
	typedef struct {
		uint32 ulType;
		char *pszBtnMsg;
		uint32 ulEnableBtn;
	}MSGTYPE;
	MSGTYPE stMsgType[] = {
		{MB_OK, "○:确定", IDOK},
		{MB_OKCANCEL, "○:确定  ×:取消", IDOK | IDCANCEL},
		{MB_YESNO, "○:是  ×:否", IDYES | IDNO}
	};

	for (cbI = 0; cbI < (sizeof(stMsgType) /sizeof(MSGTYPE)); cbI++) {
		if (stMsgType[cbI].ulType == ulTypes) {
			ulEnableBtn = stMsgType[cbI].ulEnableBtn;
			break;
		}
	}
	if (cbI == (sizeof(stMsgType) /sizeof(MSGTYPE))) {
		return 0x00000000;
	}

	nMsgLen = _strlen(pszMsg);
	nBtnLen = _strlen(stMsgType[cbI].pszBtnMsg);
	// msg
	pgDrawFrame( 240-nMsgLen*6/2-30-1, 110-1, 240+nMsgLen*6/2+30+1, 167+1, RGB(64,64,64));
	pgDrawFrame( 240-nMsgLen*6/2-30-1, 110-1, 240+nMsgLen*6/2+30, 167, RGB(212,208,200));
	pgFillBox( 240-nMsgLen*6/2-30, 110, 240+nMsgLen*6/2+30, 167, RGB(0, 0, 0));
	mh_printGBK12(240-nMsgLen*6/2, 125, pszMsg, color);
	mh_printGBK12(240-nBtnLen*6/2, 145, stMsgType[cbI].pszBtnMsg, RGB(208, 208, 208));
	pgScreenFlipV();

	for(;;){
		readpad();
		if (!bKeyReleased) {
			pgWaitVn(1);
			if (now_pad & (PSP_CTRL_CIRCLE | PSP_CTRL_CROSS)) {
				continue;
			}
			else {
				bKeyReleased = TRUE;
			}
		}
		if(new_pad & PSP_CTRL_CIRCLE){
			if (ulEnableBtn & IDOK) return IDOK;
		}
		if(new_pad & PSP_CTRL_CROSS){
			if (ulEnableBtn & IDCANCEL) return IDCANCEL;
		}
		pgWaitVn(1);
	}
}


void submenu_preferencesconfig_cn(void)
{
	enum
	{
		CONFIG_USEROMDATABASE = 0,
		CONFIG_SKIPDISKACCESS,
		CONFIG_SAVESTATETHUMBNAIL,
		CONFIG_TVMODE,
		CONFIG_RAPIDMODE,
		CONFIG_SPEEDMODETOGGLE,
		CONFIG_SPEEDMODE1,
		CONFIG_SPEEDMODE2,
		CONFIG_CPUFREQUENCY,
		CONFIG_SHOWFPS,
		CONFIG_LANG,
		CONFIG_FATREAD,
		CONFIG_DISKSYSROMCHECK,
		CONFIG_CONVERTTHUMBNAIL,
		CONFIG_END = CONFIG_CONVERTTHUMBNAIL
	};
	char msg[256], szTemp[256];
	int sel=0, x, y;
	int nCursor = 0;
	int nPushCounter = 0;
	int nDelta;

	pgWaitVn(15);

	for(;;){
		readpad();
		if (!now_pad) nPushCounter = 0;
		if(new_pad & PSP_CTRL_AD_LEFT) {
			nPushCounter++;
			nDelta = (nPushCounter >=20) ? 5:1;
			if (sel == CONFIG_SPEEDMODE1) {
				setting.speedmode1-=nDelta;
				if (setting.speedmode1 < SPEEDMODE_MIN) setting.speedmode1 = SPEEDMODE_MIN;
			}
			if (sel == CONFIG_SPEEDMODE2) {
				setting.speedmode2-=nDelta;
				if (setting.speedmode2 < SPEEDMODE_MIN) setting.speedmode2 = SPEEDMODE_MIN;
			}
			if (sel == CONFIG_CPUFREQUENCY) {
				setting.cpufrequency-=11;
				if (setting.cpufrequency == 322) setting.cpufrequency = 321;
				if (setting.cpufrequency < 222) setting.cpufrequency = 222;
			}
		}
		else if (new_pad & PSP_CTRL_AD_RIGHT){
			nPushCounter++;
			nDelta = (nPushCounter >=20) ? 5:1;
			if (sel == CONFIG_SPEEDMODE1) {
				setting.speedmode1+=nDelta;
				if (setting.speedmode1 > SPEEDMODE_MAX) setting.speedmode1 = SPEEDMODE_MAX;
			}
			if (sel == CONFIG_SPEEDMODE2) {
				setting.speedmode2+=nDelta;
				if (setting.speedmode2 > SPEEDMODE_MAX) setting.speedmode2 = SPEEDMODE_MAX;
			}
			if (sel == CONFIG_CPUFREQUENCY) {
				setting.cpufrequency+=11;
				if (setting.cpufrequency > 330) setting.cpufrequency = 333;
			}
		}else if(new_pad & PSP_CTRL_AD_UP){
			if(sel!=0)	sel--;
			else		sel=CONFIG_END;
		}else if(new_pad & PSP_CTRL_AD_DOWN){
			if(sel!=CONFIG_END)sel++;
			else				sel=0;
		}else if(new_pad & PSP_CTRL_CROSS){
			break;
		}else if(new_pad & PSP_CTRL_CIRCLE){
			if(sel==CONFIG_USEROMDATABASE)
				g_NESConfig.preferences.UseRomDataBase = g_NESConfig.preferences.UseRomDataBase ? FALSE: TRUE;
			else if(sel==CONFIG_SKIPDISKACCESS)
				g_NESConfig.preferences.UseFDSDiskASkip = g_NESConfig.preferences.UseFDSDiskASkip ? FALSE: TRUE;
			else if(sel==CONFIG_SAVESTATETHUMBNAIL)
				setting.savethumbnail = setting.savethumbnail ? FALSE: TRUE;
			else if(sel==CONFIG_TVMODE)
				g_NESConfig.preferences.TV_Mode = (g_NESConfig.preferences.TV_Mode + 1) & 3;
			else if(sel==CONFIG_RAPIDMODE)
				setting.rapidmode = (setting.rapidmode + 1) % RAPID_COUNT;
			else if(sel==CONFIG_SPEEDMODETOGGLE)
				setting.bToggle = setting.bToggle ? FALSE: TRUE;
			else if(sel==CONFIG_DISKSYSROMCHECK)
				DiskSystemBiosCheck();
			else if(sel==CONFIG_SHOWFPS)
			{	setting.showfps = setting.showfps ? FALSE: TRUE;}
			else if(sel==CONFIG_LANG)
			{
				setting.language = (setting.language+1)>=LANG_MAX ? LANG_ENGLISH: setting.language+1;
			}
			else if(sel==CONFIG_FATREAD)
				setting.fatread = setting.fatread ? FALSE: TRUE;
			else if(sel==CONFIG_CONVERTTHUMBNAIL)
				ConvertThumbnailFile();
		}

		_strcpy(msg,"○：确定  ×：返回到主菜单");

		menu_frame_cn(0, msg);

		x=3; y=3;
		_strcpy(msg, "   扩展 ROM 数据库     ： ");
		_strcat(msg, g_NESConfig.preferences.UseRomDataBase ? "开启":"关闭");
		mh_printGBK12(18,36,msg,setting.color[3]);	
		_strcpy(msg, "   跳过磁盘访问        ： ");
		_strcat(msg, g_NESConfig.preferences.UseFDSDiskASkip ? "开启":"关闭");
		mh_printGBK12(18,48,msg,setting.color[3]);
		_strcpy(msg, "   保存即时存档缩略图  ： ");
		_strcat(msg, setting.savethumbnail ? "开启":"关闭");
		mh_printGBK12(18,60,msg,setting.color[3]);
		_strcpy(msg, "   TV 制式             ： ");
		_strcat(msg, g_NESConfig.preferences.TV_Mode ?
			(g_NESConfig.preferences.TV_Mode == 2)? "PAL":"NTSC":"自动");
		mh_printGBK12(18,72,msg,setting.color[3]);
		
		_strcpy(msg, "   按键连发间隔        ： ");
		_strcat(msg, aszRapidModeName[setting.rapidmode]);
		mh_printGBK12(18,90,msg,setting.color[3]);
		
		_strcpy(msg, "   切换速度模式        ： ");
		_strcat(msg, setting.bToggle ? "开启":"关闭");
		mh_printGBK12(18,108,msg,setting.color[3]);
		_strcpy(msg, "   速度模式 1          ： ");
		_itoa(setting.speedmode1, szTemp);
		_strcat(msg, szTemp);
		_strcat(msg, " FPS");
		mh_printGBK12(18,120,msg,setting.color[3]);
		_strcpy(msg, "   速度模式 2          ： ");
		_itoa(setting.speedmode2, szTemp);
		_strcat(msg, szTemp);
		_strcat(msg, " FPS");
		mh_printGBK12(18,132,msg,setting.color[3]);		
				
		_strcpy(msg, "   CPU 频率            ： ");
		mh_printGBK12(18,150,msg,setting.color[3]);
		_itoa(setting.cpufrequency, msg);
		_strcat(msg, " MHz");
		mh_printGBK12(18+156,150,msg,RGB(255, (255+(222-setting.cpufrequency)*2), (255+(222-setting.cpufrequency)*2)));
		_strcpy(msg, "   显示 FPS            ： ");
		_strcat(msg, setting.showfps ? "开启":"关闭");
		mh_printGBK12(18,162,msg,setting.color[3]);		
		
		_strcpy(msg, "   界面语言            ： ");
		_strcat(msg, setting.language ? "中文":"英文");
		mh_printGBK12(18,180,msg,setting.color[3]);		
		_strcpy(msg, "   FAT 读取模式        ： ");
		_strcat(msg, setting.fatread ? "启用":"禁用");
		mh_printGBK12(18,192,msg,setting.color[3]);		
		
		mh_printGBK12(18,210,"   检查 FDS BIOS",setting.color[3]);
		mh_printGBK12(18,222,"   转换旧的缩略图格式为 PNG 格式",setting.color[3]);

		x = 3;
		y = 0;
		if(sel >= CONFIG_RAPIDMODE) y++;
		if(sel >= CONFIG_SPEEDMODETOGGLE) y++;
		if(sel >= CONFIG_CPUFREQUENCY) y++;
		if(sel >= CONFIG_LANG) y++;
		if(sel >= CONFIG_DISKSYSROMCHECK) y++;
		if (nCursor/5) pgPutChar((x+0)*8,(sel+3)*12+2+y*6,setting.color[3],0,127,1,0,1);
		nCursor = (nCursor + 1 ) %10;

		pgScreenFlipV();
	}
}


void submenu_graphicsconfig_cn(void)
{
	enum
	{
		CONFIG_EXTENDSPRITES = 0,
		CONFIG_BLACKANDWHITE,
		CONFIG_DEFAULTPALETTE,
		CONFIG_SCREENMODE,
		CONFIG_WAITVBLANKSTART,
	};
	char msg[256];
	int sel=0, x, y;
	int nCursor = 0;

	pgWaitVn(15);

	for(;;){
		readpad();
		if(new_pad & PSP_CTRL_AD_UP)
		{
			if(sel!=0)	sel--;
			else		sel=CONFIG_WAITVBLANKSTART;
		}
		else if(new_pad & PSP_CTRL_AD_DOWN)
		{
			if(sel!=CONFIG_WAITVBLANKSTART)sel++;
			else				sel=0;
		}
		else if(new_pad & PSP_CTRL_CROSS)
		{
			break;
		}else if(new_pad & PSP_CTRL_CIRCLE)
		{
			if(sel==CONFIG_EXTENDSPRITES)
				g_NESConfig.graphics.show_more_than_8_sprites = g_NESConfig.graphics.show_more_than_8_sprites ? FALSE: TRUE;
			else if(sel==CONFIG_BLACKANDWHITE)
				g_NESConfig.graphics.black_and_white = g_NESConfig.graphics.black_and_white ? FALSE: TRUE;
			else if(sel==CONFIG_SCREENMODE)
				setting.screenmode = (setting.screenmode + 1) % SCREEN_COUNT;
			else if(sel==CONFIG_WAITVBLANKSTART)
				setting.vsync = setting.vsync ? FALSE: TRUE;
			else if(sel==CONFIG_DEFAULTPALETTE)
				g_NESConfig.graphics.DefaultPaletteNo = (g_NESConfig.graphics.DefaultPaletteNo + 1) % 2;
		}
		else if(new_pad & PSP_CTRL_AD_LEFT)
		{
			if(sel<=CONFIG_DEFAULTPALETTE) sel=CONFIG_WAITVBLANKSTART;
			else if(sel<=CONFIG_SCREENMODE) sel=CONFIG_EXTENDSPRITES;
			else if(sel<=CONFIG_WAITVBLANKSTART) sel=CONFIG_SCREENMODE;
		}
		else if(new_pad & PSP_CTRL_AD_RIGHT)
		{
			if(sel<=CONFIG_DEFAULTPALETTE) sel=CONFIG_SCREENMODE;
			else if(sel<=CONFIG_SCREENMODE) sel=CONFIG_WAITVBLANKSTART;
			else if(sel<=CONFIG_WAITVBLANKSTART) sel=CONFIG_EXTENDSPRITES;
		}

		_strcpy(msg,"○：确定  ×：返回主菜单");

		menu_frame_cn(0, msg);

		x=3; y=3;
		_strcpy(msg, "   扩展活动块模式      ：");		
		_strcat(msg, g_NESConfig.graphics.show_more_than_8_sprites ? "开启":"关闭");
		pgPrintGBK12(x,y++,setting.color[3],msg);
		_strcpy(msg, "   黑白模式            ：");
		_strcat(msg, g_NESConfig.graphics.black_and_white ? "开启":"关闭");
		pgPrintGBK12(x,y++,setting.color[3],msg);
		_strcpy(msg, "   默认调色板类型      ：");
		_strcat(msg, g_NESConfig.graphics.DefaultPaletteNo ? "Nintendulator": "NesterJ");
		pgPrintGBK12(x,y++,setting.color[3],msg);
		y++;
		_strcpy(msg, "   屏幕大小            ：");
		_strcat(msg, aszScreenName[setting.screenmode]);
		pgPrintGBK12(x,y++,setting.color[3],msg);
		y++;
		_strcpy(msg, "   等待垂直同步(VSYNC) ：");
		_strcat(msg, setting.vsync ? "开启":"关闭");
		pgPrintGBK12(x,y++,setting.color[3],msg);

		x = 3;
		y = sel + 3;
		if(sel >= CONFIG_SCREENMODE)            y++;
		if(sel >= CONFIG_WAITVBLANKSTART)       y++;
		if (nCursor/5) pgPutChar((x+0)*8,y*12+3,setting.color[3],0,127,1,0,1);
		nCursor = (nCursor + 1 ) %10;

		pgScreenFlipV();
	}
	PSPEMU_ApplyGraphicsConfig();
}


void submenu_soundconfig_cn(void)
{
	enum
	{
		CONFIG_ENABLESOUND = 0,
		CONFIG_ENABLERECTANGLE1,
		CONFIG_ENABLERECTANGLE2,
		CONFIG_ENABLETRIANGLE,
		CONFIG_ENABLENOISE,
		CONFIG_ENABLEDPCM,
		CONFIG_ENABLEEXTRACHANNEL,
		CONFIG_SAMPLINGRATE,
		CONFIG_LAST = CONFIG_SAMPLINGRATE
	};
	char msg[256];
	int sel=0, x, y;
	int nCursor = 0;
	NES_CONFIG sOld;

	_memcpy(&sOld, &g_NESConfig, sizeof(NES_CONFIG));
	pgWaitVn(15);

	for(;;){
		readpad();
		if(new_pad & PSP_CTRL_AD_UP){
			if(sel!=0)	sel--;
			else		sel=CONFIG_LAST;
		}else if(new_pad & PSP_CTRL_AD_DOWN){
			if(sel!=CONFIG_LAST)sel++;
			else				sel=0;
		}else if(new_pad & PSP_CTRL_CROSS){
			break;
		}else if(new_pad & PSP_CTRL_CIRCLE){
			if(sel==CONFIG_ENABLESOUND)
				g_NESConfig.sound.enabled = g_NESConfig.sound.enabled ? FALSE: TRUE;
			else if(sel==CONFIG_ENABLERECTANGLE1)
				g_NESConfig.sound.rectangle1_enabled = g_NESConfig.sound.rectangle1_enabled ? FALSE: TRUE;
			else if(sel==CONFIG_ENABLERECTANGLE2)
				g_NESConfig.sound.rectangle2_enabled = g_NESConfig.sound.rectangle2_enabled ? FALSE: TRUE;
			else if(sel==CONFIG_ENABLETRIANGLE)
				g_NESConfig.sound.triangle_enabled = g_NESConfig.sound.triangle_enabled ? FALSE: TRUE;
			else if(sel==CONFIG_ENABLENOISE)
				g_NESConfig.sound.noise_enabled = g_NESConfig.sound.noise_enabled ? FALSE: TRUE;
			else if(sel==CONFIG_ENABLEDPCM)
				g_NESConfig.sound.dpcm_enabled = g_NESConfig.sound.dpcm_enabled ? FALSE: TRUE;
			else if(sel==CONFIG_ENABLEEXTRACHANNEL)
				g_NESConfig.sound.ext_enabled = g_NESConfig.sound.ext_enabled ? FALSE: TRUE;
			else if(sel==CONFIG_SAMPLINGRATE)
				setting.samplingrate = (setting.samplingrate + 1) % SAMPLINGRATE_COUNT;
		}else if(new_pad & PSP_CTRL_AD_LEFT){
			if(sel<=CONFIG_ENABLESOUND) sel=CONFIG_SAMPLINGRATE;
			else if(sel<=CONFIG_ENABLEEXTRACHANNEL) sel=CONFIG_ENABLESOUND;
			else if(sel<=CONFIG_SAMPLINGRATE) sel=CONFIG_ENABLERECTANGLE1;
		}else if(new_pad & PSP_CTRL_AD_RIGHT){
			if(sel<=CONFIG_ENABLESOUND) sel=CONFIG_ENABLERECTANGLE1;
			else if(sel<=CONFIG_ENABLEEXTRACHANNEL) sel=CONFIG_SAMPLINGRATE;
			else if(sel<=CONFIG_SAMPLINGRATE) sel=CONFIG_ENABLESOUND;
		}

		_strcpy(msg,"○：确定  ×：返回主菜单");

		menu_frame_cn(0, msg);

		x=3; y=3;
		_strcpy(msg, "   音频  ： ");
		_strcat(msg, g_NESConfig.sound.enabled ? "开启":"关闭");
		pgPrintGBK12(x,y++,setting.color[3],msg);
		y++;
		_strcpy(msg, "   矩形波 1 ： ");
		_strcat(msg, g_NESConfig.sound.rectangle1_enabled ? "开启":"关闭");
		pgPrintGBK12(x+2,y++,setting.color[3],msg);
		_strcpy(msg, "   矩形波 2 ： ");
		_strcat(msg, g_NESConfig.sound.rectangle2_enabled ? "开启":"关闭");
		pgPrintGBK12(x+2,y++,setting.color[3],msg);
		_strcpy(msg, "   角形波   ： ");
		_strcat(msg, g_NESConfig.sound.triangle_enabled ? "开启":"关闭");
		pgPrintGBK12(x+2,y++,setting.color[3],msg);
		_strcpy(msg, "   效果音   ： ");
		_strcat(msg, g_NESConfig.sound.noise_enabled ? "开启":"关闭");
		pgPrintGBK12(x+2,y++,setting.color[3],msg);
		_strcpy(msg, "   DPCM     ： ");
		_strcat(msg, g_NESConfig.sound.dpcm_enabled ? "开启":"关闭");
		pgPrintGBK12(x+2,y++,setting.color[3],msg);
		_strcpy(msg, "   扩展声道 ： ");
		_strcat(msg, g_NESConfig.sound.ext_enabled ? "开启":"关闭");
		pgPrintGBK12(x+2,y++,setting.color[3],msg);
		y++;
		_strcpy(msg, "   采样率     ： ");
		_strcat(msg, aszSamplingRateName[setting.samplingrate]);
		pgPrintGBK12(x,y++,setting.color[3],msg);

		x = 3;
		y = sel + 3;
		if(sel >= CONFIG_ENABLERECTANGLE1)       y++;
		if (sel >= CONFIG_ENABLERECTANGLE1 && sel <= CONFIG_ENABLEEXTRACHANNEL) x+=2;
		if(sel >= CONFIG_SAMPLINGRATE)       y++;
		if (nCursor/5) pgPutChar((x+0)*8,y*12+3,setting.color[3],0,127,1,0,1);
		nCursor = (nCursor + 1 ) %10;

		pgScreenFlipV();
	}
	// 曄峏偑偁偭偨偲偒偩偗峏怴
	if (_memcmp(&sOld, &g_NESConfig, sizeof(NES_CONFIG))) {
		PSPEMU_ApplySoundConfig();
	}
}


void submenu_menucolorconfig_cn(void)
{
	enum
	{
		COLOR0_R=0,
		COLOR0_G,
		COLOR0_B,
		COLOR1_R,
		COLOR1_G,
		COLOR1_B,
		COLOR2_R,
		COLOR2_G,
		COLOR2_B,
		COLOR3_R,
		COLOR3_G,
		COLOR3_B,
		BG_BRIGHT,
		EXIT,
		INIT,
	};
	char tmp[4], msg[256];
	int color[4][3];
	int sel=0, x, y, i;
	int nCursor = 0;

	_memset(color, 0, sizeof(int)*4*3);
	for(i=0; i<4; i++){
		color[i][2] = setting.color[i]>>10 & 0x1F;
		color[i][1] = setting.color[i]>>5 & 0x1F;
		color[i][0] = setting.color[i] & 0x1F;
	}

	for(;;){
		readpad();
		if(new_pad & PSP_CTRL_CIRCLE){
			if(sel==EXIT){
				break;
			}else if(sel==INIT){
				if (MessageBox("您确实要使用默认配置吗 ？", RGB(255, 64, 64), MB_OKCANCEL) == IDOK) {
					color[0][2] = DEF_COLOR0>>10 & 0x1F;
					color[0][1] = DEF_COLOR0>>5 & 0x1F;
					color[0][0] = DEF_COLOR0 & 0x1F;
					color[1][2] = DEF_COLOR1>>10 & 0x1F;
					color[1][1] = DEF_COLOR1>>5 & 0x1F;
					color[1][0] = DEF_COLOR1 & 0x1F;
					color[2][2] = DEF_COLOR2>>10 & 0x1F;
					color[2][1] = DEF_COLOR2>>5 & 0x1F;
					color[2][0] = DEF_COLOR2 & 0x1F;
					color[3][2] = DEF_COLOR3>>10 & 0x1F;
					color[3][1] = DEF_COLOR3>>5 & 0x1F;
					color[3][0] = DEF_COLOR3 & 0x1F;
					setting.bgbright = 100;
					if(g_bEnableBitmap){
						load_menu_bg();
						bgbright_change();
					}
				}
			}else if(sel == BG_BRIGHT) {
				//婸搙曄峏
				setting.bgbright += 10;
				if(setting.bgbright > 100) setting.bgbright=0;
				if(g_bEnableBitmap){
					load_menu_bg();
					bgbright_change();
				}
			}else{
				if(color[sel/3][sel%3]<31)
					color[sel/3][sel%3]++;
			}
		}else if(new_pad & PSP_CTRL_CROSS){
			if(sel == BG_BRIGHT) {
				//婸搙曄峏
				setting.bgbright -= 10;
				if(setting.bgbright < 0) setting.bgbright=100;
				if(g_bEnableBitmap){
					load_menu_bg();
					bgbright_change();
				}
			}else if(sel>=COLOR0_R && sel<=COLOR3_B){
				if(color[sel/3][sel%3]>0)
					color[sel/3][sel%3]--;
			}
		}else if(new_pad & PSP_CTRL_AD_UP){
			if(sel!=0)	sel--;
			else		sel=INIT;
		}else if(new_pad & PSP_CTRL_AD_DOWN){
			if(sel!=INIT)	sel++;
			else			sel=0;
		}else if(new_pad & PSP_CTRL_AD_RIGHT){
			if(sel<COLOR1_R) 		sel=COLOR1_R;
			else if(sel<COLOR2_R)	sel=COLOR2_R;
			else if(sel<COLOR3_R)	sel=COLOR3_R;
			else if(sel<BG_BRIGHT)	sel=BG_BRIGHT;
			else if(sel<EXIT)		sel=EXIT;
		}else if(new_pad & PSP_CTRL_AD_LEFT){
			if(sel>BG_BRIGHT)		sel=BG_BRIGHT;
			else if(sel>COLOR3_B)	sel=COLOR3_R;
			else if(sel>COLOR2_B)	sel=COLOR2_R;
			else if(sel>COLOR1_B)	sel=COLOR1_R;
			else					sel=COLOR0_R;
		}

		for(i=0; i<4; i++)
			setting.color[i]=color[i][2]<<10|color[i][1]<<5|color[i][0]|0x8000;

		x = 2;
		y = 5;

		if(sel>=COLOR0_R && sel<=BG_BRIGHT)
			_strcpy(msg, "○：增大  ×：减小");
		else
			_strcpy(msg, "○：确定");

		menu_frame_cn(0, msg);
		
		pgPrint(x,y++,0x00001F,"  `:");
		pgPrint(x,y++,0x0003E0,"  `:");
		pgPrint(x,y++,0x007C00,"  `:");
		mh_printGBK12((x+7)*8,(y-3)*8, "设置背景颜色",setting.color[3]);
		y++;
		pgPrint(x,y++,0x00001F,"  `:");
		pgPrint(x,y++,0x0003E0,"  `:");
		pgPrint(x,y++,0x007C00,"  `:");
		mh_printGBK12((x+7)*8,(y-3)*8, "设置边框颜色",setting.color[3]);
		y++;
		pgPrint(x,y++,0x00001F,"  `:");
		pgPrint(x,y++,0x0003E0,"  `:");
		pgPrint(x,y++,0x007C00,"  `:");
		mh_printGBK12((x+7)*8,(y-3)*8, "设置操作提示颜色",setting.color[3]);
		y++;
		pgPrint(x,y++,0x00001F,"  `:");
		pgPrint(x,y++,0x0003E0,"  `:");
		pgPrint(x,y++,0x007C00,"  `:");
		mh_printGBK12((x+7)*8,(y-3)*8, "设置文字颜色",setting.color[3]);
		y++;

		if(setting.bgbright / 100 == 1)
		{
			mh_printGBK12(x*8+1,y*8,"   背景亮度:100%",setting.color[3]);
		}
		else
		{
			mh_printGBK12(x*8+1,y*8,"   背景亮度:  0%",setting.color[3]);
		}
		if(setting.bgbright % 100 != 0)			// 10%乣90%
		{
			Draw_Char_Asc12(x*8+78,y*8,'0'+setting.bgbright/10,setting.color[3]);
		}

		mh_printGBK12(x*8+1,y*8+20, "   返回到主菜单",setting.color[3]);
		mh_printGBK12(x*8+1,y*8+32, "   使用默认配置",setting.color[3]);

		x=6; y=5;
		for(i=0; i<12; i++){
			if(i!=0 && i%3==0) y++;
			_itoa(color[i/3][i%3], tmp);
			pgPrint(x,y++,setting.color[3],tmp);
		}

		x = 2;
		y = sel + 5;
		if(sel>=COLOR1_R) y++;
		if(sel>=COLOR2_R) y++;
		if(sel>=COLOR3_R) y++;
		if(sel>=BG_BRIGHT) y++;
		if(sel>=EXIT) y++;
		if (nCursor/5)
		{
			//显示光标
			if(sel==BG_BRIGHT)
				pgPutChar((x+1)*8,y*8+2,setting.color[3],0,127,1,0,1);
			else if(sel==EXIT)
				pgPutChar((x+1)*8,y*8+6,setting.color[3],0,127,1,0,1);
			else if(sel==INIT)
				pgPutChar((x+1)*8,y*8+10,setting.color[3],0,127,1,0,1);
			else
				pgPutChar((x+1)*8,y*8,setting.color[3],0,127,1,0,1);
		}
		nCursor = (nCursor + 1 ) %10;

		pgScreenFlipV();
	}
}

// by kwn
void submenu_keyconfig_cn(void)
{
	enum
	{
		//CONFIG_UP = 0,
		//CONFIG_DOWN,
		//CONFIG_LEFT,
		//CONFIG_RIGHT,
		CONFIG_A = 0,
		CONFIG_B,
		CONFIG_RAPIDA,
		CONFIG_RAPIDB,
		//CONFIG_SELECT,
		//CONFIG_START,
		CONFIG_MENU,
		//CONFIG_PLAYER2MIC,
		CONFIG_SPEEDMODE1,
		CONFIG_SPEEDMODE2,
		CONFIG_SOUND,
		CONFIG_SCREENSIZE,
		CONFIG_QUICKSAVE,
		CONFIG_QUICKLOAD,
		CONFIG_STATE_SLOT,
		CONFIG_BATTERY_METER,
		CONFIG_SCREEN_SHOT,
		CONFIG_EXIT,
		CONFIG_INIT,
	};
	char msg[256];
	int sel=0, x, y, k, bPad = 0;
	int nCursor = 0;

	pgWaitVn(15);

	for(;;){
		readpad();
		if(now_pad==PSP_CTRL_LEFT || now_pad==PSP_CTRL_RIGHT)//左右
		{
			if(sel!=CONFIG_EXIT && sel!=CONFIG_MENU && sel!=CONFIG_INIT)
			{				
				if(sel<4)
					setting.key_config[sel+4] = 0;
				else if(sel==4)
					setting.key_config[10] = 0;
				else 
					setting.key_config[sel+7] = 0;
				//setting.key_config[sel] = 0;//清除设置
			}
		}
		else if(now_pad==PSP_CTRL_UP)//上
		{
			if(bPad==0)
			{
				if(sel!=0)	sel--;
				else		sel=CONFIG_INIT;
				bPad++;
			}
			else if(bPad >= 5)
			{
				if(sel!=0)	sel--;
				else		sel=CONFIG_INIT;
				bPad=4;
			}else
				bPad++;
		}
		else if(now_pad==PSP_CTRL_DOWN)//下
		{
			if(bPad==0){
				if(sel!=CONFIG_INIT)sel++;
				else				sel=0;
				bPad++;
			}else if(bPad >= 5){
				if(sel!=CONFIG_INIT)sel++;
				else				sel=0;
				bPad=4;
			}else
				bPad++;
		}
		else if(new_pad != 0)//有按键
		{
			if(sel==CONFIG_INIT) 
			{
				if (new_pad&PSP_CTRL_CIRCLE) {
					if (MessageBoxGbk12("您确实使用默认配置吗 ?", RGB(255, 64, 64), MB_OKCANCEL) == IDOK) {
						setkeydefault();
					}
				}
			}
			else if(sel==CONFIG_EXIT) 
			{
				if (new_pad&PSP_CTRL_CIRCLE) {
					break;
				}
			}
			else
			{
				if(sel<4)
					setting.key_config[sel+4] = now_pad;
				else if(sel==4)
					setting.key_config[10] = now_pad;
				else 
					setting.key_config[sel+7] = now_pad;
			}
		}else{
			bPad=0;
		}

		if(sel>=CONFIG_EXIT)
			_strcpy(msg,"○：确定");
		else
			_strcpy(msg,"");

		menu_frame_cn(0, msg);

		x=3; y=3;
		//pgPrint(x,y++,setting.color[3],"  UP BUTTON      :");
		//pgPrint(x,y++,setting.color[3],"  DOWN BUTTON    :");
		//pgPrint(x,y++,setting.color[3],"  LEFT BUTTON    :");
		//pgPrint(x,y++,setting.color[3],"  RIGHT BUTTON   :");
		pgPrintGBK12(x,y++,setting.color[3],"   A 按钮        ：");
		pgPrintGBK12(x,y++,setting.color[3],"   B 按钮        ：");
		pgPrintGBK12(x,y++,setting.color[3],"   A 按钮(连发)  ：");
		pgPrintGBK12(x,y++,setting.color[3],"   B 按钮(连发)  ：");
		//pgPrint(x,y++,setting.color[3],"  SELECT BUTTON  :");
		//pgPrint(x,y++,setting.color[3],"  START BUTTON   :");
		pgPrintGBK12(x,y++,setting.color[3],"   菜单按钮      ：");
		//pgPrint(x,y++,setting.color[3],"  PLAYER2 MIC    :");
		pgPrintGBK12(x,y++,setting.color[3],"   速度模式 1    ：");
		pgPrintGBK12(x,y++,setting.color[3],"   速度模式 2    ：");
		pgPrintGBK12(x,y++,setting.color[3],"   声音 开/关    ：");
		pgPrintGBK12(x,y++,setting.color[3],"   屏幕大小      ：");
		pgPrintGBK12(x,y++,setting.color[3],"   快速存档      ：");
		pgPrintGBK12(x,y++,setting.color[3],"   快速读档      ：");
		pgPrintGBK12(x,y++,setting.color[3],"   存档位置      ：");
		pgPrintGBK12(x,y++,setting.color[3],"   显示电量      ：");
		pgPrintGBK12(x,y++,setting.color[3],"   截图          ：");
		y++;
		pgPrintGBK12(x,y++,setting.color[3],"   返回到主菜单");
		pgPrintGBK12(x,y++,setting.color[3],"   使用默认配置");

		for (k=0; k<CONFIG_EXIT; k++)
		{
			y = k + 3;
			int j = 0;
			int i = 0;
			msg[0]=0;
			
			if( k<4 )
				i = k+4;
			else if(k==4)
				i = k+6;
			else 
				i = k+7;
			
			if(setting.key_config[i] == 0)
			{
				_strcpy(msg,"未定义");
			}else
			{
				if (setting.key_config[i] & PSP_CTRL_LTRIGGER)
				{
					msg[j++]='L'; msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & PSP_CTRL_RTRIGGER){
					msg[j++]='R'; msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & PSP_CTRL_CIRCLE){
					_strcat(&msg[j],"○");j+=2;
					msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & PSP_CTRL_CROSS){
					_strcat(&msg[j],"×");j+=2;
					msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & PSP_CTRL_SQUARE){
					_strcat(&msg[j],"□");j+=2;
					msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & PSP_CTRL_TRIANGLE){
					_strcat(&msg[j],"△");j+=2;
					msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & PSP_CTRL_START){
					_strcat(msg,"START+"); j+=6;
				}
				if (setting.key_config[i] & PSP_CTRL_SELECT){
					_strcat(msg,"SELECT+"); j+=7;
				}
				if (setting.key_config[i] & PSP_CTRL_UP){
					_strcat(&msg[j],"↑");j+=2;
					msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & PSP_CTRL_RIGHT){
					_strcat(&msg[j],"→");j+=2;
					msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & PSP_CTRL_DOWN){
					_strcat(&msg[j],"↓");j+=2;
					msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & PSP_CTRL_LEFT){
					_strcat(&msg[j],"←");j+=2;
					msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & PSP_CTRL_A_UP){
					_strcat(&msg[j],"↑");j+=2;
					_strcpy(msg+j,"(模拟摇杆)+"); j+=11;
				}
				if (setting.key_config[i] & PSP_CTRL_A_RIGHT){
					_strcat(&msg[j],"→");j+=2;
					_strcpy(msg+j,"(模拟摇杆)+"); j+=11;
				}
				if (setting.key_config[i] & PSP_CTRL_A_DOWN){
					_strcat(&msg[j],"↓");j+=2;
					_strcpy(msg+j,"(模拟摇杆)+"); j+=11;
				}
				if (setting.key_config[i] & PSP_CTRL_A_LEFT){
					_strcat(&msg[j],"←");j+=2;
					_strcpy(msg+j,"(模拟摇杆)+"); j+=11;
				}
				msg[_strlen(msg)-1]=0;
			}
			pgPrintGBK12(25,y,setting.color[3],msg);
		}

		x = 3;
		y = sel + 3;
		if(sel >= CONFIG_EXIT)        y++;//一个空格
		
		if (nCursor/5) pgPutChar((x+0)*8,y*12+3,setting.color[3],0,127,1,0,1);
		nCursor = (nCursor + 1 ) %10;

		pgScreenFlipV();
		pgWaitVn(4);
	}
}
