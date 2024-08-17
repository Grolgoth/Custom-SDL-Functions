#include "SDLObj.h"
#include <sdl_ttf_custom.h>
#include <SDL_mixer.h>

SDL::SDL(int winW, int winH, Uint32 initFlags, bool resizeable, std::string winName) : resizeable(resizeable), winName(winName)
{
	WINW = winW;
	WINH = winH;
	if(!init(initFlags))
		std::cout << "Error initializing SDL: " << SDL_GetError() << std::endl;
	else
		healthy = true;
	withMixer = initFlags & SDL_INIT_AUDIO;
	discoverPixelFormats();
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
	{
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return false;
	}
	if(TTF_Init())
		return false;
	createWindow();
	if (m_window == nullptr)
		return false;
	m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
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

void SDL::discoverPixelFormats()
{
	SDL_RendererInfo info;
	SDL_GetRendererInfo(m_renderer, &info);
	for (unsigned int i = 0; i < info.num_texture_formats; ++i)
		if (!SDL_ISPIXELFORMAT_FOURCC(info.texture_formats[i]) && SDL_ISPIXELFORMAT_ALPHA(info.texture_formats[i]))
			bestFormatForAlphaBMPs = info.texture_formats[i];
}

void SDL::createWindow()
{
	SDL_DisplayMode displayMode;
	if (SDL_GetCurrentDisplayMode(0, &displayMode))
	{
		std::cout << "SDL_GetCurrentDisplayMode Error: " << SDL_GetError() << std::endl;
		WINW = 800;
		WINH = 600;
	}
	else
	{
		if (WINW == 0 || WINH == 0)
		{
			WINW = displayMode.w - 50;
			WINH = displayMode.h - 100;
		}
		else
		{
			if (WINW > displayMode.w)
				WINW = displayMode.w;
			if (WINH > displayMode.h)
				WINH = displayMode.h;
		}
	}
	Uint32 windowFlags = SDL_WINDOW_SHOWN;
	if (resizeable)
		windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED;
	m_window = SDL_CreateWindow(winName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINW, WINH, windowFlags); //possibly use settings for fullscreen, resizable window etc
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

void SDL::rect(int x, int y, int w, int h, SDL_Color color, bool draw)
{
	if (w == 0 || h == 0)
	{
		w = WINW;
		h = WINH;
	}
	SDL_Rect screenRect = {x, y, w, h};
	SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
	if (draw)
		SDL_RenderDrawRect(m_renderer, &screenRect);
	else
		SDL_RenderFillRect(m_renderer, &screenRect);
	SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0);
}
