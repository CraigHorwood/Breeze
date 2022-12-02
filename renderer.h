#ifndef _RENDERER_H
#define _RENDERER_H
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#include "shader.h"
#include "texture.h"
GLint scrollLocation, colLocation0, colLocation1;
void initRenderer();
void deleteRenderer();
void translateRenderer(float, float);
void rotateRenderer(float, float, int);
void beginRender(GLuint*, Texture*);
void render(float, float, float, float, float, float);
void renderScale(float, float, float, float, float, float, float, float);
void renderRotate(float, float, float, float, float, float, int, float, float);
void renderText(const char*, float, float);
void renderTextScale(const char*, float, float, float);
void endRender();
void endRenderRender();
void renderWind(int, int, float, char);
void renderWindow(int, int, int, int);
#endif
