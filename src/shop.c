#include "shop.h"
#include "renderer.h"
#include "game.h"
#include "random.h"
#include "main.h"
const int COLOUR_PRICES[16] = {0, 10, 10, 20, 20, 20, 40, 40, 50, 50, 75, 75, 90, 100, 200, 400};
const int MODE_PRICES[16] = {10, 10, 10, 20, 20, 20, 30, 30, 30, 50, 50, 50, 75, 90, 100, 20};
const char* COLOUR_NAMES[16] = {"RED", "GREEN", "BLUE", "CYAN", "MAGENTA", "YELLOW", "PINK", "PURPLE", "WHITE", "ORANGE", "BLACK", "DARK BLUE", "SKY BLUE", "CHARTREUSE", "SILVER", "GOLD"};
const char* MODE_NAMES[16] = {"HIGH GRAVITY", "HIGH SPEED", "CLOUDY MODE", "ONE HIT WONDER", "FREE FOR ALL", "MIRROR MODE", "ENDLESS RUNNER", "HEARTLESS MODE", "SAVE THIS PRECIOUS BIRD", "TIME TRIAL", "JUMP MODE", "TIME IS FORWARD", "LEGACY MODE", "SLIDING SCREEN", "SPINNING MODE", "THE GAUNTLET"};
const float COLOUR_VALUES[16][3] = {{1.0f, 0.0f, 0.0f}, {0.0f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.5f, 1.0f}, {0.675f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.5f}, {0.49f, 0.71f, 0.96f}, {0.75f, 1.0f, 0.0f}, {0.75f, 0.75f, 0.75f}, {1.0f, 0.75f, 0.0f}};
const char* MODE_INFO[16] = {"In this mode, gravity is more than twice\nas powerful as usual. Bird will have a\nhard time ascending, and an even harder\ntime controlling his descent!", "The sky's the limit for speed in this\nmode, assuming you can even get that\nhigh with acceleration like this!", "In this mode, a thick haze and slowly\ndrifting clouds obstruct your vision.\nYou'll need an astute eye to even see\nwhat's going on!", "The name says it all! Your starting health\nis reduced from one heart to zero\nhearts, and it is impossible to gain any\nmore.", "Usually, certain types of obstacles will\nnot generate until you have gained\nsufficient height. In this mode, any type\nof obstacle can be generated at any\nheight. Play with One Hit Wonder mode\nenabled for a truly unpredictable\nchallenge!", "Fight your intuition! Tilt left to go right,\ntilt right to go left.", "In this mode, Bird is constantly moving\nup, unless you touch the screen, in which\ncase he is constantly moving down. The\nkey words here are \"constantly\nmoving\", thus you must control Bird's\nheight in a much more frantic and fast-\npaced way.", "In this mode, hearts will not generate in\nbetween obstacles, thus you are unable\nto recover the one mistake you are\npermitted. Perhaps you could call this\nmode \"Two Hit Wonder\".", "In this mode, you have zero control over\nBird, as he is constantly moving up and\nmoving back and forth in a regular\npattern. Your job, therefore, is to touch\nobstacles and move them out of the way,\nso that Bird's journey is still a safe one.", "From the moment you start, you have\nthirty seconds to get as high as you\npossibly can. Collect hearts to reset the\ntimer.", "Play the game in a way similar to\nanother game featuring a bird in the\nstarring role. Tap the screen to jump,\nand hold for a higher jump.", "Time only moves when you move up. Play\nwith Free For All mode enabled for the\nbest experience.", "Play the game as it was originally\nenvisioned in early 2017! Touch the\nbottom of the screen to move up, the\nleft to move right, the right to move\nleft, and if for some reason you want to,\nthe top to move down. Hopefully as you\nplay, it becomes clear why these\ncontrols were not used.", "Tilt the screen and the whole screen\nmoves! If you want a decent view of\nwhat's going on, careful balancing of\nyour speed and the tilt of the screen is\nrequired.", "The controls are the same, Bird moves in\nthe same way, but the world around him\nis anything but cooperative. As the\nworld continues to spin, the direction\nthat takes you higher is constantly\nchanging, and it is your job to adapt.", "If you were dedicated enough to make it\nthis far, The Gauntlet is your final test\nof skill. In this mode, there is no random\ngeneration, there are no hit points, no\nhearts, and no mercy. Good luck."};
int shopTime = 0;
char modesList = 0;
unsigned char buying = 0;
unsigned char scrollPositions = 0;
unsigned char windowOpen = 0;
void resetShop()
{
	modesList = 0;
	selectedColour = COLOUR_RED;
	selectedModes = 0;
	scrollPositions = 0;
	windowOpen = 0;
}
void tickShop()
{
	shopTime++;
}
unsigned short isModeEnabled(unsigned short mode)
{
	return selectedModes & mode;
}
int hasBought(int i)
{
	if (modesList) return (bought >> 16) & (1 << i);
	return bought & (1 << i);
}
char canBuy(int i)
{
	if (modesList && i == 15) return bought == 2147483647;
	const int* prices = modesList ? MODE_PRICES : COLOUR_PRICES;
	return coins >= prices[i];
}
void checkItem(int i)
{
	if (modesList)
	{
		unsigned short mode = 1 << i;
		if (mode == MODE_GAUNTLET) selectedModes = selectedModes == MODE_GAUNTLET ? 0 : MODE_GAUNTLET;
		else
		{
			if (isModeEnabled(MODE_GAUNTLET)) selectedModes &= ~MODE_GAUNTLET;
			selectedModes ^= mode;
		}
	}
	else selectedColour = i;
}
void buyItem(int i)
{
	const int* prices = modesList ? MODE_PRICES : COLOUR_PRICES;
	coins -= prices[i];
	if (modesList) bought |= (65536 << i);
	else bought |= (1 << i);
	windowOpen = 0;
	saveGame();
	checkItem(i);
}
void renderItem(int pos, int index)
{
	int yp = 304 + pos * 176;
	beginRender(colourShader, shopTexture);
	glUniform3f(colLocation1, 0.625f, 0.625f, 0.625f);
	render(72.0f, yp, 0.0f, 0.0f, 512.0f, 152.0f);
	endRender();
	if (!modesList)
	{
		beginRender(spriteShader, charsTexture);
		float offs = index == COLOUR_GOLD ? ((next() & 15) - (next() & 15)) / 256.0f : 0.0f;
		glUniform4f(colLocation0, COLOUR_VALUES[index][0] + offs, COLOUR_VALUES[index][1] + offs, COLOUR_VALUES[index][2] + offs, 1.0f);
		render(88.0f, yp + 28.0f, 0.0f, 0.0f, 120.0f, 96.0f);
		endRender();
		beginRender(spriteShader, charsTexture);
		glUniform4f(colLocation0, 0.0f, 0.0f, 0.0f, 0.0f);
		render(88.0f, yp + 28.0f, 120.0f, 0.0f, 120.0f, 96.0f);
		render(88.0f, yp + 28.0f, 240.0f, 0.0f, 120.0f, 96.0f);
		endRender();
	}
	beginRender(spriteShader, shopTexture);
	if (modesList) render(520.0f, yp + 16.0f, 0.0f, 144.0f, 48.0f, 48.0f);
	if (hasBought(index))
	{
		render(520.0f, yp + 88.0f, 0.0f, 96.0f, 48.0f, 48.0f);
		unsigned short renderTick = modesList ? isModeEnabled(1 << index) : (selectedColour == index);
		if (renderTick) render(530.0f, yp + 78.0f, 48.0f, 96.0f, 48.0f, 48.0f);
		endRender();
	}
	else
	{
		char cb = canBuy(index);
		if (!cb)
		{
			endRender();
			beginRender(spriteShader, shopTexture);
			glUniform4f(colLocation0, 1.0f, 1.0f, 1.0f, 0.5f);
		}
		render(424.0f, yp + 88.0f, 0.0f, 192.0f, 144.0f, 48.0f);
		endRender();
		if (!cb) glUniform4f(colLocation0, 0.0f, 0.0f, 0.0f, 0.0f);
	}
	int xp = modesList ? 88 : 240;
	const char** names = modesList ? MODE_NAMES : COLOUR_NAMES;
	beginRender(spriteShader, fontTexture);
	renderTextScale(names[index], xp, yp + 8.0f, modesList && index == 8 ? 0.75f : 1.0f);
	if (modesList && index == 15) renderTextScale("Buy everything else", xp, yp + 72.0f, 0.75f);
	else renderScore(NULL, 0, 1, modesList ? MODE_PRICES[index] : COLOUR_PRICES[index], xp, yp + 72.0f);
	endRender();
}
void renderShop()
{
	beginRender(skyShader, skyTexture);
	glUniform1f(scrollLocation, shopTime / 512.0f);
	render(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	endRenderRender();
	beginRender(spriteShader, shopTexture);
	renderWindow(40, 160, 20, 27);
	endRender();
	beginRender(colourShader, shopTexture);
	glUniform3f(colLocation1, 0.375f, 0.375f, 0.375f);
	render(360.0f - modesList * 304.0f, 176.0f, 0.0f, 0.0f, modesList ? 320.0f : 304.0f, 96.0f);
	endRender();
	unsigned char pos = modesList ? ((scrollPositions >> 4) & 15) : (scrollPositions & 15);
	beginRender(spriteShader, shopTexture);
	render(360.0f, 176.0f, 120.0f - modesList * 24.0f, 0.0f, 24.0f, 96.0f);
	if (pos > 0) render(600.0f, 304.0f, 160.0f, 0.0f, 48.0f, 48.0f);
	if (pos < 12) render(600.0f, 936.0f, 160.0f, 48.0f, 48.0f, 48.0f);
	endRender();
	for (int i = pos; i < pos + 4; i++)
	{
		renderItem(i - pos, i);
	}
	beginRender(spriteShader, fontTexture);
	renderTextScale("SHOP", 247.0f, 8.0f, 2.0f);
	renderScore("Coins: ", 7, 1, (long long) coins, 40.0f, 1040.0f);
	renderTextScale("PLAY", 274.0f, 1120.0f, 1.5f);
	renderText("Colours", 80.0f, 180.0f);
	renderText("Modes", 505.0f, 180.0f);
	endRender();
	if (windowOpen)
	{
		beginRender(spriteShader, shopTexture);
		renderWindow(24, 320, 21, 20);
		render(616.0f, 352.0f, 208.0f, 0.0f, 48.0f, 48.0f);
		endRender();
		beginRender(spriteShader, fontTexture);
		if (windowOpen == 255)
		{
			renderText("Buy this item?", 211.0f, 432.0f);
			renderTextScale("YES", 88.0f, 800.0f, 1.5f);
			renderTextScale("NO", 542.0f, 800.0f, 1.5f);
		}
		else
		{
			renderText(MODE_NAMES[windowOpen - 1], 56.0f, 336.0f);
			renderTextScale(MODE_INFO[windowOpen - 1], 56.0f, 432.0f, 0.75f);
		}
		endRender();
	}
}
void shopTouchEnd(int x, int y)
{
	if (windowOpen)
	{
		if (x >= 616 && y <= 400) windowOpen = 0;
		else if (y >= 800)
		{
			if (x < 360) buyItem(buying);
			else windowOpen = 0;
		}
		return;
	}
	if (y < 272)
	{
		if (x < 360) modesList = 0;
		else modesList = 1;
	}
	else if (y >= 1120) setState(STATE_GAME);
	else
	{
		unsigned char pos = scrollPositions;
		if (modesList) pos >>= 4;
		pos &= 15;
		if (y < 1024 && x >= 584)
		{
			if (y < 368 && pos > 0)
			{
				if (modesList) scrollPositions -= 16;
				else scrollPositions--;
			}
			else if (y > 920 && pos < 12)
			{
				if (modesList) scrollPositions += 16;
				else scrollPositions++;
			}
		}
		else if (y < 1024 && x >= 328 && x < 584)
		{
			for (int i = 0; i < 4; i++)
			{
				int yp = 304 + i * 176;
				if (y >= yp && y < yp + 76 && modesList) windowOpen = i + pos + 1;
				else if (y >= yp + 76 && y < yp + 152)
				{
					if (hasBought(i + pos)) checkItem(i + pos);
					else if (canBuy(i + pos))
					{
						buying = i + pos;
						windowOpen = 255;
					}
				}
			}
		}
	}
}
