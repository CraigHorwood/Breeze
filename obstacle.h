#ifndef _OBSTACLE_H
#define _OBSTACLE_H
#define MOVEMENT_NONE 0
#define MOVEMENT_WAVE_HORIZONTAL 1
#define MOVEMENT_WAVE_VERTICAL 2
#define MOVEMENT_ELLIPSE 3
#define MOVEMENT_RECTANGLE 4
#define MOVEMENT_MODULUS 5
#define MOVEMENT_WAVE_MODULUS 6
#define MOVEMENT_FIGURE_EIGHT 7
typedef struct
{
	char type, movement;
	short* movementArgs;
	int w, x, y, z;
	int xo, yo;
	int time, speed;
	int hasGate;
	char gate;
} Obstacle;
Obstacle* initObstacle(char, int);
int setObstacleMovement(Obstacle*, char, Obstacle*);
char tickObstacle(Obstacle*, float);
void renderObstacle(Obstacle*);
int getXCenter(Obstacle*);
int getYCenter(Obstacle*);
#endif
