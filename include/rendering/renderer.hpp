#pragma once

#include <chrono>
#include <memory>
#include <bgfx/bgfx.h>
#include <string>
#include <vector>
#include <shaderc/shaderc.hpp>

// #include "window.hpp"
#include "gpu.hpp"


namespace fluffy_fox::rendering
{
	class Window;

	enum RendererType
	{
		Vulkan,
		OpenGL,
		OpenGLES,
		D3D11,
		D3D12,
		Metal,
		WebGPU,
		Auto,
	};

	class Renderer
	{
	private:
		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) = delete;

		Renderer();

		static bgfx::RendererType::Enum get_bgfx_renderertype(fluffy_fox::rendering::RendererType& rendertype);

		bgfx::Init init_;
		Window* window_;

		bool running_;
		bool vsync_;
		uint16_t target_fps_;

		std::chrono::time_point<std::chrono::high_resolution_clock> start_time_;
		std::chrono::time_point<std::chrono::high_resolution_clock> last_frame_time_;
		uint64_t stat_fps_cooldown_us_;
		uint64_t stat_fps_elapsed_;
		uint64_t stat_fps_elapsed_curr_;
		uint64_t frames_;
		int64_t delta_t_correction_;
		uint64_t delta_t_us_;
		double delta_t_s_;

		bool init_set_window(Window& window);
		void init_set_resolution(uint16_t width, uint16_t height);
		void init_set_vsync(bool value);
		void init_set_renderer(RendererType renderer);
		void init_set_gpu(int8_t gpu_id);
		void init_set_gpu_default();
		bool restart();

		// static bgfx::ShaderHandle load_shader(const std::string& path);

	public:
		~Renderer();
		static Renderer& instance();

		bool start(Window& window, RendererType type, bool vsync, uint16_t max_fps);
		void stop();

		void render_frame();

		inline bool is_running() { return running_; };

		bool set_vsync(bool value);
		void set_target_fps(uint16_t target);
		void set_unlimited_fps();
		bool set_renderer(RendererType renderer);
		bool set_gpu(int8_t gpu_id);
	};
}
