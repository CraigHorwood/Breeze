#include "obstacle.h"
#include "zone.h"
#include "renderer.h"
#include "random.h"
#include "imath.h"
#include "game.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
Obstacle* initObstacle(char type, int y)
{
	Obstacle* obstacle = (Obstacle*) malloc(sizeof(Obstacle));
	obstacle->type = type;
	obstacle->y = y;
	obstacle->yo = y;
	obstacle->movement = MOVEMENT_NONE;
	obstacle->movementArgs = NULL;
	obstacle->xo = 0;
	obstacle->time = 0;
	obstacle->speed = 1;
	obstacle->hasGate = 0;
	obstacle->gate = 0;
	return obstacle;
}
int setObstacleMovement(Obstacle* o, char movement, Obstacle* last)
{
	o->movement = movement & 127;
	switch (o->movement)
	{
	case MOVEMENT_WAVE_HORIZONTAL:
	{
		if (o->type == TYPE_GIRDERS_HORIZONTAL || o->type == TYPE_BLOCKS)
		{
			char isSwitch = o->type == TYPE_GIRDERS_HORIZONTAL && (o->x == 0 || o->x == 720 - o->w);
			if (isSwitch && last != NULL) o->speed = last->speed;
			else o->speed = (int) ((nextFloat() - nextFloat() + 1.0f) / 2.0f * 8.0f) + 4;
			o->time = next() & 2047;
			o->xo = o->x;
			o->movementArgs = (short*) malloc(sizeof(short));
			o->movementArgs[0] = 0;
			if (isSwitch)
			{
				if (o->x != 0) o->xo -= o->w;
				o->w <<= 1;
				o->movementArgs[0] = (short) (o->x == 0 ? -1 : 1);
			}
		}
		else if (o->type == TYPE_GIRDERS_VERTICAL)
		{
			o->movementArgs = (short*) malloc(sizeof(short));
			o->movementArgs[0] = 0;
			if (last != NULL && last->x != 312)
			{
				o->speed = (int) ((nextFloat() - nextFloat() + 1.0f) / 2.0f * 8.0f) + 4;
				o->time = next() & 2047;
				o->xo = o->x;
				int r = 128;
				if (o->x > 312) o->xo -= r;
				int x0 = o->xo;
				int x1 = o->xo + r;
				if (o->x == 312)
				{
					o->xo = 312;
					x0 = o->xo;
					x1 = 492;
					o->z = 2;
				}
				o->movementArgs[0] = (short) (x1 - x0);
			}
		}
		return 0;
	}
	case MOVEMENT_WAVE_VERTICAL:
	{
		if (o->type == TYPE_GIRDERS_HORIZONTAL)
		{
			char isSwitch = o->x == 0 || o->x == 720 - o->w;
			if (last != NULL && isSwitch)
			{
				o->speed = last->speed;
				o->time = last->time - 1184;
				if (o->time < 0) o->time += 2048;
			}
			else if (o->movement < 0)
			{
				o->speed = last->speed;
				o->time = last->time;
			}
			else
			{
				o->speed = (int) ((nextFloat() - nextFloat() + 1.0f) / 2.0f * 8.0f) + 4;
				o->time = next() & 2047;
			}
			o->movementArgs = (short*) malloc(sizeof(short));
			o->movementArgs[0] = (short) (nextInt(176) + 144);
			return o->movementArgs[0] << 1;
		}
		else if (o->type == TYPE_GIRDERS_VERTICAL)
		{
			o->movementArgs = (short*) malloc(sizeof(short));
			if (o->x != 312)
			{
				if (last != NULL)
				{
					o->speed = last->speed;
					o->time = last->time;
					memcpy(o->movementArgs, last->movementArgs, sizeof(short));
				}
				else
				{
					o->speed = (int) ((nextFloat() - nextFloat() + 1.0f) / 2.0f * 8.0f) + 4;
					o->time = next() & 2047;
					o->movementArgs[0] = (short) (nextInt(176) + 144);
				}
				if (o->z == 1) return o->movementArgs[0] << 1;
			}
		}
		else if (o->type == TYPE_BLOCKS)
		{
			o->movementArgs = (short*) malloc(sizeof(short));
			if (last != NULL && o->y + o->z >= last->y && o->y < last->y + last->z)
			{
				o->speed = last->speed;
				o->time = last->time;
				memcpy(o->movementArgs, last->movementArgs, sizeof(short));
			}
			else
			{
				o->speed = (int) ((nextFloat() - nextFloat() + 1.0f) / 2.0f * 8.0f) + 4;
				o->time = next() & 2047;
				o->movementArgs[0] = (short) (nextInt(176) + 144);
			}
			o->z = (o->movementArgs[0] << 1) + 128;
			return o->z - 128;
		}
		else if (o->type == TYPE_BEAMS)
		{
			o->speed = (int) ((nextFloat() - nextFloat() + 1.0f) / 2.0f * 4.0f) + 4;
			o->time = next() & 2047;
			o->movementArgs = (short*) malloc(sizeof(short));
			o->movementArgs[0] = (short) ((next() & 511) + 320);
			return o->movementArgs[0] << 1;
		}
		return 0;
	}
	case MOVEMENT_ELLIPSE:
	{
		if (o->type == TYPE_RING)
		{
			o->xo = 296;
			o->yo = o->y + (o->w > 0 ? 424 : 296);
			o->time = o->x;
			o->movementArgs = (short*) malloc(sizeof(short));
			o->movementArgs[0] = 296;
			return 0;
		}
		if (o->type == TYPE_BLOCKS)
		{
			o->xo = o->x;
			o->yo = o->y;
			if (last != NULL)
			{
				o->speed = last->speed;
				if (next() & 1) o->speed = -o->speed;
				o->time = last->time;
				if (next() & 1)
				{
					o->time -= 1184;
					if (o->time < 0) o->time += 2048;
				}
			}
			else
			{
				o->speed = (int) ((nextFloat() - nextFloat() + 1.0f) / 2.0f * 8.0f) + 4;
				if (next() & 1) o->speed = -o->speed;
				o->time = next() & 2047;
			}
			o->movementArgs = (short*) malloc(sizeof(short));
			o->movementArgs[0] = o->x < 360 ? 1 : -1;
			return 256;
		}
		if (o->type == TYPE_GIRDERS_HORIZONTAL) o->yo = o->y;
		else if (o->type == TYPE_GIRDERS_VERTICAL)
		{
			o->xo = o->x + 48;
			o->yo = o->y + (o->w >> 1);
		}
		if (last == NULL || o->type != TYPE_GIRDERS_VERTICAL)
		{
			o->speed = (int) ((nextFloat() - nextFloat() + 1.0f) / 2.0f * 3.0f) + 4;
			if (next() & 1) o->speed = -o->speed;
			o->time = next() & 2047;
			o->movementArgs = (short*) malloc(sizeof(short) << 1);
			if (o->type == TYPE_GIRDERS_HORIZONTAL)
			{
				o->movementArgs[0] = (short) (nextInt(180) + 90);
				if ((next() & 3) == 0) o->movementArgs[0] = (short) (360 - (o->w >> 1));
				o->movementArgs[1] = (short) (nextInt(180) + 180);
			}
			else if (o->type == TYPE_GIRDERS_VERTICAL)
			{
				o->movementArgs[0] = (short) (nextInt(90) + 45);
				o->movementArgs[1] = (short) (nextInt(90) + 45);
			}
		}
		else
		{
			o->speed = last->speed;
			o->time = last->time;
			o->movementArgs = (short*) malloc(sizeof(short) << 1);
			memcpy(o->movementArgs, last->movementArgs, sizeof(short) << 1);
		}
		if (o->type == TYPE_GIRDERS_VERTICAL && o->z != 1) return 0;
		return o->movementArgs[1] << 1;
	}
	case MOVEMENT_RECTANGLE:
	{
		if (o->type == TYPE_GIRDERS_HORIZONTAL) o->xo = 0;
		else if (o->type == TYPE_GIRDERS_VERTICAL) o->xo = o->x;
		o->yo = o->y;
		o->speed = (int) ((nextFloat() - nextFloat() + 1.0f) / 2.0f * 3.0f) + 2;
		o->time = next() & 2047;
		o->movementArgs = (short*) malloc(sizeof(short) * 3);
		o->movementArgs[0] = (short) ((next() & 255) + 160);
		int wo = o->w;
		if (o->type == TYPE_GIRDERS_VERTICAL)
		{
			int xMax = 360;
			int xMin = 360;
			if (o->x < 256) xMax -= 128;
			else if (o->x > 616) xMin += 128;
			if (o->x < 360)
			{
				xMax += 96;
				wo = o->xo + 816 - xMax;
			}
			else
			{
				wo = xMin + 720 - o->xo;
				o->xo = xMin;
			}
		}
		o->movementArgs[1] = (short) (720 - wo);
		o->movementArgs[2] = (short) (1440 - (wo << 1) + (o->movementArgs[0] << 1));
		return o->movementArgs[0];
	}
	case MOVEMENT_MODULUS:
	case MOVEMENT_WAVE_MODULUS:
	{
		int minSpeed = 2;
		int range = 4;
		if (o->type == TYPE_BLOCKS)
		{
			minSpeed = 4;
			range = 8;
		}
		o->speed = (int) ((nextFloat() - nextFloat() + 1.0f) / 2.0f * range) + minSpeed;
		if (next() & 1) o->speed = -o->speed;
		o->time = next() & 2047;
		o->xo = 0;
		if (movement == MOVEMENT_WAVE_MODULUS)
		{
			o->movementArgs = (short*) malloc(sizeof(short) * 3);
			o->movementArgs[0] = (short) (next() & 2047);
			o->movementArgs[1] = (short) ((nextFloat() - nextFloat() + 1.0f) / 2.0f * 3.0f) + 4;
			o->movementArgs[2] = (short) (nextInt(176) + 144);
			if (o->speed > 2) o->speed--;
			return o->movementArgs[2] << 1;
		}
		return 0;
	}
	case MOVEMENT_FIGURE_EIGHT:
	{
		o->speed = (int) ((nextFloat() - nextFloat() + 1.0f) / 2.0f * 3.0f) + 3;
		o->time = next() & 2047;
		o->movementArgs = (short*) malloc(sizeof(short) << 1);
		o->movementArgs[0] = (short) (nextInt(176) + 144);
		o->movementArgs[1] = (short) ((next() & 3) ? 2 : 3);
		if (next() & 1)
		{
			o->speed <<= 1;
			o->movementArgs[1] = -o->movementArgs[1];
		}
		return o->movementArgs[0] << 1;
	}
	default:
		return 0;
	}
}
void moveWaveHorizontal(Obstacle* o)
{
	switch (o->type)
	{
	case TYPE_GIRDERS_HORIZONTAL:
	case TYPE_BLOCKS:
		if (o->movementArgs[0] == 0) o->x = (int) (((720 - o->w) >> 1) * (isin(o->time) + 1.0f));
		else o->x = (int) (o->xo + (isin(o->time) + 1.0f) / 2.0f * (o->w >> 1) * o->movementArgs[0]);
	case TYPE_GIRDERS_VERTICAL:
		if (o->movementArgs[0] != 0) o->x = (int) (o->xo + (isin(o->time) + 1.0f) / 2.0f * o->movementArgs[0]);
		break;
	case TYPE_PISTONS:
		o->z = (o->z << 1) - 1;
		int t = o->time % 640;
		if (t < 40) o->x = o->xo - (int) (t * t / 64.0f * o->z);
		else if (t < 100) o->x = o->xo - o->z * 24 + (int) (((t - 40) / 60.0f) * (o->w - o->xo + o->z * 24));
		else if (t < 320) o->x = o->w;
		else if (t < 512) o->x = o->w - (int) (((t - 320) / 192.0f) * (o->w - o->xo));
		else o->x = o->xo;
		o->z = (o->z + 1) >> 1;
		break;
	default:
		break;
	}
}
void moveWaveVertical(Obstacle* o)
{
	switch (o->type)
	{
	case TYPE_GIRDERS_HORIZONTAL:
	case TYPE_GIRDERS_VERTICAL:
	case TYPE_BLOCKS:
	case TYPE_BEAMS:
		if (o->movementArgs[0] != 0) o->y = (int) (o->yo + (isin(o->time) + 1.0f) * o->movementArgs[0]);
		break;
	case TYPE_CRANES:
	{
		int t = o->time & 1023;
		if (t < 256) o->y = (int) (o->yo + t / 256.0f * o->w);
		else if (t < 640) o->y = o->yo + o->w;
		else
		{
			int tt = (t - 640) / o->speed;
			o->y = o->yo + o->w - tt * tt * 0.5f;
			if (o->y < o->yo) o->y = o->yo;
		}
//		o->y = (int) (o->yo + (isin(o->time) + 1.0f) * (o->w >> 1));
		break;
	}
	}
}
void moveEllipse(Obstacle* o)
{
	switch (o->type)
	{
	case TYPE_GIRDERS_HORIZONTAL:
	{
		o->x = 360 + (int) (icos(o->time) * o->movementArgs[0]) - (o->w >> 1);
		o->y = o->yo + (int) ((isin(o->time) + 1.0f) * o->movementArgs[1]);
		break;
	}
	case TYPE_GIRDERS_VERTICAL:
	{
		o->x = o->xo + (int) (icos(o->time) * o->movementArgs[0]) - 48;
		o->y = o->yo + (int) ((isin(o->time) + 1.0f) * o->movementArgs[1]) - (o->w >> 1);
		break;
	}
	case TYPE_BLOCKS:
	{
		o->x = o->xo + (int) ((icos(o->time) + 1.0f) * 128.0f) * o->movementArgs[0];
		o->y = o->yo + (int) ((isin(o->time) + 1.0f) * 128.0f);
		break;
	}
	case TYPE_RING:
	{
		o->x = o->xo + (int) (icos(o->time) * o->movementArgs[0]);
		o->y = o->yo + (int) (isin(o->time) * o->movementArgs[0]);
		o->z += o->w;
		if (o->w > 0) o->movementArgs[0] = (short) (360.0f + isin(o->z) * 64.0f);
		break;
	}
	case TYPE_SWINGS:
	{
		o->x = o->xo + (int) (icos(o->time) * o->w);
		o->y = o->yo + o->w + 96 + (int) (isin(o->time) * o->w);
		break;
	}
	default:
		break;
	}
}
void moveRectangle(Obstacle* o)
{
	int t = o->time % o->movementArgs[2];
	if (t < o->movementArgs[1])
	{
		o->x = o->xo + t;
		o->y = o->yo;
	}
	else if (t < o->movementArgs[0] + o->movementArgs[1])
	{
		o->x = o->xo + o->movementArgs[1];
		o->y = o->yo + t - o->movementArgs[1];
	}
	else if (t < (o->movementArgs[1] << 1) + o->movementArgs[0])
	{
		o->x = o->xo + (o->movementArgs[1] << 1) - t + o->movementArgs[0];
		o->y = o->yo + o->movementArgs[0];
	}
	else
	{
		o->x = o->xo;
		o->y = o->yo - t + ((o->movementArgs[0] + o->movementArgs[1]) << 1);
	}
}
void moveModulus(Obstacle* o, char wave)
{
	int wait = o->type == TYPE_GIRDERS_HORIZONTAL ? (48 - o->w) % 192 : 0;
	if (wait < 0) wait += 192;
	wait += 720 + o->w;
	if (o->time < 0) o->time += wait;
	o->x = (o->time % wait) - o->w;
	if (wave)
	{
		o->movementArgs[0] += o->movementArgs[1];
		o->y = (int) (o->yo + (isin(o->movementArgs[0]) + 1.0f) * o->movementArgs[2]);
	}
}
void moveFigureEight(Obstacle* o)
{
	moveWaveVertical(o);
	int t = o->time;
	if (o->movementArgs[1] < 0) t >>= 2;
	o->x = (int) ((720 - o->w) / 2.0f * (isin(t * abs(o->movementArgs[1])) + 1.0f));
}
char tickObstacle(Obstacle* o, float scale)
{
	char add = (o->gate & 128) == 0;
	if (add && (o->gate & 127) < 8) o->gate++;
	else if (!add && (o->gate & 127) > 0) o->gate = ((o->gate & 127) - 1) | 128;
	if (scale == 1.0f) o->time += o->speed;
	else o->time = (int) (o->time + o->speed * scale);
	if (o->hasGate > 0)
	{
		if ((o->time / o->speed) % o->hasGate == 0) o->gate ^= 128;
	}
	switch (o->movement)
	{
	case MOVEMENT_WAVE_HORIZONTAL:
		moveWaveHorizontal(o);
		break;
	case MOVEMENT_WAVE_VERTICAL:
		moveWaveVertical(o);
		break;
	case MOVEMENT_ELLIPSE:
		moveEllipse(o);
		break;
	case MOVEMENT_RECTANGLE:
		moveRectangle(o);
		break;
	case MOVEMENT_MODULUS:
		moveModulus(o, 0);
		break;
	case MOVEMENT_WAVE_MODULUS:
		moveModulus(o, 1);
		break;
	case MOVEMENT_FIGURE_EIGHT:
		moveFigureEight(o);
		break;
	default:
		break;
	}
	if (o->type == TYPE_GUNS && o->time >= 512)
	{
		o->time &= 511;
		return 1;
	}
	return 0;
}
void renderObstacle(Obstacle* o)
{
	switch (o->type)
	{
	case TYPE_GIRDERS_HORIZONTAL:
	{
		render(0.0f, 1184.0f - o->y, o->xo - o->x, 0.0f, o->x, 96.0f);
		render(o->x + o->w, 1184.0f - o->y, o->xo + o->w, 0.0f, 720.0f - o->x - o->w, 96.0f);
		if (o->hasGate > 0)
		{
			float pr = (o->gate & 127) / 8.0f;
			render(o->x, 1184.0f - o->y, 192.0f - (o->w - 16.0f) * pr, 96.0f, (o->w - 16.0f) * pr + 16.0f, 16.0f);
		}
		break;
	}
	case TYPE_GIRDERS_VERTICAL:
	{
		render(o->x, 1280.0f + o->y + o->w, 0.0f, 0.0f, 96.0f, o->w);
		break;
	}
	case TYPE_GIRDER_SNAKE:
	{
		render(o->x, 1184.0f - o->y, o->xo, 0.0f, o->w, 96.0f);
		break;
	}
	case TYPE_BLOCKS:
	case TYPE_RING:
	{
		render(o->x, 1152.0f - o->y, 0.0f, 512.0f, 128.0f, 128.0f);
		break;
	}
	case TYPE_CRANES:
	{
		// TODO: o->x is either positive or negative, negative if it's on the left.
		// The absolute value is the position of the girder's edge.
		// (if o->x < 0, render from 0 to abs(o->x), else render from abs(o->x) to 720.
		render(o->x, 1088.0f - o->yo - o->w, 0.0f, 0.0f, 540.0f, 96.0f);
		endRender();
		beginRender(spriteShader, charsTexture);
		int count = (int) (o->w / 100.0f + 0.5f);
		for (int i = 0; i <= count; i++)
		{
			float yy = o->y - i * 88;
			float yyo = yy;
			if (yy > o->yo)
			{
				if (yy - 100.0f > o->yo) continue;
				yy = o->yo;
			}
			float yp = 1184.0f - yy - o->w;
			float h = 100.0f;
			if (i == count) h = o->w - i * 88;
			render(o->x > 0 ? o->x : 508.0f, yp, 0.0f, yyo > o->yo ? 256.0f - o->yo + yyo : 256.0f, 32.0f, h);
		}
		render(o->x > 0 ? o->x - 104.0f : 404.0f, 1184.0f - o->y, 128.0f, 256.0f, 240.0f, 96.0f);
		endRender();
		beginRender(spriteShader, girderTexture);
		break;
	}
	case TYPE_BEAMS:
	{
		int r = o->time;
		float xo = getXCenter(o);
		float yo = 1280.0f - getYCenter(o);
		int count = o->z == 48 ? 1 : 2;
		for (int i = 0; i < count; i++)
		{
			float yp = 1184.0f - o->y;
			if (i > 0) yp -= (o->z << 1) - 96;
			renderRotate(o->x, yp, o->xo, 0.0f, o->w, 96.0f, r, xo, yo);
		}
		break;
	}
	case TYPE_PISTONS:
	{
		render(o->x, 1120.0f - o->y, 656.0f, 864.0f, 64.0f, 160.0f);
		float xp = o->x + 64.0f;
		if (o->z) xp = o->x - 656.0f;
		render(xp, 1168.0f - o->y, 0.0f, 912.0f, 656.0f, 64.0f);
		break;
	}
	case TYPE_SWINGS:
	{
		render(o->xo == 476 ? 0.0f : 180.0f, 1088.0f - o->yo - o->w, 0.0f, 0.0f, 540.0f, 96.0f);
		endRender();
		beginRender(spriteShader, charsTexture);
		int r = 0;
		int zz = o->z & 0xFFFF;
		if (zz > 0) r = (int) (isin(o->time << 1) * zz);
		else r = o->time << 1;
		float yo = 1184.0f - o->yo - o->w;
		int count = o->w / 88;
		for (int i = 0; i < count; i++)
		{
			renderRotate(o->xo - 16.0f, yo + i * 88, 0.0f, 256.0f, 32.0f, 100.0f, r, o->xo, yo);
		}
		zz = o->z >> 16;
		float xp = o->xo - 80.0f;
		float yp = 1184.0f - o->yo;
		renderRotate(xp, yp, 256.0f, 512.0f + zz * 96.0f, 160.0f, 96.0f, r, o->xo, yo);
//		if (zz == 2)
//		{
////			xp -= (float) (isin(r) * o->w);
////			yp -= o->w - (float) (icos(r) * o->w);
//			xp = o->xo - isin(r) * (o->w + 48);
//			yp = 1184 - o->yo - o->w + icos(r) * o->w;
//			xp -= 48 * (-isin(r) + 1);
//			yp -= 48 * (-icos(r) + 1);
//			render(xp, yp, 288.0f, 704.0f, 96.0f, 96.0f);
//		}
//		else renderRotate(xp, yp, 256.0f, 512.0f + zz * 96.0f, 160.0f, 96.0f, r, o->xo, yo);
		endRender();
		beginRender(spriteShader, girderTexture);
		break;
	}
	case TYPE_GUNS:
	{
		signed int xa = 0;
		signed int ya = 0;
		if (o->w & 1) ya = o->w - 2;
		else xa = 1 - o->w;
		int t = 512 - o->time;
		if (t > o->z) t = 0;
		else t = ((o->z - t) / o->speed) << 1;
		int u = 512 + o->w * 96;
		endRender();
		beginRender(spriteShader, charsTexture);
		render(o->x + xa * t, 1184.0f - o->y - ya * t, u, 96.0f, 96.0f, 96.0f);
		render(o->x, 1184.0f - o->y, u, 0.0f, 96.0f, 96.0f);
		endRender();
		beginRender(spriteShader, girderTexture);
		break;
	}
	default:
		break;
	}
}
int getXCenter(Obstacle* o)
{
	switch (o->type)
	{
	case TYPE_GIRDERS_HORIZONTAL:
	case TYPE_GIRDER_SNAKE:
		return o->x + (o->w >> 1);
	case TYPE_GIRDERS_VERTICAL:
		return o->x + 48;
	case TYPE_BLOCKS:
	case TYPE_RING:
		return o->x + 64;
	case TYPE_CRANES:
		return o->x + 180;
	case TYPE_BEAMS:
		return o->x + (o->w >> 1);
	case TYPE_PISTONS:
		return o->x + 32;
	case TYPE_SWINGS:
		return o->xo;
	case TYPE_GUNS:
		return o->x + 48;
	default:
		return 0;
	}
}
int getYCenter(Obstacle* o)
{
	switch (o->type)
	{
	case TYPE_GIRDERS_HORIZONTAL:
	case TYPE_GIRDER_SNAKE:
		return o->y + 48;
	case TYPE_GIRDERS_VERTICAL:
		return o->y + (o->w >> 1);
	case TYPE_BLOCKS:
	case TYPE_RING:
		return o->y + 64;
	case TYPE_CRANES:
		return o->yo + o->w + 48;
	case TYPE_BEAMS:
		return o->y + o->z;
	case TYPE_PISTONS:
		return o->y + 80;
	case TYPE_SWINGS:
		return o->yo + o->w + 144;
	case TYPE_GUNS:
		return o->y + 48;
	default:
		return 0;
	}
}
