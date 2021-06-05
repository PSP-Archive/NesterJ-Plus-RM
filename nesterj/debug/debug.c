#include <pspkernel.h>
#include <pspdebug.h>
#include "debug.h"
#include "../fileio.h"
#include "../nes/types.h"

#define MAXPATH 512

// ���n���O���邩�킩��Ȃ��̂Ŗ���o��
void OutputDebugString(void *buf, int nLen)
{
	int hFile = -1;
	char szPath[MAXPATH];

	GetModulePath(szPath, sizeof(szPath));
	_strcat(szPath, "Debug.log");
	hFile = sceIoOpen(szPath, PSP_O_CREAT | PSP_O_APPEND | PSP_O_RDWR, 0777);
	if (hFile >= 0) {
		sceIoWrite(hFile, buf, nLen);
		sceIoClose(hFile);
	}
}
