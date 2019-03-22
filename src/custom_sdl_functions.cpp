#include "custom_sdl_functions.h"

//NON DEFINED FUCTIONS

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

}
