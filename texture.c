#include "texture.h"
#include "SDL.h"
#include "SDL2_image/SDL_image.h"
Texture* loadTexture(const char* path)
{
	SDL_Surface* img;
	img = IMG_Load(path);
	if (img == NULL)
	{
		SDL_Log("Unable to load texture at: %s\n", path);
		SDL_FreeSurface(img);
		return 0;
	}
	int width = img->w;
	int height = img->h;
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, img->pixels);
	SDL_FreeSurface(img);
	Texture* tex = (Texture*) malloc(sizeof(Texture));
	tex->texture = texture;
	tex->width = width;
	tex->height = height;
	return tex;
}
void loadAllTextures()
{
	charsTexture = loadTexture("tex/chars.png");
	fontTexture = loadTexture("tex/font.png");
	girderTexture = loadTexture("tex/girder.png");
	hudTexture = loadTexture("tex/hud.png");
	shopTexture = loadTexture("tex/shop.png");
	skyTexture = loadTexture("tex/sky.png");
	skyGauntletTexture = loadTexture("tex/sky_gauntlet.png");
	skyHazeTexture = loadTexture("tex/sky_haze.png");
}
void deleteAllTextures()
{
	glDeleteTextures(1, &(charsTexture->texture));
	glDeleteTextures(1, &(fontTexture->texture));
	glDeleteTextures(1, &(girderTexture->texture));
	glDeleteTextures(1, &(hudTexture->texture));
	glDeleteTextures(1, &(shopTexture->texture));
	glDeleteTextures(1, &(skyTexture->texture));
	glDeleteTextures(1, &(skyGauntletTexture->texture));
	glDeleteTextures(1, &(skyHazeTexture->texture));
	free(charsTexture);
	free(fontTexture);
	free(girderTexture);
	free(hudTexture);
	free(shopTexture);
	free(skyTexture);
	free(skyGauntletTexture);
	free(skyHazeTexture);
	IMG_Quit();
}
