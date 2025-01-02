#pragma once
#include <SFML/Graphics.hpp>

enum class Mode
{
	SPAWN = 0,
	DRAG = 1,
	DELETE = 2
};

enum class SpawnMode
{
	PARTICLE = 0,
	RECT = 1,
	CIRCLE = 2,
	POLYGON = 3
};

namespace conf
{
	int FPS = 60;
	float dt = 1.0 / FPS;
	const float G = 9.81f;

	int WIDTH = 1280, HEIGHT = 700;
	float X = 40.0f, Y = 40.0f * (1.0f * HEIGHT / WIDTH);

	float particle_radius = std::min(X, Y) / 200.0f;
	float h = std::min(X, Y) / 45.0f;										//	interaction radius
	float k = 5.0f, k_near = 8.0f * 10, k_spring = 0.0f, k_stick = 5.0f;	//	stiffness / near stiffness
	float density_rest = 10.0f;												//	rest density
	float yield_ratio = 0.2f, plasticity = 40.0f;							//	yield ratio / plasticity
	float alpha_viscosity = 4.0f, beta_viscosity = 0.0f;
	float stickness_distance = h;
	const float PI = 3.1415;

	const int START_MAX_PARTICLE_AMOUNT = 10000;

	int microseconds_passsed = 0;

	Mode mode = Mode::SPAWN;
	SpawnMode spawnMode = SpawnMode::PARTICLE;

	float polygonSpawnRadius = 0.2f;
	float rectangle_thickness = 0.8f;
	int particle_amount = 3;

	sf::Color COLOR_POLYGON_RADIUS(255, 150, 0);

	sf::RenderWindow window;
	sf::CircleShape circle(particle_radius);
	sf::Color COLOR_PARTICLE(42, 159, 223);
	sf::Color COLOR_BACKGROUND(33, 35, 33);
	sf::Color COLOR_OBJECT(255, 255, 255);
}