#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "../SDL/src/core/android/SDL_android.h"
#include "SDL.h"
#include "shader.h"
#include "texture.h"
#include "renderer.h"
#include "imath.h"
#include "random.h"
#include "title.h"
#include "game.h"
#include "shop.h"
float accel[3];
char state = STATE_TITLE_SCREEN;
void setState(char newState)
{
	switch (newState)
	{
	case STATE_GAME:
		resetGame();
		break;
	case STATE_SHOP:
		if (state == STATE_TITLE_SCREEN) resetShop();
		break;
	}
	state = newState;
}
void tickMain()
{
	switch (state)
	{
	case STATE_TITLE_SCREEN:
		tickTitle();
		break;
	case STATE_GAME:
		tickGame();
		break;
	case STATE_SHOP:
		tickShop();
		break;
	}
}
void renderMain()
{
	glClear(GL_COLOR_BUFFER_BIT);
	switch (state)
	{
	case STATE_TITLE_SCREEN:
		renderTitle();
		break;
	case STATE_GAME:
		renderGame();
		break;
	case STATE_SHOP:
		renderShop();
		break;
	}
}
void onTouchStart(int x, int y, int id)
{
	if (state == STATE_GAME) gameTouchStart(x, y, id);
}
void onTouchEnd(int x, int y, int id)
{
	switch (state)
	{
	case STATE_TITLE_SCREEN:
		titleTouchEnd(x, y);
		break;
	case STATE_GAME:
		gameTouchEnd(x, y, id);
		break;
	case STATE_SHOP:
		shopTouchEnd(x, y);
		break;
	}
}
void onTouchMove(int xd)
{
	if (state == STATE_GAME) gameTouchMove(xd);
}
int main(int argc, char* argv[])
{
	SDL_Window* window = 0;
	SDL_GLContext gl = 0;
	if (0 != SDL_Init(SDL_INIT_VIDEO))
	{
		fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
		return 1;
	}
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_DisplayMode mode;
	SDL_GetDisplayMode(0, 0, &mode);
	int width = mode.w;
	int height = mode.h;
	SDL_Log("Width = %d. Height = %d.\n", width, height);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	window = SDL_CreateWindow(NULL, 0, 0, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_RESIZABLE);
	if (window == 0)
	{
		SDL_Log("Failed to create window.\n");
		SDL_Quit();
		return 1;
	}
	gl = SDL_GL_CreateContext(window);
	glViewport(0, 0, width, height);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	loadAllShaders();
	loadAllTextures();
	initRenderer();
	initMath();
	initRandom();
	topScore = 0;
	coins = 0;
	bought = 1;
	prefPath = strcat(SDL_GetPrefPath("Craig Horwood", "Breeze"), "save.bin");
	SDL_RWops* rw = SDL_RWFromFile(prefPath, "rb");
	if (rw != NULL)
	{
		SDL_RWread(rw, &topScore, 4, 1);
		SDL_RWread(rw, &coins, 4, 1);
		SDL_RWread(rw, &bought, 4, 1);
		SDL_RWclose(rw);
	}
	resetShop();
	msg = (char*) malloc(sizeof(char) << 5);
	Uint32 then = SDL_GetTicks();
	double unprocessed = 0;
	double msPerFrame = 1000.0 / 60.0;
	char isClosed = 0;
	SDL_Log("Starting application.");
	SDL_Event e;
	while (!isClosed)
	{
		Uint32 now = SDL_GetTicks();
		unprocessed += now - then;
		then = now;
		if (unprocessed > 100) unprocessed = 100;
		while (unprocessed > 0)
		{
			Android_JNI_GetAccelerometerValues(accel);
			if (state == STATE_GAME) gameTilt(accel[2] < -0.5f ? -accel[0] : accel[0]);
			tickMain();
			unprocessed -= msPerFrame;
		}
		renderMain();
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT || e.type == SDL_APP_TERMINATING) isClosed = 1;
			else if (e.type == SDL_KEYDOWN)
			{
				if (e.key.keysym.sym == SDLK_AC_BACK) isClosed = 1;
			}
			else if (e.type == SDL_FINGERDOWN)
			{
				onTouchStart((int) (e.tfinger.x * 720), (int) (e.tfinger.y * 1280), (int) e.tfinger.fingerId);
			}
			else if (e.type == SDL_FINGERUP)
			{
				onTouchEnd((int) (e.tfinger.x * 720), (int) (e.tfinger.y * 1280), (int) e.tfinger.fingerId);
			}
			else if (e.type == SDL_FINGERMOTION)
			{
				onTouchMove((int) (e.tfinger.dx * 720));
			}
		}
		SDL_GL_SwapWindow(window);
		SDL_Delay(1);
	}
	deleteAllShaders();
	deleteAllTextures();
	deleteRenderer();
	deleteGame();
	free(msg);
	if (levelData != NULL) free(levelData);
	SDL_GL_DeleteContext(gl);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
