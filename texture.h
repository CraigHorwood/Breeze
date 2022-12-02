#ifndef _TEXTURE_H
#define _TEXTURE_H
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
typedef struct
{
	GLuint texture;
	int width, height;
} Texture;
Texture* charsTexture;
Texture* fontTexture;
Texture* girderTexture;
Texture* hudTexture;
Texture* shopTexture;
Texture* skyTexture;
Texture* skyHazeTexture;
Texture* skyGauntletTexture;
void loadAllTextures();
void deleteAllTextures();
#endif
