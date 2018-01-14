#ifndef _GAME_H
#define _GAME_H
#include "obstacle.h"
char* msg;
short* levelData;
void resetGame();
void tickGame();
void saveGame();
void renderGame();
void renderScore(const char*, char, char, long long, float, float);
void gameTouchStart(int, int, int);
void gameTouchEnd(int, int, int);
void gameTouchMove(int);
void gameTilt(float);
void deleteGame();
#endif
