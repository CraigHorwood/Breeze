#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#include "SDL.h"
#include "shader.h"
#include "texture.h"
#include "renderer.h"
#include "imath.h"
#include "random.h"
#include "title.h"
#include "game.h"
#include "shop.h"
#include "sound.h"
#include "motion.h"
#include "save.h"
float accel[3];
char state = STATE_TITLE_SCREEN;
void setState(char newState)
{
	switch (newState)
	{
	case STATE_TITLE_SCREEN:
		playAmbient(SOUND_BACKGROUND);
		break;
	case STATE_GAME:
		if (isModeEnabled(MODE_GAUNTLET)) playAmbient(SOUND_RAIN);
		else stopAmbient();
		resetGame();
		break;
	case STATE_SHOP:
		playAmbient(SOUND_BACKGROUND);
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
    SDL_DisplayMode mode;
    SDL_GetDisplayMode(0, 0, &mode);
    int width = mode.w;
    int height = mode.h;
    if ((float) width / height > 1.0f)
    {
        width = mode.h;
        height = mode.w;
    }
	SDL_Log("Width = %d. Height = %d.\n", width, height);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    window = SDL_CreateWindow(NULL, 0, 0, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	if (window == 0)
	{
		SDL_Log("Failed to create window.\n");
		SDL_Quit();
		return 1;
	}
	gl = SDL_GL_CreateContext(window);
    SDL_GL_GetDrawableSize(window, &width, &height);
    SDL_Log("Width = %d. Height = %d.\n", width, height);
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
	initSound();
    initMotion();
	topScore = 0;
	coins = 0;
	bought = 1;
    loadGame();
	resetShop();
	setState(STATE_TITLE_SCREEN);
	msg = (char*) malloc(sizeof(char) << 5);
	Uint32 then = SDL_GetTicks();
	double unprocessed = 0;
	double msPerFrame = 1000.0 / 60.0;
	char isClosed = 0;
    char isPaused = 0;
	SDL_Event e;
	while (!isClosed)
	{
		Uint32 now = SDL_GetTicks();
		unprocessed += now - then;
		then = now;
		if (unprocessed > 100) unprocessed = 100;
		while (unprocessed > 0)
		{
            getAccel(accel);
            if (state == STATE_GAME) gameTilt(-accel[0]);
			tickMain();
			tickSound();
			unprocessed -= msPerFrame;
		}
		renderMain();
		while (!isClosed && (SDL_PollEvent(&e) || isPaused))
		{
			if (e.type == SDL_QUIT || e.type == SDL_APP_TERMINATING) isClosed = 1;
            else if (e.type == SDL_APP_WILLENTERBACKGROUND || e.type == SDL_APP_DIDENTERBACKGROUND) isPaused = 1;
            else if (e.type == SDL_APP_WILLENTERFOREGROUND || e.type == SDL_APP_DIDENTERFOREGROUND) isPaused = 0;
			else if (e.type == SDL_FINGERDOWN)
			{
                if (isPaused) isPaused = 0;
				onTouchStart((int) (e.tfinger.x * 720), (int) (e.tfinger.y * 1280), (int) e.tfinger.fingerId);
			}
            if (isPaused) continue;
			if (e.type == SDL_FINGERUP)
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
	deleteSound();
	deleteGame();
	free(msg);
	if (levelData != NULL) free(levelData);
	SDL_GL_DeleteContext(gl);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
