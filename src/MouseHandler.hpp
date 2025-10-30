#pragma once
#include <SFML/Graphics.hpp>
#include "Simulation.hpp"
#include "Spawner.hpp"
#include "Conf.hpp"
#include "Objects.hpp"

class MouseInputHandler : public sf::Drawable
{
public:
	Simulation& sim;
	bool leftButtonPressed = false, enteringObject = false;
	sf::Vector2f leftButtonPressedPos = sf::Vector2f(0.0f, 0.0f), rightButtonPressedPos = sf::Vector2f(0.0f, 0.0f);
	Spawner spawner;
	std::vector<sf::Vector2f> polygon_vertices;
	CollisionObject* drag_object = nullptr;
	sf::Vector2f prev_mouse_pos = {0.0f, 0.0f};

	std::vector<sf::CircleShape*> rect_circles;

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
							for (int i = 0; i < rect_circles.size(); i++)
							{
								if (getLen(rect_circles[i]->getPosition() - mouse_pos) <= conf::polygonSpawnRadius / 1.3f)
								{
									mouse_pos = rect_circles[i]->getPosition();
									break;
								}
							}
							sim.objects.push_back(new RectangleObject(leftButtonPressedPos, mouse_pos, conf::rectangle_thickness));
						}
						else if (conf::spawnMode == SpawnMode::CIRCLE)
						{
							sim.objects.push_back(new CircleObject(leftButtonPressedPos, getLen(temp)));
						}
						else if (conf::spawnMode == SpawnMode::POLYGON)
						{

							bool polygon_good = true;
							if (polygon_vertices.size() > 2)
							{
								if (polygon_vertices.size() > 2 && getLen(mouse_pos - polygon_vertices[0]) <= conf::polygonSpawnRadius) {}
								else
									polygon_vertices.push_back(mouse_pos);

								PolygonObject polygon;
								polygon.initiateVertices(polygon_vertices);
								for (int i = 0; i < polygon_vertices.size(); i++)
								{
									Particle temp(polygon_vertices[i]);
									if (!polygon.isColliding(temp))
									{
										polygon_good = false;
										break;
									}
								}

								if (polygon_vertices.size() > 2 && getLen(mouse_pos - polygon_vertices[0]) <= conf::polygonSpawnRadius) {}
								else
									polygon_vertices.pop_back();
							}

							if (polygon_vertices.size() > 2 && getLen(mouse_pos - polygon_vertices[0]) <= conf::polygonSpawnRadius && polygon_good)
							{
								PolygonObject* polygon = new PolygonObject();
								polygon->initiateVertices(polygon_vertices);
								sim.objects.push_back(polygon);
								polygon_vertices.clear();
							}
							else
							{
								if (polygon_good)
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
						else if (conf::spawnMode == SpawnMode::RECT)
						{
							for (int i = 0; i < rect_circles.size(); i++)
							{
								if (getLen(rect_circles[i]->getPosition() - mouse_pos) <= conf::polygonSpawnRadius / 1.3f)
								{
									leftButtonPressedPos = rect_circles[i]->getPosition();
									break;
								}
							}
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

			if (conf::spawnMode == SpawnMode::RECT)
			{
				int rect_count = 0;
				for (int i = 0; i < sim.objects.size(); i++)
				{
					if (sim.objects[i]->type == ObjectType::RECT)
						rect_count++;
				}

				for (int i = 0; i < rect_circles.size(); i++)
					delete rect_circles[i];
				rect_circles.clear();
				for (int i = 0; i < sim.objects.size(); i++)
				{
					if (sim.objects[i]->type != ObjectType::RECT)
						continue;
					RectangleObject* rect = (RectangleObject*)sim.objects[i];
					sf::Vector2f vec1 = rect->vertices[1] - rect->vertices[0];
					sf::Vector2f vec2 = rect->vertices[3] - rect->vertices[2];
					vec1 /= getLen(vec1);
					vec2 /= getLen(vec2);

					sf::CircleShape* circles[4] = { new sf::CircleShape(conf::polygonSpawnRadius / 1.3f, 9),
						new sf::CircleShape(conf::polygonSpawnRadius / 1.3f, 9),
						new sf::CircleShape(conf::polygonSpawnRadius / 1.3f, 9),
						new sf::CircleShape(conf::polygonSpawnRadius / 1.3f, 9) };

					circles[0]->setPosition(rect->vertices[0] + vec1 * conf::rectangle_thickness / 2.0f - rect->normals[0] * 0.05f);
					circles[1]->setPosition(rect->vertices[1] - vec1 * conf::rectangle_thickness / 2.0f - rect->normals[0] * 0.05f);
					circles[2]->setPosition(rect->vertices[2] + vec2 * conf::rectangle_thickness / 2.0f - rect->normals[2] * 0.05f);
					circles[3]->setPosition(rect->vertices[3] - vec2 * conf::rectangle_thickness / 2.0f - rect->normals[2] * 0.05f);

					for (int j = 0; j < 4; j++)
					{
						circles[j]->setOrigin(conf::polygonSpawnRadius / 1.3f, conf::polygonSpawnRadius / 1.3f);
						circles[j]->setFillColor(conf::COLOR_POLYGON_RADIUS);
						rect_circles.push_back(circles[j]);
					}
				}

				for (int i = 0; i < rect_circles.size(); i++)
				{
					if (getLen(rect_circles[i]->getPosition() - mouse_pos) <= conf::polygonSpawnRadius / 1.3f)
					{
						rect_circles[i]->setScale(1.5f, 1.5f);
					}
					else
					{
						rect_circles[i]->setScale(1.0f, 1.0f);
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

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override
	{
		if (conf::mode == Mode::SPAWN)
		{
			if (conf::spawnMode == SpawnMode::RECT)
			{
				for (int i = 0; i < rect_circles.size(); i++)
				{
					sf::Vector2f pos = rect_circles[i]->getPosition();
					rect_circles[i]->setPosition({ pos.x, conf::Y - pos.y });
					conf::window.draw(*rect_circles[i]);
					rect_circles[i]->setPosition(pos);
				}
			}
		}
	}
};