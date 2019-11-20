#ifndef SDLOBJ_H
#define SDLOBJ_H
#include "custom_sdl_functions.h"

class SDL
{
	public:
		SDL(int winW, int winH, Uint32 initFlags = 0);
		~SDL();
		void render();
		inline bool isHealthy() {return healthy;}
		void addToTarget(SDL_Texture* texture, int x, int y, int w, int h, SDL_Rect* clip = nullptr);
		void textToTarget(SDL_Surface* text, int x, int y);
		inline SDL_Renderer* getRenderer() {return m_renderer;}

	private:
		SDL(SDL& other);
		bool init(Uint32 flags);
		bool healthy = false;
		bool withMixer = false;
		int WINW;
		int WINH;
		SDL_Window* m_window;
		SDL_Renderer* m_renderer;
};

#endif // SDLObj_H
