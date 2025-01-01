#pragma once

#include "Particle.hpp"
#include "Conf.hpp"
#include <SFML/Graphics.hpp>
#include <vector>

class CollisionObject : public sf::Drawable
{
public:
	virtual bool isColliding(const Particle& p) = 0;
	virtual void handleCollision(Particle& p) = 0;
	virtual void move(sf::Vector2f to_move) = 0;
};

class PolygonObject : public CollisionObject
{
public:
	std::vector<sf::Vector2f> vertices;
	std::vector<sf::Vector2f> normals;
	std::vector<sf::Vertex> drawable_vertices;
	sf::Vector2f min, max;

	PolygonObject() {}

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

	bool isColliding(const Particle& p) override
	{
		if (p.pos.x < min.x || p.pos.y < min.y || p.pos.x > max.x || p.pos.y > max.y)
			return false;
		const sf::Vector2f position = p.pos;
		for (int i = 0; i < vertices.size(); i++)
		{
			const sf::Vector2f vert_to_point = position - vertices[i];
			const float dot = vert_to_point.x * normals[i].x + vert_to_point.y * normals[i].y;
			if (dot >= 0)
				return false;
		}
		return true;
	}

	void handleCollision(Particle& p) override
	{
		if (p.pos.x < min.x || p.pos.y < min.y || p.pos.x > max.x || p.pos.y > max.y)
			return;

		const sf::Vector2f position = p.pos;
		float smallestDot = -1000000000.0f;
		sf::Vector2f bestNormal;

		for (int i = 0; i < vertices.size(); i++)
		{
			const sf::Vector2f vert_to_point = position - vertices[i];
			const float dot = vert_to_point.x * normals[i].x + vert_to_point.y * normals[i].y;
			if (dot >= 0)
				return;
			if (dot > smallestDot)
			{
				smallestDot = dot;
				bestNormal = normals[i];
			}
		}
		p.pos += bestNormal * -1.0f * smallestDot;
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
		drawable_circle = new sf::CircleShape(radius);
		drawable_circle->setOrigin(radius, radius);
		drawable_circle->setFillColor(conf::COLOR_OBJECT);
		drawable_circle->setPosition(sf::Vector2f(position.x, conf::Y - position.y));
		drawable_circle->setPointCount(100);
	}

	bool isColliding(const Particle& p) override
	{
		return getLen(p.pos - position) <= radius;
	}

	void handleCollision(Particle& p) override
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

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override
	{
		target.draw(*drawable_circle);
	}

	~CircleObject()
	{
		delete drawable_circle;
	}
};
