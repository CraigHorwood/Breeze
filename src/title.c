#include "title.h"
#include "main.h"
#include "renderer.h"
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
}
void titleTouchEnd(int x, int y)
{
	if (y > 720) setState(STATE_SHOP);
	else setState(STATE_GAME);
}
