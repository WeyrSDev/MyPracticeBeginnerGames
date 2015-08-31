#include "Resources.hpp"

Resources::Resources()
{
	const std::string resourcePath{ "resources/" };

	addFont("main", resourcePath + "arial.ttf");

	addSoundBuffer("paddle", resourcePath + "paddle.wav");
	addSoundBuffer("wall", resourcePath + "wall.wav");
	addSoundBuffer("player score", resourcePath + "player.wav");
	addSoundBuffer("opponent score", resourcePath + "opponent.wav");
}
