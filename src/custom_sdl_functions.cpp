#include "custom_sdl_functions.h"
#include <cmath>

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

template<class T>
void array_flip(T* array, int w)
{
	T inverse[w];
	for (int i = 0; i < w; i++)
		inverse[w - 1 - i] = array[i];
	for (int i = 0; i < w; i++)
		array[i] = inverse[i];
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

void findSameNextPlace(int* x, int* y, int w, int h, int val, int* tiles, SearchDirection direction, SDL_Rect* clip, bool right = true)
{
	int lx = *x;
	int ly = *y;
	if (clip != nullptr)
	{
		lx -= clip->x;
		ly -= clip->y;
	}
	if (direction == UP)
	{
		int modx = (right ? -1 : 1) * (ly < h/2 ? 1 : -1);
		if (ly > 0)
		{
			int upval = tiles[((ly - 1) * w) + lx];
			if (upval == val)
			{
				--*y;
				return;
			}
			if (lx + modx >= 0 && lx + modx < w)
			{
				if (tiles[((ly - 1) * w) + lx + modx] == val)
				{
					--*y;
					*x += modx;
					return;
				}
				if (upval < val || tiles[(ly * w) + lx + modx] < upval)
				{
					*x += modx;
					return;
				}
			}
			--*y;
			return;
		}
		*x += modx;
	}
	else if (direction == DOWN)
	{
		int modx = (right ? 1 : -1) * (ly < h/2 ? 1 : -1);
		if (ly < h - 1)
		{
			int dval = tiles[((ly + 1) * w) + lx];
			if (dval == val)
			{
				++*y;
				return;
			}
			if (lx + modx >= 0 && lx + modx < w)
			{
				if(tiles[((ly + 1) * w) + lx + modx] == val)
				{
					++*y;
					*x += modx;
					return;
				}
				if (dval < val || tiles[(ly * w) + lx + modx] < dval)
				{
					*x += modx;
					return;
				}
			}
			++*y;
			return;
		}
		*x += modx;
	}
}

void findNewPosForSpinningPixel(int* x, int* y, int surface_w, int surface_h, int steps, int* tiles, bool right, SDL_Rect* clip)
{
	if (steps <= 0)
		return;
	int cw = surface_w;
	int ch = surface_h;
	int cx = *x;
	int cy = *y;
	if (clip != nullptr)
	{
		cw = clip->w;
		ch = clip->h;
		cx -= clip->x;
		cy -= clip->y;
	}
	int val = tiles[(cy * cw) + cx];
	int stepsForPixel = steps - val * 2;
	for (int i = 0; i < stepsForPixel; i ++)
	{
		cx = *x;
		cy = *y;
		if (clip != nullptr)
		{
			cx -= clip->x;
			cy -= clip->y;
		}
		// (x < w/2) left Sector, (x >= (double)w/2) right Sector, (y < h/2) top Sector, (y >= (double)h/2) bottom Sector
		if (cx < cw/2)
		{
			if (right)
				findSameNextPlace(x, y, cw, ch, val, tiles, UP, clip, false);
			else
				findSameNextPlace(x, y, cw, ch, val, tiles, DOWN, clip, false);
			continue;
		}
		else if (cx >= (double)cw/2)
		{
			if (right)
				findSameNextPlace(x, y, cw, ch, val, tiles, DOWN, clip);
			else
				findSameNextPlace(x, y, cw, ch, val, tiles, UP, clip);
			continue;
		}
		if (cy < ch/2)
		{
			int xmod = right ? 1 : -1;
			if (tiles[(cy * cw) + cx + xmod] > val)
				*x += xmod;
			else if (tiles[((cy + 1) * cw) + cx + xmod] == val)
			{
				*x += xmod;
				++*y;
			}
			else
				++*y;
		}
		else if (cy >= (double)ch/2)
		{
			int xmod = right ? -1 : 1;
			if (tiles[(cy * cw) + cx + xmod] > val)
				*x += xmod;
			else if (tiles[((cy - 1) * cw) + cx + xmod] == val)
			{
				*x += xmod;
				--*y;
			}
			else
				--*y;
		}
		else
			return;
	}
}

bool dimensionCheck(int w, int h, Uint32* pixelsOld, SDL_Rect* clip, SDL_Surface* target)
{
	if (w == 1)
	{
		if (clip == nullptr)
			array_flip<Uint32>(pixelsOld, h);
		else
		{
			Uint32 inverse[h];
			for (int i = 0; i < h; i++)
				inverse[i] = pixelsOld[((h - i - 1) * target->w) + clip->x];
			for (int i = 0; i < h; i ++)
				pixelsOld[(i * target->w) + clip->x] = inverse[i];
		}
		return true;
	}
	else if (h == 1)
	{
		if (clip == nullptr)
			array_flip<Uint32>(pixelsOld, h);
		else
		{
			Uint32 inverse[w];
			for (int i = 0; i < w; i++)
				inverse[i] = pixelsOld[(clip->y * target->w) + clip->x + i];
			for (int i = 0; i < w; i ++)
				pixelsOld[(clip->y * target->w) + clip->x + i] = inverse[i];
		}
		return true;
	}
	return false;
}

template<class T>
std::vector<T> vector_flip(std::vector<T> target)
{
	std::vector<T> result = target;
	for (unsigned int i = 0; i < target.size(); i++)
		result[target.size() - 1 - i] = target[i];
	return result;
}

template <class T>
std::vector<T> mergeVectors(std::vector<T> first, std::vector<T> second)
{
    std::vector<T> result = first;
    for (unsigned int i = 0; i < second.size(); i++)
        result.push_back(second[i]);
    return result;
}

std::vector<int> syncVectors(std::vector<int> first, std::vector<int> second)
{
    if (first[0] == second[0])
    {
        first.erase(first.begin());
        second.pop_back();
    }
    second = vector_flip(second);
    return mergeVectors(first, second);
}

int calculateSpins(int tileSetSize, int largestSetSize, int spins)
{
    int result = 0;
    if (tileSetSize == largestSetSize)
        return spins;
    double diff = largestSetSize / double(largestSetSize - tileSetSize);
    double firstTurn = diff / 2.0;
    int skippedTurns = 0;
    for (int i = 0; i < spins; i++)
    {
        skippedTurns = round(firstTurn);
        if (skippedTurns == i + 1)
            firstTurn += diff;
        else
            result ++;
    }
    return result;
}

void spinTileSet(SDL_Surface* target, std::vector<int> tileSet, bool right, int spins)
{
    if (spins == 0)
        return;
    Uint32 *pixels = (Uint32*)target->pixels;
    Uint32 *pixelsNew = new Uint32[target->h * target->w];
    for (int i = 0; i < target->h * target->w; i++)
        pixelsNew[i] = pixels[i];
    for (int i = 0; i < spins; i++)
    {
        int newPosition = 0;
        int shifts = 0;
        Uint32 old = pixelsNew[tileSet[0]];
        while ((unsigned)shifts < tileSet.size())
        {
            Uint32 next = old;
            if (right)
            {
                newPosition ++;
                if ((unsigned)newPosition >= tileSet.size())
                    newPosition -= tileSet.size();
            }
            else
            {
                newPosition --;
                if (newPosition < 0)
                    newPosition += tileSet.size();
            }
            old = pixelsNew[tileSet[newPosition]];
            pixelsNew[tileSet[newPosition]] = next;
            shifts ++;
        }
    }
    for (int i = 0; i < target->h * target->w; i++)
        pixels[i] = pixelsNew[i];
    delete[] pixelsNew;
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

SDL_Surface* copy_surface(SDL_Surface* target, SDL_Rect* clip)
{
	int w = clip == nullptr ? target->w : clip->w;
	int h = clip == nullptr ? target->h : clip->h;
	SDL_Surface* result = createTransparentSurface(w, h);
	apply_surface(0, 0, target, result, clip);
	return result;
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

std::vector<std::vector<int>> getTileSetsOfSurface(int w, int h, int* numSpins)
{
    std::vector<std::vector<int>> result;
    short hor = w;
    short v = h;
    for (int i = 0; i <= ceil(hor / 2.0); i++)
    {
        std::vector<int> lefts;
        std::vector<int> rights;
        for (int j = 0; j < v; j ++)
        {
            if ((j < i || j >= v - i) && i != 0)
            {
                int mod = 0;
                if (j + 1 > v / 2)
                    mod = v - 1 - j;
                else
                    mod = j;
                lefts.push_back(j * hor - 1 + i - mod);
                rights.push_back(j * hor + hor - i + mod);
            }
            else if (j <= ceil(v / 2.0) - 1 && i == 0 && j > 0)
            {
                lefts.clear();
                rights.clear();
                for (int j2 = j; j2 < v; j2++)
                {
                    int mod = 0;
                    if (j2 + 1 > v / 2)
                        mod = v - 1 - j2 - j;
                    else
                        mod = j2 - j;
                    if (mod >= 0 && mod < ceil(hor / 2.0))
                    {
                        if ( (j2 + 1 <= v / 2 && j2 < j + ceil(hor / 2.0)) || (j2 + 1 > v / 2 && v - 1 - j2 < j + ceil(hor/2.0)) )
                            lefts.push_back(j2 * hor - 1 + ceil(hor / 2.0) - mod);
                            rights.push_back(j2 * hor + hor - ceil(hor / 2.0) + mod);
                    }
                }
                result.push_back(syncVectors(lefts, rights));
            }
        }
        if (i != 0)
            result.push_back(syncVectors(lefts, rights));
    }
    if (w > h)
        *numSpins = 2 * h;
    else
        *numSpins = 2 * w;
    return result;
}

void apply_surface(int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip)
{
    SDL_Rect offset;
    offset.x = x;
    offset.y = y;
    SDL_BlitSurface( source, clip, destination, &offset );
}

void erase_surface(SDL_Surface* target, SDL_Rect* clip)
{
	int x(0), y(0), w(target->w), h(target->h);
	if (clip == nullptr)
	{
		for (int i = 0; i < w * h; i++)
			static_cast<Uint32*>(target->pixels)[i] = 0;
		return;
	}
	x = clip->x;
	y = clip->y;
	w = x + clip->w;
	h = y + clip->h;
	for (int i = x; i < w; i++)
		for (int j = y; j < h; j++)
			static_cast<Uint32*>(target->pixels)[(j * target->w) + i] = 0;
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
	if (target->pitch > 150)
		throw "Error: Due to sdl_functionality such as pixel pitch which creates skewed representations of rows and columns in a surface's pixel array this function should not be used for large images.";
	int w(0), h(0), yInTarget(0), xInTarget(0), steps(0);
	if (degrees > 360)
		degrees = 360;
	bool right = degrees > 180;
	if (right)
		degrees = 360 - degrees;
	if (clip == nullptr)
	{
		w = target->w;
		h = target->h;
		steps = w + h - 2;
	}
	else
	{
		xInTarget = clip->x;
		yInTarget = clip->y;
		w = xInTarget + clip->w;
		h = yInTarget + clip->h;
		steps = clip->w + clip->h - 2;
	}
	//How many steps the fastest moving pixels of the image will need to spin 180 degrees
	double perc = (double)degrees / 180;
	steps = perc * steps;
	if (SDL_MUSTLOCK(target))
		SDL_LockSurface(target);
	Uint32* pixelsOld = (Uint32*)target->pixels;
	Uint32* pixelsNew = new Uint32[target->h * target->w];
	int actualw = clip == nullptr ? w : clip->w;
	int actualh = clip == nullptr ? h : clip->h;
	for (int i = 0; i < target->w * target->h; i++)
		pixelsNew[i] = pixelsOld[i];
	if (dimensionCheck(actualw, actualh, pixelsOld, clip, target))
	{
		delete[] pixelsNew;
		SDL_UnlockSurface(target);
		return;
	}
	int* tiles = new int[actualw * actualh];
	for (int x = 0; x < actualw; x++)
		for (int y = 0; y < actualh; y++)
			tiles[(y * actualw) + x] = calculateSpinValueForPixel(x, y, actualw, actualh);
	for (int x = xInTarget; x < w; x++)
	{
		for (int y = yInTarget; y < h; y++)
		{
			Uint32 oldPixel = pixelsOld[(y * target->w) + x];
			int newXpos = x;
			int newYpos = y;
			findNewPosForSpinningPixel(&newXpos, &newYpos, target->w, target->h, steps, tiles, right, clip);
			pixelsNew[(newYpos * target->w) + newXpos] = oldPixel;
		}
	}
	for (int x = 0; x < target->w; x++)
		for (int y = 0; y < target->h; y++)
			put_pixel32(target, x, y, pixelsNew[(y * target->w) + x]);
	delete[] tiles;
	delete[] pixelsNew;
	SDL_UnlockSurface(target);
}

void spin_surface_safe(SDL_Surface* target, SDL_Surface* glyph, int* previousSpins, unsigned int turns, int numspins, std::vector<std::vector<int>> tileSets, bool right, SDL_Rect* clip)
{
    int xInTarget(0), yInTarget(0);
	if (SDL_MUSTLOCK(target))
		SDL_LockSurface(target);
	if (dimensionCheck(target->w, target->h, (Uint32*)target->pixels, clip, target))
		return;
    if (clip != nullptr)
	{
		xInTarget = clip->x;
		yInTarget = clip->y;
	}
	if (*previousSpins + turns >= (unsigned)numspins)
    {
        turns -= numspins - *previousSpins;
        *previousSpins = turns;
    }
    else
        *previousSpins += turns;
	//spinTileSets
	SDL_Surface* spun = copy_surface(glyph, nullptr);
	for (unsigned int i = 0; i < tileSets.size(); i++)
    {
        int spins = calculateSpins(tileSets[i].size(), numspins, *previousSpins);
        spinTileSet(spun, tileSets[i], right, spins);
    }
    apply_surface(xInTarget, yInTarget, spun, target, nullptr);
    SDL_FreeSurface(spun);
    SDL_UnlockSurface(target);
}
