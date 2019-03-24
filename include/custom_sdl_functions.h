#ifndef CUSTOM_SDL_FUNCTIONS_H
#define CUSTOM_SDL_FUNCTIONS_H
#include <SDL2/SDL.h>
#include <iostream>

/**
	Loads the image to an SDL_Surface* and makes al pixels of color (0, 255, 255) transparent if colorKey == true
*/
SDL_Surface* load_image(std::string filename, bool colorKey = true);
SDL_Surface* createTransparentSurface(unsigned int w, unsigned int h);
SDL_Color createColor(int r, int g, int b, int a);
/**
	Blits source surface to target specified position of target surface. If clip != NULL only a clipped part of source surface will be blitted.
*/
void apply_surface(int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip = nullptr);
/**
	Sets all pixels of source color to target color for a given SDL_Surface*
*/
void set_color(SDL_Surface *surface, SDL_Color source, SDL_Color targetColor);
/**
	Moves all pixels of surface up or down one row, and fills the bottom or top row that becomes free with transparent ones.
	Set clip to non null if you wish to only apply this effect within a clipped part of surface.
*/
void shift_pixels_vertical(SDL_Surface* target, bool up, SDL_Rect* clip = nullptr);
/**
	Moves all pixels of surface 1 pixel left or right, and fills the most left or right column that becomes free with transparent ones.
	Set clip to non null if you wish to only apply this effect within a clipped part of surface.
*/
void shift_pixels_horizontal(SDL_Surface* target, bool right, SDL_Rect* clip = nullptr);
void spin_surface(SDL_Surface* target, unsigned int degrees, SDL_Rect* clip = nullptr);
/**
this will spin a surface such
that no two pixels will end up in the same spot. Therefore you can no longer specify the degrees you wish to spin, but have to spin it
rather according to how big it is. Large images will require many spins to see effect take place, smaller ones will need less.
You'll also have to specify how many times it's been spun before.
*/
void spin_surface_safe(SDL_Surface* target, unsigned int previousSpins, SDL_Rect* clip = nullptr);

#endif // CUSTOM_SDL_FUNCTIONS_H
