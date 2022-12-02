#include "shader.h"
#include "SDL.h"
#include "SDL_rwops.h"
GLuint compileShader(const char* source, int len, GLenum type)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, &len);
	glCompileShader(shader);
	GLint isCompiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled != GL_TRUE)
	{
		GLint length = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		char* log = (char*) malloc(length * sizeof(char));
		GLint result;
		glGetShaderInfoLog(shader, length, &result, log);
		SDL_Log("%s", log);
		free(log);
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}
char* readShaderFromFile(const char* path, int* len)
{
	SDL_RWops* rw = SDL_RWFromFile(path, "rb");
	if (rw == NULL) return NULL;
	*len = SDL_RWsize(rw);
	char* res = (char*) malloc(*len + 1);
	Sint64 readTotal = 0;
	Sint64 read = 1;
	char* buffer = res;
	while (readTotal < *len && read != 0)
	{
		read = SDL_RWread(rw, buffer, 1, *len - readTotal);
		readTotal += read;
		buffer += read;
	}
	SDL_RWclose(rw);
	if (readTotal != *len)
	{
		free(res);
		return NULL;
	}
	res[readTotal] = '\0';
	return res;
}
GLuint* loadShader(const char* vertexPath, const char* fragmentPath)
{
	int vertexShaderLength, fragmentShaderLength;
	char* vertexShaderCode = readShaderFromFile(vertexPath, &vertexShaderLength);
	char* fragmentShaderCode = readShaderFromFile(fragmentPath, &fragmentShaderLength);
	GLuint vertexShader = compileShader(vertexShaderCode, vertexShaderLength, GL_VERTEX_SHADER);
	GLuint fragmentShader = compileShader(fragmentShaderCode, fragmentShaderLength, GL_FRAGMENT_SHADER);
	free(vertexShaderCode);
	free(fragmentShaderCode);
	GLuint* program = (GLuint*) malloc(sizeof(GLuint));
	*program = glCreateProgram();
	glAttachShader(*program, vertexShader);
	glAttachShader(*program, fragmentShader);
	glBindAttribLocation(*program, 0, "a_pos");
	glBindAttribLocation(*program, 1, "a_tex");
	glLinkProgram(*program);
	GLint isLinked = 0;
	glGetProgramiv(*program, GL_LINK_STATUS, &isLinked);
	if (isLinked != GL_TRUE)
	{
		GLint length = 0;
		glGetProgramiv(*program, GL_INFO_LOG_LENGTH, &length);
		char* log = (char*) malloc(length * sizeof(char));
		GLint result;
		glGetProgramInfoLog(*program, length, &result, log);
		SDL_Log("%s", log);
		free(log);
		glDeleteProgram(*program);
	}
	glUseProgram(*program);
	return program;
}
void loadAllShaders()
{
	colourShader = loadShader("sdr/colour.vsh", "sdr/colour.fsh");
	skyShader = loadShader("sdr/sky.vsh", "sdr/sprite.fsh");
	spriteShader = loadShader("sdr/sprite.vsh", "sdr/sprite.fsh");
}
void deleteAllShaders()
{
	glDeleteProgram(*colourShader);
	glDeleteProgram(*skyShader);
	glDeleteProgram(*spriteShader);
	free(colourShader);
	free(skyShader);
	free(spriteShader);
}
