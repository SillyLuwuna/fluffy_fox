#include "ffge_engine.hpp"

#include <vector>
#include <iostream>
#include <stdexcept>

namespace FluffyFox::Graphics
{
	Engine::Engine()
	{

	}

	Engine::~Engine()
	{
	}

	void Engine::run()
	{
		initWindow();
		initVulkan();
		mainLoop();
	}

	void Engine::initWindow()
	{
		m_window = std::make_unique<Window>(WIDTH, HEIGHT, TITLE);
	}

	void Engine::initVulkan()
	{
		createInstance();
	}

	void Engine::createInstance()
	{
		m_instance = std::make_unique<Instance>(APP_NAME, APP_VERSION);
	}

	void Engine::mainLoop()
	{
		while (!m_window->shouldClose())
		{
			glfwPollEvents();
		}
	}
}
