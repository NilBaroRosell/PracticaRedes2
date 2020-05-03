#pragma once
#include <PlayerInfo.h>
#include <SFML\Network.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <fcntl.h>
#include "Graphics.h"
#include <Types.h>

#define SERVER_IP "localhost"
#define CLIENT_IP "localhost"
#define SERVER_PORT 50000
#define WAITING_CONNECTION_TIME 1
#define WAITING_CHALLENGE_TIME 5
#define DISCONNECTED_TIME 500000
#define VIEW_SEND_RATE 50 //MILLISECONDS

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
	Graphics g;
	sf::RenderWindow _window(sf::VideoMode(800, 600), "Ventanita");
	_window.setFramerateLimit(60);
	sf::RectangleShape shape(sf::Vector2f(SIZE, SIZE));
	shape.setOutlineColor(sf::Color::Black);
	shape.setOutlineThickness(2.f);
	sf::RectangleShape player(sf::Vector2f(50, 50));
	player.setFillColor(sf::Color::Blue);
	player.setPosition(sf::Vector2f(rand() % 600 + 100, rand() % 400 + 100));
	sf::RectangleShape player2(sf::Vector2f(50, 50));
	sf::Vector2f p2Pos(100, 300);
	player2.setFillColor(sf::Color::Red);
	player2.setPosition(p2Pos);
	bool p2Connected = false;

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
	std::string challengeAnswere;
	bool connected = false;
	uint64_t serverSalt;
	Comands comand;
	sf::Clock clock;
	sf::Time time = clock.getElapsedTime();
	sf::Time sendTime = clock.getElapsedTime();
	sf::Time firstSendTime = clock.getElapsedTime();
	sf::Time viewRef = clock.getElapsedTime();
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
				break;
			}
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
		{
			if(player.getPosition().x > g.salaInterior.origen.x * SIZE)
				player.setPosition(player.getPosition() + sf::Vector2f(-10, 0));
			else player.setPosition(g.salaInterior.origen.x * SIZE, player.getPosition().y);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
		{
			if(player.getPosition().y > g.salaInterior.origen.y * SIZE)
				player.setPosition(player.getPosition() + sf::Vector2f(0, -10));
			else player.setPosition(player.getPosition().x, g.salaInterior.origen.y * SIZE);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
		{
			if(player.getPosition().x + player.getLocalBounds().width < (g.salaInterior.origen.x + 
				g.salaInterior.longitud.x) * SIZE)
				player.setPosition(player.getPosition() + sf::Vector2f(10, 0));
			else player.setPosition((g.salaInterior.origen.x + g.salaInterior.longitud.x) * SIZE
				- player.getLocalBounds().width, player.getPosition().y);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
		{
			if(player.getPosition().y + player.getLocalBounds().height < (g.salaInterior.origen.y + 
				g.salaInterior.longitud.y) * SIZE)
				player.setPosition(player.getPosition() + sf::Vector2f(0, 10));
			else player.setPosition(player.getPosition().x, (g.salaInterior.origen.y + g.salaInterior.longitud.y) * SIZE 
				- player.getLocalBounds().height);
		}

		player2.setPosition(player2.getPosition() + (p2Pos - player2.getPosition()) / 2.0f);

		if (socket.receive(packet, server.ip, server.port) == sf::Socket::Done)
		{
			packet >> aux;
			comand = (Comands)aux;
			std::cout << "Rep algo" << std::endl;
			switch (comand)
			{
			case Comands::CHALLENGE:
			{
				std::string question;
				uint64_t playerSalt;
				packet >> question >> playerSalt;
				if (playerSalt == playerInfo.playerSalt)
				{
					packet >> serverSalt;
					std::cout << question << std::endl;
					std::cin >> challengeAnswere;
					packet.clear();
					packet << static_cast<int32_t>(Comands::CHALLENGE) << challengeAnswere << (playerInfo.playerSalt & serverSalt);
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
					std::cout << "WELCOME" << std::endl;
					connected = true;
				}
				break;
			}
			case Comands::POSITION:
			{
				float x, y;
				p2Connected = true;
				packet >> x >> y;

				p2Pos = sf::Vector2f(x, y);
				break;
			}
			default:
				break;
			}
			sendTime = clock.getElapsedTime();
		}

		packet.clear();

		if (!challenged && time.asSeconds() - sendTime.asSeconds() > WAITING_CONNECTION_TIME)
		{
			packet << static_cast<int32_t>(Comands::HELLO) << playerInfo.playerSalt << playerInfo.name;
			socket.send(packet, SERVER_IP, SERVER_PORT);

			sendTime = clock.getElapsedTime();
		}
		else if (challenged && !connected && time.asSeconds() - sendTime.asSeconds() > WAITING_CHALLENGE_TIME)
		{
			packet << static_cast<int32_t>(Comands::CHALLENGE) << challengeAnswere << (playerInfo.playerSalt & serverSalt);
			socket.send(packet, SERVER_IP, SERVER_PORT);

			sendTime = clock.getElapsedTime();
		}
		else if (connected && time.asMilliseconds() - viewRef.asMilliseconds() > VIEW_SEND_RATE)
		{
			viewRef = clock.getElapsedTime();
			std::cout << "SEND VIEW" << std::endl;
			packet << static_cast<int32_t>(Comands::POSITION) << player.getPosition().x << player.getPosition().y;
			socket.send(packet, SERVER_IP, SERVER_PORT);
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

		g.paret.Draw(_window);

		g.salaInterior.Draw(_window);

		_window.draw(player);

		if(p2Connected)_window.draw(player2);

		_window.display();
	}
	return 0;
}