#include "ffge_window.hpp"

namespace FluffyFox::Graphics
{
	Window::Window(int width, int height, std::string title) :
		m_width{width},
		m_height{height},
		m_title{title}
	{
		initWindow();
	}

	Window::~Window()
	{
		glfwDestroyWindow(m_window);
		glfwTerminate();
	}

	void Window::initWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr); //INFO 4th parameter is for choosing which monitor to open the window on
	}

	bool Window::shouldClose()
	{
		return glfwWindowShouldClose(m_window);
	}
}
