#pragma once

#include "Rui/Events/Event.h"
#include "Rui/Events/WindowEvent.h"
#include "Rui/Events/KeyEvent.h"
#include "Log.h"

#include <SDL.h>
#include <SDL_vulkan.h>

#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

namespace Rui {
    class Window {
    public:
        using EventCallbackFn = std::function<void(Event&)>;

        Window(const std::string& title, int w, int h);
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        void OnUpdate();

        inline std::string GetTitle() const { return m_Title; }
        inline int GetWidth() const { return m_Width; }
        inline int GetHeight() const { return m_Height; }
        vk::Extent2D GetExtent() {
            SDL_GetWindowSize(m_Window, &m_Width, &m_Height);
        	return { static_cast<uint32_t>(m_Width), static_cast<uint32_t>(m_Height) };
        }

        inline void SetEventCallback(const EventCallbackFn& callback) { EventCallback = callback; }

        void CreateSurface(vk::Instance instance, vk::SurfaceKHR* surface);
        static std::unique_ptr<Window> Create(const std::string& title, int w, int h);
    private:
        SDL_Window* m_Window;

        int m_Width;
        int m_Height;

        std::string m_Title;

        EventCallbackFn EventCallback;
    };
}
