
#include "rendering/window.hpp"

using namespace fluffy_fox::rendering;

Window::Window()
{
// 	if (!glfwInit())
// 	{
// 		// throw std::runtime_error("could not ")
// 		return -1;
// 	}
//
// 	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
//
// 	int width = 640;
// 	int height = 480;
// 	GLFWwindow* window = glfwCreateWindow(width, height, "window name", nullptr, nullptr);
// 	if (!window)
// 	{
// 		glfwTerminate();
// 		return -1;
// 	}
//
// 	bgfx::Init init;
// 	init.type = bgfx::RendererType::Vulkan;
//
// 	// platform specific
// 	bool valid_platform = false;
// 	switch (glfwGetPlatform())
// 	{
// #if defined(__unix__)
// 	#if defined(__has_include) && __has_include(<wayland-client.h>)
// 		case GLFW_PLATFORM_WAYLAND:
// 			init.platformData.nwh = glfwGetWaylandWindow(window);
// 			init.platformData.ndt = glfwGetWaylandDisplay();
// 			init.platformData.type = bgfx::NativeWindowHandleType::Wayland;
// 			valid_platform = true;
// 			break;
// 	#else
// 		case GLFW_PLATFORM_X11:
// 			init.platformData.nwh = glfwGetX11Window(window);
// 			init.platformData.ndt = glfwGetX11Display();
// 			init.platformData.type = bgfx::NativeWindowHandleType::Default;
// 			valid_platform = true;
// 			break;
// 	#endif
// #elif defined(__APPLE__)
// 		case GLFW_PLATFORM_COCOA:
// 			init.platformData.nwh = glfwGetCocoaWindow(window);
// 			valid_platform = true;
// 			break;
// #elif defined(_WIN32)
// 		case GLFW_PLATFORM_WIN32:
// 			init.platformData.nwh = glfwGetWin32Window(window);
// 			valid_platform = true;
// 			break;
// #endif
// 	}
//
// 	if (!valid_platform)
// 	{
// 		return -1;
// 	}
//
// 	init.resolution.width = width;
// 	init.resolution.height = height;
// 	init.resolution.reset = BGFX_RESET_NONE;
//
// 	if (!bgfx::init(init))
// 	{
// 		glfwDestroyWindow(window);
// 		glfwTerminate();
// 		return -1;
// 	}
}

Window::~Window()
{
	// bgfx::shutdown();
	// glfwDestroyWindow(window);
	// glfwTerminate();
}

void Window::event_loop()
{
	// uint32_t target_fps = 360;
	// uint64_t target_delta_us = 1000000 / target_fps;
	//
	// auto start_time = std::chrono::high_resolution_clock::now();
	// auto last_time = start_time;
	//
	// uint64_t fps_cooldown_us = 1000000;
	// uint64_t fps_elapsed = 0;
	// uint64_t fps_elapsed_curr = 0;
	// uint32_t frames = 0;
	// int64_t correction = 0;
	// while (!glfwWindowShouldClose(window))
	// {
	// 	auto frame_start_time = std::chrono::high_resolution_clock::now();
	// 	uint64_t delta_us = (std::chrono::duration_cast<std::chrono::microseconds>(frame_start_time - last_time)).count();
	// 	last_time = frame_start_time;
	//
	// 	fps_elapsed += delta_us;
	// 	fps_elapsed_curr += delta_us;
	// 	if (fps_elapsed >= fps_cooldown_us)
	// 	{
	// 		std::cout << "fps: " << (double)frames / ((double)fps_elapsed_curr / 1000000.0) << "\n";
	// 		fps_elapsed -= fps_cooldown_us;
	// 		fps_elapsed_curr = 0;
	// 		frames = 0;
	// 	}
	//
	// 	auto update_start_time = std::chrono::high_resolution_clock::now();
	//
	// 	glfwPollEvents();
	//
	// 	bgfx::setViewRect(0, 0, 0, width, height);
	// 	bgfx::touch(0);
	// 	bgfx::frame();
	//
	// 	frames++;
	//
	// 	auto frame_end_time = std::chrono::high_resolution_clock::now();
	//
	// 	correction += target_delta_us - delta_us;
	// 	correction = std::clamp(correction, -(int64_t)target_delta_us, (int64_t)target_delta_us);
	// 	uint64_t wait_us = (int64_t)(target_delta_us + correction) < (int64_t)delta_us ? 0 : (target_delta_us - delta_us + correction);
	// 	std::this_thread::sleep_for(std::chrono::microseconds(wait_us));
	// }
}
