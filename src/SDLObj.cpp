#include "SDLObj.h"
#include <iostream>
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

// Helper function to draw a filled circle
void drawFilledCircle(SDL_Renderer* renderer, int x, int y, int radius, bool fill)
{
	if (!fill)
	{
		int x_offset = 0;
		int y_offset = radius;
		int decision_over2 = 1 - y_offset;   // Decision criterion divided by 2

		while (y_offset >= x_offset)
		{
			// Draw the perimeter points
			SDL_RenderDrawPoint(renderer, x + x_offset, y + y_offset);
			SDL_RenderDrawPoint(renderer, x + y_offset, y + x_offset);
			SDL_RenderDrawPoint(renderer, x - x_offset, y + y_offset);
			SDL_RenderDrawPoint(renderer, x - y_offset, y + x_offset);
			SDL_RenderDrawPoint(renderer, x + x_offset, y - y_offset);
			SDL_RenderDrawPoint(renderer, x + y_offset, y - x_offset);
			SDL_RenderDrawPoint(renderer, x - x_offset, y - y_offset);
			SDL_RenderDrawPoint(renderer, x - y_offset, y - x_offset);

			x_offset++;

			if (decision_over2 <= 0)
			{
				decision_over2 += 2 * x_offset + 1;   // Change in decision criterion for x_offset -> x_offset+1
			}
			else
			{
				y_offset--;
				decision_over2 += 2 * (x_offset - y_offset) + 1;   // Change for y_offset -> y_offset-1
			}
		}
	}
	else
	{
		for (int w = 0; w < radius * 2; w++)
		{
			for (int h = 0; h < radius * 2; h++)
			{
				int dx = radius - w; // horizontal offset
				int dy = radius - h; // vertical offset
				if ((dx*dx + dy*dy) <= (radius * radius))
				{
					SDL_RenderDrawPoint(renderer, x + dx, y + dy);
				}
			}
		}
	}
}

// Function to fill a rectangle with rounded corners
void SDL::rectRounded(SDL_Rect rect, int cornerRadius, SDL_Color color, bool draw)
{
	SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    // Draw the main filled rectangle without the corners
    SDL_Rect innerRect = {rect.x + cornerRadius, rect.y, rect.w - 2 * cornerRadius, rect.h};
    if (draw)
		SDL_RenderDrawRect(m_renderer, &innerRect);
	else
		SDL_RenderFillRect(m_renderer, &innerRect);

    innerRect = {rect.x, rect.y + cornerRadius, rect.w, rect.h - 2 * cornerRadius};
    if (draw)
		SDL_RenderDrawRect(m_renderer, &innerRect);
	else
		SDL_RenderFillRect(m_renderer, &innerRect);

    // Draw the four corners
    drawFilledCircle(m_renderer, rect.x + cornerRadius, rect.y + cornerRadius, cornerRadius, !draw); // Top-left corner
    drawFilledCircle(m_renderer, rect.x + rect.w - cornerRadius - 1, rect.y + cornerRadius, cornerRadius, !draw); // Top-right corner
    drawFilledCircle(m_renderer, rect.x + cornerRadius, rect.y + rect.h - cornerRadius - 1, cornerRadius, !draw); // Bottom-left corner
    drawFilledCircle(m_renderer, rect.x + rect.w - cornerRadius - 1, rect.y + rect.h - cornerRadius - 1, cornerRadius, !draw); // Bottom-right corner
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0);
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);

    if (draw)
	{ 	innerRect = {rect.x + cornerRadius, rect.y + 1, rect.w - 2 * cornerRadius, rect.h - 2};
		SDL_RenderFillRect(m_renderer, &innerRect);
		innerRect = {rect.x + 1, rect.y + cornerRadius, rect.w - 2, rect.h - 2 * cornerRadius};
		SDL_RenderFillRect(m_renderer, &innerRect);
	}
}
