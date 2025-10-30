#pragma once
#include <array>
#include <set>
#include <unordered_map>
#include <SFML/Graphics.hpp>
#include "Particle.hpp"
#include <iostream>

class ParticleGrid
{
public:
	int N, M, particleAmount = 0, maxParticleAmount = conf::START_MAX_PARTICLE_AMOUNT;
	std::vector<std::vector<std::vector<int>>> grid;
	std::vector<sf::Vector2i> key_to_tile;
	sf::Vector2f SIZE, SIZE_PER_TILE;

	ParticleGrid(int N, int M, sf::Vector2f SIZE) : N(N), M(M), SIZE(SIZE)
	{
		SIZE_PER_TILE = sf::Vector2f(SIZE.x / M, SIZE.y / N);
		grid.resize(N, std::vector<std::vector<int>>(M));
		key_to_tile.resize(maxParticleAmount, { -1, -1 });
	}

	sf::Vector2i getTile(const Particle& p) const
	{
		return sf::Vector2i(p.pos.x / SIZE_PER_TILE.x, p.pos.y / SIZE_PER_TILE.y);
	}

	sf::Vector2i getKeyTile(int key) const
	{
		return key_to_tile[key];
	}

	void addParticle(const Particle& p, int key)
	{
		particleAmount++;
		if (particleAmount > maxParticleAmount)
		{
			maxParticleAmount *= 2;
			key_to_tile.resize(maxParticleAmount, { -1, -1 });
		}
		sf::Vector2i tile = getTile(p);
		grid[tile.y][tile.x].push_back(key);
		key_to_tile[key] = tile;
	}

	void deleteParticle(int key)
	{
		if (key_to_tile[key].x == -1)
			return;
		sf::Vector2i tile = key_to_tile[key];
		grid[tile.y][tile.x].erase(std::find(grid[tile.y][tile.x].begin(), grid[tile.y][tile.x].end(), key));
		key_to_tile[key] = { -1, -1 };
	}
};