#ifndef _SOUND_MANAGER
#define _SOUND_MANAGER

#include "nes/types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// ���Lenum��char*array�͓��������ŕ���ł���K�v������܂��B
enum {
	SAMPLINGRATE_44100 = 0,
	SAMPLINGRATE_22050,
	SAMPLINGRATE_11025,
	SAMPLINGRATE_COUNT	// Last
};

extern const char *aszSamplingRateName[];

// �T���v�����O���[�g
int Snd_GetSampleRate(void);

// �ʎq���r�b�g
int Snd_GetSampleBits(void);

// ���ݍĐ����̏ꏊ��������(�S�̂𒆐S����݂đO����납)
// returns SOUND_BUF_LOW or SOUND_BUF_HIGH
int Snd_GetCurrentPlayingBank();

// Lock buffer
boolean Snd_Lock(int which, int16** buf, uint32* buf_len);

// Unlock buffer
void Snd_UnLock(void);

// clear buffer
void Snd_ClearBuffer(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
