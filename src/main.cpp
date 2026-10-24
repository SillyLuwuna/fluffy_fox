// #include "engine/components/transform_component.hpp"
// #include "engine/entity.hpp"
// #include "rendering/renderer.hpp"
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <iostream>
#include "rendering/window.hpp"
#include "rendering/vulkan_tutorial.hpp"


using namespace fluffy_fox;
using namespace fluffy_fox::rendering;

void setup_logger()
{
	spdlog::enable_backtrace(32);

	auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	stdout_sink->set_level(spdlog::level::debug);
	stdout_sink->set_pattern("[%^%=9l%$] %v");

	auto file_rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logs/log.txt", 1048576 * 10, 5);
	file_rotating_sink->set_level(spdlog::level::trace);
	file_rotating_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%t] [%^%l%$] %v");

	std::vector<spdlog::sink_ptr> sinks { stdout_sink, file_rotating_sink };
	auto logger = std::make_shared<spdlog::logger>("default", sinks.begin(), sinks.end());
	logger->set_level(spdlog::level::debug);
	spdlog::set_default_logger(logger);
}

int main()
{
	setup_logger();

	Window window("owo", 840, 630);

	try
	{
		VulkanTutorial app(window);
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}

	// Entity entity("owo");
	// TransformComponent t;

	return 0;
}
