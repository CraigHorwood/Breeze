#ifndef _GAME_H
#define _GAME_H
#include "obstacle.h"
char* msg;
unsigned char* levelData;
void resetGame();
void tickGame();
char isOnScreen(float, float);
void renderGame();
void renderScore(const char*, char, char, long long, float, float);
void gameTouchStart(int, int, int);
void gameTouchEnd(int, int, int);
void gameTouchMove(int);
void gameTilt(float);
void deleteGame();
#endif
