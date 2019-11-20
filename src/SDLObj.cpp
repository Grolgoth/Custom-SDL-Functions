#include "SDLObj.h"
#include <sdl_ttf_custom.h>
#include <SDL_mixer.h>

SDL::SDL(int winW, int winH, Uint32 initFlags)
{
	WINW = winW;
	WINH = winH;
	if(!init(initFlags))
		std::cout << "Error initializing SDL: " << SDL_GetError() << std::endl;
	else
		healthy = true;
	withMixer = initFlags && SDL_INIT_AUDIO;
}

SDL::~SDL()
{
	SDL_DestroyWindow(m_window);
    SDL_DestroyRenderer(m_renderer);
    if (withMixer)
		Mix_Quit();
    TTF_Quit();
    SDL_Quit();
}

void SDL::render()
{
    SDL_RenderPresent(m_renderer);
}

bool SDL::init(Uint32 flags)
{
	if(SDL_Init(flags))
		return false;
	if(TTF_Init())
		return false;
	m_window = SDL_CreateWindow("Fools' Crypt", 100, 100, WINW, WINH, SDL_WINDOW_SHOWN); //possibly use settings for fullscreen, resizable window etc
	if (m_window == nullptr)
		return false;
	m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_SOFTWARE);
	if (m_renderer == nullptr)
		return false;
	if (m_window == nullptr || m_renderer == nullptr)
		return false;
	if (flags | SDL_INIT_AUDIO)
		if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
			return false;
	return true;
}

void SDL::addToTarget(SDL_Texture* texture, int x, int y, int w, int h, SDL_Rect* clip)
{
	SDL_Rect clipobj;
	SDL_Rect* clipdest = nullptr;
	if (x != 0 || y != 0)
	{
		clipobj.x = x;
		clipobj.y = y;
		clipobj.w = w;
		clipobj.h = h;
		clipdest = &clipobj;
	}
	SDL_RenderCopy(m_renderer, texture, clip, clipdest);
}

void SDL::textToTarget(SDL_Surface* text, int x, int y)
{
	SDL_Texture* converted = SDL_CreateTextureFromSurface(m_renderer, text);
	SDL_Rect clip;
	clip.x = x;
	clip.y = y;
	SDL_QueryTexture(converted, nullptr, nullptr, &clip.w, &clip.h);
	if (clip.x + clip.w >= WINW)
		clip.w = WINW - clip.x;
	if (clip.y + clip.h >= WINH)
		clip.h = WINH - clip.y;
	SDL_RenderCopy(m_renderer, converted, nullptr, &clip);
	SDL_DestroyTexture(converted);
}
