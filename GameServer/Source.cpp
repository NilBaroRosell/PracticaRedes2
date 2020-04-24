#pragma once
#include <iostream>
#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <PlayerInfo.h>
#include <Types.h>

#define SERVER_IP "localhost"
#define DISCONNECTED_TIME 5

struct Clients
{
	sf::IpAddress ip;
	unsigned short port;
	sf::Time lastRecive;
	int clientID;
};

int main()
{
	bool serverRunning = true;
	PlayerInfo playerInfo;
	sf::Packet packet;
	sf::UdpSocket socket;
	std::vector<Clients> clients;

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
				for (auto c1 : clients)
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
					recievedClient.clientID = numPlayers;
					clients.push_back(recievedClient);
					numPlayers++;
					std::cout << clients.back().ip << " " << clients.back().port << std::endl;
				}
				packet.clear();
				packet << static_cast<int32_t>(Comands::WELCOME);
				socket.send(packet, recievedClient.ip.toString(), recievedClient.port);
				break;
			}
			default:
				break;
			}

			clients[recievedClient.clientID].lastRecive = clock.getElapsedTime();
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