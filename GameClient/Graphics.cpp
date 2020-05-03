#include "Graphics.h"
#include <iostream>


Graphics::Graphics()
{
	Sala _paret(" ", 0, 0, 40, 30, sf::Color(150, 150, 150));
	////Sala salaBillar("Sala de billar", 12, 0, 6, 10, sf::Color::Red);
	////Sala biblioteca("Biblioteca", 22, 0, 6, 10, sf::Color::Cyan);
	////Sala estudio("Estudio", 32, 0, 8, 10, sf::Color::Magenta);
	////Sala salaBaile("Sala de baile", 0, 12, 8, 6, sf::Color::Blue);
	////Sala vestibulo("vestibulo", 30, 12, 10, 8, sf::Color::White);
	////Sala cocina("cocina", 0, 20, 10, 10, sf::Color::Green);
	////Sala comedor("comedor", 13, 20, 13, 10, sf::Color(150,0,150));
	////Sala salon("salon", 30, 22, 10, 8, sf::Color(0,150,150));
	//salas[0] = invernadero;
	///*salas[1] = salaBillar;
	//salas[2] = biblioteca;
	//salas[3] = estudio;
	//salas[4] = salaBaile;
	//salas[5] = vestibulo;
	//salas[6] = cocina;
	//salas[7] = comedor;
	//salas[8] = salon;*/

	paret = _paret;

	salaInterior.color = sf::Color(45, 45, 45);
	salaInterior.origen.x = 2;
	salaInterior.origen.y = 2;
	salaInterior.longitud.x = 36;
	salaInterior.longitud.y = 26;
}


Graphics::~Graphics()
{
}
