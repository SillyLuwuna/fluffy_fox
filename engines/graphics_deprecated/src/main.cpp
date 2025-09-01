#include "ffge_engine.hpp"
#include <iostream>
#include <cstdlib>

int main()
{
	FluffyFox::Graphics::Engine ffGraphicEngine;

	try
	{
		ffGraphicEngine.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
