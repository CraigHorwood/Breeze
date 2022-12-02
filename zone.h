#ifndef _ZONE_H
#define _ZONE_H
#include "obstacle.h"
#define TYPE_GIRDERS_HORIZONTAL 0
#define TYPE_GIRDERS_VERTICAL 1
#define TYPE_BLOCKS 2
#define TYPE_RING 3
#define TYPE_GIRDER_SNAKE 4
#define TYPE_SWINGS 5
#define TYPE_BEAMS 6
#define TYPE_PISTONS 7
#define TYPE_CRANES 8
#define TYPE_GUNS 9
typedef struct Zone Zone;
struct Zone
{
	int y, height;
	Zone* lastZone;
	char baseType, baseMovement;
	Obstacle** obstacles;
	int count;
};
Zone* initZone(int, Zone*, char);
char checkCollide(int, int, int, int, int, int, int, int);
char collideZone(Zone*, int, int, int, int);
void deleteZone(Zone*);
#endif
