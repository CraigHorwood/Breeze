#include "sound.h"
#include "SDL2_mixer/SDL_mixer.h"
Mix_Chunk* SOUND_TABLE[16];
char hasSound = 0, incWind = 0;
short baseVolume = 0, realVolume = 0;
signed char currentAmbient = -1;
void initSound()
{
	if (Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 2048) != -1)
	{
		Mix_AllocateChannels(32);
		Mix_ReserveChannels(2);
		SOUND_TABLE[0] = Mix_LoadWAV("snd/cash.wav");
		SOUND_TABLE[1] = Mix_LoadWAV("snd/hit.wav");
		SOUND_TABLE[2] = Mix_LoadWAV("snd/bonk.wav");
		SOUND_TABLE[3] = Mix_LoadWAV("snd/wobble.wav");
		SOUND_TABLE[4] = Mix_LoadWAV("snd/wind.wav");
		SOUND_TABLE[5] = Mix_LoadWAV("snd/health.wav");
		SOUND_TABLE[6] = Mix_LoadWAV("snd/coin.wav");
		SOUND_TABLE[7] = Mix_LoadWAV("snd/fall.wav");
		SOUND_TABLE[8] = Mix_LoadWAV("snd/spin.wav");
		SOUND_TABLE[9] = Mix_LoadWAV("snd/metal.wav");
		SOUND_TABLE[10] = Mix_LoadWAV("snd/slam.wav");
		SOUND_TABLE[11] = Mix_LoadWAV("snd/gate0.wav");
		SOUND_TABLE[12] = Mix_LoadWAV("snd/gate1.wav");
		SOUND_TABLE[13] = Mix_LoadWAV("snd/bg.wav");
		SOUND_TABLE[14] = Mix_LoadWAV("snd/rain.wav");
		SOUND_TABLE[15] = Mix_LoadWAV("snd/jump.wav");
		Mix_Volume(0, 0);
		Mix_PlayChannel(0, SOUND_TABLE[SOUND_WIND], -1);
		hasSound = 1;
	}
}
void tickSound()
{
	if (!hasSound) return;
	char isBaseVolume = realVolume != 16;
	if (incWind && realVolume < 16)
	{
		realVolume++;
		Mix_Volume(0, (int) (realVolume / 16.0f * MIX_MAX_VOLUME));
		isBaseVolume = 0;
	}
	else if (!incWind && realVolume > baseVolume)
	{
		realVolume--;
		Mix_Volume(0, (int) (realVolume / 16.0f * MIX_MAX_VOLUME));
		isBaseVolume = 0;
	}
	if (isBaseVolume) Mix_Volume(0, (int) (baseVolume / 16.0f * MIX_MAX_VOLUME));
//	if (windTime > 0)
//	{
//		if (windPlaying && windTime < 16) windTime++;
//		else if (!windPlaying && windTime > 0) windTime--;
//		Mix_Volume(0, (int) (windTime / 16.0f * MIX_MAX_VOLUME));
//	}
}
void playSound(char i)
{
	if (hasSound && i >= 0 && i < 16) Mix_PlayChannel(-1, SOUND_TABLE[i], 0);
}
void playAmbient(char i)
{
	if (currentAmbient != i)
	{
		Mix_HaltChannel(1);
		Mix_PlayChannel(1, SOUND_TABLE[i], -1);
		currentAmbient = i;
	}
}
void stopAmbient()
{
	if (currentAmbient != -1)
	{
		Mix_HaltChannel(1);
		currentAmbient = -1;
	}
}
void setBaseVolume(short volume)
{
	if (volume < 0) volume = -volume;
	baseVolume = volume;
}
void beginWind()
{
	if (hasSound && !incWind)
	{
		realVolume = baseVolume;
		incWind = 1;
//		Mix_FadeInChannel(0, SOUND_TABLE[SOUND_WIND], -1, 256);
	}
}
void endWind()
{
	if (hasSound && incWind)
	{
		incWind = 0;
//		Mix_FadeOutChannel(0, 256);
	}
}
void deleteSound()
{
	if (!hasSound) return;
	for (int i = 0; i < 16; i++)
	{
		Mix_FreeChunk(SOUND_TABLE[i]);
	}
	Mix_CloseAudio();
}
