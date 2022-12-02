#include "title.h"
#include "main.h"
#include "renderer.h"
#include "game.h"
int titleTime = 0;
void tickTitle()
{
	titleTime++;
}
void renderTitle()
{
	beginRender(skyShader, skyTexture);
	glUniform1f(scrollLocation, titleTime / 512.0f);
	render(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	endRenderRender();
	if (titleTime >= 16)
	{
		beginRender(spriteShader, fontTexture);
		render(104.0f, titleTime < 80 ? (titleTime - 16) * 12 - 512 : 256.0f, 0.0f, 256.0f, 512.0f, 192.0f);
		render(124.0f, titleTime < 80 ? (titleTime - 16) * 12 - 288 : 480.0f, 0.0f, 472.0f, 471.0f, 40.0f);
		if (titleTime >= 80)
		{
			renderTextScale("PLAY GAME", 175.0f, titleTime < 112 ? 1360 - (titleTime - 80) * 20 : 720.0f, 1.5f);
			if (titleTime >= 96)
			{
				renderTextScale("SHOP", 276.0f, titleTime < 128 ? 1520 - (titleTime - 96) * 20 : 880.0f, 1.5f);
				if (titleTime >= 112) renderScore("TOP SCORE: ", 11, 10, (long long) topScore, 16.0f, titleTime < 144 ? -624 + (titleTime - 112) * 20 : 16.0f);
			}
		}
		endRender();
	}
}
void titleTouchEnd(int x, int y)
{
	if (titleTime < 128)
	{
		titleTime = 144;
		return;
	}
	if (y >= 720 && y < 840) setState(STATE_GAME);
	else if (y >= 840) setState(STATE_SHOP);
}
