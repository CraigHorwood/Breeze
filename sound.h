#ifndef _SOUND_H
#define _SOUND_H
#define SOUND_CASH 0
#define SOUND_HIT 1
#define SOUND_BONK 2
#define SOUND_WOBBLE 3
#define SOUND_WIND 4
#define SOUND_HEALTH 5
#define SOUND_COIN 6
#define SOUND_FALL 7
#define SOUND_SPIN 8
#define SOUND_METAL 9
#define SOUND_SLAM 10
#define SOUND_GATE_OPEN 11
#define SOUND_GATE_CLOSE 12
#define SOUND_BACKGROUND 13
#define SOUND_RAIN 14
#define SOUND_JUMP 15
void initSound();
void playSound(char);
void playAmbient(char);
void stopAmbient();
void tickSound();
void setBaseVolume(short);
void beginWind();
void endWind();
void deleteSound();
#endif
