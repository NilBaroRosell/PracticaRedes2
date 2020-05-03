#pragma once
#include <PlayerInfo.h>
#include <SFML\Network.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <fcntl.h>
#include "Graphics.h"
#include <Types.h>

#define SERVER_IP "192.168.1.43"
#define CLIENT_IP "192.168.1.43"
#define SERVER_PORT 50000
#define WAITING_CONNECTION_TIME 1
#define WAITING_CHALLENGE_TIME 2

struct Server
{
	sf::IpAddress ip;
	unsigned short port;
	uint64_t serverSalt;
};

int main()
{
	srand(time(NULL));
	PlayerInfo playerInfo;
	std::vector<PlayerInfo> players;
	Graphics g;
	sf::RenderWindow _window(sf::VideoMode(800, 600), "Ventanita");
	sf::RectangleShape shape(sf::Vector2f(SIZE, SIZE));
	shape.setOutlineColor(sf::Color::Black);
	shape.setOutlineThickness(2.f);

	std::cout << "Welcome, choose your nickname" << std::endl;
	std::cin >> playerInfo.name;

	sf::UdpSocket socket;
	sf::Packet packet;
	packet << static_cast<int32_t>(Comands::HELLO) << playerInfo.playerSalt << playerInfo.name;
	//Enviamos a una IP:Puerto concreto, porque el socket no está vinculado
	//a ningún otro socket en exclusiva
	socket.send(packet, SERVER_IP, SERVER_PORT);

	sf::IpAddress senderIP;
	unsigned short senderPort;
	//Cuando recibo me indican de qué IP:Puerto recibo,
	//para poder distinguir a los clientes
	socket.receive(packet, senderIP, senderPort);
	Server server;
	int aux;
	bool challenged = false;
	std::string question;
	std::string answer;
	bool connected = false;
	uint64_t serverSalt;
	Comands comand;
	sf::Clock clock;
	sf::Time time = clock.getElapsedTime();
	sf::Time sendTime = clock.getElapsedTime();
	sf::Time firstSendTime = clock.getElapsedTime();
	socket.setBlocking(false);

	while (_window.isOpen())
	{
		packet.clear();
		time = clock.getElapsedTime();
		sf::Event event;
		bool playerMoved = false;
		while (_window.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				_window.close();
				break;
			case sf::Event::KeyPressed:
				if (event.key.code == sf::Keyboard::Escape)
				{
					_window.close();
				}
				if (event.key.code == sf::Keyboard::Left)
				{
					std::cout << "LEFT\n";
				}
				else if (event.key.code == sf::Keyboard::Up)
				{
					std::cout << "UP\n";
				}
				else if (event.key.code == sf::Keyboard::Right)
				{
					std::cout << "RIGTH\n";
				}
				else if (event.key.code == sf::Keyboard::Down)
				{
					std::cout << "DOWN\n";
				}
				break;
			}
		}

		if (socket.receive(packet, server.ip, server.port) == sf::Socket::Done)
		{
			packet >> aux;
			comand = (Comands)aux;
			switch (comand)
			{
			case Comands::CHALLENGE:
			{
				uint64_t playerSalt;
				packet >> question >> playerSalt;
				if (playerSalt == playerInfo.playerSalt)
				{
					packet >> serverSalt;
					std::cout << question << std::endl;
					std::cin >> answer;
					packet.clear();
					packet << static_cast<int32_t>(Comands::CHALLENGE) << answer << (playerInfo.playerSalt & serverSalt);
					socket.send(packet, SERVER_IP, SERVER_PORT);
					challenged = true;
				}
				break;
			}
			case Comands::WELCOME:
			{
				uint64_t salts;
				packet >> salts;
				if (salts == (playerInfo.playerSalt & serverSalt))
				{
					system("CLS");
					int otherPlayers;
					packet >> playerInfo.position.x >> playerInfo.position.y >> otherPlayers;
					std::cout << "Your position is " << playerInfo.position.x << ", " << playerInfo.position.y << std::endl;
					if (otherPlayers != 0)
					{
						for (int i = 0; i < otherPlayers; i++)
						{
							PlayerInfo aux;
							packet >> aux.name >> aux.position.x >> aux.position.y;
							players.push_back(aux);
						}
						std::cout << "The other players in this server are:" << std::endl;
						for (int i = 0; i < players.size(); i++)
						{
							std::cout << players[i].name << " in position " << players[i].position.x << ", " << players[i].position.y << std::endl;
						}
						std::cout << "The game will start soon" << std::endl;
					}
					else
					{
						std::cout << "Waiting for players" << std::endl;
					}
					connected = true;
				}
				break;
			}
			case Comands::NEW_PLAYER:
			{
				uint64_t salts;
				packet >> salts;
				if (salts == (playerInfo.playerSalt & serverSalt))
				{
					PlayerInfo aux;
					packet >> aux.name >> aux.position.x >> aux.position.y;
					players.push_back(aux);
					std::cout << "There's a new player, " << players.back().name << " in position " << players.back().position.x << ", " << players.back().position.y << std::endl;
					std::cout << "The game will start soon" << std::endl;
				}
				break;
			}
			case Comands::END:
			{
				uint64_t salts;
				packet >> salts;
				if (salts == (playerInfo.playerSalt & serverSalt))
				{
					players.erase(players.begin(), players.end());

					system("CLS");
					do
					{
						std::cout << "Do you want to play another game? (Y/N)" << std::endl;
						answer.clear();
						std::cin >> answer;
						system("CLS");
					} while (answer != "y" && answer != "Y" && answer != "N" && answer != "n");
					if (answer == "y" || answer == "Y") 
					{
						packet << static_cast<int32_t>(Comands::HELLO) << playerInfo.playerSalt << playerInfo.name;
						socket.send(packet, SERVER_IP, SERVER_PORT);

						sendTime = clock.getElapsedTime();
						challenged = false;
						connected = false;
					}
					else
					{
						std::cout << "Disconnected" << std::endl;
						return 0;
					}
				}
				break;
			}
			default:
				break;
			}
			sendTime = clock.getElapsedTime();
		}

		if (!challenged && time.asSeconds() - sendTime.asSeconds() > WAITING_CONNECTION_TIME)
		{
			packet << static_cast<int32_t>(Comands::HELLO) << playerInfo.playerSalt << playerInfo.name;
			socket.send(packet, SERVER_IP, SERVER_PORT);

			sendTime = clock.getElapsedTime();
		}
		else if (challenged && !connected && time.asSeconds() - sendTime.asSeconds() > WAITING_CHALLENGE_TIME)
		{
			std::cout << question << std::endl;
			std::cin >> answer;
			packet << static_cast<int32_t>(Comands::CHALLENGE) << answer << (playerInfo.playerSalt & serverSalt);
			socket.send(packet, SERVER_IP, SERVER_PORT);

			sendTime = clock.getElapsedTime();
		}
		else if (connected && time.asSeconds() - sendTime.asSeconds() > DISCONNECTED_TIME)
		{
			sendTime = clock.getElapsedTime();
			std::cout << "Disconnected" << std::endl;
			challenged = false;
			connected = false;
		}

		
		_window.clear();
		for (int i = 0; i < W_WINDOW_TITLE; i++)
		{
			for (int j = 0; j < H_WINDOW_TITLE; j++)
			{

				shape.setFillColor(sf::Color(90, 90, 90, 255));


				shape.setPosition(sf::Vector2f(i*SIZE, j*SIZE));
				_window.draw(shape);
			}
		}

		for (size_t i = 0; i < g.salas.size(); i++)
		{
			g.salas[i].Draw(_window);

		}
		g.centroMensajes.Draw(_window);

		_window.display();
	}
	return 0;
}