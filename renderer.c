#include "renderer.h"
#include "imath.h"
#include <stdlib.h>
#include <string.h>
#include "SDL.h"
#define MAX_SPRITES 256
#define FLOATS_PER_VERTEX 4
typedef struct
{
	short x, y;
	unsigned char w, h;
	signed char xo, yo, xa;
	int kernings[256];
} GlyphInfo;
unsigned char lineHeight = 0;
GlyphInfo glyphs[256];
float vertexData[MAX_SPRITES * FLOATS_PER_VERTEX << 2];
GLuint vbo, ibo, windVbo, windIbo;
float projectionMatrix[16];
float modelviewMatrix[16];
char transpose;
GLint projectionMatrixLocation[2];
GLint modelviewMatrixLocation[2];
GLuint* lastShader;
Texture* lastTexture;
int spriteCount;
void resetModelview()
{
	for (int i = 0; i < 16; i++)
	{
		if (i % 5 == 0) modelviewMatrix[i] = 1.0f;
		else modelviewMatrix[i] = 0.0f;
	}
}
short readShort(SDL_RWops* rw)
{
	unsigned char c0, c1;
	SDL_RWread(rw, &c0, 1, 1);
	SDL_RWread(rw, &c1, 1, 1);
	return (c0 << 8) | c1;
}
int readInt(SDL_RWops* rw)
{
	unsigned char c0, c1, c2, c3;
	SDL_RWread(rw, &c0, 1, 1);
	SDL_RWread(rw, &c1, 1, 1);
	SDL_RWread(rw, &c2, 1, 1);
	SDL_RWread(rw, &c3, 1, 1);
	return (c0 << 24) | (c1 << 16) | (c2 << 8) | c3;
}
void initRenderer()
{
	SDL_RWops* rw = SDL_RWFromFile("tex/font.fnt", "rb");
	if (rw != NULL)
	{
		int hdr = readInt(rw);
		if (hdr == 842346549)
		{
			SDL_RWread(rw, &lineHeight, 1, 1);
			int charCount = readInt(rw);
			int kernCount = readInt(rw);
			for (int i = 0; i < charCount; i++)
			{
				unsigned char ch, w, h;
				char xo, yo, xa;
				SDL_RWread(rw, &ch, 1, 1);
				short x = readShort(rw);
				short y = readShort(rw);
				SDL_RWread(rw, &w, 1, 1);
				SDL_RWread(rw, &h, 1, 1);
				SDL_RWread(rw, &xo, 1, 1);
				SDL_RWread(rw, &yo, 1, 1);
				SDL_RWread(rw, &xa, 1, 1);
				GlyphInfo* gi = &glyphs[ch];
				gi->x = x;
				gi->y = y;
				gi->w = w;
				gi->h = h;
				gi->xo = xo;
				gi->yo = yo;
				gi->xa = xa;
				for (int j = 0; j < 256; j++)
				{
					gi->kernings[j] = 0;
				}
			}
			for (int i = 0; i < kernCount; i++)
			{
				unsigned char ch, with, kern;
				SDL_RWread(rw, &ch, 1, 1);
				SDL_RWread(rw, &with, 1, 1);
				SDL_RWread(rw, &kern, 1, 1);
				glyphs[ch].kernings[with] = kern;
			}
		}
		SDL_RWclose(rw);
	}
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_DYNAMIC_DRAW);
	short indexData[MAX_SPRITES * 6];
	for (int i = 0; i < MAX_SPRITES; i++)
	{
		int j = i * 6;
		short k = (short) (i << 2);
		indexData[j] = k;
		indexData[j + 1] = k + 1;
		indexData[j + 2] = k + 2;
		indexData[j + 3] = k + 2;
		indexData[j + 4] = k + 3;
		indexData[j + 5] = k;
	}
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	float windVertices[256 * FLOATS_PER_VERTEX << 2];
	int vertexSize = FLOATS_PER_VERTEX << 2;
	srand(1280659017);
	float u0 = 0.146484375f;
	float v0 = 0.017578125f;
	float u1 = 0.169921875f;
	float v1 = 0.041015625f;
	for (int i = 0; i < 64; i++)
	{
		int x = rand() % 720;
		int y = i * 20;
		int j = i * vertexSize << 2;
		for (int k = 0; k < 4; k++)
		{
			int xa = 0;
			int ya = 0;
			if (k & 1) xa = 1;
			if (k > 1) ya = 1;
			int xx = x - xa * 720;
			int yy = y - ya * 1280;
			windVertices[j] = xx;
			windVertices[j + 1] = yy;
			windVertices[j + 2] = u0;
			windVertices[j + 3] = v0;
			windVertices[j + 4] = xx;
			windVertices[j + 5] = yy + 12;
			windVertices[j + 6] = u0;
			windVertices[j + 7] = v1;
			windVertices[j + 8] = xx + 12;
			windVertices[j + 9] = yy + 12;
			windVertices[j + 10] = u1;
			windVertices[j + 11] = v1;
			windVertices[j + 12] = xx + 12;
			windVertices[j + 13] = yy;
			windVertices[j + 14] = u1;
			windVertices[j + 15] = v0;
			j += vertexSize;
		}
	}
	glGenBuffers(1, &windVbo);
	glBindBuffer(GL_ARRAY_BUFFER, windVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(windVertices), windVertices, GL_STATIC_DRAW);
	glGenBuffers(1, &windIbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, windIbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 1536 * sizeof(short), indexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, FLOATS_PER_VERTEX * sizeof(float), (void*) 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, FLOATS_PER_VERTEX * sizeof(float), (void*) (2 * sizeof(float)));
	for (int i = 0; i < 16; i++)
	{
		projectionMatrix[i] = 0.0f;
	}
	projectionMatrix[0] = 1.0f / 360.0f;
	projectionMatrix[5] = -1.0f / 640.0f;
	projectionMatrix[10] = -1.0f;
	projectionMatrix[12] = -1.0f;
	projectionMatrix[13] = 1.0f;
	projectionMatrix[14] = 0.0f;
	projectionMatrix[15] = 1.0f;
	resetModelview();
	projectionMatrixLocation[0] = glGetUniformLocation(*spriteShader, "u_pr");
	modelviewMatrixLocation[0] = glGetUniformLocation(*spriteShader, "u_mv");
	projectionMatrixLocation[1] = glGetUniformLocation(*colourShader, "u_pr");
	modelviewMatrixLocation[1] = glGetUniformLocation(*colourShader, "u_mv");
	scrollLocation = glGetUniformLocation(*skyShader, "u_scroll");
	colLocation0 = glGetUniformLocation(*spriteShader, "u_col");
	colLocation1 = glGetUniformLocation(*colourShader, "u_col");
	lastShader = NULL;
	lastTexture = NULL;
	spriteCount = 0;
}
void deleteRenderer()
{
	glDeleteBuffers(1, &windVbo);
	glDeleteBuffers(1, &windIbo);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ibo);
}
void translateRenderer(float x, float y)
{
	modelviewMatrix[12] += x;
	modelviewMatrix[13] += y;
}
void rotateRenderer(float xo, float yo, int r)
{
	char back = r < 0;
	if (back)
	{
		xo = -xo;
		yo = -yo;
		r = -r;
	}
	float rCos = icos(r);
	float rSin = isin(r);
	translateRenderer(xo - xo * rCos + yo * rSin, yo - yo * rCos - xo * rSin);
	if (back)
	{
		rCos = 1.0f;
		rSin = 0.0f;
	}
	modelviewMatrix[0] = rCos;
	modelviewMatrix[1] = rSin;
	modelviewMatrix[4] = -rSin;
	modelviewMatrix[5] = rCos;
}
void beginRender(GLuint* shader, Texture* texture)
{
	if (shader != lastShader)
	{
		glUseProgram(*shader);
		lastShader = shader;
	}
	if (texture != lastTexture)
	{
		glBindTexture(GL_TEXTURE_2D, texture->texture);
		lastTexture = texture;
	}
	spriteCount = 0;
}
void render(float x, float y, float uo, float vo, float w, float h)
{
	renderScale(x, y, uo, vo, w, h, 1.0f, 1.0f);
}
void renderScale(float x, float y, float uo, float vo, float w, float h, float xs, float ys)
{
	if (spriteCount == MAX_SPRITES)
	{
		endRender();
		beginRender(lastShader, lastTexture);
	}
	int uScale = lastTexture->width;
	int vScale = lastTexture->height;
	if (y > 1280.0f)
	{
		uScale = lastTexture->height;
		vScale = lastTexture->width;
	}
	float u0 = uo / uScale;
	float v0 = vo / vScale;
	float u1 = (uo + w) / uScale;
	float v1 = (vo + h) / vScale;
	w *= xs;
	h *= ys;
	if (y > 1280.0f) h = -h;
	int offs = spriteCount * FLOATS_PER_VERTEX << 2;
	vertexData[offs] = x;
	vertexData[offs + 1] = y;
	vertexData[offs + 2] = u0;
	vertexData[offs + 3] = v0;
	vertexData[offs + 4] = x;
	vertexData[offs + 5] = y + h;
	vertexData[offs + 6] = u0;
	vertexData[offs + 7] = v1;
	vertexData[offs + 8] = x + w;
	vertexData[offs + 9] = y + h;
	vertexData[offs + 10] = u1;
	vertexData[offs + 11] = v1;
	vertexData[offs + 12] = x + w;
	vertexData[offs + 13] = y;
	vertexData[offs + 14] = u1;
	vertexData[offs + 15] = v0;
	spriteCount++;
}
void renderRotate(float x, float y, float uo, float vo, float w, float h, int r, float xo, float yo)
{
	float rCos = icos(r);
	float rSin = isin(r);
	float xx0 = x - xo;
	float yy0 = y - yo;
	float xx1 = x + w - xo;
	float yy1 = y + h - yo;
	float x00 = xx0 * rCos - yy0 * rSin + xo;
	float y00 = yy0 * rCos + xx0 * rSin + yo;
	float x01 = xx0 * rCos - yy1 * rSin + xo;
	float y01 = yy1 * rCos + xx0 * rSin + yo;
	float x10 = xx1 * rCos - yy0 * rSin + xo;
	float y10 = yy0 * rCos + xx1 * rSin + yo;
	float x11 = xx1 * rCos - yy1 * rSin + xo;
	float y11 = yy1 * rCos + xx1 * rSin + yo;
	float u0 = uo / lastTexture->width;
	float v0 = vo / lastTexture->height;
	float u1 = (uo + w) / lastTexture->width;
	float v1 = (vo + h) / lastTexture->height;
	int offs = spriteCount * FLOATS_PER_VERTEX << 2;
	vertexData[offs] = x00;
	vertexData[offs + 1] = y00;
	vertexData[offs + 2] = u0;
	vertexData[offs + 3] = v0;
	vertexData[offs + 4] = x01;
	vertexData[offs + 5] = y01;
	vertexData[offs + 6] = u0;
	vertexData[offs + 7] = v1;
	vertexData[offs + 8] = x11;
	vertexData[offs + 9] = y11;
	vertexData[offs + 10] = u1;
	vertexData[offs + 11] = v1;
	vertexData[offs + 12] = x10;
	vertexData[offs + 13] = y10;
	vertexData[offs + 14] = u1;
	vertexData[offs + 15] = v0;
	spriteCount++;
}
void renderText(const char* msg, float x, float y)
{
	renderTextScale(msg, x, y, 1.0f);
}
void renderTextScale(const char* msg, float x, float y, float scale)
{
	if (lineHeight == 0) return;
	int xp = (int) x;
	char alignRight = xp < 0;
	if (alignRight) xp = -xp;
	int yp = (int) y;
	size_t len = strlen(msg) - 1;
	int i = alignRight ? len : 0;
	int xpo = xp;
	while (alignRight ? (i >= 0) : (i <= len))
	{
		unsigned char ch = (unsigned char) msg[i];
		if (ch == '\n')
		{
			xp = xpo;
			yp = (int) (yp + lineHeight * scale);
			if (alignRight) i--;
			else i++;
			continue;
		}
		GlyphInfo* gi = &glyphs[ch];
		renderScale(xp + (alignRight ? -gi->xo - gi->w : gi->xo) * scale, yp + gi->yo * scale, gi->x, gi->y, gi->w, gi->h, scale, scale);
		int xa = gi->xa * scale;
		if (i < len) xa += gi->kernings[(unsigned char) msg[i + 1]] * scale;
		if (alignRight)
		{
			xp -= xa;
			i--;
		}
		else
		{
			xp += xa;
			i++;
		}
	}
}
void endRender()
{
	if (spriteCount == 0) return;
	int i = lastShader == colourShader ? 1 : 0;
	glUniformMatrix4fv(projectionMatrixLocation[i], 1, GL_FALSE, projectionMatrix);
	glUniformMatrix4fv(modelviewMatrixLocation[i], 1, GL_FALSE, modelviewMatrix);
	endRenderRender();
}
void endRenderRender()
{
	glBufferSubData(GL_ARRAY_BUFFER, 0, spriteCount * FLOATS_PER_VERTEX * sizeof(float) << 2, vertexData);
	glDrawElements(GL_TRIANGLES, spriteCount * 6, GL_UNSIGNED_SHORT, NULL);
}
void renderWind(int xo, int yo, float a, char rain)
{
	glBindBuffer(GL_ARRAY_BUFFER, windVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, windIbo);
	lastShader = NULL;
	if (rain) beginRender(spriteShader, hudTexture);
	else beginRender(spriteShader, charsTexture);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, FLOATS_PER_VERTEX * sizeof(float), (void*) 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, FLOATS_PER_VERTEX * sizeof(float), (void*) (2 * sizeof(float)));
	yo = -yo;
	xo %= 720;
	yo %= 1280;
	if (xo < 0) xo += 720;
	if (yo < 0) yo += 1280;
	translateRenderer(xo, yo);
	glUniform4f(colLocation0, 1.0f, 1.0f, 1.0f, a);
	glUniformMatrix4fv(projectionMatrixLocation[0], 1, GL_FALSE, projectionMatrix);
	glUniformMatrix4fv(modelviewMatrixLocation[0], 1, GL_FALSE, modelviewMatrix);
	glDrawElements(GL_TRIANGLES, 1536, GL_UNSIGNED_SHORT, NULL);
	translateRenderer(-xo, -yo);
	lastShader = NULL;
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, FLOATS_PER_VERTEX * sizeof(float), (void*) 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, FLOATS_PER_VERTEX * sizeof(float), (void*) (2 * sizeof(float)));
	glUniform4f(colLocation0, 1.0f, 1.0f, 1.0f, 1.0f);
}
void renderWindow(int xp, int yp, int w, int h)
{
	w--;
	h--;
	for (int y = 0; y <= h; y++)
	{
		for (int x = 0; x <= w; x++)
		{
			float uo = 32.0f;
			float vo = 32.0f;
			if (x == 0) uo = 0.0f;
			else if (x == w) uo = 64.0f;
			if (y == 0) vo = 0.0f;
			else if (y == h) vo = 64.0f;
			render(xp + (x << 5), yp + (y << 5), uo, vo, 32.0f, 32.0f);
		}
	}
}
