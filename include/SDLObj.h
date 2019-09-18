#ifndef SDLOBJ_H
#define SDLOBJ_H
#include "custom_sdl_functions.h"

class SDL
{
	public:
		SDL(int winW, int winH, Uint32 initFlags = 0);
		~SDL();
		void render();
		inline void addSurfaceToTarget(int x, int y, SDL_Surface* source, SDL_Rect* clip = nullptr) {apply_surface(x, y, source, target, clip);}
		inline SDL_Surface* getTarget() {return target;}
		inline bool isHealthy() {return healthy;}

	private:
		SDL(SDL& other);
		bool init(Uint32 flags);
		bool healthy = false;
		int WINW;
		int WINH;
		SDL_Window* m_window;
		SDL_Renderer* m_renderer;
		SDL_Surface* target;
};

#endif // SDLObj_H
