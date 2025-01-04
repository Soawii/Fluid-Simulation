#pragma once
#include "Conf.hpp"
#include "Particle.hpp"
#include "ParticleGrid.hpp"
#include "ParticleSprings.hpp"
#include "Util.hpp"
#include "Objects.hpp"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <omp.h>
#include <algorithm>
#include <random>
#include <mutex>

class Simulation : public sf::Drawable
{
public:
	int n = 0;
	std::vector<sf::CircleShape*> circles;
	std::vector<Particle> particles;
	std::vector<CollisionObject*> objects;
	ParticleGrid grid = ParticleGrid(10, 10, sf::Vector2f(conf::X, conf::Y));
	ParticleSprings springs = ParticleSprings();

	Simulation()
	{
		createGrid();

		const float i_max = 40;
		const float j_max = 40;

		const float X_START = conf::X * 0.4;
		const float Y_START = conf::Y * 0.8;
		const float space_between = 2.0f * conf::particle_radius;

		for (int i = 0; i < i_max; i++) {
			for (int j = 0; j < j_max; j++) {
				addParticle(Particle(sf::Vector2f(X_START + j * space_between, Y_START - i * space_between)));
			}
		}

		springs = ParticleSprings();
	}

	void deleteWater()
	{
		particles.clear();
		createGrid();
		springs = ParticleSprings();

		for (int i = 0; i < circles.size(); i++)
		{
			delete circles[i];
		}
		circles.clear();
	}

	void createGrid()
	{
		int GRID_SIZE_X = ceil(conf::X / conf::h), GRID_SIZE_Y = ceil(conf::Y / conf::h);
		grid = ParticleGrid(GRID_SIZE_Y, GRID_SIZE_X, sf::Vector2f(conf::X + 0.0001, conf::Y + 0.0001));

		for (int i = 0; i < particles.size(); i++)
		{
			grid.addParticle(particles[i], i);
		}
	}

	void addParticle(Particle p)
	{
		if (p.pos.x < 0 || p.pos.y < 0 || p.pos.x >= conf::X || p.pos.y >= conf::Y)
			return;
		particles.emplace_back(p);
		grid.addParticle(particles.back(), particles.size() - 1);
		springs.addParticle();

		sf::CircleShape* circle = new sf::CircleShape(conf::particle_radius, 9);
		circle->setOrigin(conf::particle_radius, conf::particle_radius);
		circle->setFillColor(sf::Color(conf::COLOR_PARTICLE.r + (rand() % 21 - 10), conf::COLOR_PARTICLE.g + (rand() % 21 - 10), conf::COLOR_PARTICLE.b + (rand() % 21 - 10)));
		circles.push_back(circle);
	}

	void update(float dt)
	{
		sf::Clock clock;

		applyGravity(dt);
		float gravity = clock.restart().asMicroseconds();

		applyViscosity(dt);
		float viscosity = clock.restart().asMicroseconds();

		applyVelocities(dt);
		float velocity = clock.restart().asMicroseconds();

		adjustStrings(dt);
		float adjust_strings = clock.restart().asMicroseconds();
		applyStrings(dt);
		float apply_strings = clock.restart().asMicroseconds();

		doubleDensityRelaxation(dt);
		float relaxation = clock.restart().asMicroseconds();

		handleStickiness(dt);
		float stickiness = clock.restart().asMicroseconds();

		applyCollisions();
		float collisions = clock.restart().asMicroseconds();

		checkBounds();
		updateGrid();
		float bounds_update = clock.restart().asMicroseconds();

		for (Particle& p : particles)
		{
			p.v = (p.pos - p.prev_pos) / dt;
		}

		/*std::cout << "viscosity " << viscosity / 1000.0f << ", relaxation " << relaxation / 1000.0f
			<< ", adjust strings " << (adjust_strings) / 1000.0f 
			<< ", apply strings " << (apply_strings) / 1000.0f <<  ", collisions " << collisions / 1000.0f
			<< ", stickiness " << (stickiness) / 1000.0f << ", other " << (gravity + velocity + bounds_update) / 1000.0f << std::endl;*/
	}

	void handleStickiness(float dt)
	{
		const sf::Vector2f NULL_VECTOR = { 0.0f, 0.0f };
		const int PARTICLES_SIZE = particles.size();
		const int OBJECTS_SIZE = objects.size();

		#pragma omp parallel for
		for (int i = 0; i < PARTICLES_SIZE; i++)
		{
			for (int j = 0; j < OBJECTS_SIZE; j++)
			{
				sf::Vector2f nearest_vector = objects[j]->getNearestVector(particles[i]);
				if (nearest_vector != NULL_VECTOR)
				{
					const float len = getLen(nearest_vector);
					const float sticky_term = dt * conf::k_stick * len * (1 - len / conf::stickness_distance) * -1;
					nearest_vector = nearest_vector / len * sticky_term;
					particles[i].pos += nearest_vector;
				}
			}
		}
	}

	void applyCollisions()
	{
		#pragma omp parallel for
		for (int i = 0; i < particles.size(); i++)
		{
			for (int j = 0; j < objects.size(); j++)
			{
				objects[j]->handleCollision(particles[i]);
			}
		}
	}

	void applyGravity(float dt)
	{
		for (Particle& p : particles)
		{
			p.v.y -= conf::G * dt;
		}
	}

	void doubleDensityRelaxation(float dt)
	{
		std::vector<std::pair<int, int>> ijs = { {0, 0}, {0, 1}, {0, 2}, {1, 0}, {1, 1}, {1, 2}, {2, 0}, {2, 1}, {2, 2} };
		std::random_device rd;
		std::mt19937 g(rd());

		std::shuffle(ijs.begin(), ijs.end(), g);

		for (int ij = 0; ij < 9; ij++)
		{
			const int start_i = ijs[ij].first, start_j = ijs[ij].second;
			std::vector<sf::Vector2i> tiles_to_check;
			tiles_to_check.reserve(10);
			const int N = grid.N, M = grid.M;
			for (int i = start_i; i < N; i += 3)
			{
				for (int j = start_j; j < M; j += 3)
				{
					if (!grid.grid[i][j].empty())
						tiles_to_check.push_back(sf::Vector2i(i, j));
				}
			}
			std::shuffle(tiles_to_check.begin(), tiles_to_check.end(), g);

			const int SIZE = tiles_to_check.size();

			#pragma omp parallel for
			for (int t = 0; t < SIZE; t++)
			{
				const int CURRENT_I = tiles_to_check[t].x, CURRENT_J = tiles_to_check[t].y;
				for (int i : grid.grid[CURRENT_I][CURRENT_J])
				{
					Particle& p = particles[i];
					float density = 0, density_near = 0;

					sf::Vector2i tile = grid.getKeyTile(i);
					for (int di = -1; di <= 1; di++)
					{
						for (int dj = -1; dj <= 1; dj++)
						{
							const int new_i = tile.y + di, new_j = tile.x + dj;
							if (new_i < 0 || new_i >= grid.N || new_j < 0 || new_j >= grid.M)
								continue;
							for (int neighbour_key : grid.grid[new_i][new_j])
							{
								if (neighbour_key == i)
									continue;
								Particle& neighbour = particles[neighbour_key];
								float q = p.distanceTo(neighbour) / conf::h;
								if (q < 1)
								{
									float temp = (1 - q) * (1 - q);
									density += temp;
									density_near += temp * (1 - q);
								}
							}
						}
					}

					const float P = conf::k * (density - conf::density_rest);
					const float P_near = conf::k_near * density_near;

					sf::Vector2f dx(0.0f, 0.0f);

					for (int di = -1; di <= 1; di++)
					{
						for (int dj = -1; dj <= 1; dj++)
						{
							const int new_i = tile.y + di, new_j = tile.x + dj;
							if (new_i < 0 || new_i >= grid.N || new_j < 0 || new_j >= grid.M)
								continue;
							for (int neighbour_key : grid.grid[new_i][new_j])
							{
								if (neighbour_key == i)
									continue;
								Particle& neighbour = particles[neighbour_key];
								float q = p.distanceTo(neighbour) / conf::h;
								if (q < 1)
								{
									float D = (dt * dt) * (1 - q) * (P + P_near * (1 - q));
									sf::Vector2f r_ij = p.distanceVectorTo(neighbour);
									float r_ij_len = getLen(r_ij);
									sf::Vector2f r_ij_unit = r_ij / r_ij_len;
									if (r_ij_len == 0.0f)
									{
										float angle = randFloat(0.0f, 360.0f) * conf::PI / 180.0f;
										r_ij_unit = sf::Vector2f(cos(angle), sin(angle));
									}
									sf::Vector2f D_vec = D * r_ij_unit / 2.0f;

									neighbour.pos += D_vec;
									dx -= D_vec;
								}
							}
						}
					}

					p.pos += dx;
				}
			}
		}
	}

	void adjustStrings(float dt)
	{
		if (conf::k_spring == 0.0f)
			return;

		const int N = particles.size();

		std::mutex m;

		#pragma omp parallel for
		for (int i = 0; i < N; i++)
		{
			const Particle& const p = particles[i];

			sf::Vector2i tile = grid.getKeyTile(i);
			std::vector<int> to_add;
			to_add.reserve(8);

			for (int di = -1; di <= 1; di++)
			{
				for (int dj = -1; dj <= 1; dj++)
				{
					const int new_i = tile.y + di, new_j = tile.x + dj;
					if (new_i < 0 || new_i >= grid.N || new_j < 0 || new_j >= grid.M)
						continue;
					for (const int neighbour_key : grid.grid[new_i][new_j])
					{
						if (neighbour_key <= i || springs.springExists(i, neighbour_key))
							continue;
						const Particle& const neighbour = particles[neighbour_key];
						const float r = p.distanceTo(neighbour);
						const float q = r / conf::h;

						if (q < 1)
						{
							to_add.push_back(neighbour_key);
						}
					}
				}
			}

			while (!m.try_lock()) {}
			for (int j = 0; j < to_add.size(); j++)
				springs.addSpring(i, to_add[j], conf::h);
			m.unlock();
		}

		const int KEYS_SIZE = (int)springs.keys.size();
		const int CHUNK_SIZE = 400;
		const int LOOP_SIZE = KEYS_SIZE / CHUNK_SIZE * CHUNK_SIZE;

		int deleted = 0, set = 0;

		std::vector<float> Ls(KEYS_SIZE, 0.0f);

		#pragma omp parallel for
		for (int j = 0; j < LOOP_SIZE; j += CHUNK_SIZE)
		{
			const int k_end = j + CHUNK_SIZE;
			for (int k = j; k < k_end; k++)
			{
				const std::pair<int, int> id = springs.reverseId(springs.keys[k]);
				const std::pair<int, float> p = { springs.keys[k], springs.arr[springs.keys[k]] };
				const float r = particles[id.first].distanceTo(particles[id.second]);
				float L_ij = p.second;
				const float d = conf::yield_ratio * L_ij;

				Ls[k] = L_ij;
				if (r > L_ij + d)
				{
					Ls[k] += dt * conf::plasticity * (r - L_ij - d);
				}
				else if (r < L_ij - d)
				{
					Ls[k] -= dt * conf::plasticity * (L_ij - d - r);
				}
			}
		}

		for (int j = LOOP_SIZE; j < KEYS_SIZE; j++)
		{
			const std::pair<int, int> id = springs.reverseId(springs.keys[j]);
			const std::pair<int, float> p = { springs.keys[j], springs.arr[springs.keys[j]] };
			const float r = particles[id.first].distanceTo(particles[id.second]);
			float L_ij = p.second;
			const float d = conf::yield_ratio * L_ij;

			Ls[j] = L_ij;

			if (r > L_ij + d)
			{
				Ls[j] += dt * conf::plasticity * (r - L_ij - d);
			}
			else if (r < L_ij - d)
			{
				Ls[j] -= dt * conf::plasticity * (L_ij - d - r);
			}
		}

		for (int j = 0; j < KEYS_SIZE; j++)
		{
			springs.arr[springs.keys[j]] = Ls[j];
		}

		int idx = KEYS_SIZE - 1;
		for (int j = 0; j <= idx; j++)
		{
			if (Ls[j] > conf::h)
			{
				std::swap(springs.keys[j], springs.keys[idx]);
				std::swap(Ls[j], Ls[idx]);
				j--;
				idx--;
			}
		}

		for (int j = KEYS_SIZE - 1; j > idx; j--)
		{
			springs.arr[springs.keys[j]] = 0.0f;
			springs.keys.pop_back();
		}
	}

	void applyStrings(float dt)
	{
		if (conf::k_spring == 0.0f)
			return;

		const int KEYS_SIZE = springs.keys.size();
		constexpr int BATCH_SIZE = 300;
		const int LOOP_SIZE = KEYS_SIZE / BATCH_SIZE * BATCH_SIZE;

		std::mutex m;

		#pragma omp parallel for
		for (int i = 0; i < LOOP_SIZE; i += BATCH_SIZE)
		{
			const int j_end = i + BATCH_SIZE;
			std::array<sf::Vector2f, BATCH_SIZE> Ds;

			for (int j = i; j < j_end; j++)
			{
				const std::pair<int, int> id = springs.reverseId(springs.keys[j]);
				const std::pair<int, float> p = { springs.keys[j], springs.arr[springs.keys[j]] };
				const float r = particles[id.first].distanceTo(particles[id.second]);
				const sf::Vector2f r_ij = particles[id.first].distanceVectorTo(particles[id.second]);
				sf::Vector2f r_ij_unit = r_ij / r;
				if (r == 0)
				{
					float angle = randFloat(0.0f, 360.0f) * conf::PI / 180.0f;
					r_ij_unit = sf::Vector2f(cos(angle), sin(angle));
				}
				const float len = p.second;
				const sf::Vector2f D = (dt * dt) * conf::k_spring * (1 - len / conf::h) * (len - r) * r_ij_unit;
				Ds[j - i] = D / 2.0f;
			}

			while (!m.try_lock())
				continue;
			for (int j = i; j < j_end; j++)
			{
				const std::pair<int, int> id = springs.reverseId(springs.keys[j]);
				particles[id.first].pos -= Ds[j - i];
				particles[id.second].pos += Ds[j - i];
			}
			m.unlock();
		}

		for (int j = LOOP_SIZE; j < KEYS_SIZE; j++)
		{
			const std::pair<int, int> id = springs.reverseId(springs.keys[j]);
			const std::pair<int, float> p = { springs.keys[j], springs.arr[springs.keys[j]] };
			const float r = particles[id.first].distanceTo(particles[id.second]);
			const sf::Vector2f r_ij = particles[id.first].distanceVectorTo(particles[id.second]);
			sf::Vector2f r_ij_unit = r_ij / r;
			if (r == 0)
			{
				float angle = randFloat(0.0f, 360.0f) * conf::PI / 180.0f;
				r_ij_unit = sf::Vector2f(cos(angle), sin(angle));
			}
			const float len = p.second;
			const sf::Vector2f D = (dt * dt) * conf::k_spring * (1 - len / conf::h) * (len - r) * r_ij_unit;
			particles[id.first].pos -= D / 2.0f;
			particles[id.second].pos += D / 2.0f;
		}
	}


	void applyViscosity(float dt)
	{
		if (conf::alpha_viscosity == 0.0 && conf::beta_viscosity == 0.0f)
			return;

		std::vector<std::pair<int, int>> ijs = { {0, 0}, {0, 1}, {0, 2}, {1, 0}, {1, 1}, {1, 2}, {2, 0}, {2, 1}, {2, 2} };
		std::random_device rd;
		std::mt19937 g(rd());

		std::shuffle(ijs.begin(), ijs.end(), g);

		for (int ij = 0; ij < 9; ij++)
		{
			const int start_i = ijs[ij].first, start_j = ijs[ij].second;
			std::vector<sf::Vector2i> tiles_to_check;
			int N = grid.N, M = grid.M;
			for (int i = start_i; i < N; i += 3)
			{
				for (int j = start_j; j < M; j += 3)
				{
					if (!grid.grid[i][j].empty())
						tiles_to_check.push_back(sf::Vector2i(i, j));
				}
			}
			std::shuffle(tiles_to_check.begin(), tiles_to_check.end(), g);

			const int SIZE = tiles_to_check.size();

			#pragma omp parallel for
			for (int t = 0; t < SIZE; t++)
			{
				const int CURRENT_I = tiles_to_check[t].x, CURRENT_J = tiles_to_check[t].y;
				for (int i : grid.grid[CURRENT_I][CURRENT_J])
				{
					Particle& p = particles[i];

					sf::Vector2i tile = grid.getKeyTile(i);
					for (int di = -1; di <= 1; di++)
					{
						for (int dj = -1; dj <= 1; dj++)
						{
							const int new_i = tile.y + di, new_j = tile.x + dj;
							if (new_i < 0 || new_i >= grid.N || new_j < 0 || new_j >= grid.M)
								continue;
							for (int neighbour_key : grid.grid[new_i][new_j])
							{
								if (neighbour_key <= i)
									continue;
								Particle& neighbour = particles[neighbour_key];
								float r = p.distanceTo(neighbour);
								float q = r / conf::h;
								if (q < 1 && q > 0)
								{
									sf::Vector2f r_ij = p.distanceVectorTo(neighbour);
									sf::Vector2f r_ij_unit = r_ij / r;
									if (r == 0)
									{
										float angle = randFloat(0.0f, 360.0f) * conf::PI / 180.0f;
										r_ij_unit = sf::Vector2f(cos(angle), sin(angle));
									}

									sf::Vector2f dv = p.v - neighbour.v;
									float u = dv.x * r_ij_unit.x + dv.y * r_ij_unit.y;
									if (u > 0)
									{
										sf::Vector2f I = dt * (1 - q) * (conf::alpha_viscosity * u + conf::beta_viscosity * u * u) * r_ij_unit;
										p.v -= I / 2.0f;
										neighbour.v += I / 2.0f;
									}
								}
							}
						}
					}
				}
			}
		}
	}


	void applyVelocities(const float dt)
	{
		for (Particle& p : particles)
		{
			p.prev_pos = p.pos;
			p.pos += p.v * dt;
		}
	}

	void checkBounds()
	{
		for (Particle& p : particles)
		{
			p.checkBounds();
		}
	}

	void updateGrid()
	{
		for (size_t i = 0; i < particles.size(); i++)
		{
			if (grid.getTile(particles[i]) == grid.key_to_tile[i])
				continue;
			grid.deleteParticle(i);
			grid.addParticle(particles[i], i);
		}
	}

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override
	{
		for (int i = 0; i < particles.size(); i++)
		{
			circles[i]->setPosition({ particles[i].pos.x, conf::Y - particles[i].pos.y });
			target.draw(*circles[i]);
		}

		for (CollisionObject* object : objects)
		{
			target.draw(*object);
		}
	}
};