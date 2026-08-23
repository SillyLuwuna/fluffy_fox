#include <iostream>
// #if defined(__unix__)
// 	#if defined(__has_include) && __has_include(<wayland-client.h>)
// 		#define IS_WAYLAND
// 		#define GLFW_EXPOSE_NATIVE_WAYLAND
// 	#else
// 		#define IS_X11
// 		#define GLFW_EXPOSE_NATIVE_X11
// 	#endif
// #elif defined(_WIN32)
// 	#define IS_WINDOWS
// 	#define GLFW_EXPOSE_NATIVE_WIN32
// #elif defined(__APPLE__)
// 	#define IS_APPLE
// 	#define GLFW_EXPOSE_NATIVE_COCOA
// #endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include "rendering/window.hpp"
#include "spdlog/spdlog.h"
#include "rendering/renderer.hpp"

#include <stdexcept>

using namespace fluffy_fox::rendering;

Window::Window(const std::string& name, int width, int height) :
	width_(width),
	height_(height),
	window_(nullptr),
	attached_renderer_(nullptr)
{
	// if (Renderer::instance().is_running())
	// {
	// 	spdlog::critical("glfx does not support multiple instantiations, limiting to one window");
	// 	throw std::runtime_error("glfx does not support multiple instantiations, limiting to one window");
	// }

	if (!glfwInit())
	{
		spdlog::critical("could not initialize glfw");
		throw std::runtime_error("could not initialize glfw");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window_ = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
	if (!window_)
	{
		glfwTerminate();
		spdlog::critical("could not create window");
		throw std::runtime_error("could not create window");
	}
}

Window::~Window()
{
	// if (attached_renderer_)
	// {
	// 	attached_renderer_->stop();
	// }
	glfwDestroyWindow(window_);
	glfwTerminate();
}

void Window::start_event_loop()
{
	if (!attached_renderer_)
	{
		spdlog::error("cannot execute event loop without an attached renderer");
		throw std::runtime_error("cannot execute event loop without an attached renderer");
	}

	// if (!attached_renderer_->is_running())
	// {
	// 	spdlog::error("cannot execute event loop without renderer running");
	// 	throw std::runtime_error("cannot execute event loop without renderer running");
	// }

	// while (!glfwWindowShouldClose(window_) && attached_renderer_->is_running())
	// {
	// 	glfwPollEvents();
	// 	attached_renderer_->render_frame(); //TODO should probably be calling something else
	// }
	while (!glfwWindowShouldClose(window_))
	{
		glfwPollEvents();
	}

	spdlog::info("closing window");
}

// void* Window::get_platform_window() const
// {
// 	switch (glfwGetPlatform())
// 	{
// #if defined(IS_WAYLAND)
// 		case GLFW_PLATFORM_WAYLAND:
// 			return glfwGetWaylandWindow(window_);
// 			break;
// #elif defined(IS_X11)
// 		case GLFW_PLATFORM_X11:
// 			return glfwGetX11Window(window_);
// 			break;
// #elif defined(IS_APPLE)
// 		case GLFW_PLATFORM_COCOA:
// 			return glfwGetCocoaWindow(window_);
// 			break;
// #elif defined(IS_WINDOWS)
// 		case GLFW_PLATFORM_WIN32:
// 			return glfwGetWin32Window(window_);
// 			break;
// #endif
// 	}
//
// 	return nullptr;
// }

// void* Window::get_platform_display() const
// {
// 	switch (glfwGetPlatform())
// 	{
// #if defined(IS_WAYLAND)
// 		case GLFW_PLATFORM_WAYLAND:
// 			return glfwGetWaylandDisplay();
// 			break;
// #elif defined(IS_X11)
// 		case GLFW_PLATFORM_X11:
// 			return glfwGetX11Display();
// 			break;
// #endif
// 	}
//
// 	return nullptr;
// }

// bool Window::is_wayland() const
// {
// #if defined(IS_WAYLAND)
// 	return true;
// #else
// 	return false;
// #endif
// }

GLFWwindow* Window::get_handle() const
{
	return window_;
}

uint16_t Window::width() const
{
	return width_;
}

uint16_t Window::height() const
{
	return height_;
}

void Window::attach_renderer(Renderer& renderer)
{
	attached_renderer_ = &renderer;
}

void Window::detach_renderer()
{
	attached_renderer_ = nullptr;
}
