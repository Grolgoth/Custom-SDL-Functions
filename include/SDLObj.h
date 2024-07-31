#ifndef SDLOBJ_H
#define SDLOBJ_H
#include "custom_sdl_functions.h"

class SDL
{
	public:
		SDL(int winW, int winH, Uint32 initFlags = 0, bool resizeable = false, std::string winName = "program");
		~SDL();
		void render();
		inline bool isHealthy() {return healthy;}
		void addToTarget(SDL_Texture* texture, int x, int y, int w, int h, SDL_Rect* clip = nullptr);
		void textToTarget(SDL_Surface* text, int x, int y);
		inline SDL_Renderer* getRenderer() {return m_renderer;}
		void changeWindowSize(int winw, int winh);
		void fullscreen();
		void modColors(int a, int r, int g, int b, SDL_Rect* clip = nullptr);
		void rect(int x, int y, int w, int h, SDL_Color color, bool draw = false);
		inline int getWinW() {return WINW;}
		inline int getWinH() {return WINH;}
		inline Uint32 getBestFormatForAlphaBMPs() {return bestFormatForAlphaBMPs;}

	private:
		SDL(SDL& other);
		bool init(Uint32 flags);
		void discoverPixelFormats();
		void createWindow();
		bool healthy = false;
		bool withMixer = false;
		bool isFullscreen = false;
		double sizeChangeFactorW = 1.0;
		double sizeChangeFactorH = 1.0;
		int WINW;
		int WINH;
		bool resizeable;
		std::string winName;
		SDL_Window* m_window;
		SDL_Renderer* m_renderer;
		Uint32 bestFormatForAlphaBMPs;
};

#endif // SDLObj_H
