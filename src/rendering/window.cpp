#include <iostream>
#include <stdexcept>

#include <fluff/events.hpp>

#include "rendering/window.hpp"
#include "spdlog/spdlog.h"
#include "rendering/renderer.hpp"

using namespace fluffy_fox::rendering;

Window::Window(const std::string& name, i32 width, i32 height) :
	width_(width),
	height_(height),
	window_(nullptr),
	attached_renderer_(nullptr),
	waiting_for_focus_(false),
	surface_(nullptr)
{
	// if (Renderer::instance().is_running())
	// {
	// 	spdlog::critical("glfx does not support multiple instantiations, limiting to one window");
	// 	throw std::runtime_error("glfx does not support multiple instantiations, limiting to one window");
	// }

	glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11); // FIXME remove this line
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

	setup_events();
}

void Window::setup_events()
{
	glfwSetWindowUserPointer(window_, this);

	glfwSetFramebufferSizeCallback(window_, resize_callback);
	glfwSetWindowFocusCallback(window_, focus_callback);
}

void Window::resize_callback(GLFWwindow* window, i32 width, i32 height)
{
	Window* obj = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	obj->resize_event_.invoke(width, height);
}

void Window::focus_callback(GLFWwindow* window, i32 focused)
{
	Window* obj = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));

	obj->waiting_for_focus_ = false;
	obj->focus_event_.invoke(focused);
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

void Window::attach_renderer(Renderer& renderer)
{
	attached_renderer_ = &renderer;
}

void Window::detach_renderer()
{
	attached_renderer_ = nullptr;
}

void Window::block_while_minimized()
{
	i32 width = 0;
	i32 height = 0;
	glfwGetFramebufferSize(window_, &width, &height);

	while (width == 0 || height == 0)
	{
		std::cout << "owo\n";
		glfwWaitEvents();
		glfwGetFramebufferSize(window_, &width, &height);
	}
}

void Window::block_while_unfocused()
{
	waiting_for_focus_ = true;
	while (waiting_for_focus_)
	{
		glfwWaitEvents();
	}
	// if (!waiting_for_focus)
}

std::vector<std::string> Window::get_required_extensions()
{
	u32 glfw_extension_count = 0;
	const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

	std::vector<std::string> required_extensions;

	for (u32 i = 0; i < glfw_extension_count; i++)
	{
		required_extensions.emplace_back(glfw_extensions[i]);
	}

	return required_extensions;
}

// returns nullptr when surface
bool Window::create_surface(vk::raii::Instance& vulkan_instance)
{
	VkSurfaceKHR surface;
	bool success = glfwCreateWindowSurface(*vulkan_instance, window_, nullptr, &surface) == 0;

	if (success)
	{
		surface_ = vk::raii::SurfaceKHR(vulkan_instance, surface);
	}

	return success;
}

void Window::destroy_surface()
{
	surface_ = nullptr;
}

std::pair<i32, i32> Window::framebuffer_size() const
{
	i32 width, height;
	glfwGetFramebufferSize(window_, &width, &height);
	return { width, height };
}
