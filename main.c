#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#define SCREEN_WIDTH		(320)
#define SCREEN_HEIGHT		(240)
#define SCREEN_BPP			(32)
#define LIGHT_SIZE			(128)
#define LIGHT_NUM			(3)

#define blue(c)				((c) & 0xff)
#define green(c)			(((c) >> 8) & 0xff)
#define red(c)				(((c) >> 16) & 0xff)

SDL_Surface *screen = NULL;
SDL_Surface *light = NULL;
SDL_Surface *heightmap = NULL;

Uint32 getPixel(SDL_Surface *s, int x, int y)
{
	if (x < 0 || x >= s->w || y < 0 || y >= s->h)
		return 0;
	int i = y * s->w + x;
	Uint32 *p = (Uint32*)s->pixels;
	return p[i];
}

void setPixel(SDL_Surface *s, int x, int y, Uint32 c)
{
	if (x < 0 || x >= s->w || y < 0 || y >= s->h)
		return;
	int i = y * s->w + x;
	Uint32 *p = (Uint32*)s->pixels;
	p[i] = c;
}

void setComponent(SDL_Surface *s, int x, int y, Uint8 c, int subpixel)
{
	if (x < 0 || x >= s->w || y < 0 || y >= s->h)
		return;
	int i = 4 * (y * s->w + x);
	Uint8 *p = (Uint8*)s->pixels;
	p[i + subpixel] = c;
}

Uint32 mix(Uint32 pixel, Uint32 mask)
{
	Uint32 val;
	Uint8 *p = (Uint8*)&pixel;
	Uint8 *m = (Uint8*)&mask;
	Uint8 *v = (Uint8*)&val;
	v[0] = p[0] * m[0] / 255.0;			// blue
	v[1] = p[1] * m[1] / 255.0;			// green
	v[2] = p[2] * m[2] / 255.0;			// red
	v[3] = p[3] * m[3] / 255.0;			// alpha (if present)
	return val;
}

int main(int argc, char *argv[])
{
	srand(time(NULL));
	SDL_Init(SDL_INIT_VIDEO);
	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_HWSURFACE | SDL_DOUBLEBUF);
	SDL_EnableKeyRepeat(100, 50);

	// memory leaks, I know ;)
	heightmap = IMG_Load("heightmap.png");
	heightmap = SDL_DisplayFormat(heightmap);
	light = IMG_Load("light.png");
	light = SDL_DisplayFormat(light);
	double px[LIGHT_NUM] = {0, 0, 0};
	double py[LIGHT_NUM] = {0, 0, 0};
	double vx[LIGHT_NUM] = {40.0, 40.0, 40.0};
	double vy[LIGHT_NUM] = {60.0, 60.0, 60.0};
	int normalx[SCREEN_WIDTH * SCREEN_HEIGHT];
	int normaly[SCREEN_WIDTH * SCREEN_HEIGHT];
	for (int y = 1; y < SCREEN_HEIGHT; ++y)
		for (int x = 1; x < SCREEN_WIDTH; ++x)
		{
			Uint8 *hp = (Uint8*)heightmap->pixels;
			int i = y * heightmap->w + x;
			int b = hp[4 * i];
			int g = hp[4 * i + 1];
			int r = hp[4 * i + 2];
			int v = (r + g + b) / 3;
			int ix = y * heightmap->w + x - 1;
			int bx = hp[4 * ix];
			int gx = hp[4 * ix + 1];
			int rx = hp[4 * ix + 2];
			int vx = (rx + gx + bx) / 3;
			int iy = (y - 1) * heightmap->w + x;
			int by = hp[4 * iy];
			int gy = hp[4 * iy + 1];
			int ry = hp[4 * iy + 2];
			int vy = (ry + gy + by) / 3;
			normalx[i] = (v - vx) / 2;
			normaly[i] = (v - vy) / 2;
		}

	bool quit = false;
	Uint32 curr = SDL_GetTicks();
	Uint32 prev = curr;
	while (!quit)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
						/*
						case SDLK_UP:
							py -= 3;
							if (py < -LIGHT_SIZE / 2) py = -LIGHT_SIZE / 2;
							break;
						case SDLK_DOWN:
							py += 3;
							if (py >= SCREEN_HEIGHT - LIGHT_SIZE / 2) py = SCREEN_HEIGHT - LIGHT_SIZE / 2 - 1;
							break;
						case SDLK_LEFT:
							px -= 3;
							if (px < -LIGHT_SIZE / 2) px = -LIGHT_SIZE / 2;
							break;
						case SDLK_RIGHT:
							px += 3;
							if (px >= SCREEN_WIDTH - LIGHT_SIZE / 2) px = SCREEN_WIDTH - LIGHT_SIZE / 2 - 1;
							break;
						*/
						case SDLK_ESCAPE:
							quit = true;
							break;
					}
					break;
				case SDL_QUIT:
					quit = true;
					break;
			}
		}
		curr = SDL_GetTicks();
		Uint32 delta = curr - prev;
		prev = curr;

		for (int i = 0; i < LIGHT_NUM; ++i)
		{
			px[i] += vx[i] * (delta / 1000.0);
			py[i] += vy[i] * (delta / 1000.0);
			if (px[i] < -LIGHT_SIZE / 2)
			{
				vx[i] = 20.0 + rand() % 40;
			}
			if (px[i] > SCREEN_WIDTH - LIGHT_SIZE / 2)
			{
				vx[i] = -(20.0 + rand() % 40);
			}
			if (py[i] < -LIGHT_SIZE / 2)
			{
				vy[i] = 30.0 + rand() % 60;
			}
			if (py[i] > SCREEN_HEIGHT - LIGHT_SIZE / 2)
			{
				vy[i] = -(30.0 + rand() % 60);
			}
		}

		SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
		//SDL_BlitSurface(heightmap, NULL, screen, NULL);
		SDL_LockSurface(screen);
		for (int y = 0; y < light->h; ++y)
			for (int x = 0; x < light->w; ++x)
			{
				/*
				Uint32 lp = getPixel(light, x, y);
				Uint32 hp = getPixel(heightmap, x + px, y + py);
				Uint32 c = mix(hp, lp);
				setPixel(screen, x + px, y + py, c);
				*/
				for (int i = 0; i < LIGHT_NUM; ++i)
				{
					int yy = y + py[i];
					int xx = x + px[i];
					int nx;
					int ny;
					if (xx < 1 || xx >= SCREEN_WIDTH || yy < 1 || yy >= SCREEN_HEIGHT)
					{
						nx = 0;
						ny = 0;
					}
					else
					{
						int ind = yy * screen->w + xx;
						nx = normalx[ind];
						ny = normaly[ind];
					}
					Uint32 c = getPixel(light, x - nx, y - ny);
					setComponent(screen, x + px[i], y + py[i], c & 0xff, i);
				}
			}
		SDL_UnlockSurface(screen);
		SDL_Flip(screen);
	}
	SDL_Quit();
	return 0;
}
