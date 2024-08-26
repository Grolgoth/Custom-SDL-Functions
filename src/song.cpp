#include "song.h"
#include <Timer.h>
#include <fstream>

std::string Song::songBeingPlayed = "";

Song::Song(std::string title, std::string path, int from, int size) : name(title)
{
	if (from > -1)
	{
		std::ifstream file(path, std::ios::binary);
		char *buffer = (char *)malloc(size);
		file.seekg(from);
		file.read(buffer, size);
		file.close();
		SDL_RWops* songWop = SDL_RWFromMem(buffer, size);
		music = Mix_LoadMUS_RW(songWop, 1);
		free(buffer);
	}
	else
		music = Mix_LoadMUS(path.c_str());
	if (music == nullptr)
		throw "Error loading song: " + title + " file: " + path;
	timer = new TimerManager();
}

Song::Song(Mix_Music* songInitialized)
{
	if (songInitialized == nullptr)
		throw "Can't create song. Parameter was not initialized.";
	music = songInitialized;
	timer = new TimerManager();
}

Song::~Song()
{
	if (songBeingPlayed == name)
		Mix_HaltMusic();
	Mix_FreeMusic(music);
	delete timer;
}

void Song::play(int loops)
{
	songBeingPlayed = name;
	Mix_PlayMusic(music, loops);
	timer->start();
}

int Song::getPlayTime()
{
	if(isPlaying())
		return timer->get_elt();
	else
	{
		timer->stop();
		return -1;
	}
}

void Song::pause()
{
	if (songBeingPlayed != name)
		return;
	if (isPlaying() && !Mix_PausedMusic())
	{
		Mix_PauseMusic();
		timer->stop();
	}
	else if(isPlaying() && Mix_PausedMusic())
	{
		Mix_ResumeMusic();
		timer->start();
	}
}

void Song::stop()
{
	if (songBeingPlayed != name || !isPlaying())
		return;
	timer->stop();
	Mix_HaltMusic();
	songBeingPlayed = "";
}
