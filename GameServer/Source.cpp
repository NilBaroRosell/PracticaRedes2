#pragma once
#include <iostream>
#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <PlayerInfo.h>
#include <Types.h>

#define SERVER_IP "localhost"
#define DISCONNECTED_TIME 50000
#define BULLET_ERASE_REF 1

struct Clients
{
	sf::IpAddress ip;
	unsigned short port;
	sf::Time lastRecive;
	int clientID;
	uint64_t clientSalt;
	std::string nickname;
};

struct Bullet
{
	sf::Vector2f position;
	sf::Vector2f direction;
	float shotRef;
	float serverRef;
	std::string senderIP;
	unsigned short senderPort;
};

int main()
{
	bool serverRunning = true;
	uint64_t serverSalt = std::rand();
	PlayerInfo playerInfo;
	sf::Packet packet;
	sf::UdpSocket socket;
	std::vector<Clients> clients;
	std::vector<Clients> newClients;
	std::vector<Bullet> bullets;

	sf::Socket::Status status = socket.bind(50000);
	if (status != sf::Socket::Done)
	{
		//No se puede vincular al puerto 50000 
		std::cout << "Fuck you bitch" << std::endl;
	}
	
	int numPlayers = 0;
	int aux;
	Comands comand;
	Clients recievedClient;
	socket.setBlocking(false);
	sf::Clock clock;
	sf::Time time = clock.getElapsedTime();
	while (serverRunning)
	{
		time = clock.getElapsedTime();
		packet.clear();
		
		if (socket.receive(packet, recievedClient.ip, recievedClient.port) == sf::Socket::Done)
		{
			packet >> aux;
			comand = (Comands)aux;
			std::cout << "Rep algo" << std::endl;
			switch (comand)
			{
			case Comands::HELLO:
			{
				bool exist = false;
				for (auto c1 : newClients)
				{
					if (c1.ip == recievedClient.ip && c1.port == recievedClient.port)
					{
						exist = true;
						recievedClient.clientID = c1.clientID;
						break;
					}
				}
				if (!exist)
				{
					packet >> recievedClient.clientSalt >> recievedClient.nickname;
					recievedClient.clientID = numPlayers;
					newClients.push_back(recievedClient);
					numPlayers++;
					std::cout << newClients.back().ip << " " << newClients.back().port << std::endl;
				}
				packet.clear();
				packet << static_cast<int32_t>(Comands::CHALLENGE) << "How many wood could a woodchuck chuck if a woodchuck could chuck wood?" << recievedClient.clientSalt << serverSalt;
				socket.send(packet, recievedClient.ip.toString(), recievedClient.port);
				break;
			}
			case Comands::CHALLENGE:
			{
				std::string answere;
				uint64_t salts;
				packet >> answere >> salts;
				if (answere == "stfu")
				{
					for (auto c1 : newClients)
					{
						if (c1.ip == recievedClient.ip && c1.port == recievedClient.port)
						{
							if (salts == (c1.clientSalt & serverSalt))
							{
								clients.push_back(c1);
								packet.clear();
								packet << static_cast<int32_t>(Comands::WELCOME) << (recievedClient.clientSalt & serverSalt);
								socket.send(packet, recievedClient.ip.toString(), recievedClient.port);
								break;
							}
						}
					}
				}
				break;
			}
			case Comands::POSITION:
			{
				float x, y;
				packet >> x >> y;
				packet.clear();
				packet << static_cast<int32_t>(Comands::POSITION) << x << y;
				for (auto c1 : clients)
				{
					if (c1.ip != recievedClient.ip || c1.port != recievedClient.port)
					{
						socket.send(packet, c1.ip.toString(), c1.port);
						break;
					}
				}
				break;
			}
			case Comands::BULLETS:
			{
				float x, y, xDir, yDir, shotRef;
				packet >> x >> y >> xDir >> yDir >> shotRef;
				packet.clear();
				bool found = false;
				for (auto bullet : bullets)
				{
					if (bullet.senderIP == recievedClient.ip.toString() && bullet.senderPort == recievedClient.port &&
						bullet.shotRef == shotRef) found = true;
				}
				if (!found) {
					Bullet newBullet;
					newBullet.position = sf::Vector2f(x, y);
					newBullet.direction = sf::Vector2f(xDir, yDir);
					newBullet.shotRef = shotRef;
					newBullet.serverRef = time.asSeconds();
					newBullet.senderIP = recievedClient.ip.toString();
					newBullet.senderPort = recievedClient.port;
					bullets.push_back(newBullet);
					packet << static_cast<int32_t>(Comands::BULLETS) << x << y << xDir << yDir << newBullet.serverRef;
					for (auto c1 : clients)
					{
						if (c1.ip != recievedClient.ip || c1.port != recievedClient.port)
						{
							socket.send(packet, c1.ip.toString(), c1.port);
							break;
						}
					}
					packet.clear();
				}
				packet << static_cast<int32_t>(Comands::BULLET_OK) << shotRef;
				socket.send(packet, recievedClient.ip.toString(), recievedClient.port);
				break;
			}
			case Comands::BULLET_OK:
			{
				float ref;
				packet >> ref;
				for (auto it = bullets.begin(); it != bullets.end(); ++it)
				{
					if (it->serverRef == ref) {
						bullets.erase(it); break;
					}
				}
				break;
			}
			default:
				break;
			}

			if(!clients.empty() && recievedClient.clientID < clients.size()) clients[recievedClient.clientID].lastRecive = clock.getElapsedTime();
		}

		for (auto it = bullets.begin(); it != bullets.end();)
		{
			if(time.asSeconds() - it->serverRef > BULLET_ERASE_REF)
				it = bullets.erase(it);
			else {
				packet.clear();
				packet << static_cast<int32_t>(Comands::BULLETS) << it->position.x << it->position.y << it->direction.x 
					<< it->direction.y << it->serverRef;
				++it;
			}
		}

		for (int i = 0; i < clients.size(); i++)
		{
			if (time.asSeconds() - clients[i].lastRecive.asSeconds() > DISCONNECTED_TIME)
			{
				std::cout << clients[i].clientID << " has disconnected" << std::endl;
				clients.erase(clients.begin() + i);
				numPlayers--;
				break;
			}
		}
	}

	return 0;
}