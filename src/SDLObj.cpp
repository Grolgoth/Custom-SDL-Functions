#include "SDLObj.h"
#include <sdl_ttf_custom.h>
#include <SDL_mixer.h>

SDL::SDL(int winW, int winH, Uint32 initFlags) : target(nullptr)
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
	if (target != nullptr)
		SDL_FreeSurface(target);
	SDL_DestroyWindow( m_window );
    SDL_DestroyRenderer( m_renderer );
    if (withMixer)
		Mix_Quit();
    TTF_Quit();
    SDL_Quit();
}

void SDL::render()
{
	SDL_RenderClear(m_renderer);
    SDL_Texture* screen = SDL_CreateTextureFromSurface(m_renderer, target);
    SDL_RenderCopy(m_renderer, screen, NULL, NULL);
    SDL_RenderPresent(m_renderer);
    SDL_DestroyTexture(screen);
    SDL_FreeSurface(target);
	target = SDL_CreateRGBSurface(SDL_SWSURFACE,WINW,WINH,32,0xff000000,0x00ff0000,0x0000ff00,0x000000ff);
}

bool SDL::init(Uint32 flags)
{
	if(SDL_Init(flags))
		return false;
	if(TTF_Init())
		return false;
	if(SDL_CreateWindowAndRenderer( WINW, WINH, SDL_WINDOW_SHOWN, &m_window, &m_renderer)) //possibly use settings for fullscreen, resizable window etc
		return false;
	if (m_window == nullptr || m_renderer == nullptr)
		return false;
	if (flags && SDL_INIT_AUDIO)
		if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
			return false;
	return true;
}
