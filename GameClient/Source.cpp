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
#define DISCONNECTED_TIME 5

struct Server
{
	sf::IpAddress ip;
	unsigned short port;
};

int main()
{
	PlayerInfo playerInfo;
	Graphics g;
	sf::RenderWindow _window(sf::VideoMode(800, 600), "Ventanita");
	sf::RectangleShape shape(sf::Vector2f(SIZE, SIZE));
	shape.setOutlineColor(sf::Color::Black);
	shape.setOutlineThickness(2.f);

	sf::UdpSocket socket;
	sf::Packet packet;
	packet << static_cast<int32_t>(Comands::HELLO);
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
	bool connected = false;
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
			std::cout << "Rep algo" << std::endl;
			switch (comand)
			{
			case Comands::WELCOME:
			{
				std::cout << "You are welcome" << std::endl;
				connected = true;
				break;
			}
			default:
				break;
			}
			sendTime = clock.getElapsedTime();
		}

		if (!connected && time.asSeconds() - sendTime.asSeconds() > WAITING_CONNECTION_TIME)
		{
			packet << static_cast<int32_t>(Comands::HELLO);
			socket.send(packet, SERVER_IP, SERVER_PORT);

			sendTime = clock.getElapsedTime();
		}
		else if (connected && time.asSeconds() - sendTime.asSeconds() > DISCONNECTED_TIME)
		{
			sendTime = clock.getElapsedTime();
			std::cout << "Disconnected" << std::endl;
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