#ifndef _SHADER_H
#define _SHADER_H
#include <GLES2/gl2.h>
GLuint* colourShader;
GLuint* skyShader;
GLuint* spriteShader;
void loadAllShaders();
void deleteAllShaders();
#endif
