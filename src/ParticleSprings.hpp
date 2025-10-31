#pragma once
#include <vector>
#include <set>
#include <unordered_map>

class ParticleSprings
{
public:
	std::vector<int> keys;
	std::vector<float> arr;
	int particleAmount = 0, maxParticleAmount = conf::START_MAX_PARTICLE_AMOUNT;

	ParticleSprings()
	{
		arr.resize(maxParticleAmount * maxParticleAmount, 0.0f);
	}

	void addParticle()
	{
		particleAmount++;
		if (particleAmount <= maxParticleAmount)
			return;

		maxParticleAmount *= 2;
		std::vector<float> new_arr(maxParticleAmount * maxParticleAmount, 0.0f);
		std::vector<int> new_keys;
		new_keys.reserve(keys.size());

		for (int p : keys)
		{
			std::pair<int, int> id = std::make_pair(p / (maxParticleAmount / 2), p % (maxParticleAmount / 2));
			int new_id = getSpringId(id.first, id.second);
			new_keys.push_back(new_id);
			new_arr[new_id] = arr[p];
		}

		keys = new_keys;
		arr = new_arr;
	}

	int getSpringId(int i, int j) const
	{
		std::pair<int, int> p = std::minmax(i, j);
		return p.first * maxParticleAmount + p.second;
	}

	std::pair<int, int> reverseId(int id) const
	{
		return std::make_pair(id / maxParticleAmount, id % maxParticleAmount);
	}

	float springLen(int i, int j)
	{
		return arr[i * maxParticleAmount + j];
	}

	bool springExists(int i, int j) const
	{
		return arr[i * maxParticleAmount + j] != 0.0f;
	}

	void addSpring(int i, int j, float L)
	{
		keys.push_back(getSpringId(i, j));
		arr[i * maxParticleAmount + j] = L;
	}

	void setSpring(int i, int j, float L)
	{
		arr[i * maxParticleAmount + j] = L;
	}

	void deleteSpring(int i, int j)
	{
		arr[i * maxParticleAmount + j] = 0.0f;
	}
};