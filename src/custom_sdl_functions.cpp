#include "custom_sdl_functions.h"

//NON DEFINED FUCTIONS

enum SearchDirection
{
	UP,
	DOWN,
	RIGHT,
	LEFT
};

Uint32 get_pixel32( SDL_Surface *surface, int x, int y )
{
    Uint32 *pixels = (Uint32*)surface->pixels;
    return pixels[(y * surface->w)+x];
}

void put_pixel32( SDL_Surface *surface, int x, int y, Uint32 pixel )
{
    Uint32 *pixels = (Uint32*)surface->pixels;
    pixels[(y * surface->w)+x] = pixel;
}

int calculateSpinValueForPixel(int x, int y, int surface_w, int surface_h)
{
	int w = surface_w;
	int h = surface_h;
	int val = 0;
	val += (x < w/2) ? x : w-x - 1;
	val += (y < h/2) ? y : h-y - 1;
	return val;
}

void findSameNextPlace(int* x, int* y, int w, int h, int val, int* tiles, SearchDirection direction, bool rightOrUp = true)
{
	int lx = *x;
	int ly = *y;
	if (direction == UP)
	{
		int modx = (rightOrUp ? -1 : 1) * (ly < h/2 ? 1 : -1);
		if (ly > 0)
		{
			int upval = tiles[(ly - 1) * lx];
			if (upval == val)
			{
				--*y;
				return;
			}
			if ((lx + modx >= 0 && lx + modx < w) && tiles[(ly - 1) * (lx + modx)] == val)
			{
				--*y;
				x += modx;
				return;
			}
			if (upval > val)
				*x += modx;
			else
				--*y;
			return;
		}
		*x += modx;
	}
	else if (direction == DOWN)
	{
		int modx = (rightOrUp ? 1 : -1) * (ly < h/2 ? 1 : -1);
		if (ly < h - 1)
		{
			int dval = tiles[(ly + 1) * lx];
			if (dval == val)
			{
				++*y;
				return;
			}
			if ((lx + modx >= 0 && lx + modx < w) && tiles[(ly + 1) * (lx + modx)] == val)
			{
				++*y;
				x += modx;
				return;
			}
			if (dval > val)
				*x += modx;
			else
				++*y;
			return;
		}
		*x += modx;
	}
}

void findNewPosForSpinningPixel(int* x, int* y, int surface_w, int surface_h, int steps, int* tiles, bool right)
{
	int w = surface_w;
	int h = surface_h;
	int val = tiles[*x * *y];
	int stepsForPixel = steps - val * 2;
	for (int i = 0; i < stepsForPixel; i ++)
	{
		// (x < w/2) left Sector, (x >= (double)w/2) right Sector, (y < h/2) top Sector, (y >= (double)w/2) bottom Sector
		if (*x < w/2)
		{
			if (right)
				findSameNextPlace(x, y, w, h, val, tiles, UP, false);
			else
				findSameNextPlace(x, y, w, h, val, tiles, DOWN, false);
			return;
		}
		else if (*x >= (double)w/2)
		{
			if (right)
				findSameNextPlace(x, y, w, h, val, tiles, DOWN);
			else
				findSameNextPlace(x, y, w, h, val, tiles, UP);
			return;
		}
		if (*y < h/2)
		{
			if (right)
				++*x;
			else
				--*x;
		}
		else if (*y >= (double)w/2)
		{
			if (right)
				--*x;
			else
				++*x;
		}
		return;
	}
}

//DEFINED FUNCTIONS

SDL_Surface* load_image(std::string filename, bool colorKey)
{
    SDL_Surface* loadedImage = SDL_LoadBMP(filename.c_str());
    if(loadedImage != nullptr)
	{
		if (colorKey)
		{
			Uint32 colorkey = SDL_MapRGB( loadedImage->format, 0, 0xFF, 0xFF );
			SDL_SetColorKey( loadedImage, SDL_TRUE, colorkey );
		}
	}
    else
	{
		std::string error = "Error loading image with SDL: ";
		const char* sdlError = SDL_GetError();
		for (unsigned int i = 0; i < SDL_strlen(sdlError); i ++)
			error += SDL_GetError()[i];
		throw error;
	}
    return loadedImage;
}

SDL_Surface* createTransparentSurface(unsigned int w, unsigned int h)
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
return SDL_CreateRGBSurface(SDL_HWSURFACE, w, h, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
#else
return SDL_CreateRGBSurface(0,w,h,32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
#endif
}

SDL_Color createColor(int r, int g, int b, int a)
{
    SDL_Color C;
    C.r = r;
    C.b = b;
    C.g = g;
    C.a = a;
    return C;
}

void apply_surface(int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip)
{
    SDL_Rect offset;
    offset.x = x;
    offset.y = y;
    SDL_BlitSurface( source, clip, destination, &offset );
}

void set_color(SDL_Surface *surface, SDL_Color source, SDL_Color targetColor)
{
	SDL_Color* colors = surface->format->palette->colors;
	for (int i = 0; i < 256; i++)
		if (colors[i].r == source.r && colors[i].g == source.g && colors[i].b == source.b && colors[i].a > 127)
			colors[i] = targetColor;
	SDL_SetPaletteColors(surface->format->palette, colors, 0 , 256);
}

void shift_pixels_vertical(SDL_Surface* target, bool up, SDL_Rect* clip)
{
	int startx, endx, starty, endy;
	if (clip != nullptr)
	{
		startx = clip->x;
		starty = clip->y;
		endx = startx + clip->w;
		endy = starty + clip->h;
	}
	else
	{
		startx = 0;
		starty = 0;
		endx = target->w;
		endy = target->h;
	}
	if (SDL_MUSTLOCK(target))
		SDL_LockSurface(target);
	Uint32 transparent = 0;
	if (up)
	{
		for(int x = startx; x < endx; x++)
		{
			for(int y = starty + 1; y < endy; y++)
			{
				Uint32 pixel = get_pixel32(target, x, y);
				put_pixel32(target, x, y - 1, pixel);
			}
		}
		for(int x = startx; x < endx; x++)
		{
			put_pixel32(target, x, endy - 1, transparent);
		}
	}
	else
	{
		for(int x = startx; x < endx; x++)
		{
			for(int y = endy - 2; y >= starty; y--)
			{
				Uint32 pixel = get_pixel32(target, x, y);
				put_pixel32(target, x, y + 1, pixel);
			}
		}
		for(int x = startx; x < endx; x++)
		{
			put_pixel32(target, x, starty, transparent);
		}
	}
	SDL_UnlockSurface(target);
}

void shift_pixels_horizontal(SDL_Surface* target, bool right, SDL_Rect* clip)
{
	int startx, endx, starty, endy;
	if (clip != nullptr)
	{
		startx = clip->x;
		starty = clip->y;
		endx = startx + clip->w;
		endy = starty + clip->h;
	}
	else
	{
		startx = 0;
		starty = 0;
		endx = target->w;
		endy = target->h;
	}
	if (SDL_MUSTLOCK(target))
		SDL_LockSurface(target);
	Uint32 transparent = 0;
	if (right)
	{
		for(int x = startx; x < endx - 1; x++)
		{
			for(int y = starty; y < endy; y++)
			{
				Uint32 pixel = get_pixel32(target, x, y);
				put_pixel32(target, x, x+1, pixel);
			}
		}
		for(int y = starty; y < endy; y++)
		{
			put_pixel32(target, startx, y, transparent);
		}
	}
	else
	{
		for(int x = endx - 1; x >= startx; x--)
		{
			for(int y = starty; y < endy; y ++)
			{
				Uint32 pixel = get_pixel32(target, x, y);
				put_pixel32(target, x - 1, y, pixel);
			}
		}
		for(int y = starty; y < endy; y++)
		{
			put_pixel32(target, endx - 1, starty, transparent);
		}
	}
	SDL_UnlockSurface(target);
}

void spin_surface(SDL_Surface* target, unsigned int degrees, SDL_Rect* clip)
{
	if (degrees > 360)
		degrees = 360;
	bool right = degrees > 180;
	if (right)
		degrees = 360 - degrees;
	int w = target->w; int h = target->h;
	//How many steps the fastest moving pixels of the image will need to spin 180 degrees
	int steps = w + h - 2;
	double perc = (double)degrees / 180;
	steps = perc * steps;
	Uint32* pixelsOld = (Uint32*)target->pixels;
	Uint32 pixelsNew[w * h];
	int tiles[w* h];
	for (int x = 0; x < w; x++)
		for (int y = 0; y < h; y++)
			tiles[x * y] = calculateSpinValueForPixel(x, y, w, h);
	for (int x = 0; x < w; x++)
	{
		for (int y = 0; y < h; y++)
		{
			Uint32 oldPixel = pixelsOld[y * x];
			int newXpos = x;
			int newYpos = y;
			findNewPosForSpinningPixel(&newXpos, &newYpos, w, h, steps, tiles, right);
			pixelsNew[newXpos * newYpos] = oldPixel;
		}
	}
	target->pixels = pixelsNew;
	delete pixelsOld;
}
