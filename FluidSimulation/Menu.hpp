#pragma once
#include <SFML/Graphics.hpp>
#include "Buttons.hpp"
#include "Conf.hpp"
#include "Simulation.hpp"

class Menu : public sf::Drawable
{
public:
	Buttons::WidgetHandler handler = Buttons::WidgetHandler(new Buttons::RectShape(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(conf::window.getSize().x, conf::window.getSize().y)), 1);
	Simulation& sim;

	Menu(Simulation& sim) : sim(sim)
	{
		using namespace Buttons;

		float MIN_INTERACTION_RADIUS = 0.1f, MAX_INTERACTION_RADIUS = 2.0f;
		float MIN_STIFFNESS = 0.0f, MAX_STIFFNESS = 20.0f;
		float MIN_NEAR_STIFFNESS = 0.0f, MAX_NEAR_STIFFNESS = 200.0f;
		float MIN_SPRING_STIFFNESS = 0.0f, MAX_SPRING_STIFFNESS = 4000.0f;
		float MIN_REST_DENSITY = 1.0f, MAX_REST_DENSITY = 30.0f;
		float MIN_YIELD_RATIO = 0.0f, MAX_YIELD_RATIO = 1.0f;
		float MIN_PLASTICITY = 0.0f, MAX_PLASTICITY = 100.0f;
		float MIN_ALPHA_VISCOSITY = 0.0f, MAX_ALPHA_VISCOSITY = 10.0f;
		float MIN_BETA_VISCOSITY = 0.0f, MAX_BETA_VISCOSITY = 10.0f;

		DEFAULT_FONT.loadFromFile("font.ttf");

		RectShape* buttonShape1 = new RectShape(sf::Vector2f(100.0f, 100.0f), sf::Vector2f(300.0f, 100.0f));

		sf::Text text("text", DEFAULT_FONT, 16);

		Slider* interaction_radius = new Slider(new RoundRectShape(sf::Vector2f(200.0f, 200.0f), sf::Vector2f(150.0f, 30.0f), 10.0f), text, 0.5, 1); 
		interaction_radius->setChangingValue(&conf::h, MIN_INTERACTION_RADIUS, MAX_INTERACTION_RADIUS, "interaction radius");
		interaction_radius->setOnAction([&sim]() {
			sim.createGrid();
			});

		Slider* stiffness = new Slider(new RoundRectShape(sf::Vector2f(200.0f, 200.0f), sf::Vector2f(150.0f, 30.0f), 10.0f), text, 0.5, 1);
		stiffness->setChangingValue(&conf::k, MIN_STIFFNESS, MAX_STIFFNESS, "stiffness");

		Slider* near_stiffness = new Slider(new RoundRectShape(sf::Vector2f(200.0f, 200.0f), sf::Vector2f(150.0f, 30.0f), 10.0f), text, 0.5, 1);
		near_stiffness->setChangingValue(&conf::k_near, MIN_NEAR_STIFFNESS, MAX_NEAR_STIFFNESS, "near stiffness");

		Slider* spring_stiffness = new Slider(new RoundRectShape(sf::Vector2f(200.0f, 200.0f), sf::Vector2f(150.0f, 30.0f), 10.0f), text, 0.5, 1);
		spring_stiffness->setChangingValue(&conf::k_spring, MIN_SPRING_STIFFNESS, MAX_SPRING_STIFFNESS, "spring stiffness");

		Slider* rest_density = new Slider(new RoundRectShape(sf::Vector2f(200.0f, 200.0f), sf::Vector2f(150.0f, 30.0f), 10.0f), text, 0.5, 1);
		rest_density->setChangingValue(&conf::density_rest, MIN_REST_DENSITY, MAX_REST_DENSITY, "rest density");

		Slider* yield_ratio = new Slider(new RoundRectShape(sf::Vector2f(200.0f, 200.0f), sf::Vector2f(70.0f, 30.0f), 10.0f), text, 0.5, 1);
		yield_ratio->setChangingValue(&conf::yield_ratio, MIN_YIELD_RATIO, MAX_YIELD_RATIO, "yield ratio");
		Slider* plasticity = new Slider(new RoundRectShape(sf::Vector2f(200.0f, 200.0f), sf::Vector2f(70.0f, 30.0f), 10.0f), text, 0.5, 1);
		plasticity->setChangingValue(&conf::plasticity, MIN_PLASTICITY, MAX_PLASTICITY, "plasticity");
		LinearLayout* yield_plasticity_layout = new LinearLayout(new RoundRectShape(sf::Vector2f(100.0f, 100.0f), sf::Vector2f(150.0f, 50.0f), 10.0f), 1, false);
		yield_plasticity_layout->widget_padding = 10.0f;
		yield_plasticity_layout->addItem(yield_ratio);
		yield_plasticity_layout->addItem(plasticity);
		yield_plasticity_layout->mode = ContainerMode::FIT_WIDGETS;

		Slider* alpha_viscosity = new Slider(new RoundRectShape(sf::Vector2f(200.0f, 200.0f), sf::Vector2f(70.0f, 30.0f), 10.0f), text, 0.5, 1);
		alpha_viscosity->setChangingValue(&conf::alpha_viscosity, MIN_ALPHA_VISCOSITY, MAX_ALPHA_VISCOSITY, "alpha visc.");
		Slider* beta_viscosity = new Slider(new RoundRectShape(sf::Vector2f(200.0f, 200.0f), sf::Vector2f(70.0f, 30.0f), 10.0f), text, 0.5, 1);
		beta_viscosity->setChangingValue(&conf::beta_viscosity, MIN_BETA_VISCOSITY, MAX_BETA_VISCOSITY, "beta visc.");
		LinearLayout* viscosity_layout = new LinearLayout(new RoundRectShape(sf::Vector2f(100.0f, 100.0f), sf::Vector2f(150.0f, 50.0f), 10.0f), 1, false);
		viscosity_layout->widget_padding = 10.0f;
		viscosity_layout->addItem(alpha_viscosity);
		viscosity_layout->addItem(beta_viscosity);
		viscosity_layout->mode = ContainerMode::FIT_WIDGETS;

		LinearLayout* settings_layout = new LinearLayout(new RoundRectShape(sf::Vector2f(100.0f, 100.0f), sf::Vector2f(200.0f, 400.0f), 20.0f), 1, true);
		settings_layout->mainShape->setAllColorsTo(sf::Color(255, 255, 255, 100));
		settings_layout->mainShape->setActiveColorToState(0);
		settings_layout->mainShape->updateColors();
		settings_layout->addItem(interaction_radius);
		settings_layout->addItem(stiffness);
		settings_layout->addItem(near_stiffness);
		settings_layout->addItem(spring_stiffness);
		settings_layout->addItem(rest_density);
		settings_layout->addItem(yield_plasticity_layout);
		settings_layout->addItem(viscosity_layout);
		settings_layout->drawShape = true;
		settings_layout->widget_padding = 10.0f;
		settings_layout->start_padding = sf::Vector2f(10.0f, 10.0f);
		settings_layout->end_padding = sf::Vector2f(10.0f, 10.0f);
		settings_layout->mode = ContainerMode::FIT_WIDGETS;

		text.setString("settings");
		DropDown* settings_dropDown = new DropDown(
			new SwitchableButton(new RoundRectShape(sf::Vector2f(10.0f, 10.0f), sf::Vector2f(100.0f, 40.0f), 15.0f), text, 2),
			settings_layout);

		sf::Vector2f BUTTON_SIZE(60.0f, 60.0f);
		float BUTTON_RADIUS = 10.0f;
		
		text.setString("particle");
		SwitchableButton* particle = new SwitchableButton(new RoundRectShape({0.0f, 0.0f}, BUTTON_SIZE, BUTTON_RADIUS), text, 1);
		particle->setOnAction([particle]() {if (particle->isPressed()) conf::spawnMode = SpawnMode::PARTICLE; });
		
		text.setString("rectangle");
		SwitchableButton* rectangle = new SwitchableButton(new RoundRectShape({ 0.0f, 0.0f }, BUTTON_SIZE, BUTTON_RADIUS), text, 1);
		rectangle->setOnAction([rectangle]() {if (rectangle->isPressed()) conf::spawnMode = SpawnMode::RECT; });

		text.setString("circle");
		SwitchableButton* circle = new SwitchableButton(new RoundRectShape({ 0.0f, 0.0f }, BUTTON_SIZE, BUTTON_RADIUS), text, 1);
		circle->setOnAction([circle]() {if (circle->isPressed()) conf::spawnMode = SpawnMode::CIRCLE; });

		text.setString("polygon");
		SwitchableButton* polygon = new SwitchableButton(new RoundRectShape({ 0.0f, 0.0f }, BUTTON_SIZE, BUTTON_RADIUS), text, 1);
		polygon->setOnAction([polygon]() {if (polygon->isPressed()) conf::spawnMode = SpawnMode::POLYGON; });

		SwitchableButtonGroup* spawn_group = new SwitchableButtonGroup(new RectShape({ 0.0f, 0.0f }, { 0.0f, 0.0f }), 1);
		spawn_group->viewable = false;
		spawn_group->addSwitchable(particle);
		spawn_group->addSwitchable(rectangle);
		spawn_group->addSwitchable(circle);
		spawn_group->addSwitchable(polygon);
		spawn_group->setAlwaysPressed(particle);

		LinearLayout* spawn_layout = new LinearLayout(new RoundRectShape({ 0.0f, 0.0f}, { 100.0f, 100.0f }, BUTTON_RADIUS), 1, true);
		spawn_layout->addItem(particle);
		spawn_layout->addItem(rectangle);
		spawn_layout->addItem(circle);
		spawn_layout->addItem(polygon);
		spawn_layout->mainShape->setAllColorsTo(sf::Color(255, 255, 255, 100));
		spawn_layout->mainShape->setActiveColorToState(0);
		spawn_layout->mainShape->updateColors();
		spawn_layout->widget_padding = 5.0f;
		spawn_layout->start_padding = { spawn_layout->widget_padding, spawn_layout->widget_padding };
		spawn_layout->end_padding = { spawn_layout->widget_padding, spawn_layout->widget_padding };
		spawn_layout->mode = ContainerMode::FIT_WIDGETS;
		spawn_layout->drawShape = true;

		sf::Vector2f NEW_BUTTON_SIZE(BUTTON_SIZE.x + spawn_layout->widget_padding * 2, BUTTON_SIZE.y + spawn_layout->widget_padding * 2);

		text.setString("spawn");
		DropDown* spawn_dropDown = new DropDown(
			new SwitchableButton(new RoundRectShape({conf::WIDTH - BUTTON_SIZE.x - spawn_layout->widget_padding * 2 - 10.0f, 10.0f }, sf::Vector2f(NEW_BUTTON_SIZE.x, 40.0f), 15.0f), text, 2),
			spawn_layout);
		spawn_dropDown->enabled = false;

		text.setString("spawn");
		SwitchableButton* spawn_switch = new SwitchableButton(new RoundRectShape({ 0.0f, 0.0f }, NEW_BUTTON_SIZE, BUTTON_RADIUS), text, 1);
		spawn_switch->setOnAction([spawn_switch, spawn_dropDown]() {if (spawn_switch->isPressed()) { spawn_dropDown->enabled = true; conf::mode = Mode::SPAWN; } });

		text.setString("drag");
		SwitchableButton* drag_switch = new SwitchableButton(new RoundRectShape({ 0.0f, 0.0f }, NEW_BUTTON_SIZE, BUTTON_RADIUS), text, 1);
		drag_switch->setOnAction([drag_switch, spawn_dropDown]() {if (drag_switch->isPressed()) { spawn_dropDown->enabled = false; conf::mode = Mode::DRAG; } });

		text.setString("delete");
		SwitchableButton* delete_switch = new SwitchableButton(new RoundRectShape({ 0.0f, 0.0f }, NEW_BUTTON_SIZE, BUTTON_RADIUS), text, 1);
		delete_switch->setOnAction([delete_switch, spawn_dropDown]() {if (delete_switch->isPressed()) { spawn_dropDown->enabled = false;  conf::mode = Mode::DELETE; }});

		SwitchableButtonGroup* mode_group = new SwitchableButtonGroup(new RectShape({ 0.0f, 0.0f }, { 0.0f, 0.0f }), 1);
		mode_group->viewable = false;
		mode_group->addSwitchable(spawn_switch);
		mode_group->addSwitchable(drag_switch);
		mode_group->addSwitchable(delete_switch);
		mode_group->setAlwaysPressed(spawn_switch);

		LinearLayout* mode_layout = new LinearLayout(new RoundRectShape({ 0.0f, 0.0f }, { 100.0f, 100.0f }, BUTTON_RADIUS), 1, true);
		mode_layout->addItem(spawn_switch);
		mode_layout->addItem(drag_switch);
		mode_layout->addItem(delete_switch);
		mode_layout->addItem(spawn_dropDown);
		mode_layout->mainShape->setAllColorsTo(sf::Color(255, 255, 255, 100));
		mode_layout->mainShape->setActiveColorToState(0);
		mode_layout->mainShape->updateColors();
		mode_layout->widget_padding = 5.0f;
		mode_layout->start_padding = { mode_layout->widget_padding, mode_layout->widget_padding };
		mode_layout->end_padding = { mode_layout->widget_padding, mode_layout->widget_padding };
		mode_layout->mode = ContainerMode::FIT_WIDGETS;
		mode_layout->drawShape = true;

		text.setString("mode");
		DropDown* mode_dropDown = new DropDown(
			new SwitchableButton(new RoundRectShape({ conf::WIDTH - NEW_BUTTON_SIZE.x - mode_layout->widget_padding * 2 - 10.0f, 10.0f }, sf::Vector2f(NEW_BUTTON_SIZE.x + mode_layout->widget_padding * 2, 40.0f), 15.0f), text, 2),
			mode_layout);

		handler.addItem(settings_dropDown);
		handler.addItem(mode_dropDown);

		handler.addItem(spawn_group);
		handler.addItem(mode_group);
	}
	
	void processEvent(sf::Event& event)
	{
		handler.processEvent(event);
	}

	void update(sf::Vector2i mouse_pos)
	{
		handler.update(mouse_pos);
	}

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override
	{
		sf::View view = target.getView();
		sf::Vector2u targetSize(target.getSize());
		target.setView(sf::View(sf::FloatRect(0.0f, 0.0f, targetSize.x, targetSize.y)));

		target.draw(handler, states);

		target.setView(view);
	}
};