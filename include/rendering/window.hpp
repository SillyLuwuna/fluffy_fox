#pragma once

#include <fluff/events.hpp>
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

		GLFWwindow* get_handle() const;
		i32 width() const;
		i32 height() const;

		void start_event_loop();
		void attach_renderer(Renderer& renderer);
		void detach_renderer();

		fluff::Event<i32, i32>& resize_event();
		fluff::Event<bool>& focus_event();

		void block_while_minimized();
		void block_while_unfocused();

		static std::vector<std::string> get_required_extensions();
	};
}
