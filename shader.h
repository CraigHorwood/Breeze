#ifndef _SHADER_H
#define _SHADER_H
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
GLuint* colourShader;
GLuint* skyShader;
GLuint* spriteShader;
void loadAllShaders();
void deleteAllShaders();
#endif
