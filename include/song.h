#ifndef SONG_H
#define SONG_H
#include <SDL_mixer.h>
#include <string>

class Timer;

class Song
{

	public:
		Song(std::string title, std::string path, int song = -1, int size = -1);
		Song(Mix_Music* songInitialized);
		~Song();
		void play();
		inline bool isPlaying() {return Mix_PlayingMusic();}
		int getPlayTime();
		void pause();
		void stop();
	private:
		Song(Song& other);
		static std::string songBeingPlayed;
		std::string name;
		Mix_Music* music;
		Timer* timer;
};
#endif // SONG_H
