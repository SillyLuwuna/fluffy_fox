#include "rendering/renderer.hpp"
#include "rendering/window.hpp"
#include "spdlog/spdlog.h"
#include <iostream>

using namespace fluffy_fox::rendering;


Renderer::Renderer() :
	running_(false)
{
}

Renderer::~Renderer()
{
	stop();
}

Renderer& Renderer::instance()
{
	static Renderer instance;
	return instance;
}

bool Renderer::start(Window& window, RendererType type, bool vsync, uint16_t max_fps)
{
	if (running_) return false;

	init_ = bgfx::Init();

	init_set_renderer(type);
	if (!init_set_window(window))
	{
		spdlog::error("not a valid platform");
		return false;
	}

	init_set_resolution(window_->width(), window_->height());
	init_set_vsync(vsync);

	set_target_fps(max_fps);

	if (!bgfx::init(init_))
	{
		spdlog::error("could not initialize bgfx");
		return false;
	}

	start_time_ = std::chrono::high_resolution_clock::now();
	last_frame_time_ = start_time_;
	stat_fps_cooldown_us_ = 1 * 1000000;
	stat_fps_elapsed_ = 0;
	stat_fps_elapsed_curr_ = 0;
	frames_ = 0;
	delta_t_correction_ = 0;
	delta_t_us_ = 0;
	delta_t_s_ = 0.0;

	running_ = true;

	window_->attach_renderer(*this);
	// FIXME doesn't return true, blocks
	window_->start_event_loop();
	return true;
}

void Renderer::init_set_vsync(bool value)
{
	vsync_ = value;
	if (value)
	{
		init_.resolution.reset = BGFX_RESET_VSYNC;
	}
	else
	{
		init_.resolution.reset = BGFX_RESET_NONE;
	}
}

bool Renderer::init_set_window(Window& window)
{
	window_ = &window;
	init_.platformData.nwh = window.get_platform_window();
	init_.platformData.ndt = window.get_platform_display();
	init_.platformData.type = window.is_wayland() ? bgfx::NativeWindowHandleType::Wayland : bgfx::NativeWindowHandleType::Default;

	return init_.platformData.nwh != nullptr;
}

void Renderer::init_set_resolution(uint16_t width, uint16_t height)
{
	init_.resolution.width = width;
	init_.resolution.height = height;
}

void Renderer::init_set_renderer(RendererType renderer)
{
	init_.type = get_bgfx_renderertype(renderer);
}

void Renderer::init_set_gpu(int8_t gpu_id)
{
	// FIXME DOESN'T WORK
	if (gpu_id < 0)
	{
		init_set_gpu_default();
		return;
	}

	GpuInstance gpu(gpu_id);
	init_.deviceId = gpu.device_id();
	init_.vendorId = gpu.vendor_id();
}

void Renderer::init_set_gpu_default()
{
	init_.deviceId = 0;
	init_.vendorId = BGFX_PCI_ID_NONE;
}


bool Renderer::set_vsync(bool value)
{
	bgfx::Init backup = init_;
	init_set_vsync(value);
	if (!restart())
	{
		spdlog::error("error setting vsync");
		init_ = backup;
		if (!restart())
		{
			spdlog::error("could not return to initial state");
		}
		return false;
	}
	return true;
}

void Renderer::set_target_fps(uint16_t target)
{
	target_fps_ = target;
}

void Renderer::set_unlimited_fps()
{
	target_fps_ = 0;
}

bool Renderer::set_renderer(RendererType renderer)
{
	bgfx::Init backup = init_;
	init_set_renderer(renderer);
	if (!restart())
	{
		spdlog::error("error setting renderer");
		init_ = backup;
		if (!restart())
		{
			spdlog::error("could not return to initial state");
		}
		return false;
	}
	return true;
}

bool Renderer::set_gpu(int8_t gpu)
{
	bgfx::Init backup = init_;
	init_set_gpu(gpu);
	if (!restart())
	{
		spdlog::error("error setting gpu");
		init_ = backup;
		if (!restart())
		{
			spdlog::error("could not return to initial state");
		}
		return false;
	}
	return true;
}


bgfx::RendererType::Enum Renderer::get_bgfx_renderertype(fluffy_fox::rendering::RendererType& rendertype)
{
	switch (rendertype)
	{
		case Vulkan:
			return bgfx::RendererType::Vulkan;
			break;
		case OpenGL:
			return bgfx::RendererType::OpenGL;
			break;
		case OpenGLES:
			return bgfx::RendererType::OpenGLES;
			break;
		case D3D11:
			return bgfx::RendererType::Direct3D11;
			break;
		case D3D12:
			return bgfx::RendererType::Direct3D12;
			break;
		case Metal:
			return bgfx::RendererType::Metal;
			break;
		case WebGPU:
			return bgfx::RendererType::WebGPU;
			break;
		case Auto:
			return bgfx::RendererType::Count;
			break;
	}

	return bgfx::RendererType::Count;
}

void Renderer::stop()
{
	if (!running_) return;

	bgfx::shutdown();

	running_ = false;
	window_->detach_renderer();
}

bool Renderer::restart()
{
	bgfx::shutdown();
	if (!bgfx::init(init_))
	{
		spdlog::error("could not restart bgfx");
		return false;
	}
	return true;
}

void Renderer::render_frame()
{
	auto frame_start_time = std::chrono::high_resolution_clock::now();
	uint64_t delta_us = (std::chrono::duration_cast<std::chrono::microseconds>(frame_start_time - last_frame_time_)).count();
	last_frame_time_ = frame_start_time;

	// call update with delta time

	stat_fps_elapsed_ += delta_us;
	stat_fps_elapsed_curr_ += delta_us;
	if (stat_fps_elapsed_ >= stat_fps_cooldown_us_)
	{
		spdlog::info("fps: {:.2f}", ((double)frames_ / ((double)stat_fps_elapsed_curr_ / 1000000.0)));
		stat_fps_elapsed_ -= stat_fps_cooldown_us_;
		stat_fps_elapsed_curr_ = 0;
		frames_ = 0;
	}

	auto update_start_time = std::chrono::high_resolution_clock::now();

	bgfx::setViewRect(0, 0, 0, window_->width(), window_->height());
	bgfx::touch(0);
	bgfx::frame();

	frames_++;

	auto frame_end_time = std::chrono::high_resolution_clock::now();

	uint64_t target_delta_us = 1000000 / target_fps_;
	delta_t_correction_ += target_delta_us - delta_us;
	delta_t_correction_ = std::clamp(delta_t_correction_, -(int64_t)target_delta_us, (int64_t)target_delta_us);
	uint64_t wait_us = (int64_t)(target_delta_us + delta_t_correction_) < (int64_t)delta_us ? 0 : (target_delta_us - delta_us + delta_t_correction_);
	std::this_thread::sleep_for(std::chrono::microseconds(wait_us));
}
