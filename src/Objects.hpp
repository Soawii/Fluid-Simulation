#pragma once

#include "Particle.hpp"
#include "Conf.hpp"
#include <SFML/Graphics.hpp>
#include <vector>

enum class ObjectType
{
	CIRCLE = 0,
	RECT = 1,
	POLYGON = 2
};

class CollisionObject : public sf::Drawable
{
public:
	ObjectType type;
	virtual bool isColliding(const Particle& p) const = 0;
	virtual void handleCollision(Particle& p) const = 0;
	virtual void move(sf::Vector2f to_move) = 0;
	virtual sf::Vector2f getNearestVector(const Particle& p) const = 0;
};

class PolygonObject : public CollisionObject
{
public:
	std::vector<sf::Vector2f> vertices;
	std::vector<sf::Vector2f> normals;
	std::vector<sf::Vertex> drawable_vertices;
	sf::Vector2f min, max;

	PolygonObject()
	{
		type = ObjectType::POLYGON;
	}

	void initiateVertices(std::vector<sf::Vector2f> vertices_)
	{
		vertices = vertices_;

		drawable_vertices.resize(vertices.size());
		for (int i = 0; i < vertices.size(); i++)
		{
			drawable_vertices[i] = sf::Vertex(sf::Vector2f(vertices[i].x, conf::Y - vertices[i].y), conf::COLOR_OBJECT);
			min.x = std::min(min.x, vertices_[i].x);
			min.y = std::min(min.y, vertices_[i].y);
			max.x = std::max(max.x, vertices_[i].x);
			max.y = std::max(max.y, vertices_[i].y);
		}

		calculateNormals();
	}

	void calculateNormals()
	{
		normals.resize(vertices.size());
		for (int i = 1; i < vertices.size(); i++)
		{
			sf::Vector2f temp = vertices[i] - vertices[i - 1];
			normals[i - 1] = sf::Vector2f(-temp.y, temp.x);
		}
		sf::Vector2f temp = vertices[0] - vertices.back();
		normals.back() = sf::Vector2f(-temp.y, temp.x);

		for (int i = 0; i < normals.size(); i++)
			normals[i] /= getLen(normals[i]);
	}

	bool isColliding(const Particle& p) const override
	{
		if (p.pos.x < min.x || p.pos.y < min.y || p.pos.x > max.x || p.pos.y > max.y)
			return false;
		const sf::Vector2f position = p.pos;
		
		bool all_positive = true, all_negative = true;
		for (int i = 0; i < vertices.size(); i++)
		{
			const sf::Vector2f vert_to_point = position - vertices[i];
			const float dot = vert_to_point.x * normals[i].x + vert_to_point.y * normals[i].y;
			all_positive &= (dot >= 0);
			all_negative &= (dot <= 0);
			if (!all_positive && !all_negative)
				return false;
		}
		return true;
	}

	void handleCollision(Particle& p) const override
	{
		if (p.pos.x < min.x || p.pos.y < min.y || p.pos.x > max.x || p.pos.y > max.y)
			return;

		const sf::Vector2f position = p.pos;
		float smallestDot = 1000000000.0f;
		sf::Vector2f bestNormal;

		bool all_positive = true, all_negative = true;
		for (int i = 0; i < vertices.size(); i++)
		{
			const sf::Vector2f vert_to_point = position - vertices[i];
			const float dot = vert_to_point.x * normals[i].x + vert_to_point.y * normals[i].y;
			all_positive &= (dot >= 0);
			all_negative &= (dot <= 0);
			if (!all_positive && !all_negative)
				return;
			if (abs(dot) < abs(smallestDot))
			{
				smallestDot = dot;
				bestNormal = normals[i];
			}
		}
		p.pos += bestNormal * -1.0f * smallestDot;
	}

	sf::Vector2f getNearestVector(const Particle& p) const override
	{
		if (p.pos.x < min.x - conf::stickness_distance || p.pos.y < min.y - conf::stickness_distance
			|| p.pos.x > max.x + conf::stickness_distance || p.pos.y > max.y + conf::stickness_distance)
			return { 0.0f, 0.0f };

		sf::Vector2f best_normal = { 0.0f, 0.0f };
		float smallest = 1000000000.0f;

		for (int i = 0; i < vertices.size(); i++)
		{
			sf::Vector2f edge = vertices[(i + 1) % vertices.size()] - vertices[i];
			const float len = getLen(edge);
			edge /= len;
			sf::Vector2f vertex_to_particle = p.pos - vertices[i];
			const float dot = edge.x * vertex_to_particle.x + edge.y * vertex_to_particle.y;

			if (dot > 0 && dot < len)
			{
				const float distance = vertex_to_particle.x * normals[i].x + vertex_to_particle.y * normals[i].y;
				if (distance < smallest && distance > 0 && distance < conf::stickness_distance)
				{
					best_normal = normals[i];
					smallest = distance;
				}
			}
		}

		return best_normal * smallest;
	}

	void move(sf::Vector2f to_move) override
	{
		for (int i = 0; i < vertices.size(); i++)
		{
			vertices[i] += to_move;
		}
		initiateVertices(vertices);
	}

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override
	{
		target.draw(&drawable_vertices[0], drawable_vertices.size(), sf::PrimitiveType::TriangleFan);
	}
};

class RectangleObject : public PolygonObject
{
public:
	RectangleObject(sf::Vector2f start, sf::Vector2f end, float thickness)
	{
		type = ObjectType::RECT;
		sf::Vector2f p = end - start;
		sf::Vector2f p_n(-p.y, p.x);
		p_n = p_n / getLen(p_n) * thickness / 2.0f;

		initiateVertices(std::vector<sf::Vector2f>{ start + p_n, end + p_n, end - p_n, start - p_n });
	}
};

class CircleObject : public CollisionObject
{
public:
	sf::Vector2f position;
	float radius;
	sf::CircleShape* drawable_circle;

	CircleObject(sf::Vector2f position, float radius) : position(position), radius(radius)
	{
		type = ObjectType::CIRCLE;
		drawable_circle = new sf::CircleShape(radius);
		drawable_circle->setOrigin(radius, radius);
		drawable_circle->setFillColor(conf::COLOR_OBJECT);
		drawable_circle->setPosition(sf::Vector2f(position.x, conf::Y - position.y));
		drawable_circle->setPointCount(100);
	}

	bool isColliding(const Particle& p) const override
	{
		return getLen(p.pos - position) <= radius;
	}

	void handleCollision(Particle& p) const override
	{
		sf::Vector2f direction = p.pos - position;
		const float length = getLen(direction);
		if (length > radius)
			return;
		const float penetration = radius - length;
		direction = direction / getLen(direction) * penetration;
		p.pos += direction;
	}

	void move(sf::Vector2f to_move) override
	{
		position += to_move;
		drawable_circle->setPosition(sf::Vector2f(position.x, conf::Y - position.y));
	}


	sf::Vector2f getNearestVector(const Particle& p) const override
	{
		sf::Vector2f diff = p.pos - position;
		const float len = getLen(diff);
		if (len >= radius + conf::stickness_distance)
			return { 0.0f, 0.0f };
		diff /= len;
		return diff * (radius + conf::stickness_distance - len);
	}

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override
	{
		target.draw(*drawable_circle);
	}

	~CircleObject()
	{
		delete drawable_circle;
	}
};
