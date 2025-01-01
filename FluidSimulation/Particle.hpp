#pragma once
#include <SFML/Graphics.hpp>
#include "Conf.hpp"
#include "Util.hpp"

class alignas(32) Particle
{
public:
	sf::Vector2f pos, prev_pos, v;

	Particle(sf::Vector2f pos) : pos(pos), prev_pos(pos), v(0.0f, 0.0f)
	{
	}

	sf::Vector2f distanceVectorTo(const Particle& other) const
	{
		return other.pos - pos;
	}

	float distanceTo(const Particle& other) const
	{
		return getLen(other.pos - pos);
	}

	void checkBounds()
	{
		if (pos.x < 0)
		{
			pos.x = 0;
			prev_pos.x = 0;
		}
		if (pos.x > conf::X)
		{
			pos.x = conf::X;
			prev_pos.x = conf::X;
		}
		if (pos.y < 0)
		{
			pos.y = 0;
			prev_pos.y = 0;
		}
		if (pos.y > conf::Y)
		{
			pos.y = conf::Y;
			prev_pos.y = conf::Y;
		}
	}
};