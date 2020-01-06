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
	SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
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
	if (w != 0 || h != 0)
	{
		clipobj.x = x * sizeChangeFactorW;
		clipobj.y = y * sizeChangeFactorH;
		clipobj.w = w * sizeChangeFactorW;
		clipobj.h = h * sizeChangeFactorH;
		clipdest = &clipobj;
	}
	SDL_RenderCopy(m_renderer, texture, clip, clipdest);
}

void SDL::textToTarget(SDL_Surface* text, int x, int y)
{
	SDL_Texture* converted = SDL_CreateTextureFromSurface(m_renderer, text);
	SDL_Rect clip;
	clip.x = x * sizeChangeFactorW;
	clip.y = y * sizeChangeFactorH;
	SDL_QueryTexture(converted, nullptr, nullptr, &clip.w, &clip.h);
	clip.w *= sizeChangeFactorW;
	clip.h *= sizeChangeFactorH;
	if (clip.x + clip.w >= WINW)
		clip.w = WINW * sizeChangeFactorW - clip.x;
	if (clip.y + clip.h >= WINH)
		clip.h = WINH * sizeChangeFactorH - clip.y;
	SDL_RenderCopy(m_renderer, converted, nullptr, &clip);
	SDL_DestroyTexture(converted);
}

void SDL::fullscreen()
{
	isFullscreen = !isFullscreen;
	if (isFullscreen)
		SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN);
	else
		SDL_SetWindowFullscreen(m_window, 0);
}

void SDL::changeWindowSize(int winw, int winh)
{
	SDL_SetWindowSize(m_window, winw, winh);
	sizeChangeFactorW = double(winw) / double(WINW);
	sizeChangeFactorH = double(winh) / double(WINH);
}

void SDL::modColors(int a, int r, int g, int b, SDL_Rect* clip)
{
	SDL_Rect screenrect;
	screenrect.x = 0; screenrect.y = 0; screenrect.w = WINW; screenrect.h = WINH;
	SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
	if (clip != nullptr)
		SDL_RenderFillRect(m_renderer, clip);
	else
		SDL_RenderFillRect(m_renderer, &screenrect);
}

void SDL::rect(int x, int y, int w, int h, SDL_Color color)
{
	SDL_Rect screenRect = {x, y, w, h};
	SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
	SDL_RenderDrawRect(m_renderer, &screenRect);
}
