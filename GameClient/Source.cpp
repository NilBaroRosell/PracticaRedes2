#pragma once
#include <PlayerInfo.h>
#include <SFML\Network.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <queue>
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
#define BULLET_RATE 250 //MILLISECONDS
#define BULLET_ERASE_REF 2

struct Server
{
	sf::IpAddress ip;
	unsigned short port;
	uint64_t serverSalt;
};

struct Bullet
{
	sf::CircleShape body;
	sf::Vector2f direction = sf::Vector2f(0.0f, 0.0f);
	float speed = 10.0f;
	float shotRef;
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

	std::vector<Bullet> bullets;
	std::queue<Bullet> bulletsToSend;
	std::vector<std::pair<Bullet, sf::Time>> bulletsToConfirm;

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
	sf::Time bulletRef = clock.getElapsedTime();
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

		if (_window.hasFocus()) {
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
			{
				if (player.getPosition().x > g.salaInterior.origen.x * SIZE)
					player.setPosition(player.getPosition() + sf::Vector2f(-5, 0));
				else player.setPosition(g.salaInterior.origen.x * SIZE, player.getPosition().y);
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
			{
				if (player.getPosition().y > g.salaInterior.origen.y * SIZE)
					player.setPosition(player.getPosition() + sf::Vector2f(0, -5));
				else player.setPosition(player.getPosition().x, g.salaInterior.origen.y * SIZE);
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
			{
				if (player.getPosition().x + player.getLocalBounds().width < (g.salaInterior.origen.x +
					g.salaInterior.longitud.x) * SIZE)
					player.setPosition(player.getPosition() + sf::Vector2f(5, 0));
				else player.setPosition((g.salaInterior.origen.x + g.salaInterior.longitud.x) * SIZE
					- player.getLocalBounds().width, player.getPosition().y);
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
			{
				if (player.getPosition().y + player.getLocalBounds().height < (g.salaInterior.origen.y +
					g.salaInterior.longitud.y) * SIZE)
					player.setPosition(player.getPosition() + sf::Vector2f(0, 5));
				else player.setPosition(player.getPosition().x, (g.salaInterior.origen.y + g.salaInterior.longitud.y) * SIZE
					- player.getLocalBounds().height);
			}

			if (connected && time.asMilliseconds() - bulletRef.asMilliseconds() > BULLET_RATE)
			{
				Bullet newBullet;
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
					newBullet.direction = sf::Vector2f(-newBullet.speed, 0.0f);
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
					newBullet.direction = sf::Vector2f(0.0f, -newBullet.speed);
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
					newBullet.direction = sf::Vector2f(newBullet.speed, 0.0f);
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
					newBullet.direction = sf::Vector2f(0.0f, newBullet.speed);
				if (newBullet.direction != sf::Vector2f(0.0f, 0.0f))
				{
					newBullet.body = sf::CircleShape(25.0f);
					newBullet.body.setPosition(player.getPosition());
					newBullet.body.setFillColor(sf::Color::Blue);
					newBullet.shotRef = time.asSeconds();
					bullets.push_back(newBullet);
					bulletsToSend.push(newBullet);
					bulletRef = clock.getElapsedTime();
				}
			}
		}

		for ( auto it = bullets.begin(); it != bullets.end();)
		{
			it->body.setPosition(it->body.getPosition() + it->direction);
			if(it->body.getPosition().x < 0.0f || it->body.getPosition().y < 0.0f ||
				it->body.getPosition().x > 800 || it->body.getPosition().y > 600)
				it = bullets.erase(it);
			else ++it;
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
			case Comands::BULLETS:
			{
				float x, y, xDir, yDir, shotRef;
				packet >> x >> y >> xDir >> yDir >> shotRef;
				bool found = false;
				for (auto &bullet : bulletsToConfirm)
				{
					if (bullet.first.shotRef == shotRef) {
						found == true;
						bullet.second = clock.getElapsedTime();
					}
				}
				if (!found) {
					Bullet newBullet;
					newBullet.direction = sf::Vector2f(xDir, yDir);
					newBullet.body.setRadius(25.0f);
					newBullet.body.setPosition(x, y);
					newBullet.body.setFillColor(sf::Color::Red);
					newBullet.shotRef = shotRef;
					bullets.push_back(newBullet);
					bulletsToConfirm.push_back(std::make_pair(newBullet, clock.getElapsedTime()));
				}
				break;
			}
			case Comands::BULLET_OK:
			{
				float ref;
				packet >> ref;
				if (bulletsToSend.front().shotRef == ref) bulletsToSend.pop();
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
			sendTime = clock.getElapsedTime();
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

		if (connected && !bulletsToSend.empty())
		{
			packet.clear();
			packet << static_cast<int32_t>(Comands::BULLETS) << bulletsToSend.front().body.getPosition().x 
				<< bulletsToSend.front().body.getPosition().y << bulletsToSend.front().direction.x 
				<< bulletsToSend.front().direction.y << bulletsToSend.front().shotRef;
			socket.send(packet, SERVER_IP, SERVER_PORT);
			sendTime = clock.getElapsedTime();
		}

		if (connected && !bulletsToConfirm.empty())
		{
			for (auto it = bulletsToConfirm.begin(); it != bulletsToConfirm.end();)
			{
				if (time.asSeconds() - it->second.asSeconds() > BULLET_ERASE_REF)
					it = bulletsToConfirm.erase(it);
				else if (it->second == clock.getElapsedTime())
				{
					packet.clear();
					packet << static_cast<int32_t>(Comands::BULLET_OK) << it->first.shotRef;
					socket.send(packet, SERVER_IP, SERVER_PORT);
					sendTime = clock.getElapsedTime();
					break;
				}
				else ++it;
			}
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

		for (auto bullet : bullets)
			_window.draw(bullet.body);

		_window.draw(player);

		if(p2Connected)_window.draw(player2);

		_window.display();
	}
	return 0;
}