#pragma once

#include <GLFW/glfw3.h>
// #include <GLFW/glfw3native.h>
#include <string>

namespace fluffy_fox::rendering
{
	class Renderer;

	class Window
	{
	private:
		uint16_t width_;
		uint16_t height_;
		GLFWwindow* window_;
		Renderer* attached_renderer_;

	public:
		Window(const Window&) = delete;
		Window(Window&&) = delete;
		Window& operator=(const Window&) = delete;
		Window& operator=(Window&&) = delete;

		Window(const std::string& name, int width, int height);
		~Window();

		GLFWwindow* get_handle() const;
		uint16_t width() const;
		uint16_t height() const;

		// void* get_platform_window() const;
		// void* get_platform_display() const;
		// bool is_wayland() const;

		void start_event_loop();
		void attach_renderer(Renderer& renderer);
		void detach_renderer();
	};
}
