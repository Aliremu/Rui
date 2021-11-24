#include "Window.h"

namespace Rui {
	Window::Window(const std::string& title, int w, int h) {
		SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
		m_Window = SDL_CreateWindow(
		title.c_str(), 
		SDL_WINDOWPOS_CENTERED, 
		SDL_WINDOWPOS_CENTERED, 
		w, 
		h, 
		SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

		RUI_CORE_INFO("Creating Window {0} {1} {2}!", title, w, h);
	}

	Window::~Window() {
		RUI_CORE_INFO("Destroying Window!");

		SDL_DestroyWindow(m_Window);
		SDL_Quit();
	}

	void Window::OnUpdate() {
		SDL_Event e;

		while(SDL_PollEvent(&e)) {
			switch(e.type) {
				case SDL_QUIT: {
					WindowClosedEvent event;
					EventCallback(event);
					break;
				}

				case SDL_WINDOWEVENT: {
					switch(e.window.event) {
						case SDL_WINDOWEVENT_SIZE_CHANGED: {
							int w = e.window.data1;
							int h = e.window.data2;
							RUI_CORE_ERROR("1 # Window Resized: {0} {1}", w, h);

							m_Width = w;
							m_Height = h;

							WindowResizedEvent event(w, h);
							EventCallback(event);
							break;
						}

						default: {
							//RUI_CORE_ERROR()
							break;
						}
					}
				}

				case SDL_KEYDOWN: {
					KeyPressedEvent event(e.key.keysym.sym, 1);
					EventCallback(event);
					break;
				}

				case SDL_KEYUP: {
					KeyReleasedEvent event(e.key.keysym.sym);
					EventCallback(event);
					break;
				}
			}
		}
	}

	void Window::CreateSurface(vk::Instance instance, vk::SurfaceKHR* surface) {
		VkSurfaceKHR temp;
		if(SDL_Vulkan_CreateSurface(m_Window, instance, &temp)) {
			*surface = temp;
			return;
		}

		RUI_CORE_FATAL("Failed to create surface!");
	}

	std::unique_ptr<Window> Window::Create(const std::string& title, int w, int h) {
		return std::make_unique<Window>(title, w, h);
	}
}
