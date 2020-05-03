#pragma once
#include <iostream>
#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <PlayerInfo.h>
#include <Types.h>
#include <math.h>

#define SERVER_IP "localhost"

struct Vector2
{
	int x;
	int y;
	Vector2(int _x, int _y)
	{
		x = _x;
		y = _y;
	}
};

void InitializeBoard(std::string _board[COLUMNS][ROWS])
{
	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLUMNS; j++)
		{
			_board[i][j] = "N";
		}
	}
}

struct Clients
{
	sf::IpAddress ip;
	unsigned short port;
	sf::Time lastRecive;
	uint64_t clientSalt;
	Vector2 pos = Vector2(0, 0);
	std::string nickname;
};

Vector2 GetRandomPos()
{
	return Vector2(rand() % ROWS, rand() % COLUMNS);
}

void GetRandomPositionInRoom(std::string _board[COLUMNS][ROWS], Clients &_client)
{
	Vector2 aux = GetRandomPos();
	while (_board[aux.x][aux.y] != "N")
	{
		aux = GetRandomPos();
	}
	_board[aux.x][aux.y] = _client.nickname;
	_client.pos = aux;
}

struct Room
{
	int roomTag;
	std::vector<Clients> clients;
	std::string board[COLUMNS][ROWS];
	sf::Time lastPlayerEntered;
	int numPlayers;

	Room(Clients _client, int _lastTag)
	{
		roomTag = _lastTag + 1;
		clients.push_back(_client);
		InitializeBoard(board);
		numPlayers = 0;
		lastPlayerEntered;
	}
};


int main()
{
	srand(time(NULL));
	bool serverRunning = true;
	uint64_t serverSalt = std::rand();
	PlayerInfo playerInfo;
	sf::Packet packet;
	sf::UdpSocket socket;
	std::vector<Clients> newClients;
	std::vector<Room> rooms;

	sf::Socket::Status status = socket.bind(50000);
	if (status != sf::Socket::Done)
	{
		std::cout << "Unable to connect" << std::endl;
	}
	
	//int numPlayers = 0;
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
						break;
					}
				}
				if (!exist)
				{
					packet >> recievedClient.clientSalt >> recievedClient.nickname;
					newClients.push_back(recievedClient);
				}
				packet.clear();
				packet << static_cast<int32_t>(Comands::CHALLENGE) << "Which mark will Bartomeu and Nil get in this task?" << recievedClient.clientSalt << serverSalt;
				socket.send(packet, recievedClient.ip.toString(), recievedClient.port);
				break;
			}
			case Comands::CHALLENGE:
			{
				std::string answere;
				uint64_t salts;
				packet >> answere >> salts;
				if (answere == "10")
				{
					for (int client = 0; client < newClients.size(); client++)
					{
						if (newClients[client].ip == recievedClient.ip && newClients[client].port == recievedClient.port)
						{
							if (salts == (newClients[client].clientSalt & serverSalt))
							{
								if (rooms.empty())
								{
									rooms.push_back(Room(newClients[client], 0));
									GetRandomPositionInRoom(rooms.back().board, rooms.back().clients.back());
									rooms.back().numPlayers++;
									packet.clear();
									packet << static_cast<int32_t>(Comands::WELCOME) << (rooms.back().clients.back().clientSalt & serverSalt) << rooms.back().clients.back().pos.x << rooms.back().clients.back().pos.y << 0;
									socket.send(packet, recievedClient.ip.toString(), recievedClient.port);
								}
								else 
								{
									bool newRoom = true;
									for (int i = 0; i < rooms.size(); i++)
									{
										if (rooms[i].numPlayers < MAX_ROOM_PLAYERS && abs((int)rooms[i].clients.front().nickname[0] - (int)newClients[client].nickname[0]) <= 10 && newRoom)
										{
											rooms[i].clients.push_back(newClients[client]);
											newRoom = false;
											GetRandomPositionInRoom(rooms[i].board, rooms[i].clients.back());
											rooms[i].numPlayers++;
											packet.clear();
											packet << static_cast<int32_t>(Comands::WELCOME) << (rooms[i].clients.back().clientSalt & serverSalt) << rooms[i].clients.back().pos.x << rooms[i].clients.back().pos.y << rooms[i].numPlayers - 1;
											for (int j = 0; j < rooms[i].clients.size() - 1; j++)
											{
												packet << rooms[i].clients[j].nickname << rooms[i].clients[j].pos.y << rooms[i].clients[j].pos.y;
											}
											socket.send(packet, recievedClient.ip.toString(), recievedClient.port);

											for (int j = 0; j < rooms[i].clients.size() - 1; j++)
											{
												packet.clear();
												packet << static_cast<int32_t>(Comands::NEW_PLAYER) << (rooms[i].clients[j].clientSalt & serverSalt) << rooms[i].clients.back().nickname << rooms[i].clients.back().pos.x << rooms[i].clients.back().pos.y;
												socket.send(packet, rooms[i].clients[j].ip.toString(), rooms[i].clients[j].port);
											}

											if (rooms[i].numPlayers == MAX_ROOM_PLAYERS)
											{
												rooms[i].lastPlayerEntered = clock.getElapsedTime();
											}
										}
									}
									if (newRoom)
									{
										rooms.push_back(Room(newClients[client], rooms.back().roomTag));
										GetRandomPositionInRoom(rooms.back().board, rooms.back().clients.back());
										rooms.back().numPlayers++;
										packet.clear();
										packet << static_cast<int32_t>(Comands::WELCOME) << (rooms.back().clients.back().clientSalt & serverSalt) << rooms.back().clients.back().pos.x << rooms.back().clients.back().pos.y << 0;
										socket.send(packet, recievedClient.ip.toString(), recievedClient.port);
									}
								}

								newClients.erase(newClients.begin() + client);
								break;
							}
						}
					}
				}
				break;
			}
			default:
				break;
			}

			for (int i = 0; i < rooms.size(); i++)
			{
				for (int j = 0; j < rooms[i].clients.size(); j++)
				{
					if (rooms[i].clients[j].clientSalt == recievedClient.clientSalt)
					{
						rooms[i].clients[j].lastRecive = clock.getElapsedTime();
					}
				}
			}
		}

		system("CLS");

		for (int i = 0; i < rooms.size(); i++)
		{
			if (rooms[i].numPlayers == MAX_ROOM_PLAYERS && (time.asSeconds() - rooms[i].lastPlayerEntered.asSeconds()) > PLAYING_TIME)
			{
				for (int j = 0; j < rooms[i].clients.size(); j++)
				{
					packet.clear();
					packet << static_cast<int32_t>(Comands::END) << (rooms[i].clients[j].clientSalt & serverSalt);
					socket.send(packet, rooms[i].clients[j].ip.toString(), rooms[i].clients[j].port);
				}

				rooms.erase(rooms.begin() + i);
			}
			else
			{
				std::cout << "Room " << rooms[i].roomTag << std::endl;
				for (int j = 0; j < rooms[i].clients.size(); j++)
				{
					std::cout << rooms[i].clients[j].nickname << std::endl;
					if (time.asSeconds() - rooms[i].clients[j].lastRecive.asSeconds() > DISCONNECTED_TIME)
					{
						std::cout << rooms[i].clients[j].nickname << " has disconnected" << std::endl;
						rooms[i].clients.erase(rooms[i].clients.begin() + j);
						rooms[i].numPlayers--;
						break;
					}
				}
				std::cout << std::endl;
			}
		}
	}

	return 0;
}