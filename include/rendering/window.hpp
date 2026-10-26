#pragma once

#include <fluff/events.hpp>
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

#include <fluff/types.hpp>
using namespace fluff::types;

namespace fluffy_fox::rendering
{
	class Renderer;

	class Window
	{
	private:
		i32 width_;
		i32 height_;
		GLFWwindow* window_;
		Renderer* attached_renderer_;
		fluff::Event<i32, i32> resize_event_;
		fluff::Event<bool> focus_event_;
		bool waiting_for_focus_;
		vk::raii::SurfaceKHR surface_;

		void setup_events();
		static void resize_callback(GLFWwindow* window, i32 width, i32 height);
		static void focus_callback(GLFWwindow* window, int focused);

	public:
		Window(const Window&) = delete;
		Window(Window&&) = delete;
		Window& operator=(const Window&) = delete;
		Window& operator=(Window&&) = delete;

		Window(const std::string& name, i32 width, i32 height);
		~Window();

		inline GLFWwindow* get_handle() const { return window_; }
		inline i32 width() const { return width_; }
		inline i32 height() const { return height_; }

		void start_event_loop();
		void attach_renderer(Renderer& renderer);
		void detach_renderer();

		inline fluff::Event<i32, i32>& resize_event() { return resize_event_; }
		inline fluff::Event<bool>& focus_event() { return focus_event_; }

		void block_while_minimized();
		void block_while_unfocused();

		static std::vector<std::string> get_required_extensions();

		bool create_surface(vk::raii::Instance& vulkan_instance);
		inline vk::raii::SurfaceKHR& surface() { return surface_; };
		void destroy_surface();
	};
}
