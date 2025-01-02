#include <SFML/Graphics.hpp>
#include "Simulation.hpp"
#include "Conf.hpp"
#include "MouseHandler.hpp"
#include "Menu.hpp"

int main() 
{
	srand(time(NULL));

	conf::window.create(sf::VideoMode(conf::WIDTH, conf::HEIGHT), "Fluid Simulation", sf::Style::Titlebar | sf::Style::Close);
	conf::window.setFramerateLimit(conf::FPS);
	conf::window.setView(sf::View(sf::FloatRect(0, 0, conf::X, conf::Y)));

	conf::circle.setRadius(conf::particle_radius);
	conf::circle.setFillColor(conf::COLOR_PARTICLE);
	conf::circle.setOrigin(sf::Vector2f(conf::particle_radius, conf::particle_radius));
	conf::circle.setPointCount(9);
	
	Simulation sim;
	MouseInputHandler mouse_handler(sim);
	Menu menu(sim);
	sf::Clock clock;

	while (conf::window.isOpen())
	{
		sf::Event event;
		while (conf::window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				conf::window.close();
			}
			menu.processEvent(event);
			if (!menu.handler.interacted_this_frame)
				mouse_handler.handleEvent(event);
		}

		conf::window.clear(conf::COLOR_BACKGROUND);

		mouse_handler.update();

		menu.update(sf::Mouse::getPosition(conf::window));

		sim.update(conf::dt);

		conf::window.draw(sim);
		conf::window.draw(menu);
		conf::window.draw(mouse_handler);

		conf::window.display();
	}
}