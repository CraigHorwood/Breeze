#include "game.h"
#include "renderer.h"
#include "zone.h"
#include "random.h"
#include "title.h"
#include "shop.h"
#include "imath.h"
#include "main.h"
#include "SDL.h"
#include <stdlib.h>
#define GAME_STATE_GOT_HIT 0
#define GAME_STATE_ON_GROUND 1
#define GAME_STATE_IN_GAME 2
#define MAX_ZONES 4
typedef struct
{
	Zone* zone;
	Obstacle* obstacle;
	signed int zonePos;
	signed int obstaclePos;
} LookTarget;
typedef struct
{
	float x, y, xa, ya;
	char r, removed;
} DirtParticle;
typedef struct Bullet Bullet;
struct Bullet
{
	int x, y, xa, ya;
	Bullet* next;
	Zone* zone;
};
const char debugMode = 0;
int gameTime = 0, animTime = 0, heartCollectTime = 0;
char animationId = 0;
char gameState = 1;
float xWind = 0.0f, yWind = 0.0f;
float xWindPos = 0.0f, yWindPos = 0.0f;
float xPlayer = 360.0f, yPlayer = 112.0f, xaPlayer = 0.0f, yaPlayer = 0.0f;
int rPlayer = 0;
float xHeart = 0.0f, yHeart = 0.0f;
float yMin = 0.0f;
float scroll = 0.0f;
int nextZoneTime = 320;
unsigned char hitPoints = 1;
int hitTime = 0;
int zonesTotal = 0;
Zone* zones[MAX_ZONES];
int zoneCount = 0;
Zone* inZone = NULL;
LookTarget look, lastLook;
float xEyes = 0.0f, yEyes = 0.0f, xEyesO = -256.0f, yEyesO = 0.0f;
DirtParticle dirtParticles[8];
Bullet* bullets;
Bullet* lastBullet;
int bulletCount = 0;
int cursorPosition = 192, xCursor = 192, yCursor = 0;
int xButton = 0, yButton = 0;
int leftId = -1, rightId = -1;
signed char tallyPhase = -1;
int tallyTime = 0;
int heightScore = 0, coinsGained = 0, coinsBonus = 0;
long long heightScoreRender = 0, coinsGainedRender = 0, coinsBonusRender = 0;
int tutorialTime = 0;
Zone* selectedZone;
Obstacle* selectedObstacle;
float xCloud = -1024.0f, yCloud = 0.0f;
float xsCloud = 1.0f, ysCloud = 1.0f;
int timeLeft = 0;
float screenOffs = 0.0f;
short* levelDataPtr = NULL;
void resetGame()
{
	if (isModeEnabled(MODE_GAUNTLET))
	{
		if (levelData == NULL)
		{
			levelData = malloc(sizeof(short) * 64);
			// TODO: SDL_RWops from gauntlet.bin.
		}
		levelDataPtr = levelData;
	}
	xPlayer = 360.0f;
	yPlayer = 112.0f;
	xaPlayer = 0.0f;
	yaPlayer = 0.0f;
	rPlayer = 0;
	xHeart = 0.0f;
	yHeart = 0.0f;
	yMin = 0.0f;
	scroll = 0.0f;
	if (isModeEnabled(MODE_ONE_HIT | MODE_GAUNTLET)) hitPoints = 0;
	else hitPoints = 1;
	hitTime = 0;
	zonesTotal = 0;
	gameTime = 0;
	heartCollectTime = 0;
	animTime = 0;
	animationId = 0;
	nextZoneTime = 320;
	for (int i = 0; i < MAX_ZONES; i++)
	{
		if (zones[i] != NULL) free(zones[i]);
		zones[i] = NULL;
	}
	if (bulletCount > 0)
	{
		while (bullets != NULL)
		{
			Bullet* next = bullets->next;
			free(bullets);
			bullets = next;
		}
	}
	bullets = NULL;
	lastBullet = NULL;
	bulletCount = 0;
	zoneCount = 0;
	inZone = NULL;
	look.zone = NULL;
	look.obstacle = NULL;
	look.zonePos = 0;
	look.obstaclePos = 0;
	lastLook.zone = NULL;
	lastLook.obstacle = NULL;
	lastLook.zonePos = 0;
	lastLook.obstaclePos = 0;
	xEyes = 0.0f;
	yEyes = 0.0f;
	xEyesO = -256.0f;
	yEyesO = 0.0f;
	for (int i = 0; i < 8; i++)
	{
		dirtParticles[i].removed = 1;
	}
	gameState = GAME_STATE_ON_GROUND;
	xWind = 0.0f;
	yWind = 0.0f;
	xWindPos = 0.0f;
	yWindPos = 0.0f;
	cursorPosition = 192;
	xCursor = 192;
	yCursor = 0;
	xButton = 0;
	yButton = 0;
	leftId = -1;
	rightId = -1;
	tallyPhase = -1;
	tallyTime = 0;
	heightScore = 0;
	coinsGained = 0;
	coinsBonus = 0;
	tutorialTime = 0;
	selectedZone = NULL;
	selectedObstacle = NULL;
	xCloud = -1024.0f;
	yCloud = 0.0f;
	xsCloud = 1.0f;
	ysCloud = 1.0f;
	if (isModeEnabled(MODE_TIME_TRIAL)) timeLeft = 1800;
	else timeLeft = 0;
	screenOffs = 0.0f;
}
void addZone()
{
	char baseType;
	Zone* lastZone = NULL;
	int max = TYPE_BLOCKS + ((int) scroll / 120 + 8) / 48;
	if (max < TYPE_BLOCKS) max = TYPE_BLOCKS;
	else if (max > TYPE_CRANES) max = TYPE_CRANES;
	if (isModeEnabled(MODE_PROTECT)) max = TYPE_BLOCKS;
	else if (isModeEnabled(MODE_EVERYTHING)) max = TYPE_GUNS;
	max++;
	if (zoneCount > 0)
	{
		lastZone = zones[zoneCount - 1];
		int i = 2;
		do
		{
			baseType = (char) (nextInt(max));
		}
		while (i-- > 0 && baseType == lastZone->baseType);
	}
	else baseType = (char) nextInt(max);
	Zone* zone = initZone(nextZoneTime + 960, lastZone, baseType);
	int i = 0;
	while (i < MAX_ZONES)
	{
		if (zones[i] == NULL)
		{
			zones[i] = zone;
			zoneCount++;
			break;
		}
		i++;
	}
	if (i == MAX_ZONES)
	{
		i = 0;
		while (i < MAX_ZONES)
		{
			if (i == MAX_ZONES - 1) zones[i] = zone;
			else
			{
				if (i == 0)
				{
					yMin = zones[0]->y + zones[0]->height;
					deleteZone(zones[0]);
				}
				zones[i] = zones[i + 1];
			}
			i++;
		}
	}
	int ha = (next() & 7) * 48 + 320;
	char addHeart = yHeart <= scroll && nextInt(3) == 0 && !isModeEnabled(MODE_ONE_HIT | MODE_HEARTLESS) && hitPoints < 2;
	if (isModeEnabled(MODE_TIME_TRIAL) && !isModeEnabled(MODE_ONE_HIT | MODE_HEARTLESS)) addHeart = yHeart <= scroll && nextInt(1800) > timeLeft;
	if (isModeEnabled(MODE_GAUNTLET)) addHeart = 0;
	if (addHeart)
	{
		xHeart = nextInt(656);
		yHeart = zone->y + zone->height + (ha >> 1) + 32;
	}
	nextZoneTime += ha + zone->height;
	zonesTotal++;
}
void addZoneGauntlet()
{
	// TODO: Read from level data in ram, offset pointer, keep the original pointer to free memory later.
}
void addBullet(Obstacle* gun, Zone* zone)
{
	if (bulletCount < 64)
	{
		int x = (int) gun->x + 36;
		int y = (int) gun->y + 36;
		int xa = 0;
		int ya = 0;
		if (gun->w & 1) ya = 2 - gun->w;
		else xa = gun->w - 1;
		x += xa * 48;
		y += ya * 48;
		Bullet* b = (Bullet*) malloc(sizeof(Bullet));
		b->x = x;
		b->y = y;
		b->xa = xa << 2;
		b->ya = ya << 2;
		b->next = NULL;
		b->zone = zone;
		if (bulletCount == 0) bullets = b;
		if (lastBullet != NULL) lastBullet->next = b;
		lastBullet = b;
		bulletCount++;
	}
}
void die(char animation)
{
	if (hitTime > 0) return;
	if (hitPoints > 0 && animation != 1 && animation != 4)
	{
		xaPlayer = 0;
		yaPlayer = 0;
		xWind = 0.0f;
		hitPoints--;
		hitTime = 128;
		return;
	}
	hitPoints = 0;
	gameState = GAME_STATE_GOT_HIT;
	animationId = animation;
	switch (animationId)
	{
	case 1:
		animTime = 90;
		float a = -yaPlayer / 16.0f;
		if (a > 1.0f) a = 1.0f;
		for (int i = 0; i < 8; i++)
		{
			DirtParticle* dp = dirtParticles + i;
			dp->x = xPlayer;
			dp->y = yPlayer;
			dp->xa = (next() & 3) + 2;
			if (next() & 1) dp->xa = -dp->xa;
			dp->ya = ((next() & 7) + 4) * a;
			dp->r = nextInt(9) + 10;
			dp->removed = 0;
		}
		break;
	case 2:
		animTime = 64;
		xaPlayer = -7.0f;
		yaPlayer = 16.0f;
		break;
	case 3:
		animTime = 64;
		xaPlayer = 7.0f;
		yaPlayer = 16.0f;
		break;
	case 4:
		animTime = 48;
		// Play sound of falling.
		break;
	case 8:
		animTime = 64;
		xaPlayer = -4.0f;
		yaPlayer = 12.0f;
		break;
	case 9:
		animTime = 64;
		xaPlayer = 4.0f;
		yaPlayer = 12.0f;
		break;
	case 10:
		animTime = 64;
		xaPlayer = 0.0f;
		yaPlayer = 12.0f;
		break;
	case 11:
	case 16:
	case 17:
		animTime = 64;
		yaPlayer = 0.0f;
		break;
	default:
		animTime = 128;
		break;
	}
	xWind = 0.0f;
	yWind = 0.0f;
}
void tickGame()
{
	if (isModeEnabled(MODE_SLIDING))
	{
		screenOffs += xWind * 16.0f;
		if (screenOffs < -360.0f) screenOffs = -360.0f;
		else if (screenOffs > 360.0f) screenOffs = 360.0f;
	}
	if (gameState == GAME_STATE_IN_GAME) gameTime++;
	if (gameState == GAME_STATE_ON_GROUND && tutorialTime < 64) tutorialTime++;
	else if (gameState != GAME_STATE_ON_GROUND && tutorialTime > 0) tutorialTime -= 4;
	if (hitTime > 0) hitTime--;
	if (heartCollectTime > 0)
	{
		if (--heartCollectTime == 0) yHeart = 0.0f;
	}
	if (timeLeft > 0 && gameState == GAME_STATE_IN_GAME)
	{
		if (--timeLeft == 0)
		{
			hitPoints = 0;
			die(10);
		}
	}
	xWindPos += xWind * 16.0f;
	yWindPos += yWind * 16.0f;
	if (isModeEnabled(MODE_CLOUDS))
	{
		if (yCloud > 0.0f)
		{
			if ((xCloud += 2.0f) > 720.0f)
			{
				if (yCloud < scroll + 1280.0f) yCloud = 0.0f;
				else xCloud -= 1744.0f;
			}
		}
		else if ((gameTime & 127) == 0)
		{
			xCloud = -1024.0f;
			yCloud = scroll + 1280.0f + ((next() & 511) << 2);
			xsCloud = 0.5f + ((next() & 255) + 1) / 256.0f * 1.5f;
			ysCloud = 0.5f + ((next() & 255) + 1) / 256.0f * 1.5f;
		}
	}
	switch (animationId)
	{
	case 1:
		for (int i = 0; i < 8; i++)
		{
			DirtParticle* dp = dirtParticles + i;
			if (dp->removed || dp->xa == 0.0f) continue;
			dp->x += dp->xa;
			dp->y += dp->ya;
			dp->ya -= 0.5f;
			if (dp->x < -16.0f || dp->x >= 720.0f) dp->removed = 1;
			else if (dp->y < 56.0f)
			{
				dp->y = 56.0f;
				dp->xa = 0.0f;
				dp->ya = 0.0f;
			}
		}
		break;
	case 2:
	case 3:
		xPlayer += xaPlayer;
		yPlayer += yaPlayer;
		yaPlayer -= 0.5f;
		if (animationId == 2) rPlayer -= 128;
		else rPlayer += 128;
		break;
	case 8:
	case 9:
	case 10:
		xPlayer += xaPlayer;
		yPlayer += yaPlayer;
		yaPlayer -= 0.5f;
		if (animationId == 10) rPlayer += 16;
		else rPlayer += (int) (xaPlayer) << 4;
		break;
	case 11:
		if (animTime <= 56)
		{
			yPlayer += yaPlayer;
			yaPlayer -= 0.5f;
			rPlayer += (int) (yaPlayer) << 2;
		}
		break;
	case 16:
	case 17:
		if (animTime <= 56)
		{
			if (animTime == 56)
			{
				if (animationId == 16) xaPlayer = -1.0f;
				else xaPlayer = 1.0f;
			}
			yPlayer += yaPlayer;
			yaPlayer -= 0.5f;
			rPlayer += (int) (xaPlayer) << 2;
		}
		break;
	default:
		break;
	}
	if (tallyTime > 0)
	{
		if (tallyPhase == 4)
		{
			int heightScoreO = heightScoreRender / 100;
			if ((heightScoreRender -= 10) < 0) heightScoreRender = 0;
			if (heightScoreRender / 100 < heightScoreO && coinsGainedRender < coinsGained) coinsGainedRender++;
		}
		else if (tallyPhase == 6)
		{
			if (coinsGainedRender > 0)
			{
				coinsGainedRender--;
				coins++;
			}
			if (coinsBonusRender > 0)
			{
				coinsBonusRender--;
				coins++;
			}
		}
		if (--tallyTime == 0)
		{
			// 0 = window enter
			// 1 = wait 1
			// 2 = top score? skip
			// 3 = wait 2
			// 4 = count coins from height
			// 5 = wait 3
			// 6 = count coins and high score bonus
			switch (++tallyPhase)
			{
			case 1:
				tallyTime = 60;
				break;
			case 2:
				if (coinsBonus)
				{
					// Play high score sound.
					tallyTime = 127;
					break;
				}
				tallyPhase = 3;
			case 3:
				tallyTime = 60;
				break;
			case 4:
				tallyTime = coinsGained * 10;
				if (tallyTime == 0) tallyTime = 1;
				break;
			case 5:
				heightScoreRender = 0;
				coinsGainedRender = coinsGained;
				tallyTime = 60;
				break;
			case 6:
				if (coinsBonus > coinsGained) tallyTime = coinsBonus;
				else tallyTime = coinsGained;
				if (tallyTime == 0) tallyTime = 1;
				break;
			case 7:
			case 8:
				if (coinsGainedRender > 0) coins += coinsGainedRender;
				if (coinsBonusRender > 0) coins += coinsBonusRender;
				coinsGainedRender = 0;
				coinsBonusRender = 0;
				saveGame();
				break;
			default:
				break;
			}
		}
	}
	if (animTime > 0)
	{
		if (--animTime == 0)
		{
			// Score calculated here.
			heightScore = scroll / 120 * 10;
			coinsGained = heightScore / 100;
			coinsBonus = 0;
			if (heightScore > topScore)
			{
//				coinsBonus = (int) ((heightScore - topScore) * heightScore / (topScore < 2000 ? 25600.0f : 51200.0f));
				coinsBonus = (((heightScore - topScore) / 100) << 1);
				if (coinsGained == 1) coinsBonus++;
				else coinsBonus += (coinsGained >> 1);
				if (coinsBonus == 0) coinsBonus = 1;
				else if (coinsBonus > 100) coinsBonus = 100;
				topScore = heightScore;
				saveGame();
			}
			if (isModeEnabled(MODE_PROTECT | MODE_GAUNTLET))
			{
				coinsGained = 0;
				coinsBonus = 0;
			}
			heightScoreRender = (long long) heightScore;
			coinsGainedRender = (long long) 0;
			coinsBonusRender = (long long) coinsBonus;
			tallyPhase = 0;
			tallyTime = 30;
		}
	}
	if (gameState == GAME_STATE_IN_GAME)
	{
		if (look.obstacle == NULL && zoneCount > 0)
		{
			look.zonePos = zoneCount - 1;
			look.obstaclePos = 0;
			look.zone = zones[look.zonePos];
			look.obstacle = look.zone->obstacles[look.obstaclePos];
			xEyesO = xEyes;
			yEyesO = yEyes;
		}
		if ((gameTime & 31) == 0 && look.zone != NULL)
		{
			if (yPlayer - 48.0f > getYCenter(look.obstacle))
			{
				lastLook.zone = look.zone;
				lastLook.obstacle = look.obstacle;
				lastLook.zonePos = look.zonePos;
				lastLook.obstaclePos = look.obstaclePos;
				if (++look.obstaclePos >= look.zone->count)
				{
					if (++look.zonePos >= zoneCount)
					{
						look.zone = NULL;
						look.obstacle = NULL;
					}
					else
					{
						look.zone = zones[look.zonePos];
						look.obstaclePos = 0;
						look.obstacle = look.zone->obstacles[look.obstaclePos];
					}
				}
				else look.obstacle = look.zone->obstacles[look.obstaclePos];
				xEyesO = xEyes;
				yEyesO = yEyes;
			}
			else if (lastLook.obstacle != NULL && yPlayer + 48.0f < getYCenter(lastLook.obstacle))
			{
				look.zone = lastLook.zone;
				look.obstacle = lastLook.obstacle;
				look.zonePos = lastLook.zonePos;
				look.obstaclePos = lastLook.obstaclePos;
				if (--lastLook.obstaclePos < 0)
				{
					if (--lastLook.zonePos < 0)
					{
						lastLook.zone = NULL;
						lastLook.obstacle = NULL;
					}
					else
					{
						lastLook.zone = zones[lastLook.zonePos];
						lastLook.obstaclePos = lastLook.zone->count - 1;
						lastLook.obstacle = lastLook.zone->obstacles[lastLook.obstaclePos];
					}
				}
				else lastLook.obstacle = lastLook.zone->obstacles[lastLook.obstaclePos];
				xEyesO = xEyes;
				yEyesO = yEyes;
			}
		}
		float scale = 1.0f;
		if (isModeEnabled(MODE_TIME))
		{
			if (yaPlayer <= 0.0f) scale = 0.0f;
			else scale = yaPlayer / 8.0f;
			if (scale > 1.0f) scale = 1.0f;
		}
		for (int i = 0; i < zoneCount; i++)
		{
			Zone* z = zones[i];
			for (int j = 0; j < z->count; j++)
			{
				Obstacle* o = z->obstacles[j];
				if (tickObstacle(o, scale)) addBullet(o, z);
			}
		}
		if (bulletCount > 0)
		{
			Bullet* b = bullets;
			Bullet* last = NULL;
			while (b != NULL)
			{
				char removed = 0;
				if (!isModeEnabled(MODE_TIME) || yaPlayer > 0.0f)
				{
					b->x += b->xa;
					b->y += b->ya;
					int scr = (int) scroll;
					removed = b->x < -24 || b->x >= 720 || b->y < scr - 24 || b->y >= scr + 1280 || b->y < b->zone->y || b->y > b->zone->y + b->zone->height - 24;
					if (!removed)
					{
						int xp = (int) xPlayer;
						int yp = (int) yPlayer;
						removed = checkCollide(xp - 40, yp - 40, xp + 40, yp + 40, b->x + 2, b->y + 2, b->x + 20, b->y + 20);
						if (removed) die(xPlayer < b->x + 12 ? 8 : 9);
						else
						{
							int x0 = b->x;
							int y0 = b->y;
							int x1 = b->x + 24;
							int y1 = b->y + 24;
							for (int i = 0; i < b->zone->count; i++)
							{
								Obstacle* o = b->zone->obstacles[i];
								if (o->type == TYPE_GIRDERS_HORIZONTAL)
								{
									removed = y1 >= o->y && y0 <= o->y + 96 && (x0 <= o->x || x1 >= o->x + o->w);
								}
								else if (o->type == TYPE_GIRDERS_VERTICAL)
								{
									removed = checkCollide(o->x, o->y, o->x + 96, o->y + o->w, x0, y0, x1, y1);
								}
								if (removed) break;
							}
						}
					}
					if (removed)
					{
						if (b == lastBullet) lastBullet = last;
						if (last != NULL) last->next = b->next;
						else bullets = b->next;
						free(b);
						b = last;
						bulletCount--;
					}
				}
				if (last != NULL || !removed)
				{
					last = b;
					b = b->next;
				}
				else b = bullets;
			}
		}
		if (isModeEnabled(MODE_SPINNING))
		{
			float rCos = icos(gameTime);
			float rSin = isin(gameTime);
			float xa = -xaPlayer * rCos - yaPlayer * rSin;
			float ya = yaPlayer * rCos - xaPlayer * rSin;
			xPlayer += xa;
			yPlayer += ya;
		}
		else
		{
			if (isModeEnabled(MODE_PROTECT)) xPlayer = 360.0f + isin(gameTime << 2) * 180.0f;
			xPlayer += xaPlayer;
			yPlayer += yaPlayer;
		}
		if (yPlayer < yMin - 48 || yPlayer < 112.0f)
		{
			if (yPlayer < 112.0f)
			{
				yPlayer += yaPlayer * 3.0f;
				if (yPlayer < 64.0f) yPlayer = 64.0f;
				die(1);
			}
			else die(4);
		}
		if (xPlayer < 48.0f)
		{
			xPlayer = 48.0f;
			xaPlayer = 0.0f;
		}
		else if (xPlayer > 672.0f)
		{
			xPlayer = 672.0f;
			xaPlayer = 0.0f;
		}
		if (yHeart != 0.0f && heartCollectTime == 0)
		{
			float xd = xPlayer - xHeart - 32.0f;
			float yd = yPlayer - yHeart - 32.0f;
			if (xd * xd + yd * yd < 9216.0f)
			{
				// Play sound.
				if (isModeEnabled(MODE_TIME_TRIAL)) timeLeft = 1800;
				if (hitPoints < 2) hitPoints++;
				heartCollectTime = 16;
			}
		}
		if (isModeEnabled(MODE_HIGH_SPEED | MODE_LEGACY))
		{
			xaPlayer += xWind;
			xaPlayer *= 0.875f;
		}
		else xaPlayer = xWind * 8.0f;
		if (isModeEnabled(MODE_JUMP))
		{
			if (yWind > 0.0f) yWind -= 0.0625f;
			yaPlayer -= 0.375f;
		}
		else if (yWind != 0.0f && hitTime < 112) yaPlayer += yWind;
		else
		{
			if (isModeEnabled(MODE_HIGH_GRAVITY)) yaPlayer -= 2.0f;
			else if (yaPlayer > 0.0f) yaPlayer -= 0.75f;
			else yaPlayer -= 0.375f;
		}
		if (isModeEnabled(MODE_HIGH_GRAVITY))
		{
			if (yaPlayer < -24.0f) yaPlayer = -24.0f;
			else if (yaPlayer > 7.0f) yaPlayer = 7.0f;
		}
		else
		{
			float maxSpeed = 12.0f;
			if (isModeEnabled(MODE_HIGH_SPEED)) maxSpeed = 24.0f;
			else if (isModeEnabled(MODE_PROTECT)) maxSpeed = 8.0f;
			if (yaPlayer < -maxSpeed) yaPlayer = -maxSpeed;
			else if (yaPlayer > maxSpeed) yaPlayer = maxSpeed;
		}
		scroll = yPlayer - 320.0f;
		if (scroll < yMin) scroll = yMin;
		if (yPlayer >= nextZoneTime)
		{
			if (isModeEnabled(MODE_GAUNTLET)) addZoneGauntlet();
			else addZone();
		}
		if (inZone == NULL)
		{
			for (int i = 0; i < zoneCount; i++)
			{
				Zone* z = zones[i];
				if (yPlayer + 48 >= z->y && yPlayer - 48 < z->y + z->height)
				{
					inZone = z;
					break;
				}
			}
		}
		if (inZone != NULL)
		{
			if (yPlayer + 48 >= inZone->y && yPlayer - 48 < inZone->y + inZone->height)
			{
				char animation = collideZone(inZone, (int) xPlayer, (int) yPlayer, (int) xaPlayer, (int) yaPlayer);
				if (animation && !debugMode) die(animation);
			}
			else inZone = NULL;
		}
	}
}
void saveGame()
{
	SDL_Log("Attempting to save game.");
	SDL_RWops* rw = SDL_RWFromFile(prefPath, "wb");
	SDL_Log("Opened SDL_RWops.");
	if (rw == NULL)
	{
		SDL_Log("%s", SDL_GetError());
		return;
	}
	SDL_RWwrite(rw, &topScore, 4, 1);
	SDL_RWwrite(rw, &coins, 4, 1);
	SDL_RWwrite(rw, &bought, 4, 1);
	SDL_Log("Finished writing to file.");
	SDL_RWclose(rw);
	SDL_Log("Closed SDL_Rwops.");
}
void renderScore(const char* start, char i, char hasSuffix, long long var, float x, float y)
{
	for (int j = 0; j < i; j++)
	{
		msg[j] = start[j];
	}
	long long pow = 10000000000;
	char started = 0;
	while (pow > 0)
	{
		if (var / pow > 0 || pow == hasSuffix) started = 1;
		if (started)
		{
			msg[i++] = (char) (((var / pow) % 10) + 48);
		}
		if (hasSuffix == 10 && pow == 10) msg[i++] = '.';
		pow /= 10;
	}
	if (hasSuffix == 10)
	{
		msg[i++] = ' ';
		msg[i++] = 'm';
	}
	msg[i] = '\0';
	renderText(msg, x, y);
}
void renderGame()
{
	if (isModeEnabled(MODE_SLIDING | MODE_SPINNING))
	{
		if (isModeEnabled(MODE_SLIDING)) translateRenderer(screenOffs, 0.0f);
		if (isModeEnabled(MODE_SPINNING)) rotateRenderer(360.0f, 640.0f, gameTime);
		beginRender(spriteShader, skyTexture);
		renderScale(0.0f, 0.0f, 0.0f, -scroll / 2.0f, 576.0f, 1024.0f, 1.25f, 1.25f);
		endRender();
		if (isModeEnabled(MODE_SPINNING)) rotateRenderer(360.0f, 640.0f, -gameTime);
	}
	else
	{
		beginRender(skyShader, isModeEnabled(MODE_GAUNTLET) ? skyGauntletTexture : (isModeEnabled(MODE_CLOUDS) ? skyHazeTexture : skyTexture));
		glUniform1f(scrollLocation, scroll / 2560.0f);
		render(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f);
		endRenderRender();
	}
	if (isModeEnabled(MODE_SPINNING)) rotateRenderer(360.0f, 640.0f - scroll, gameTime);
	translateRenderer(0.0f, scroll);
	if (debugMode)
	{
		beginRender(colourShader, girderTexture);
		glUniform3f(colLocation1, 0.0f, 0.0f, 1.0f);
		for (int i = 0; i < zoneCount; i++)
		{
			Zone* z = zones[i];
			render(0.0f, 1280.0f - z->y - z->height, 0.0f, 0.0f, 720.0f, z->height);
		}
		endRender();
	}
	beginRender(spriteShader, charsTexture);
	if (isModeEnabled(MODE_CLOUDS)) glUniform4f(colLocation0, 1.0f, 1.0f, 1.0f, 0.0625f);
	Bullet* b = bullets;
	while (b != NULL)
	{
		render(b->x, 1256.0f - b->y, 150.0f, 18.0f, 24.0f, 24.0f);
		b = b->next;
	}
	endRender();
	char texture = -1;
	for (int i = 0; i < zoneCount; i++)
	{
		Zone* z = zones[i];
		char switchTexture = z->baseType == TYPE_BLOCKS || z->baseType == TYPE_RING || z->baseType == TYPE_PISTONS;
		if (texture != switchTexture)
		{
			if (i > 0) endRender();
			beginRender(spriteShader, switchTexture ? charsTexture : girderTexture);
			texture = switchTexture;
		}
		for (int j = 0; j < z->count; j++)
		{
			renderObstacle(z->obstacles[j]);
		}
	}
	if (zoneCount > 0) endRender();
	if (yPlayer > 0.0f && ((hitTime >> 3) & 1) == 0)
	{
		float x = xPlayer - 60.0f;
		float y = 1232.0f - yPlayer;
		char flap = gameState == GAME_STATE_IN_GAME && yaPlayer > 2.0f && (((int) yPlayer >> 5) & 1) == 0;
		float xs = 1.0f;
		float ys = 1.0f;
		if (animTime > 56)
		{
			if (animationId == 11) ys = (72 - animTime) / 16.0f;
			else if (animationId >= 16)
			{
				xs = (72 - animTime) / 16.0f;
				if (animationId == 16) x += (1.0f - xs) * 120.0f;
			}
		}
		beginRender(spriteShader, charsTexture);
		float offs = selectedColour == COLOUR_GOLD ? ((next() & 15) - (next() & 15)) / 256.0f : 0.0f;
		glUniform4f(colLocation0, COLOUR_VALUES[selectedColour][0] + offs, COLOUR_VALUES[selectedColour][1] + offs, COLOUR_VALUES[selectedColour][2] + offs, isModeEnabled(MODE_CLOUDS) ? 0.0625f : 1.0f);
		if (rPlayer == 0) renderScale(x, y, 0.0f, flap ? 96.0f : 0.0f, 120.0f, 96.0f, xs, ys);
		else renderRotate(x, y, 0.0f, flap ? 96.0f : 0.0f, 120.0f, 96.0f, rPlayer, xPlayer, 1280.0f - yPlayer);
		endRender();
		beginRender(spriteShader, charsTexture);
		if (isModeEnabled(MODE_CLOUDS)) glUniform4f(colLocation0, 1.0f, 1.0f, 1.0f, 0.0625f);
		else glUniform4f(colLocation0, 0.0f, 0.0f, 0.0f, 0.0f);
		if (rPlayer == 0) renderScale(x, y, 120.0f, (animationId == 0 && hitTime < 64) ? 0.0f : 96.0f, 120.0f, 96.0f, xs, ys);
		else renderRotate(x, y, 120.0f, animationId == 0 ? 0.0f : 96.0f, 120.0f, 96.0f, rPlayer, xPlayer, 1280.0f - yPlayer);
		if (animationId == 0 && hitTime < 64)
		{
			float xo = 0.0f;
			float yo = 0.0f;
			if (look.obstacle != NULL)
			{
				float xd = getXCenter(look.obstacle) - xPlayer;
				float yd = yPlayer - scroll - getYCenter(look.obstacle);
				xo = xd / 16.0f;
				yo = yd / 16.0f;
				float r = 4.0f;
				if (xo < -r) xo = -r;
				else if (xo > r) xo = r;
				if (yo < -r) yo = -r;
				else if (yo > r) yo = r;
			}
			xEyes = xo;
			yEyes = yo;
			float xe = x + xEyes;
			float ye = y + yEyes;
			if (xEyesO > -256.0f)
			{
				float xd = xEyes - xEyesO;
				float yd = yEyes - yEyesO;
				float xa = xd / 32.0f;
				float ya = yd / 32.0f;
				xEyesO += xa;
				yEyesO += ya;
				if (abs(xd) < 4.0f && abs(yd) < 4.0f) xEyesO = -256.0f;
				else
				{
					xe = x + xEyesO;
					ye = y + yEyesO;
				}
			}
			if (rPlayer == 0) renderScale(xe, ye, 240.0f, 0.0f, 120.0f, 96.0f, xs, ys);
			else renderRotate(xe, ye, 240.0f, 0.0f, 120.0f, 96.0f, rPlayer, xPlayer, 960.0f - yPlayer);
		}
		endRender();
	}
	if (yHeart != 0.0f)
	{
		beginRender(spriteShader, charsTexture);
		float xp = xHeart;
		float wh = 1.0f;
		if (heartCollectTime)
		{
			wh = (heartCollectTime << 2) / 64.0f;
			xp = xHeart + 32.0f - wh / 2.0f;
		}
		renderScale(xp, 1280.0f - yHeart - wh, 640.0f, heartCollectTime ? 576.0f : 512.0f, 64.0f, 64.0f, wh, wh);
		endRender();
	}
	if (isModeEnabled(MODE_CLOUDS))
	{
		glUniform4f(colLocation0, 0.0f, 0.0f, 0.0f, 0.0f);
		if (yCloud > 0.0f)
		{
			beginRender(spriteShader, hudTexture);
			renderScale(xCloud, 1280.0f - yCloud - 256.0f * ysCloud, 0.0f, 256.0f, 512.0f, 256.0f, xsCloud, ysCloud);
			endRender();
		}
	}
	translateRenderer(0.0f, -scroll);
//	if (animationId > 0) translateRenderer(0.0f, -scroll);
	if (scroll < 64.0f)
	{
		beginRender(colourShader, girderTexture);
		if (isModeEnabled(MODE_CLOUDS)) glUniform3f(colLocation0, 0.8203125f, 0.8515625f, 0.8203125f);
		else glUniform3f(colLocation1, 0.0f, 0.5f, 0.0f);
		render(0.0f, 1216.0f + scroll, 0.0f, 0.0f, 720.0f, 64.0f - scroll);
		endRender();
	}
	if (isModeEnabled(MODE_CLOUDS)) glUniform4f(colLocation0, 1.0f, 1.0f, 1.0f, 0.0625f);
	beginRender(spriteShader, charsTexture);
	if (animationId == 1)
	{
		render(xPlayer - 64.0f, 1204.0f, 512.0f, 256.0f, 128.0f, 24.0f);
		for (int i = 0; i < 8; i++)
		{
			DirtParticle* dp = dirtParticles + i;
			float scale = dp->r / 16.0f;
			if (!dp->removed) renderScale(dp->x, 1264.0f - dp->y, 512.0f, 512.0f, dp->r, dp->r, scale, scale);
		}
	}
	endRender();
	if (isModeEnabled(MODE_SLIDING)) translateRenderer(-screenOffs, 0.0f);
	if (isModeEnabled(MODE_CLOUDS)) glUniform4f(colLocation0, 0.0f, 0.0f, 0.0f, 0.0f);
	if (isModeEnabled(MODE_SPINNING)) rotateRenderer(360.0f, 640 - scroll, -gameTime);
	if (isModeEnabled(MODE_GAUNTLET)) renderWind(0, -(gameTime << 4), 1.0f, 1);
	if (hitTime < 112 && (xWind != 0.0f || yWind != 0.0f))
	{
		float a = (xWind * xWind + yWind * yWind) * 2.0f;
		if (a > 1.0f) a = 1.0f;
		renderWind(xWindPos, yWindPos, a, 0);
	}
	beginRender(spriteShader, fontTexture);
	renderScore("Height: ", 8, 10, (long long) (scroll / 120 * 10), 16.0f, 16.0f);
	if (isModeEnabled(MODE_TIME_TRIAL)) renderScore(NULL, 0, 1, (long long) (timeLeft == 1800 ? 30 : (timeLeft == 0 ? 0 : timeLeft / 60 + 1)), 16.0f, 96.0f);
	endRender();
	beginRender(spriteShader, charsTexture);
	for (int i = 0; i < hitPoints; i++)
	{
		render(640.0f - i * 72, 16.0f, 640.0f, 512.0f, 64.0f, 64.0f);
	}
	endRender();
	if (tutorialTime > 32)
	{
		float a = -(tutorialTime - 32) / 32.0f;
		beginRender(spriteShader, fontTexture);
		glUniform4f(colLocation0, 1.0f, 1.0f, 1.0f, -a * 1.5f);
		renderText("Tilt to move", 48.0f, 320.0f);
		renderText("sideways!", 48.0f, 384.0f);
		renderText("Touch and hold", -672.0f, 320.0f);
		renderText("to rise!", -672.0f, 384.0f);
		endRender();
		beginRender(spriteShader, hudTexture);
		renderRotate(64.0f, 512.0f, 368.0f, 0.0f, 144.0f, 256.0f, isin(gameTime << 3) * 128.0f, 136.0f, 640.0f);
		render(528.0f, 512.0f, 368.0f, 0.0f, 144.0f, 256.0f);
		if ((gameTime / 72) & 1)
		{
			int gt = gameTime - gameTime / 72 * 72;
			float scale = 1.0f;
			if (gt < 8) scale = 0.5f + gt / 16.0f;
			renderScale(600.0f - scale * 16.0f, 704.0f - scale * 16.0f, 0.0f, 0.0f, 32.0f, 32.0f, scale, scale);
		}
		endRender();
		glUniform4f(colLocation0, 0.0f, 0.0f, 0.0f, 0.0f);
	}
	if (tallyPhase >= 0)
	{
		int yp = 64;
		if (tallyPhase == 0) yp = (int) ((1 - tallyTime / 30.0) * 704) - 640;
		beginRender(spriteShader, shopTexture);
		renderWindow(40, yp, 20, 20);
		endRender();
		if (tallyPhase > 0)
		{
			beginRender(spriteShader, fontTexture);
			renderText("SCORE", 72.0f, 96.0f);
			renderText("COINS GAINED", 72.0f, 288.0f);
			renderText("TOP SCORE BONUS", 72.0f, 352.0f);
			renderText("COINS", 72.0f, 608.0f);
			renderScore(NULL, 0, 10, heightScoreRender, -648.0f, 96.0f);
			renderScore(NULL, 0, 1, coinsGainedRender, -648.0f, 288.0f);
			renderScore(NULL, 0, 1, coinsBonusRender, -648.0f, 352.0f);
			renderScore(NULL, 0, 1, (long long) coins, -648.0f, 608.0f);
			if (tallyPhase != 2 || ((tallyTime >> 3) & 1))
			{
				renderText("TOP SCORE", 72.0f, 160.0f);
				renderScore(NULL, 0, 10, (long long) topScore, -648.0f, 160.0f);
			}
			if (tallyPhase >= 7)
			{
				renderTextScale("RETRY", 260.0f, 800.0f, 1.5f);
				renderTextScale("SHOP", 275.0f, 960.0f, 1.5f);
			}
			endRender();
		}
	}
}
void gameTouchStart(int x, int y, int id)
{
	if (gameState != GAME_STATE_GOT_HIT)
	{
		unsigned short endlessMode = isModeEnabled(MODE_ENDLESS);
		if (gameState == GAME_STATE_ON_GROUND && (!isModeEnabled(MODE_SPINNING) || y >= 1024))
		{
			gameState = GAME_STATE_IN_GAME;
			if (endlessMode) endlessMode = 0;
		}
		if (isModeEnabled(MODE_PROTECT))
		{
			y = 1280 - y + scroll;
			yWind = 1.0f;
			if (yaPlayer < 2.0f) yaPlayer = 2.0f;
			selectedObstacle = NULL;
			for (int i = 0; i < zoneCount; i++)
			{
				Zone* z = zones[i];
				for (int j = 0; j < z->count; j++)
				{
					Obstacle* o = z->obstacles[j];
					switch (o->type)
					{
					case TYPE_GIRDERS_HORIZONTAL:
						if (y >= o->y && y < o->y + 96) selectedObstacle = o;
						break;
					case TYPE_GIRDERS_VERTICAL:
						if (x >= o->x && x < o->x + 96 && y >= o->y && y < o->y + o->w) selectedObstacle = o;
						break;
					case TYPE_GIRDER_SNAKE:
						if (x >= o->x && x < o->x + o->w && y >= o->y && y < o->y + 96) selectedObstacle = o;
						break;
					case TYPE_BLOCKS:
						if (x >= o->x && x < o->x + 128 && y >= o->y && y < o->y + 128) selectedObstacle = o;
						break;
					}
					if (selectedObstacle != NULL)
					{
						selectedZone = z;
						break;
					}
				}
			}
			return;
		}
		if (hitTime < 112)
		{
			if (endlessMode)
			{
				yWind = -1.0f;
				if (yaPlayer > 0.0f) yaPlayer = -yaPlayer;
			}
			else if (isModeEnabled(MODE_JUMP))
			{
				yWind = 1.0f;
				yaPlayer = 12.0f;
			}
			else if (isModeEnabled(MODE_LEGACY))
			{
				if (y >= 1024) yWind = 1.0f;
				else if (x < 256) xWind = 1.0f;
				else if (x >= 464) xWind = -1.0f;
				else if (y < 256) yWind = -1.0f;
			}
			else
			{
				yWind = 1.0f;
				if (yaPlayer < 2.0f) yaPlayer = 2.0f;
			}
		}
	}
}
void gameTouchEnd(int x, int y, int id)
{
	if (gameState == GAME_STATE_GOT_HIT)
	{
		if (animTime == 0)
		{
			if (tallyPhase < 7) tallyTime = 1;
			else
			{
				if (y >= 960) setState(STATE_SHOP);
				else resetGame();
			}
		}
		return;
	}
	if (selectedObstacle != NULL)
	{
		selectedZone = NULL;
		selectedObstacle = NULL;
		return;
	}
	if (isModeEnabled(MODE_JUMP))
	{
		if (yaPlayer > 8.0f) yaPlayer = 8.0f;
		return;
	}
	if (isModeEnabled(MODE_ENDLESS | MODE_PROTECT) && gameState == GAME_STATE_IN_GAME)
	{
		yWind = 1.0f;
		if (yaPlayer < 0.0f) yaPlayer = -yaPlayer;
	}
	else
	{
		if (isModeEnabled(MODE_LEGACY)) xWind = 0.0f;
		yWind = 0.0f;
	}
}
void gameTouchMove(int xd)
{
	if (selectedObstacle != NULL)
	{
		if (xd < -64) xd = -64;
		else if (xd > 64) xd = 64;
		selectedObstacle->x += xd;
		for (int i = 0; i < selectedZone->count; i++)
		{
			Obstacle* o = selectedZone->obstacles[i];
			if (o == selectedObstacle) continue;
			switch (o->type)
			{
			case TYPE_GIRDERS_VERTICAL:
				if (checkCollide(selectedObstacle->x, selectedObstacle->y, selectedObstacle->x + 96, selectedObstacle->y + selectedObstacle->w, o->x, o->y, o->x + 96, o->y + o->w))
				{
					if (xd > 0) selectedObstacle->x = o->x - 96;
					else if (xd < 0) selectedObstacle->x = o->x + 96;
				}
				break;
			case TYPE_BLOCKS:
				if (checkCollide(selectedObstacle->x, selectedObstacle->y, selectedObstacle->x + 128, selectedObstacle->y + 128, o->x, o->y, o->x + 128, o->y + 128))
				{
					if (xd > 0) selectedObstacle->x = o->x - 128;
					else if (xd < 0) selectedObstacle->x = o->x + 128;
				}
				break;
			}
		}
	}
}
void gameTilt(float a)
{
	if (isModeEnabled(MODE_PROTECT | MODE_LEGACY)) return;
	if (!isModeEnabled(MODE_MIRROR)) a = -a;
	if (a > -0.125f && a < 0.125f) xWind = 0.0f;
	else xWind = a * 2.0f;
	if (xWind < -2.0f) xWind = -2.0f;
	else if (xWind > 2.0f) xWind = 2.0f;
}
void deleteGame()
{
	if (bulletCount > 0)
	{
		while (bullets != NULL)
		{
			Bullet* next = bullets->next;
			free(bullets);
			bullets = next;
		}
	}
	for (int i = 0; i < zoneCount; i++)
	{
		if (zones[i] != NULL)
		{
			deleteZone(zones[i]);
			free(zones[i]);
		}
	}
}
