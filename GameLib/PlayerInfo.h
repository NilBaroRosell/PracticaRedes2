#pragma once
#include <SFML\Graphics.hpp>

class PlayerInfo
{
public:
	std::string name;
	uint64_t playerSalt;
	sf::Vector2i position;
	int lives;
	PlayerInfo();
	~PlayerInfo();
};