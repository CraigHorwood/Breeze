#include "zone.h"
#include "random.h"
#include "imath.h"
#include "renderer.h"
#include "shop.h"
#include <stdlib.h>
#include <math.h>
char getLastBaseMovement(Zone* z)
{
	if (z->lastZone != NULL) return z->lastZone->baseMovement;
	return -1;
}
void addGirder(Zone* zone, int x, int y, int w, char free, char overlapsLast, int i)
{
	Obstacle* o = initObstacle(free ? TYPE_GIRDER_SNAKE : TYPE_GIRDERS_HORIZONTAL, y);
	o->x = x;
	o->w = w;
	Obstacle* last = NULL;
	if (i > 0) last = zone->obstacles[i - 1];
	char movement = zone->baseMovement;
	if (!free)
	{
		int gateOdds = 3;
		if (x == 0 || x == 720 - w) gateOdds = 0;
		else if (movement == MOVEMENT_NONE) gateOdds = 4;
		else if (movement == MOVEMENT_WAVE_VERTICAL || movement == MOVEMENT_WAVE_MODULUS) gateOdds = 1;
		else if (movement == MOVEMENT_FIGURE_EIGHT) gateOdds = 2;
		if (isModeEnabled(MODE_PROTECT)) gateOdds = 0;
		if (nextInt(7) < gateOdds) o->hasGate = ((next() & 3) << 4) + (movement == MOVEMENT_WAVE_VERTICAL ? 72 : 48);
	}
	else o->xo = x;
	if (overlapsLast) movement |= 128;
	zone->height += setObstacleMovement(o, movement, last);
	zone->height += 96;
	zone->obstacles[i] = o;
}
void addVerticalGirder(Zone* zone, int x, int y, int h, char offs, int i)
{
	float pr = 1.0f;
	Obstacle* last = NULL;
	if (offs && i > 0)
	{
		last = zone->obstacles[i - 1];
		pr = nextFloat() * 0.5f + 0.25f;
		if (last->x == 312 && last->movement == MOVEMENT_WAVE_HORIZONTAL) pr = 160.0f / h + 1.0f;
		else if (x == 312)
		{
			if (last->x == 312) pr = 192.0f / h + 1.0f;
			else if (zone->baseMovement == MOVEMENT_WAVE_HORIZONTAL) pr = 160.0f / h + 1.0f;
		}
		last = NULL;
		int ii = i - 1;
		if (zone->baseMovement == MOVEMENT_WAVE_VERTICAL)
		{
			while (ii >= 0)
			{
				if (zone->obstacles[ii]->x != 312) break;
				ii--;
			}
		}
		if (ii >= 0) last = zone->obstacles[ii];
	}
	int ya = (int) (pr * h) - h;
	if (ya < -128) ya = -128;
	y += ya;
	Obstacle* o = initObstacle(TYPE_GIRDERS_VERTICAL, y);
	o->x = x;
	o->w = h;
	if (offs && i == zone->count - 1) o->z = 1;
	zone->height += setObstacleMovement(o, zone->baseMovement, last);
	if (offs && i < zone->count - 1 && o->z == 2) ya = 192;
	zone->height += h + ya;
	zone->obstacles[i] = o;
}
void addBlock(Zone* zone, int x, int y, Obstacle* last, char movement, int i)
{
	Obstacle* o = initObstacle(TYPE_BLOCKS, y);
	o->x = x;
	o->w = 128;
	zone->height += setObstacleMovement(o, movement, last);
	zone->height += 128;
	zone->obstacles[i] = o;
}
void addGun(Zone* zone, int x, int y, int dir, int speed, int time, int i)
{
	Obstacle* o = initObstacle(TYPE_GUNS, y);
	o->x = x;
	o->w = dir;
	if (speed <= 0)
	{
		speed = nextInt(3) + 2;
		if (nextInt(3) == 0)
		{
			if (speed == 2) speed = 1;
			else if (speed == 4 && speed == -1) speed = 5;
		}
	}
	if (time == -1) time = next() & 2047;
	o->speed = speed;
	o->time = time;
	o->z = o->speed << 4;
	zone->obstacles[i] = o;
}
void buildGirders(Zone* zone)
{
	int count = (next() & 1) + 2;
	if ((next() & 3) == 0) count++;
	if (nextInt(6) == 0) count = 1;
	zone->obstacles = (Obstacle**) malloc(sizeof(Obstacle*) * count);
	zone->count = count;
	count--;
	char isSwitch = count > 0 && (next() & 3) == 0;
	zone->baseMovement = MOVEMENT_NONE;
	if (!isModeEnabled(MODE_PROTECT) && ((next() & 1) || getLastBaseMovement(zone) == MOVEMENT_NONE))
	{
		zone->baseMovement = (char) (nextInt(7) + 1);
		if (isSwitch) zone->baseMovement = (next() & 1) ? MOVEMENT_WAVE_HORIZONTAL : MOVEMENT_WAVE_VERTICAL;
	}
	int lastPos = 0;
	char overlapsLast = 0;
	int left = next() & 1;
	for (int i = 0; i <= count; i++)
	{
		int pos = nextInt(3) - 1;
		if (pos == lastPos) pos = nextInt(3) - 1;
		if (lastPos && pos == 0 && nextInt(3) == 0) pos = -lastPos;
		int x = 0;
		int w = ((next() & 3) * 24) + 192;
		if (isSwitch)
		{
			if ((i & 1) == left) x = 0;
			else x = 720 - w;
		}
		else
		{
			x = 360 + pos * 180 - (w >> 1);
			if (nextInt(3) == 0) x -= pos * 90;
		}
		char overlapsNext = zone->baseMovement == MOVEMENT_WAVE_VERTICAL && !isSwitch && i > 0 && (next() & 3) == 0;
		addGirder(zone, x, zone->y + zone->height, w, 0, overlapsLast, i);
		if (i < count)
		{
			float range = (nextFloat() - nextFloat() + 1.0f) / 2.0f;
			if (zone->baseMovement == MOVEMENT_WAVE_VERTICAL || zone->baseMovement == MOVEMENT_ELLIPSE || zone->baseMovement == MOVEMENT_RECTANGLE || zone->baseMovement == MOVEMENT_WAVE_MODULUS || zone->baseMovement == MOVEMENT_FIGURE_EIGHT)
			{
				if (overlapsNext)
				{
					zone->height -= (int) (range * 40 + 160);
					overlapsLast = overlapsNext;
				}
				else zone->height += (int) (range * 80 + 160);
			}
			else zone->height += (int) (range * 152 + 360);
		}
		lastPos = pos;
	}
}
void buildVerticalGirders(Zone* zone)
{
	int count = nextInt(3) + 4;
	if ((next() & 3) == 0) count++;
	zone->baseMovement = MOVEMENT_NONE;
	if (!isModeEnabled(MODE_PROTECT) && (nextInt(3) || getLastBaseMovement(zone) == MOVEMENT_NONE))
	{
		zone->baseMovement = (next() & 3) + 1;
		count -= 2;
	}
	zone->baseMovement = MOVEMENT_WAVE_VERTICAL;
	zone->obstacles = (Obstacle**) malloc(sizeof(Obstacle*) * count);
	zone->count = count;
	count--;
	char pos = next() & 3;
	for (int i = 0; i <= count; i++)
	{
		int x = (next() & 1) ? 84 : 132;
		if (zone->baseMovement == MOVEMENT_RECTANGLE) x = (pos & 1) ? x : 624 - x;
		else x = (pos & 1) ? ((pos & 3) == 1 ? x : 624 - x) : 312;
		addVerticalGirder(zone, x, zone->y + zone->height, ((next() & 3) + 4) * 96, 1, i);
		if ((pos & 1) == 0 && (next() & 3) == 0 && zone->baseMovement == MOVEMENT_RECTANGLE) pos += 2;
		else pos++;
	}
}
void buildGirderSnake(Zone* zone)
{
	int count = (next() & 3) + 4;
	if ((next() & 3) == 0) count += (next() & 1) + 1;
	zone->obstacles = (Obstacle**) malloc(sizeof(Obstacle*) * count);
	zone->count = count;
	count--;
	int offs = next() & 3;
	int ha = 0;
	for (int i = 0; i <= count; i++)
	{
		if (((i + offs) & 1) == 0)
		{
			int x = 96;
			if (nextInt(3) == 0) x = 84;
			if (((i + offs) & 3) == 0) x = 624 - x;
			int h = (nextInt(15) << 5) + 320;
			addVerticalGirder(zone, x, zone->y + zone->height, h, 0, i);
			if (i < count)
			{
				ha = (next() & 1) ? 160 : 96;
				if (ha == 160 && (next() & 1)) ha = 192;
				zone->height += ha;
			}
		}
		else
		{
			char free = ha < 192 || (next() & 3);
			if (!free)
			{
				int w = 192 + (next() & 3) * 24;
				addGirder(zone, nextInt(360) + 180 - w, zone->y + zone->height, w, 0, 0, i);
			}
			else
			{
				int x0 = 132;
				if ((next() & 3) == 0) x0 = 96;
				int x1 = 720 - x0;
				if (i > 0)
				{
					if ((next() & 3) == 0)
					{
						zone->height -= ha + 96;
						int xx = zone->obstacles[i - 1]->x;
						if (xx < 312) x0 = xx + 256;
						else x1 = xx - 160;
					}
					else if (ha == 96)
					{
						if (zone->obstacles[i - 1]->x < 312) x0 = 192;
						else x1 = 528;
					}
				}
				addGirder(zone, x0, zone->y + zone->height, x1 - x0, 1, 0, i);
			}
			if (i < count)
			{
				if (free)
				{
					ha = (next() & 1) ? 160 : 96;
					if (ha == 160 && (next() & 1)) ha = 192;
				}
				else ha = 192;
				zone->height += ha;
			}
		}
	}
}
void buildBlocks(Zone* zone)
{
	if (nextInt(7) == 0 || isModeEnabled(MODE_PROTECT)) zone->baseMovement = MOVEMENT_NONE;
	else zone->baseMovement = nextInt(5) + 1;
	int count = (next() & 3) + 4;
	if ((next() & 15) < count) count += nextInt(3) + 1;
	if (zone->baseMovement == MOVEMENT_WAVE_VERTICAL && count > 7) count >>= 1;
	zone->obstacles = (Obstacle**) malloc(sizeof(Obstacle*) * count);
	zone->count = count;
	count--;
	if (zone->baseMovement == MOVEMENT_NONE || zone->baseMovement == MOVEMENT_WAVE_HORIZONTAL || zone->baseMovement == MOVEMENT_MODULUS)
	{
		for (int i = 0; i <= count; i++)
		{
			addBlock(zone, nextInt(592), zone->y + zone->height, NULL, zone->baseMovement, i);
			float ha = (nextFloat() - nextFloat() + 1.0f) / 2.0f;
			if (i < count)
			{
				if (zone->baseMovement == MOVEMENT_NONE && (next() & 1)) zone->height += (int) (ha * 80 + 160);
				else zone->height += (int) (ha * 152 + 360);
			}
		}
	}
	else if (zone->baseMovement == MOVEMENT_WAVE_VERTICAL)
	{
		Obstacle* lastAt[3];
		for (int i = 0; i < 3; i++)
		{
			lastAt[i] = NULL;
		}
		for (int i = 0; i <= count; i++)
		{
			int pos = nextInt(3);
			Obstacle* last = lastAt[pos];
			int x = 296 + (pos - 1) * 164;
			if (nextInt(3) == 0) x += (pos - 1) * 48;
			int y = zone->y + zone->height;
			if (last != NULL && y - last->y < 288)
			{
				y = last->y + 288;
				zone->height = y - zone->y;
			}
			addBlock(zone, x, y, last, zone->baseMovement, i);
			last = zone->obstacles[i];
			lastAt[pos] = last;
			if (i < count) zone->height -= (int) (last->z * (nextFloat() * 0.75f + 0.25f));
		}
	}
	else
	{
		int maxAlts = zone->baseMovement == MOVEMENT_RECTANGLE ? (next() & 3) + 1 : 0;
		Obstacle* last = NULL;
		int synchRun = 0;
		char left = next() & 1;
		for (int i = 0; i <= count; i++)
		{
			if (synchRun == 0 && nextInt(3) == 0) synchRun = nextInt(((count + 1) >> 1) + 1) + 1;
			if (maxAlts > 0 && (next() & 3) == 0)
			{
				if (i > 0) zone->height += 160;
				char movement = (next() & 1) ? MOVEMENT_WAVE_HORIZONTAL : MOVEMENT_MODULUS;
				addBlock(zone, 0, zone->y + zone->height, NULL, movement, i);
				if (i < count) zone->height += (int) (((nextFloat() - nextFloat() + 1.0f) / 2.0f) * 152) + 160;
				maxAlts--;
			}
			else
			{
				int x = (i & 1) == left ? 16 : 576;
				if (synchRun > 0)
				{
					if (i > 0 && zone->obstacles[i - 1]->movement == MOVEMENT_ELLIPSE)
					{
						last = zone->obstacles[i - 1];
						synchRun--;
					}
				}
				addBlock(zone, x, zone->y + zone->height, last, MOVEMENT_ELLIPSE, i);
				last = NULL;
			}
		}
	}
}
void buildCranes(Zone* zone)
{
	int count = (next() & 3) + 3;
	if ((next() & 3) == 0) count = 2;
	else if (nextInt(3) == 0) count--;
	zone->obstacles = (Obstacle**) malloc(sizeof(Obstacle*) * count);
	zone->count = count;
	count--;
	char left = next() & 1;
	char randomTime = nextInt(3) == 0;
	int speed = (next() & 3) + 3;
	for (int i = 0; i <= count; i++)
	{
		Obstacle* o = initObstacle(TYPE_CRANES, zone->y + zone->height);
		o->movement = MOVEMENT_WAVE_VERTICAL;
		if ((i & 1) == left) o->x = 0;
		else o->x = 180;
		o->w = nextInt(320) + 240;
		if (randomTime || i == 0) o->time = next() & 2047;
		else o->time = zone->obstacles[i - 1]->time;
		o->speed = speed;
		zone->obstacles[i] = o;
		zone->height += o->w;
		zone->height += 192;
		if (i < count && nextInt(3)) zone->height += nextInt(240) + 160;
	}
}
void buildRing(Zone* zone)
{
	int count = (nextInt(7) >> 1) + 2;
	zone->obstacles = (Obstacle**) malloc(sizeof(Obstacle*) * count);
	zone->count = count;
	int speed = nextInt(3) + 3;
	if (count == 2) speed = (next() & 3) + 5;
	else if (count >= 4) speed = nextInt(3) + 2;
	if (next() & 1) speed = -speed;
	int w = nextInt(3) == 0 && count < 5 ? (next() & 3) + 7 : 0;
	for (int i = 0; i < count; i++)
	{
		Obstacle* o = initObstacle(TYPE_RING, zone->y);
		o->x = (int) ((float) i / count * 2048);
		o->w = w;
		setObstacleMovement(o, MOVEMENT_ELLIPSE, NULL);
		o->speed = speed;
		zone->obstacles[i] = o;
	}
	zone->height = w > 0 ? 976 : 720;
}
void buildBeams(Zone* zone)
{
	int count = (nextInt(3) >> 1) + 1;
	zone->obstacles = (Obstacle**) malloc(sizeof(Obstacle*) * count);
	zone->count = count;
	count--;
	char hasWavers = nextInt(3) == 0;
	for (int i = 0; i <= count; i++)
	{
		Obstacle* o = initObstacle(TYPE_BEAMS, zone->y + zone->height);
		o->w = (next() & 511) + 640;
		o->x = 360 - (o->w >> 1);
		o->z = (next() & 3) == 0 ? 48 : (nextInt(7) * 48) + 192;
		float ha = (float) sqrt(o->w * o->w + ((o->z * o->z) << 2));
		o->y += (int) (ha / 2.0f - o->z);
		o->speed = (next() & 7) + 1;
		if (next() & 1) o->speed = -o->speed;
		o->time = next() & 2047;
		zone->obstacles[i] = o;
		zone->height += ha;
//		if (hasWavers && ((next() & 1) || (i > 0 && zone->obstacles[i - 1]->movement == MOVEMENT_NONE))) zone->height += setObstacleMovement(o, MOVEMENT_WAVE_VERTICAL, NULL);
		if (i < count) zone->height += nextInt(320) + 320;
	}
}
void buildPistons(Zone* zone)
{
	int count = (next() & 3) + 3;
	if (count < 5 && nextInt(3) == 0) count--;
	zone->obstacles = (Obstacle**) malloc(sizeof(Obstacle*) * count);
	zone->count = count;
	count--;
	char left = next() & 1;
	for (int i = 0; i <= count; i++)
	{
		Obstacle* o = initObstacle(TYPE_PISTONS, zone->y + zone->height);
		o->w = nextInt(240) + 360;
		o->z = 0;
		if ((i & 1) == left) o->z = 1;
		else o->w = 656 - o->w;
		o->speed = nextInt(3) + 2;
		if (o->z == 0) o->xo = 624;
		else o->xo = 32;
		o->time = next() & 2047;
		o->movement = MOVEMENT_WAVE_HORIZONTAL;
		zone->obstacles[i] = o;
		zone->height += 160;
		if (i < count) zone->height += (nextInt(240) + 240);
	}
}
void buildSwings(Zone* zone)
{
	int count = (next() & 3) + 2;
	zone->obstacles = (Obstacle**) malloc(sizeof(Obstacle*) * count);
	zone->count = count;
	count--;
	char left = next() & 1;
	char isCircles = nextInt(3) == 0;
	for (int i = 0; i <= count; i++)
	{
		Obstacle* o = initObstacle(TYPE_SWINGS, zone->y + zone->height);
		if ((i & 1) == left) o->x = 0;
		else o->x = 270;
		o->xo = o->x > 0 ? 244 : 476;
		o->w = ((next() & 3) * 88) + 264;
		if (((next() & 3) == 0) ^ isCircles) o->z = 0;
		else o->z = ((next() & 15) << 4) + 128;
		o->speed = (next() & 3) + 2;
		if (next() & 1) o->speed = -o->speed;
		o->time = next() & 2047;
		o->movement = MOVEMENT_ELLIPSE;
		char v = next() & 7;
		if (v > 0 && v < 3) v = 1;
		else if (v > 2) v = 2;
		o->z |= ((2 - v) << 16);
		zone->obstacles[i] = o;
		zone->height += o->w;
		zone->height += 192;
		if ((o->z & 0xFFFF) == 0) zone->height += o->w;
		else if (i < count) zone->height += nextInt(240) + 160;
	}
}
void buildGuns(Zone* zone)
{
	int count = (next() & 3) + 4;
	if (nextInt(3) == 0) count--;
	else if (nextInt(3) == 0) count += (next() & 1) + 1;
	zone->obstacles = (Obstacle**) malloc(sizeof(Obstacle*) << 6);
	count--;
	zone->baseMovement = MOVEMENT_NONE;
	char hasVertical = next() & 1;
	int verticalCount = 0;
	char left = next() & 1;
	for (int i = 0; i <= count; i++)
	{
		if (hasVertical && verticalCount == 0 && nextInt(3) == 0) verticalCount = nextInt(3) + 1;
		if (verticalCount > 0)
		{
			int x = 132;
			if (left) x = 624 - x;
			addVerticalGirder(zone, x, zone->y + zone->height, ((next() & 7) + 4) * 96, 0, i);
			if (i < count) zone->height += (int) ((nextFloat() - nextFloat() + 1.0f) / 2.0f * 80) + 160;
			if (--verticalCount == 0) verticalCount = -(next() & 1) - 1;
			int guns = nextInt(3) + 1;
			Obstacle* parent = zone->obstacles[i];
			int space = ((next() & 3) << 4) + 32;
			int size = guns * 96 + (guns - 1) * space;
			space += 96;
			int pos = nextInt(3);
			if (pos == 0) pos = parent->y;
			else if (pos == 1) pos = parent->y + ((parent->w - size) >> 1);
			else pos = parent->y + parent->w - size;
			int speed = guns == 1 ? -1 : 0;
			if (nextInt(3) == 0) speed = nextInt(3) + 2;
			int time = -1;
			if (nextInt(3) == 0 && speed == 0) time = next() & 2047;
			int timeA = (int) (((nextFloat() - nextFloat() + 1.0f) / 2.0f) * 100) + 30;
			for (int j = 0; j < guns; j++)
			{
				int yy = pos + j * space;
				if (yy < parent->y) break;
				int xx = parent->x;
				int dir = 0;
				if (parent->x < 360)
				{
					xx += 96;
					dir = 2;
				}
				else xx -= 96;
				if (time >= 0) time += timeA;
				addGun(zone, xx, yy, dir, speed, time, ++i);
				count++;
			}
			left = (~left) & 1;
			continue;
		}
		else if (verticalCount < 0) verticalCount++;
		int x = 0;
		int w = ((next() & 3) * 24) + 192;
		if (left) x = 0;
		else x = 720 - w;
		addGirder(zone, x, zone->y + zone->height, w, 0, 0, i);
		int guns = nextInt(3) + 1;
		Obstacle* parent = zone->obstacles[i];
		int space = ((next() & 3) * 12) + 16;
		int size = guns * 96 + (guns - 1) * space;
		space += 96;
		int pos = nextInt(3);
		if (parent->x == 0)
		{
			if (pos == 0) pos = parent->w;
			else if (pos == 1) pos = parent->w + ((720 - parent->w - size) >> 1);
			else pos = 720 - size;
		}
		else
		{
			if (pos == 0) pos = 0;
			else if (pos == 1) pos = (720 - parent->w - size) >> 1;
			else pos = parent->x - size;
		}
		char isFirst = i == 0;
		char isLast = i == count;
		int speed = guns == 1 ? -1 : 0;
		if (nextInt(3) == 0) speed = nextInt(3) + 2;
		int time = -1;
		if (nextInt(3) == 0 && speed == 0) time = next() & 2047;
		int timeA = (int) (((nextFloat() - nextFloat() + 1.0f) / 2.0f) * 100) + 30;
		for (int j = 0; j < guns; j++)
		{
			int xx = pos + j * space;
			int yy = parent->y;
			int dir = 3;
			if (((next() & 1) && !isLast) || isFirst)
			{
				dir = 1;
				yy += 96;
			}
			else yy -= 96;
			if (time >= 0) time += timeA;
			addGun(zone, xx, yy, dir, speed, time, ++i);
			count++;
		}
		if (i < count) zone->height += (int) ((nextFloat() - nextFloat() + 1.0f) / 2.0f * 152) + 360;
		left = (~left) & 1;
	}
	zone->obstacles = realloc(zone->obstacles, ++count * sizeof(Obstacle*));
	zone->count = count;
}
Zone* initZone(int y, Zone* lastZone, char baseType)
{
	Zone* zone = (Zone*) malloc(sizeof(Zone));
	zone->y = y;
	zone->lastZone = lastZone;
	zone->baseType = baseType;
	zone->height = 0;
	zone->count = 0;
	if (isModeEnabled(MODE_GAUNTLET)) return zone;
	switch (baseType)
	{
	case TYPE_GIRDERS_HORIZONTAL:
		buildGirders(zone);
		break;
	case TYPE_GIRDERS_VERTICAL:
		buildVerticalGirders(zone);
		break;
	case TYPE_GIRDER_SNAKE:
		buildGirderSnake(zone);
		break;
	case TYPE_BLOCKS:
		buildBlocks(zone);
		break;
	case TYPE_CRANES:
		buildCranes(zone);
		break;
	case TYPE_RING:
		buildRing(zone);
		break;
	case TYPE_BEAMS:
		buildBeams(zone);
		break;
	case TYPE_PISTONS:
		buildPistons(zone);
		break;
	case TYPE_SWINGS:
		buildSwings(zone);
		break;
	case TYPE_GUNS:
		buildGuns(zone);
		break;
	default:
		free(zone);
		zone = NULL;
		break;
	}
	return zone;
}
char checkCollide(int x00, int y00, int x10, int y10, int x01, int y01, int x11, int y11)
{
	return x10 >= x01 && x00 <= x11 && y10 >= y01 && y00 <= y11;
}
char checkLine(int x, int y, int w, int xo, int yo)
{
	int yd = abs(y - yo);
	if (yd > 48) return 0;
	int r = (int) (icos(yd / 48.0f * 512) * 96) >> 1;
	return xo + r >= x && xo - r <= x + w;
}
char collideZone(Zone* zone, int xPlayer, int yPlayer, int xaPlayer, int yaPlayer)
{
	int x0 = xPlayer - 40;
	int y0 = yPlayer - 40;
	int x1 = xPlayer + 40;
	int y1 = yPlayer + 40;
	for (int i = 0; i < zone->count; i++)
	{
		Obstacle* o = zone->obstacles[i];
		switch (o->type)
		{
		case TYPE_GIRDERS_HORIZONTAL:
		{
			if (o->hasGate && (o->gate & 127) >= 4)
			{
				if (checkCollide(o->x, o->y + 80, o->x + o->w, o->y + 96, x0, y0, x1, y1))
				{
					if (yPlayer > o->y + 112) return 10;
					return 11;
				}
			}
			int xp0 = o->x + 80;
			int xp1 = o->x + o->w - 80;
			if (o->x == 0) xp0 = 0;
			else if (o->x == 720 - o->w) xp1 = 720;
			if (y1 >= o->y && y0 <= o->y + 96 && (x0 <= o->x || x1 >= o->x + o->w))
			{
				if (yPlayer - 48.0f > o->y + 48) return 10;
				int xd = o->x - xPlayer + 48;
				if (xd < 0)
				{
					xd = o->x + o->w - xPlayer + 64;
					if (xd > 0) return 8;
				}
				else if (xd < 64) return 9;
				return 11;
			}
			break;
		}
		case TYPE_GIRDERS_VERTICAL:
		{
			if (checkCollide(o->x, o->y, o->x + 96, o->y + o->w, x0, y0, x1, y1))
			{
				if (o->y > yPlayer) return 11;
				int xc = getXCenter(o);
				if (xPlayer < xc)
				{
					if (xaPlayer >= 4) return 16;
					return 8;
				}
				if (xaPlayer <= -4) return 17;
				return 9;
			}
			break;
		}
		case TYPE_GIRDER_SNAKE:
		{
			if (checkCollide(o->x, o->y, o->x + o->w, o->y + 96, x0, y0, x1, y1))
			{
				if (yPlayer - 48 > o->y + 48) return 10;
				int xd = o->x - xPlayer + 64;
				if (xd < 0)
				{
					xd = o->x + o->w - xPlayer;
					if (xd < 0) return 9;
					return 11;
				}
				return 8;
			}
			break;
		}
		case TYPE_BLOCKS:
		case TYPE_RING:
		{
			if (checkCollide(o->x, o->y, o->x + 128, o->y + 128, x0, y0, x1, y1))
			{
				if (yPlayer - 48 > o->y + 64) return 10;
				if (yPlayer + 16 < o->y) return 11;
				if (xPlayer < o->x + 64) return 8;
				return 9;
			}
			break;
		}
		case TYPE_CRANES:
		{
			if (checkCollide(o->x, o->yo + o->w + 96, o->x + 540, o->yo + o->w + 192, x0, y0, x1, y1))
			{
				int xd = xPlayer - getXCenter(o);
				if (abs(xd) < 116)
				{
					int yd = yaPlayer;
					if (yd < 8) yd = 8;
					if (yPlayer + 48 < o->yo + yd) return 11;
					return 10;
				}
				if (xd < 0) return 8;
				return 9;
			}
			int xx = o->x > 0 ? o->x - 104 : 404;
			if (checkCollide(xx, o->y, xx + 240, o->y + 96, x0, y0, x1, y1))
			{
				if (yPlayer + 24 < o->y) return 11;
				return 10;
			}
			break;
		}
		case TYPE_BEAMS:
		{
			int xx = xPlayer;
			int yy = yPlayer;
			if (o->time & 2047)
			{
				int xc = getXCenter(o);
				int yc = getYCenter(o);
				xx -= xc;
				yy -= yc;
				int xxo = xx;
				int yyo = yy;
				float rCos = icos(o->time);
				float rSin = isin(o->time);
				xx = (int) (xxo * rCos - yyo * rSin) + xc;
				yy = (int) (yyo * rCos + xxo * rSin) + yc;
			}
			int count = o->z == 48 ? 1 : 2;
			for (int i = 0; i < count; i++)
			{
				int yp = o->y;
				if (i > 0) yp += (o->z << 1) - 96;
				if (checkLine(o->x, yp, o->w, xx, yy)) return 10;
				if (checkLine(o->x, yp + 96, o->w, xx, yy)) return 10;
			}
			break;
		}
		case TYPE_PISTONS:
		{
			if (checkCollide(o->x, o->y, o->x + 64, o->y + 160, x0, y0, x1, y1))
			{
				int yd = o->y + 80 - yPlayer;
				if (abs(yd) >= 80)
				{
					if (yd < 0) return 10;
					return 11;
				}
				if (o->z)
				{
					if (xPlayer > o->x + 32) return 17;
					return 8;
				}
				if (xPlayer < o->x + 32) return 16;
				return 9;
			}
			int xp = o->x + 64;
			if (o->z) xp = o->x - 656;
			int yp = o->y + 48;
			if (checkCollide(xp, yp, xp + 656, yp + 64, x0, y0, x1, y1))
			{
				int yd = o->y + 80 - yPlayer;
				if (yd < 0) return 10;
				return 11;
			}
			break;
		}
		case TYPE_SWINGS:
		{
			if (checkCollide(o->xo == 476 ? 0 : 180, o->yo + o->w + 96, o->xo == 476 ? 540 : 720, o->yo + o->w + 192, x0, y0, x1, y1))
			{
				int xd = xPlayer - o->xo;
				if (o->xo == 180 && xd > 80) return 8;
				if (o->xo != 180 && xd < -80) return 9;
				if (yPlayer > o->y + 48) return 10;
				return 11;
			}
			int r = 0;
			int zz = o->z & 0xFFFF;
			if (zz > 0) r = (int) (isin(o->time << 1) * zz);
			else r = o->time << 1;
			int xc = o->xo - isin(r) * (o->w + 48);
			int yc = o->yo + o->w - icos(r) * (o->w + 48) + 96;
			int xd = xPlayer - xc;
			int yd = yPlayer - yc;
			int rSqr = 12544;
			if ((o->z >> 16) == 2) rSqr = 9216;
			if (xd * xd + yd * yd <= rSqr)
			{
				if (xd < 0)
				{
					if (xd < -80) return 2;
					return 8;
				}
				if (xd > 80) return 3;
				return 9;
			}
			break;
		}
		default:
			break;
		}
	}
	return 0;
}
void deleteZone(Zone* zone)
{
	for (int i = 0; i < zone->count; i++)
	{
		Obstacle* o = zone->obstacles[i];
		if (o->movementArgs != NULL) free(o->movementArgs);
		free(o);
	}
}
