#pragma once
#include <SFML/Graphics.hpp>
#include "Simulation.hpp"
#include "Spawner.hpp"
#include "Conf.hpp"
#include "Objects.hpp"

class MouseInputHandler
{
public:
	Simulation& sim;
	bool leftButtonPressed = false, enteringObject = false;
	sf::Vector2f leftButtonPressedPos = sf::Vector2f(0.0f, 0.0f), rightButtonPressedPos = sf::Vector2f(0.0f, 0.0f);
	Spawner spawner;
	std::vector<sf::Vector2f> polygon_vertices;
	CollisionObject* drag_object = nullptr;
	sf::Vector2f prev_mouse_pos = {0.0f, 0.0f};

	MouseInputHandler(Simulation& sim) : sim(sim), spawner(sim) {}

	void handleEvent(sf::Event& event)
	{
		if (event.type == sf::Event::MouseButtonPressed)
		{
			sf::Vector2f mouse_pos = conf::window.mapPixelToCoords(sf::Mouse::getPosition(conf::window));
			mouse_pos.y = conf::Y - mouse_pos.y;

			if (event.mouseButton.button == sf::Mouse::Left)
			{
				if (conf::mode == Mode::SPAWN)
				{
					if (enteringObject)
					{
						sf::Vector2f temp = mouse_pos - leftButtonPressedPos;
						conf::COLOR_OBJECT.a = 255;
						enteringObject = false;

						if (conf::spawnMode == SpawnMode::RECT)
						{
							sim.objects.push_back(new RectangleObject(leftButtonPressedPos, mouse_pos, conf::rectangle_thickness));
						}
						else if (conf::spawnMode == SpawnMode::CIRCLE)
						{
							sim.objects.push_back(new CircleObject(leftButtonPressedPos, getLen(temp)));
						}
						else if (conf::spawnMode == SpawnMode::POLYGON)
						{
							if (polygon_vertices.size() > 2 && getLen(mouse_pos - polygon_vertices[0]) <= conf::polygonSpawnRadius)
							{
								PolygonObject* polygon = new PolygonObject();
								polygon->initiateVertices(polygon_vertices);
								sim.objects.push_back(polygon);
							}
							else
							{
								polygon_vertices.push_back(mouse_pos);
								enteringObject = true;
								leftButtonPressedPos = mouse_pos;
							}
						}
					}
					else
					{
						enteringObject = conf::spawnMode != SpawnMode::PARTICLE;

						leftButtonPressed = true;
						leftButtonPressedPos = mouse_pos;

						if (conf::spawnMode == SpawnMode::POLYGON)
						{
							polygon_vertices.clear();
							polygon_vertices.push_back(leftButtonPressedPos);
						}
					}
				}
				else if (conf::mode == Mode::DRAG)
				{
					leftButtonPressed = true;
					leftButtonPressedPos = mouse_pos;

					for (int i = 0; i < sim.objects.size(); i++)
					{
						Particle test_particle(mouse_pos);
						if (sim.objects[i]->isColliding(test_particle))
						{
							drag_object = sim.objects[i];
							break;
						}
					}
				}
				else if (conf::mode == Mode::DELETE)
				{
					leftButtonPressed = true;
					leftButtonPressedPos = mouse_pos;

					for (int i = 0; i < sim.objects.size(); i++)
					{
						Particle test_particle(mouse_pos);
						if (sim.objects[i]->isColliding(test_particle))
						{
							sim.objects.erase(sim.objects.begin() + i);
							break;
						}
					}
				}
			}
			else if (event.mouseButton.button == sf::Mouse::Right)
			{
				Particle test_particle(mouse_pos);
				for (int i = 0; i < sim.objects.size(); i++)
				{
					if (sim.objects[i]->isColliding(test_particle))
					{
						rightButtonPressedPos = mouse_pos;

					}
				}
			}
		}
		else if (event.type == sf::Event::MouseButtonReleased)
		{
			if (event.mouseButton.button == sf::Mouse::Left)
			{
				leftButtonPressed = false;
				drag_object = nullptr;
			}
		}
	}

	void update()
	{
		sf::Vector2f mouse_pos = conf::window.mapPixelToCoords(sf::Mouse::getPosition(conf::window));
		mouse_pos.y = conf::Y - mouse_pos.y;
		sf::Vector2f temp = mouse_pos - leftButtonPressedPos;


		if (conf::mode == Mode::SPAWN)
		{
			if (enteringObject)
			{
				conf::COLOR_OBJECT.a = 200;
				if (conf::spawnMode == SpawnMode::RECT)
				{
					RectangleObject rect = RectangleObject(leftButtonPressedPos, mouse_pos, conf::rectangle_thickness);
					conf::window.draw(rect);
				}
				else if (conf::spawnMode == SpawnMode::CIRCLE)
				{
					CircleObject circle = CircleObject(leftButtonPressedPos, getLen(temp));
					conf::window.draw(circle);
				}
				else if (conf::spawnMode == SpawnMode::POLYGON)
				{
					std::vector<sf::Vertex> vertices;
					for (int i = 0; i < polygon_vertices.size(); i++)
					{
						vertices.push_back(sf::Vertex(sf::Vector2f(polygon_vertices[i].x, conf::Y - polygon_vertices[i].y), conf::COLOR_OBJECT));
					}
					vertices.push_back(sf::Vertex(sf::Vector2f(mouse_pos.x, conf::Y - mouse_pos.y), conf::COLOR_OBJECT));
					conf::window.draw(&vertices[0], vertices.size(), sf::LinesStrip);

					if (polygon_vertices.size() > 2)
					{
						sf::CircleShape polygonCircle(conf::polygonSpawnRadius);
						polygonCircle.setOrigin(conf::polygonSpawnRadius, conf::polygonSpawnRadius);

						polygonCircle.setPosition(vertices[0].position);
						polygonCircle.setFillColor(conf::COLOR_POLYGON_RADIUS);

						if (getLen(sf::Vector2f(mouse_pos.x, conf::Y - mouse_pos.y) - vertices[0].position) < conf::polygonSpawnRadius)
						{
							polygonCircle.setScale(sf::Vector2f(1.5f, 1.5f));
						}

						conf::window.draw(polygonCircle);
					}
				}
			}
		}
		else if (conf::mode == Mode::DRAG)
		{
			if (drag_object != nullptr)
			{
				sf::Vector2f to_move = mouse_pos - prev_mouse_pos;
				drag_object->move(to_move);
			}
		}
		else if (conf::mode == Mode::DELETE)
		{

		}


		if (leftButtonPressed)
		{
			if (conf::mode == Mode::SPAWN)
			{
				if (conf::spawnMode == SpawnMode::PARTICLE)
				{
					Particle p(mouse_pos);
					p.v = -temp * 2.0f;
					spawner.spawnLine(p, conf::particle_radius * 2.5f);
				}
			}
			else if (conf::mode == Mode::DRAG)
			{

			}
			else if (conf::mode == Mode::DELETE)
			{

			}
		}

		prev_mouse_pos = mouse_pos;
	}
};