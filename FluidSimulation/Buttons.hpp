#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <functional>
#include <string>
#include <algorithm>
#include <exception>

namespace Buttons
{
	sf::Font DEFAULT_FONT;

	std::vector<sf::Color> DEFAULT_COLORS = { sf::Color(150, 150, 150), sf::Color(170, 170, 170), sf::Color(180, 180, 200) , sf::Color(210, 210, 210) };

	enum class PressableState
	{
		NORMAL = 0,
		HOVER = 1,
		BEING_PRESSED = 2,
		PRESSED = 3
	};
	std::vector<sf::Color> DEFAULT_PRESSABLE_COLORS = DEFAULT_COLORS;

	enum class FocusableState
	{
		NORMAL = 0,
		HOVER = 1,
		FOCUSED = 2
	};
	std::vector<sf::Color> DEFAULT_FOCUSABLE_COLORS = { sf::Color(200, 150, 0), sf::Color(230, 150, 0), sf::Color(255, 150, 0) };


	class Shape : public sf::Transformable, public sf::Drawable
	{
	public:
		sf::Color activeColor = sf::Color::White;
		std::vector<sf::Color> activeColors = DEFAULT_COLORS;
		sf::Vector2f size;

		Shape(sf::Vector2f topLeft_, sf::Vector2f size_)
		{
			changePosition(topLeft_);
			changeSize(size_);
			activeColors = DEFAULT_COLORS;
			activeColor = activeColors[0];
		}

		virtual bool isHovering(sf::Vector2f mouse_pos) const = 0;

		virtual void updateColors() = 0;

		virtual bool changePosition(sf::Vector2f newPosition)
		{
			if (newPosition == getPosition())
				return false;
			setPosition(newPosition);
			return true;
		}

		virtual bool changeSize(sf::Vector2f newSize)
		{
			if (newSize == size)
				return false;
			size = newSize;
			return true;
		}

		virtual bool move(sf::Vector2f amount)
		{
			return changePosition(getPosition() + amount);
		}

		sf::Vector2f getPositionAtPoint(sf::Vector2f point)
		{
			return getPosition() + sf::Vector2f(size.x * point.x, size.y * point.y);
		}

		sf::Vector2f getCenter() const
		{
			return getPosition() + size / 2.0f;
		}

		virtual void setActiveColorToState(size_t state)
		{
			activeColor = activeColors[state];
		}

		void setAllColorsTo(sf::Color color)
		{
			for (int i = 0; i < activeColors.size(); i++)
				activeColors[i] = color;
			activeColor = color;
		}
	};


	class Widget : public sf::Drawable
	{
	public:
		int priority;
		Shape* shape;
		bool enabled = true, viewable = true;

		Widget(Shape* shape_, int priority_) : shape(shape_), priority(priority_) {}

		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			if (!enabled)
				return;
			target.draw(*shape, states);
		}
	};


	class InteractableWidget : public Widget
	{
	public:
		bool interacted_this_frame = false;
		std::function<void()> onAction = []() {};

		InteractableWidget(Shape* shape_, int priority_) : Widget(shape_, priority_) {}

		virtual void processEvent(sf::Event& event) = 0;
		virtual void update(sf::Vector2i mouse_pos) = 0;

		virtual void setOnAction(std::function<void()> onAction_)
		{
			onAction = onAction_;
		}

		void activateOnAction()
		{
			onAction();
		}
	};


	class PressableWidget : public InteractableWidget
	{
	protected:
		PressableState state = PressableState::NORMAL;
		bool is_pressed;
	public:

		PressableWidget(Shape* shape_, int priority_, bool is_pressed_) : InteractableWidget(shape_, priority_), is_pressed(is_pressed_) {}

		virtual void press() = 0;

		bool isPressed() const
		{
			return is_pressed;
		}

		void setState(PressableState state_)
		{
			state = state_;
		}

		PressableState getState() const
		{
			return state;
		}
	};


	class FocusableWidget : public InteractableWidget
	{
	protected:
		FocusableState state = FocusableState::NORMAL;
	public:
		FocusableWidget(Shape* shape_, int priority_, bool is_focused_) : InteractableWidget(shape_, priority_)
		{
			if (is_focused_)
				state = FocusableState::FOCUSED;
		}

		virtual void focus()
		{
			state = FocusableState::FOCUSED;
		}

		virtual void unfocus()
		{
			state = FocusableState::NORMAL;
		}

		bool isFocused() const
		{
			return state == FocusableState::FOCUSED;
		}

		void setState(FocusableState state_)
		{
			state = state_;
		}

		FocusableState getState()
		{
			return state;
		}
	};


	class ShapeGroup : public Shape
	{
	public:
		std::vector<Shape*> shapes;

		ShapeGroup(std::vector<Shape*> shapes_) : Shape(shapes_[0]->getPosition(), shapes_[0]->size), shapes(shapes_)
		{
			recompose();
		}

		void recompose()
		{
			float min_x = 10000, min_y = 10000;
			float max_x = -10000, max_y = -10000;
			for (Shape* shape : shapes)
			{
				min_x = std::min(min_x, shape->getPosition().x);
				min_y = std::min(min_y, shape->getPosition().y);

				max_x = std::max(max_x, shape->getPosition().x + shape->size.x);
				max_y = std::max(max_y, shape->getPosition().y + shape->size.y);
			}
			setPosition(min_x, min_y);
			size = sf::Vector2f(max_x - min_x, max_y - min_y);
		}

		void addShape(Shape* shape_)
		{
			shapes.push_back(shape_);
			recompose();
		}

		virtual bool isHovering(sf::Vector2f mouse_pos) const override
		{
			for (Shape* shape : shapes)
			{
				if (shape->isHovering(mouse_pos))
					return true;
			}
			return false;
		}

		virtual void updateColors() override
		{
			for (Shape* shape : shapes)
				shape->updateColors();
		}

		virtual bool changePosition(sf::Vector2f newPosition) override
		{
			if (newPosition == getPosition())
				return false;
			sf::Vector2f toMove = newPosition - getPosition();
			setPosition(newPosition);
			for (Shape* shape : shapes)
				shape->move(toMove);
			recompose();
			return true;
		}

		virtual bool move(sf::Vector2f amount) override
		{
			return changePosition(getPosition() + amount);
		}

		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			for (Shape* shape : shapes)
				target.draw(*shape, states);
		}
	};


	class PanelShape : public Shape
	{
	public:
		Shape *panelShape, *widgetShape;
		std::pair<bool, bool> matchParent = { false, false }, wrapContent = {false, false};
		bool fitShape = true;
		bool drawShape = false;
		bool fixedPointToPoint = true, fixedPointToPosition = false;
		sf::Vector2f fixedShapePoint = sf::Vector2f(0.0f, 0.0f), fixedShapePosition = sf::Vector2f(0.0f, 0.0f), fixedWidgetPoint = sf::Vector2f(0.0f, 0.0f);

		PanelShape(Shape* panelShape, Shape* widgetShape) :
			Shape(panelShape->getPosition(), panelShape->size), panelShape(panelShape), widgetShape(widgetShape)
		{
			fixPointToPoint(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f));
		}

		void checkSize()
		{
			sf::Vector2f min_pos(10000.0f, 10000.0f), max_pos(-10000.0f, -10000.0f);
			min_pos.x = std::min(panelShape->getPosition().x + widgetShape->getPosition().x, panelShape->getPosition().x);
			min_pos.y = std::min(panelShape->getPosition().y + widgetShape->getPosition().y, panelShape->getPosition().y);

			max_pos.x = std::max(panelShape->getPosition().x + widgetShape->getPosition().x + widgetShape->size.x, panelShape->getPosition().x + panelShape->size.x);
			max_pos.y = std::max(panelShape->getPosition().y + widgetShape->getPosition().y + widgetShape->size.y, panelShape->getPosition().y + panelShape->size.y);

			setPosition(min_pos);
			size = max_pos - min_pos;

			if (widgetShape->getPosition().x < 0)
				widgetShape->move(sf::Vector2f(-widgetShape->getPosition().x, 0.0f));
			if (widgetShape->getPosition().y < 0)
				widgetShape->move(sf::Vector2f(0.0f, -widgetShape->getPosition().y));
		}

		void checkBounds()
		{
			sf::Vector2f temp = widgetSize();
			if (wrapContent.first && widgetSize().x != panelShape->size.x)
				changeSize(sf::Vector2f(widgetSize().x, panelShape->size.y));
			if (wrapContent.second && widgetSize().y != panelShape->size.y)
				changeSize(sf::Vector2f(panelShape->size.x, widgetSize().y));

			if (fitShape && widgetSize().x > panelShape->size.x)
				widgetShape->changeSize(sf::Vector2f(panelShape->size.x * 0.95f, widgetSize().y));
			if (fitShape && widgetSize().y > panelShape->size.y)
				widgetShape->changeSize(sf::Vector2f(widgetSize().x, panelShape->size.y * 0.95f));

			if (matchParent.first && widgetSize().x != panelShape->size.x)
				widgetShape->changeSize(sf::Vector2f(panelShape->size.x, widgetSize().y));
			if (matchParent.second && widgetSize().y != panelShape->size.y)
				widgetShape->changeSize(sf::Vector2f(widgetSize().x, panelShape->size.y));

			if (fixedPointToPoint)
				setPositionAt(fixedWidgetPoint, fixedShapePoint);
			if (fixedPointToPosition)
				setPositionAtPosition(fixedWidgetPoint, fixedShapePosition);
			checkSize();
		}

		sf::Vector2f widgetSize() const
		{
			return widgetShape->size;
		}

		sf::Vector2f getWidgetPositionAtPoint(sf::Vector2f point) const
		{
			return sf::Vector2f(
				widgetShape->getPosition().x + widgetSize().x * point.x,
				widgetShape->getPosition().y + widgetSize().y * point.y);
		}

		void setPositionAt(sf::Vector2f text_point, sf::Vector2f shape_point)
		{
			sf::Vector2f text_pos = getWidgetPositionAtPoint(text_point);
			sf::Vector2f shape_pos = getPositionAtPoint(shape_point) - getPosition();
			widgetShape->move(shape_pos - text_pos);
		}

		void setPositionAtPosition(sf::Vector2f text_point, sf::Vector2f shape_position)
		{
			sf::Vector2f text_pos = getWidgetPositionAtPoint(text_point);
			widgetShape->move(shape_position - text_pos);
		}

		bool isHovering(sf::Vector2f mouse_pos) const override
		{
			return panelShape->isHovering(mouse_pos);
		}

		void updateColors()
		{
			panelShape->updateColors();
			widgetShape->updateColors();
		}

		void setCenter(sf::Vector2f center)
		{
			setPositionAt({ 0.5f, 0.5f }, center);
			checkBounds();
		}

		void setSize(sf::Vector2f size)
		{
			changeSize(size);
			checkBounds();
		}

		void setWidgetSize(sf::Vector2f size)
		{
			widgetShape->changeSize(size);
			checkBounds();
		}

		void fixPointToPoint(sf::Vector2f textPoint, sf::Vector2f shapePoint)
		{
			fixedPointToPoint = true;
			fixedPointToPosition = false;
			fixedWidgetPoint = textPoint;
			fixedShapePoint = shapePoint;
			checkBounds();
		}

		void fixPointToPosition(sf::Vector2f textPoint, sf::Vector2f shapePosition)
		{
			fixedPointToPoint = false;
			fixedPointToPosition = true;
			fixedWidgetPoint = textPoint;
			fixedShapePosition = shapePosition;
			checkBounds();
		}

		virtual bool changeSize(sf::Vector2f newSize) override
		{
			if (newSize == size)
				return false;
			size = newSize;
			panelShape->changeSize(newSize);
			checkBounds();
			return true;
		}

		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			if (drawShape)
				target.draw(*panelShape, states);
			states.transform.combine(getTransform());
			target.draw(*widgetShape, states);
		}
	};


	class Panel : public InteractableWidget
	{
	public:
		PanelShape* panelShape;
		InteractableWidget* widget;

		Panel(Shape* shape_, InteractableWidget* widget_, int priority_) : InteractableWidget(shape_, priority_), widget(widget_)
		{
			panelShape = new PanelShape(shape_, widget_->shape);
		}

		void processEvent(sf::Event& event) override
		{
			if (!enabled)
				return;
			widget->processEvent(event);
		}

		void update(sf::Vector2i mouse_pos) override
		{
			if (!enabled)
				return;
			activateOnAction();
			widget->update(mouse_pos);
		}

		void setOnAction(std::function<void()> onAction_) override
		{
			onAction = onAction_;
		}

		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			if (!enabled)
				return;
			target.draw(*panelShape, states);
		}
	};


	class TextShape : public Shape
	{
	public:
		sf::Text text;

		TextShape(sf::Text text_) : Shape(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f))
		{
			setText(text_);
		}

		sf::Vector2f getSize()
		{
			return sf::Vector2f(text.getLocalBounds().width, text.getLocalBounds().height);
		}

		void setText(sf::Text text_)
		{
			text = text_;
			text.setPosition(-sf::Vector2f(text.getLocalBounds().left, text.getLocalBounds().top));
			changeSize(getSize());
		}

		void setString(std::string s)
		{
			text.setString(s);
			text.setPosition(-sf::Vector2f(text.getLocalBounds().left, text.getLocalBounds().top));
			changeSize(getSize());
		}

		bool isHovering(sf::Vector2f mouse_pos) const override
		{
			sf::Transform inv = getInverseTransform();
			return text.getLocalBounds().contains(inv.transformPoint(mouse_pos));
		}

		void updateColors() override {}

		bool changeSize(sf::Vector2f newSize) override
		{
			if (!Shape::changeSize(newSize))
				return false;

			while ((getSize().x < newSize.x) || (getSize().y < newSize.y)) {
				text.setCharacterSize(text.getCharacterSize() + 1);
			}

			while ((getSize().x > newSize.x) || (getSize().y > newSize.y)) {
				text.setCharacterSize(text.getCharacterSize() - 1);
			}

			text.setPosition(-sf::Vector2f(text.getLocalBounds().left, text.getLocalBounds().top));

			Shape::changeSize(getSize());

			return true;
		}

		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			states.transform.combine(getTransform());
			target.draw(text, states);
		}
	};


	class TextWidget : public InteractableWidget
	{
	public:
		TextShape* textShape;

		TextWidget(sf::Text text_, int priority_) : InteractableWidget(new TextShape(text_), priority_)
		{
			textShape = (TextShape*)shape;
		}

		void processEvent(sf::Event& event) override {}

		void update(sf::Vector2i mouse_pos) override
		{
			if (!enabled)
				return;
			activateOnAction();
		}
	};


	class Label : public Panel
	{
	public:
		TextWidget* textWidget;

		Label(Shape* shape_, sf::Text text_, int priority_)
			: Panel(shape_,
				new TextWidget(text_, priority_),
				priority_)
		{
			textWidget = (TextWidget*)widget;
			
			setText(text_);
		}

		sf::Text getText()
		{
			return textWidget->textShape->text;
		}

		void setText(sf::Text text_)
		{
			textWidget->textShape->setText(text_);
		}

		void setString(std::string s)
		{
			textWidget->textShape->setString(s);
		}

		std::string getString() const
		{
			return textWidget->textShape->text.getString();
		}
	};


	class RectShape : public Shape
	{
	public:
		sf::RectangleShape* shape;

		RectShape(sf::Vector2f topLeft_, sf::Vector2f size_)
			: Shape(topLeft_, size_)
		{
			shape = new sf::RectangleShape(size_);
			shape->setPosition(0.0f, 0.0f);
		}

		bool isHovering(sf::Vector2f point) const override
		{
			sf::Transform inv_trasform = getInverseTransform();
			sf::Vector2f point_as_local = inv_trasform.transformPoint(point);
			return shape->getLocalBounds().contains(point_as_local);
		}

		virtual void updateColors() override
		{
			shape->setFillColor(activeColor);
		}

		virtual bool changeSize(sf::Vector2f newSize) override
		{
			if (!Shape::changeSize(newSize))
				return false;
			shape->setSize(newSize);
			return true;
		}

		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			states.transform.combine(getTransform());
			target.draw(*shape, getTransform());
		}
	};


	class RoundRectShape : public Shape
	{
	public:
		sf::RectangleShape *horizontalRect = new sf::RectangleShape(), *verticalRect = new sf::RectangleShape();
		sf::CircleShape* circles[4];

		sf::VertexArray vertices;
		float radius;

		RoundRectShape(sf::Vector2f topLeft_, sf::Vector2f size_, float radius_)
			: Shape(topLeft_, size_), radius(radius_)
		{
			radius = std::min(radius, size.y / 2.0f);

			for (int i = 0; i < 4; i++)
			{
				circles[i] = new sf::CircleShape();
				circles[i]->setRadius(radius);
				circles[i]->setOrigin(radius, radius);
			}
			circles[0]->setPosition(sf::Vector2f(radius, radius));
			circles[1]->setPosition(sf::Vector2f(size.x - radius, radius));
			circles[2]->setPosition(sf::Vector2f(radius, size.y - radius));
			circles[3]->setPosition(sf::Vector2f(size.x - radius, size.y - radius));

			horizontalRect->setSize(sf::Vector2f(size.x, size.y - radius * 2));
			verticalRect->setSize(sf::Vector2f(size.x - radius * 2, size.y));
			horizontalRect->setPosition(sf::Vector2f(0, radius));
			verticalRect->setPosition(sf::Vector2f(radius, 0));

			setVertices();
		}

		void setVertices()
		{
			vertices.clear();
			vertices.setPrimitiveType(sf::TriangleFan);
			const float startingAngles[] = { 180.0f, 90.0f, 0.0f, 270.0f };
			const int pointCount = 30;
			for (int i = 0; i < 4; i++)
			{
				float angle = startingAngles[i];
				float incr = 90.0f / (pointCount - 1);

				sf::Vector2f centerPoint = circles[(i < 2) ? (i) : (i == 2 ? 3 : 2)]->getPosition();

				for (int j = 0; j < pointCount; j++)
				{
					vertices.append(sf::Vertex(sf::Vector2f(cos(angle * conf::PI / 180.0f), -sin(angle * conf::PI / 180.0f)) * radius + centerPoint));
					angle -= incr;
				}
			}
			updateColors();
		}

		virtual void updateColors() override
		{
			horizontalRect->setFillColor(activeColor);
			verticalRect->setFillColor(activeColor);
			for (int i = 0; i < 4; i++)
				circles[i]->setFillColor(activeColor);

			for (int i = 0; i < vertices.getVertexCount(); i++)
			{
				vertices[i].color = activeColor;
			}
		}

		bool isHovering(sf::Vector2f point) const override
		{
			sf::Transform inv_transform = getInverseTransform();
			sf::Vector2f point_as_local = inv_transform.transformPoint(point);
			if (horizontalRect->getLocalBounds().contains(point_as_local) || verticalRect->getLocalBounds().contains(point_as_local))
			{
				return true;
			}
			for (int i = 0; i < 4; i++)
			{
				if ((circles[i]->getPosition().x - point_as_local.x) * (circles[i]->getPosition().x - point_as_local.x) + (circles[i]->getPosition().y - point_as_local.y) * (circles[i]->getPosition().y - point_as_local.y) <= circles[i]->getRadius() * circles[i]->getRadius())
					return true;
			}
			return false;
		}

		virtual bool changeSize(sf::Vector2f newSize) override
		{
			if (!Shape::changeSize(newSize))
				return false;
			circles[1]->setPosition(sf::Vector2f(size.x - radius, radius));
			circles[2]->setPosition(sf::Vector2f(radius, size.y - radius));
			circles[3]->setPosition(sf::Vector2f(size.x - radius, size.y - radius));

			horizontalRect->setSize(sf::Vector2f(size.x, size.y - radius * 2));
			verticalRect->setSize(sf::Vector2f(size.x - radius * 2, size.y));
			updateColors();

			setVertices();
			return true;
		}

		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			states.transform.combine(getTransform());

			target.draw(vertices, states);
		}
	};


	class ButtonShape : public Shape
	{
	public:
		Shape* shape;
		PanelShape* panelShape;

		ButtonShape(Shape* shape_, PanelShape* panelShape_) : shape(shape_), panelShape(panelShape_), Shape(shape_->getPosition(), shape_->size)
		{}

		bool isHovering(sf::Vector2f mouse_pos) const override
		{
			return shape->isHovering(mouse_pos);
		}

		void updateColors() override
		{
			shape->activeColor = activeColor;
			shape->updateColors();
			panelShape->updateColors();
		}

		bool changeSize(sf::Vector2f newSize) override
		{
			if (!Shape::changeSize(newSize))
				return false;
			shape->changeSize(newSize);
			panelShape->checkBounds();
			return true;
		}

		bool changePosition(sf::Vector2f newPosition) override
		{
			if (!Shape::changePosition(newPosition))
				return false;
			shape->changePosition(newPosition);
			panelShape->changePosition(newPosition);
			return true;
		}

		bool move(sf::Vector2f amount) override
		{
			return changePosition(getPosition() + amount);
		}

		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			target.draw(*shape, states);
			target.draw(*panelShape, states);
		}
	};


	class Button : public PressableWidget
	{
	public:
		Label* label;

		Button(Shape* shape_, sf::Text text_, int priority_) : PressableWidget(shape_, priority_, false)
		{
			label = new Label(shape_, text_, priority_);
			label->panelShape->fixPointToPoint(sf::Vector2f(0.5f, 0.5f), sf::Vector2f(0.5f, 0.5f));
			shape = new ButtonShape(shape_, label->panelShape);
		}

		virtual void processEvent(sf::Event& event) override
		{
			interacted_this_frame = false;
			if (!enabled)
				return;
			if (event.type == sf::Event::MouseButtonPressed)
			{
				sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
				if (event.mouseButton.button == sf::Mouse::Left && shape->isHovering(mousePos))
				{
					state = PressableState::BEING_PRESSED;
					interacted_this_frame = true;
				}
			}
			else if (event.type == sf::Event::MouseButtonReleased)
			{
				sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					if (state == PressableState::BEING_PRESSED)
					{
						if (shape->isHovering(mousePos))
						{
							press();
							interacted_this_frame = true;
						}
						else
						{
							state = isPressed() ? PressableState::PRESSED : PressableState::NORMAL;
						}
					}
				}
			}

			label->processEvent(event);
		}

		virtual void update(sf::Vector2i mouse_pos) override
		{
			if (!enabled)
				return;
			sf::Vector2f mouse_pos_f(mouse_pos.x, mouse_pos.y);

			switch (state)
			{
			case PressableState::NORMAL:
				if (shape->isHovering(mouse_pos_f))
					state = PressableState::HOVER;
				break;
			case PressableState::HOVER:
				if (!shape->isHovering(mouse_pos_f))
					state = PressableState::NORMAL;
				break;
			case PressableState::BEING_PRESSED:
				break;
			case PressableState::PRESSED:
				break;
			}

			bool is_hovering = shape->isHovering(mouse_pos_f);
			if (state == PressableState::BEING_PRESSED)
			{
				if (is_hovering)
				{
					shape->setActiveColorToState((size_t)PressableState::BEING_PRESSED);
				}
				else
				{
					shape->setActiveColorToState(isPressed() ? (size_t)PressableState::PRESSED : (size_t)PressableState::NORMAL);
				}
			}
			else
			{
				shape->setActiveColorToState((size_t)state);
			}
			shape->updateColors();

			label->update(mouse_pos);
		}

		virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			if (!enabled)
				return;
			target.draw(*shape, states);
		}
	};


	class ClickableButton : public Button
	{
	public:
		ClickableButton(Shape* shape_, sf::Text text_, int priority_) : Button(shape_, text_, priority_) {}

		void press() override
		{
			state = PressableState::PRESSED;
			is_pressed = true;
			activateOnAction();
			is_pressed = false;
			state = PressableState::HOVER;
		}
	};


	class SwitchableButton : public Button
	{
	public:
		SwitchableButton(Shape* shape_, sf::Text text_, int priority_) : Button(shape_, text_, priority_) {}

		void press() override
		{
			if (is_pressed)
				state = PressableState::HOVER;
			else
				state = PressableState::PRESSED;
			is_pressed = !is_pressed;
			activateOnAction();
		}
	};


	class CheckBox : public InteractableWidget
	{
	public:
		SwitchableButton* button;
		Label* label;

		CheckBox(Shape* buttonShape_, Shape* labelShape_, sf::Text text, int priority_)
			: InteractableWidget(new RectShape(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f)), priority_)
		{
			label = new Label(labelShape_, text, priority_);
			button = new SwitchableButton(buttonShape_, sf::Text(), priority_);
			label->panelShape->fixPointToPosition(sf::Vector2f(0.0f, 0.5f), 
				sf::Vector2f(buttonShape_->size.x, buttonShape_->size.y / 2.0f));

			shape = new ShapeGroup(std::vector<Shape*>{buttonShape_, label->panelShape});
		}

		void setOnAction(std::function<void()> onAction_) override
		{
			button->setOnAction(onAction_);
		}

		void processEvent(sf::Event& event) override
		{
			if (!enabled)
				return;
			button->processEvent(event);
			label->processEvent(event);
			interacted_this_frame = button->interacted_this_frame;
		}

		void update(sf::Vector2i mouse_pos) override
		{
			if (!enabled)
				return;
			button->update(mouse_pos);
			label->update(mouse_pos);
		}
		

		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			if (!enabled)
				return;
			target.draw(*button, states);
			target.draw(*label, states);
		}
	};


	class SliderShape : public ShapeGroup
	{
	public:
		RoundRectShape* sliderShape;
		RoundRectShape* lineShape;
		PanelShape* panelShape;
		float currentPoint = 0.0f;

		SliderShape(RoundRectShape* lineShape_, RoundRectShape* sliderShape_, PanelShape* panelShape_)
			: lineShape(lineShape_), sliderShape(sliderShape_), panelShape(panelShape_),
			ShapeGroup(std::vector<Shape*>{lineShape_, sliderShape_, panelShape_})
		{
			activeColors = DEFAULT_FOCUSABLE_COLORS;
			activeColor = activeColors[0];

			sliderShape->activeColors = activeColors;
			sliderShape->activeColor = activeColors[0];
			sliderShape->updateColors();

			lineShape->activeColors = DEFAULT_PRESSABLE_COLORS;
			lineShape->updateColors();

			panelShape->checkBounds();
			recompose();
		}

		virtual bool isHovering(sf::Vector2f mouse_pos) const override
		{
			return lineShape->isHovering(mouse_pos);
		}

		void updateSlider(float currentPoint_)
		{
			currentPoint = currentPoint_;
			sliderShape->changePosition(sf::Vector2f(lineShape->getPosition().x + (lineShape->size.x - sliderShape->size.x) * currentPoint, lineShape->getPosition().y));
			panelShape->checkBounds();
			recompose();
		}

		bool changePosition(sf::Vector2f newPosition) override
		{
			if (!ShapeGroup::changePosition(newPosition))
				return false;

			updateSlider(currentPoint);
			panelShape->checkBounds();
			recompose();
			return true;
		}

		bool move(sf::Vector2f amount) override
		{
			return changePosition(getPosition() + amount);
		}

		void updateColors() override
		{
			sliderShape->activeColor = activeColor;
			lineShape->updateColors();
			sliderShape->updateColors();
		}
	};


	class Slider : public FocusableWidget
	{
	public:
		SliderShape* sliderShape;
		Label* label;
		float point;

		bool changing_value_set = false;
		float* float_changing_value = nullptr;
		int* int_changing_value = nullptr;
		float min = 0.0f, max = 1.0f;
		std::string name;
		int precision = 2;

		Slider(RoundRectShape* shape_, sf::Text text, float start, int priority_)
			: point(start), FocusableWidget(shape_, priority_, false)
		{
			state = FocusableState::NORMAL;

			RoundRectShape* lineShape_ = shape_;
			RoundRectShape* sliderShape_ = new RoundRectShape(lineShape_->getPosition(), sf::Vector2f(lineShape_->size.y, lineShape_->size.y), lineShape_->radius);
			label = new Label(lineShape_, text, priority_);
			label->panelShape->fixPointToPoint(sf::Vector2f(0.5f, 1.0f), sf::Vector2f(0.5f, -0.05f));

			shape = new SliderShape(lineShape_, sliderShape_, label->panelShape);
			sliderShape = (SliderShape*)shape;
			sliderShape->updateSlider(point);
			sliderShape->recompose();
		}

		void setParameters(float min_, float max_, std::string name_, int precision_ = 2)
		{
			precision = precision_;
			name = name_;
			min = min_;
			max = max_;
		}

		void setChangingValue(float* float_changing_value_, float min_, float max_, std::string name_, int precision_ = 2)
		{
			setParameters(min_, max_, name_, precision_);
			float_changing_value = float_changing_value_;
			point = (*float_changing_value - min) / (max - min);

			label->setString(name + ": " + to_string_with_precision(*float_changing_value, precision));
			sliderShape->updateSlider(point);
			sliderShape->recompose();
		}

		void setChangingValue(int* int_changing_value_, float min_, float max_, std::string name_, int precision_ = 2)
		{
			setParameters(min_, max_, name_, precision_);
			int_changing_value = int_changing_value_;
			point = (*int_changing_value - min) / (max - min);

			label->setString(name + ": " + to_string_with_precision(*int_changing_value, precision));
			sliderShape->updateSlider(point);
			sliderShape->recompose();
		}

		void processEvent(sf::Event& event) override
		{
			interacted_this_frame = false;
			if (!enabled)
				return;
			interacted_this_frame = isFocused();
			if (event.type == sf::Event::MouseButtonPressed)
			{
				sf::Vector2f mouse_pos_f(event.mouseButton.x, event.mouseButton.y);
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					if (shape->isHovering(mouse_pos_f))
					{
						focus();
					}
				}
			}
			else if (event.type == sf::Event::MouseButtonReleased)
			{
				sf::Vector2f mouse_pos_f(event.mouseButton.x, event.mouseButton.y);
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					unfocus();
				}
			}
			interacted_this_frame |= isFocused();
		}

		void update(sf::Vector2i mouse_pos) override
		{
			if (!enabled)
				return;
			sf::Vector2f mouse_pos_f(mouse_pos.x, mouse_pos.y);

			bool is_hovering = shape->isHovering(mouse_pos_f);

			switch (state)
			{
			case FocusableState::NORMAL:
				if (is_hovering)
					state = FocusableState::HOVER;
				break;
			case FocusableState::HOVER:
				if (!is_hovering)
					state = FocusableState::NORMAL;
				break;
			case FocusableState::FOCUSED:
				break;
			}

			sliderShape->setActiveColorToState((size_t)state);
			sliderShape->lineShape->setActiveColorToState((size_t)state);
			sliderShape->updateColors();

			if (isFocused())
			{
				float new_point = (mouse_pos_f.x - sliderShape->lineShape->getPosition().x - sliderShape->sliderShape->size.x / 2.0f) / (sliderShape->lineShape->size.x - sliderShape->sliderShape->size.x);
				new_point = std::min(new_point, 1.0f);
				new_point = std::max(new_point, 0.0f);
				if (point != new_point)
				{
					point = new_point;
					sliderShape->updateSlider(point);

					if (float_changing_value != nullptr)
					{
						float new_value = min + point * (max - min);
						if (new_value != *float_changing_value) {
							*float_changing_value = new_value;
							label->setString(name + ": " + to_string_with_precision(*float_changing_value, precision));
						}
					}
					else if (int_changing_value != nullptr)
					{
						int new_value = round(min + point * (max - min));
						if (new_value != *int_changing_value) {
							*int_changing_value = new_value;
							label->setString(name + ": " + to_string_with_precision(*int_changing_value, precision));
						}
					}

					activateOnAction();
				}
			}
		}

		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			if (!enabled)
				return;
			target.draw(*shape, states);
		}

		~Slider()
		{
			delete shape;
		}
	};


	class SwitchableButtonGroup : public InteractableWidget
	{
	public:
		std::vector<SwitchableButton*> switchables;
		int prevPressed = -1;
		bool alwaysPressed = false;

		SwitchableButtonGroup(Shape* shape, int priority_) : InteractableWidget(shape, priority_) {}

		void addSwitchable(SwitchableButton* button)
		{
			switchables.push_back(button);
		}

		void setAlwaysPressed(SwitchableButton* button_to_press)
		{
			alwaysPressed = true;
			for (int i = 0; i < switchables.size(); i++)
			{
				if (switchables[i] == button_to_press)
				{
					prevPressed = i;
					button_to_press->press();
				}
			}
		}

		virtual void processEvent(sf::Event& event)
		{
			interacted_this_frame = false;

			if (!enabled)
				return;

			if (viewable)
			{
				for (int i = 0; i < switchables.size(); i++)
				{
					switchables[i]->processEvent(event);
					interacted_this_frame = interacted_this_frame || switchables[i]->interacted_this_frame;
				}
			}
		}

		virtual void update(sf::Vector2i mouse_pos)
		{
			if (!enabled)
				return;

			if (viewable)
			{
				for (int i = 0; i < switchables.size(); i++)
				{
					switchables[i]->update(mouse_pos);
				}
			}
			for (int i = int(switchables.size()) - 1; i >= 0; i--)
			{
				if (switchables[i]->isPressed())
				{
					if (prevPressed == i)
						continue;
					prevPressed = i;
					for (int j = 0; j < switchables.size(); j++)
					{
						if (i == j)
							continue;
						if (switchables[j]->isPressed())
							switchables[j]->press();
					}
				}
				else if (prevPressed == i)
				{
					if (alwaysPressed)
						switchables[i]->press();
					else
						prevPressed = -1;
				}
			}
		}

		bool wasInteractedThisFrame() const
		{
			return interacted_this_frame;
		}

		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			if (!enabled || !viewable)
				return;
			if (shape != nullptr)
				target.draw(*shape, states);
			for (int i = 0; i < switchables.size(); i++)
			{
				target.draw(*switchables[i], states);
			}
		}
	};


	class WidgetHandler : public FocusableWidget
	{
	public:
		std::vector<Widget*> Items, sortedItems;
		std::vector<InteractableWidget*> Interactables, sortedInteractables;
		std::vector<FocusableWidget*> Focusables, sortedFocusables;
		FocusableWidget* focused_item = nullptr;
		ShapeGroup* shapeGroup;
		Shape* mainShape;
		bool drawShape = false;

		WidgetHandler(Shape* shape_, int priority_) : FocusableWidget(new ShapeGroup({shape_}), priority_, false)
		{
			shapeGroup = (ShapeGroup*)shape;
			mainShape = shape_;
		}

		virtual void addItem(FocusableWidget* focusable)
		{
			Items.push_back(focusable);
			Focusables.push_back(focusable);

			sortedItems.push_back(focusable);
			sortedFocusables.push_back(focusable);

			shapeGroup->addShape(focusable->shape);

			bool(*pred)(Widget*, Widget*) = [](Widget* l, Widget* r) { return l->priority < r->priority; };
			std::sort(sortedItems.begin(), sortedItems.end(), pred);
			std::sort(sortedFocusables.begin(), sortedFocusables.end(), pred);
			update(sf::Vector2i(-1, -1));
		}

		virtual void addItem(InteractableWidget* interactable)
		{
			Items.push_back(interactable);
			Interactables.push_back(interactable);

			sortedItems.push_back(interactable);
			sortedInteractables.push_back(interactable);

			shapeGroup->addShape(interactable->shape);

			bool(*pred)(Widget*, Widget*) = [](Widget* l, Widget* r) { return l->priority < r->priority; };
			std::sort(sortedItems.begin(), sortedItems.end(), pred);
			std::sort(sortedInteractables.begin(), sortedInteractables.end(), pred);
			update(sf::Vector2i(-1, -1));
		}

		virtual void addItem(Widget* widget)
		{
			Items.push_back(widget);

			sortedItems.push_back(widget);

			shapeGroup->addShape(widget->shape);

			bool(*pred)(Widget*, Widget*) = [](Widget* l, Widget* r) { return l->priority < r->priority; };
			std::sort(sortedItems.begin(), sortedItems.end(), pred);
			update(sf::Vector2i(-1, -1));
		}

		void processEvent(sf::Event& event) override
		{
			interacted_this_frame = false;

			if (!enabled)
				return;

			if (focused_item != nullptr)
			{
				focused_item->processEvent(event);
				if (!focused_item->interacted_this_frame)
					focused_item = nullptr;
				interacted_this_frame = true;
				return;
			}

			int l = int(sortedInteractables.size()) - 1, r = int(sortedFocusables.size()) - 1;
			while (l >= 0 && r >= 0)
			{
				if (sortedInteractables[l]->priority <= sortedFocusables[r]->priority)
				{
					sortedFocusables[r]->processEvent(event);
					if (sortedFocusables[r]->interacted_this_frame)
					{
						focused_item = sortedFocusables[r];
						interacted_this_frame = true;
						return;
					}
					r--;
				}
				else
				{
					sortedInteractables[l]->processEvent(event);
					if (sortedInteractables[l]->interacted_this_frame)
					{
						interacted_this_frame = true;
						return;
					}
					l--;
				}
			}
			while (l >= 0)
			{
				sortedInteractables[l]->processEvent(event);
				if (sortedInteractables[l]->interacted_this_frame)
				{
					interacted_this_frame = true;
					return;
				}
				l--;
			}
			while (r >= 0)
			{
				sortedFocusables[r]->processEvent(event);
				if (sortedFocusables[r]->interacted_this_frame)
				{
					focused_item = sortedFocusables[r];
					interacted_this_frame = true;
					return;
				}
				r--;
			}
			return;
		}

		virtual void update(sf::Vector2i mouse_pos)
		{
			if (!enabled)
				return;
			for (int i = 0; i < sortedInteractables.size(); i++)
				sortedInteractables[i]->update(mouse_pos);
			for (int i = 0; i < sortedFocusables.size(); i++)
				sortedFocusables[i]->update(mouse_pos);
		}

		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			if (!enabled)
				return;
			if (drawShape)
				target.draw(*mainShape, states);
			for (int i = 0; i < Items.size(); i++)
			{
				target.draw(*Items[i], states);
			}
		}

		~WidgetHandler()
		{
			for (int i = 0; i < Items.size(); i++)
				delete Items[i];
		}
	};

	enum class ContainerMode {
		NORMAL = 0,
		FIT_WIDGETS = 1
	};

	class LinearLayout : public WidgetHandler
	{
	public:
		bool isVertical = false;
		sf::Vector2f start_padding = sf::Vector2f(0.0f, 0.0f), end_padding = sf::Vector2f(0.0f, 0.0f);
		float widget_padding = 0.0f;
		ContainerMode mode = ContainerMode::NORMAL;

		LinearLayout(Shape* shape_, int priority_, bool isVertical_) : WidgetHandler(shape_, priority_), isVertical(isVertical_) {}

		virtual void addItem(FocusableWidget* focusable)
		{
			WidgetHandler::addItem(focusable);
			if (focusable->shape->getPosition().x != mainShape->getPosition().x)
				focusable->shape->changePosition(sf::Vector2f(mainShape->getPosition().x, focusable->shape->getPosition().y));
		}

		virtual void addItem(InteractableWidget* interactable)
		{
			WidgetHandler::addItem(interactable);
			if (interactable->shape->getPosition().x != mainShape->getPosition().x)
				interactable->shape->changePosition(sf::Vector2f(mainShape->getPosition().x, interactable->shape->getPosition().y));
		}

		virtual void addItem(Widget* widget)
		{
			WidgetHandler::addItem(widget);
			if (widget->shape->getPosition().x != mainShape->getPosition().x)
				widget->shape->changePosition(sf::Vector2f(mainShape->getPosition().x, widget->shape->getPosition().y));
		}

		virtual void update(sf::Vector2i mouse_pos) override
		{
			WidgetHandler::update(mouse_pos);

			if (!enabled)
				return;

			if (Items.empty())
				return;

			sf::Vector2f cursor(mainShape->getPosition() + start_padding);
			for (int i = 0; i < Items.size(); i++)
			{
				Widget* widget = Items[i];

				if (!widget->enabled)
					continue;

				if (cursor != widget->shape->getPosition())
				{
					widget->shape->changePosition(cursor);
				}

				if (isVertical)
					cursor += sf::Vector2f(0.0f, widget->shape->size.y + widget_padding);
				else 
					cursor += sf::Vector2f(widget->shape->size.x + widget_padding, 0.0f);
			}

			if (mode == ContainerMode::NORMAL) {
				return;
			}

			float min_x = 10000.0f, min_y = 10000.0f, max_x = -10000.0f, max_y = -10000.0f;
			for (Widget* widget : Items)
			{
				if (!widget->enabled)
					continue;

				sf::Vector2f widget_position = widget->shape->getPosition();
				sf::Vector2f widget_size = widget->shape->size;
				if (widget_position.x < min_x)
					min_x = widget_position.x;
				if (widget_position.y < min_y)
					min_y = widget_position.y;
				if (widget_position.x + widget_size.x > max_x)
					max_x = widget_position.x + widget_size.x;
				if (widget_position.y + widget_size.y > max_y)
					max_y = widget_position.y + widget_size.y;
			}

			if (min_x == 10000.0f && shape->size != sf::Vector2f(0.0f, 0.0f))
			{
				shape->changeSize(sf::Vector2f(0.0f, 0.0f));
				mainShape->changeSize(sf::Vector2f(0.0f, 0.0f));
			}
			else
			{
				min_x -= start_padding.x;
				min_y -= start_padding.y;
				max_x += end_padding.x;
				max_y += end_padding.y;
				sf::Vector2f current_position = shape->getPosition(), current_size = shape->size;

				if (min_x != current_position.x || min_y != current_position.y
					|| max_x != current_position.x + current_size.x || max_y != current_position.y + current_size.y)
				{
					shape->changePosition(sf::Vector2f(min_x, min_y));
					shape->changeSize(sf::Vector2f(max_x - min_x, max_y - min_y));

					mainShape->changePosition(sf::Vector2f(min_x, min_y));
					mainShape->changeSize(sf::Vector2f(max_x - min_x, max_y - min_y));
				}
			}
		}
	};

	class DropDown : public LinearLayout
	{
	public:
		DropDown(SwitchableButton* button_, WidgetHandler* handler_)
			: LinearLayout(new RectShape(button_->shape->getPosition(), button_->shape->size), button_->priority, true)
		{
			handler_->enabled = false;
			mode = ContainerMode::FIT_WIDGETS;
			addItem(button_);
			addItem(handler_);

			button_->setOnAction([handler_]() {
				handler_->enabled = !handler_->enabled;
				});

			//handler_->shape->changePosition(sf::Vector2f(button_->shape->getPosition().x, button_->shape->getPosition().y + button_->shape->size.y));
		}
	};
}